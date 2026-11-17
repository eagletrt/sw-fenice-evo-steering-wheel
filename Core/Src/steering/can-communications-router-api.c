/*!
 * \file can-communications-router-api.c
 * \date 2026-08-27
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief Implementation of the steering-wheel CAN routing layer.
 */

#include "can-communications-router-api.h"

#include "can-communications-api.h"
#include "can-primary-api.h"
#include "eagletrt-api.h"
#include "flash-monitor-api.h"
#include "parameters-api.h"
#include "timeout-api.h"
#include "ui-data-api.h"

#include <string.h>

/*!
 * \brief Primary identifiers the FDCAN lets through, in programming order.
 *
 * \details Flashing traffic comes first, so that if the peripheral has fewer
 *     filter slots than entries the wheel can still be reset into its
 *     bootloader and reflashed. Then every message the dashboard decodes.
 */
EAGLETRT_STATIC const struct CanCommunicationsRouterFilter prv_router_api_primary_filters[] = {
    // Every <Board>FlashingTx/Rx pair, the wheel's own included (flash monitor).
    { CAN_PRIMARY_MESSAGE_FRAME_ID_ECUFLASHINGTX, CAN_PRIMARY_MESSAGE_FRAME_ID_STEERINGBOARDFLASHINGRX },
    { CAN_PRIMARY_MESSAGE_FRAME_ID_ECUFSM, CAN_PRIMARY_MESSAGE_FRAME_ID_ECUFSM },
    { CAN_PRIMARY_MESSAGE_FRAME_ID_TSACMAINBOARDESTIMATEDSOC, CAN_PRIMARY_MESSAGE_FRAME_ID_TSACMAINBOARDESTIMATEDSOC },
    { CAN_PRIMARY_MESSAGE_FRAME_ID_TSACMAINBOARDTEMPERATUREINFO, CAN_PRIMARY_MESSAGE_FRAME_ID_TSACMAINBOARDTEMPERATUREINFO },
    { CAN_PRIMARY_MESSAGE_FRAME_ID_INVERTERTEMPERATURE, CAN_PRIMARY_MESSAGE_FRAME_ID_INVERTERTEMPERATURE },
    { CAN_PRIMARY_MESSAGE_FRAME_ID_MOTORTEMPERATURE, CAN_PRIMARY_MESSAGE_FRAME_ID_MOTORTEMPERATURE },
    { CAN_PRIMARY_MESSAGE_FRAME_ID_CURRENTLAP, CAN_PRIMARY_MESSAGE_FRAME_ID_CURRENTLAP },
    { CAN_PRIMARY_MESSAGE_FRAME_ID_LVACESTIMATEDSOC, CAN_PRIMARY_MESSAGE_FRAME_ID_LVACESTIMATEDSOC },
    { CAN_PRIMARY_MESSAGE_FRAME_ID_ESTIMATEDVEHICLEVELOCITY, CAN_PRIMARY_MESSAGE_FRAME_ID_ESTIMATEDVEHICLEVELOCITY },
    { CAN_PRIMARY_MESSAGE_FRAME_ID_PEDALSTHROTTLE, CAN_PRIMARY_MESSAGE_FRAME_ID_PEDALSTHROTTLE },
    { CAN_PRIMARY_MESSAGE_FRAME_ID_PEDALSBRAKE, CAN_PRIMARY_MESSAGE_FRAME_ID_PEDALSBRAKE },
    { CAN_PRIMARY_MESSAGE_FRAME_ID_TSACMAINBOARDFEEDBACKSHUTDOWN, CAN_PRIMARY_MESSAGE_FRAME_ID_TSACMAINBOARDFEEDBACKSHUTDOWN },
};

static_assert(sizeof(prv_router_api_primary_filters) / sizeof(prv_router_api_primary_filters[0]) == CAN_COMMUNICATIONS_ROUTER_PRIMARY_FILTER_COUNT,
              "update CAN_COMMUNICATIONS_ROUTER_PRIMARY_FILTER_COUNT (at most FDCAN1 StdFiltersNbr in the .ioc)");

/*!
 * \brief Scale from the 0..1 state-of-charge libcan reports to a percentage.
 */
#define CAN_COMMUNICATIONS_ROUTER_SOC_PERCENT (100.0F)

/*!
 * \brief Whether an EcuFsm vehicle status belongs to the AS branch (the wheel shows the AS page).
 *
 * \details Not taken from prv_router_api_map_vehicle_state: AS_EMERGENCY maps
 *     to the generic ERROR there, but the car is still running its AS mission.
 *     TODO: stand-in only. Once libcan has the mission message the
 *     AS page MUST follow it, and no page may be shown before the selected
 *     mission is reported (rules).
 */
EAGLETRT_STATIC bool prv_router_api_is_autonomous(uint8_t vehicle_status) {
    switch ((enum CanPrimaryEcufsmVehiclestatus)vehicle_status) {
        case CAN_PRIMARY_ECUFSM_VEHICLESTATUS_ASOFF:
        case CAN_PRIMARY_ECUFSM_VEHICLESTATUS_AS_READY:
        case CAN_PRIMARY_ECUFSM_VEHICLESTATUS_AS_R2D:
        case CAN_PRIMARY_ECUFSM_VEHICLESTATUS_AS_DRIVING:
        case CAN_PRIMARY_ECUFSM_VEHICLESTATUS_AS_FINISHED:
        case CAN_PRIMARY_ECUFSM_VEHICLESTATUS_AS_EMERGENCY:
            return true;
        default:
            return false;
    }
}

/*!
 * \brief EcuFsm silence after which the ECU state is unknown again: three missed frames (A4).
 */
#define CAN_COMMUNICATIONS_ROUTER_ECU_STALE_MS (3U * (uint32_t)can_primary_cycle_time_ecufsm)

/*!
 * \brief Restarted by every EcuFsm frame.
 */
EAGLETRT_STATIC timeout_id prv_router_api_ecu_timeout;

#define CAN_COMMUNICATIONS_ROUTER_KMH_PER_MPS (3.6F)        /*!< m/s to km/h */
#define CAN_COMMUNICATIONS_ROUTER_SPEED_MAX_KMH (999.0F)    /*!< Clamp: three digits on the dashboard */
#define CAN_COMMUNICATIONS_ROUTER_SHUTDOWN_CLOSED_V (12.0F) /*!< TODO(design): sdEnd voltage above which the shutdown circuit counts as closed */
#define CAN_COMMUNICATIONS_ROUTER_STALE_MIN_MS (500U)       /*!< Shortest stale time: fast messages (5 ms) must not flicker to "--" on a late routine */

/*!
 * \brief Stale time of a message: three missed frames, at least CAN_COMMUNICATIONS_ROUTER_STALE_MIN_MS.
 */
#define CAN_COMMUNICATIONS_ROUTER_STALE_MS(cycle_ms) EAGLETRT_API_MAX(3U * (uint32_t)(cycle_ms), CAN_COMMUNICATIONS_ROUTER_STALE_MIN_MS)

/*!
 * \brief The message each UIDataSource comes from, and how long it stays fresh.
 */
struct CanCommunicationsRouterSource {
    enum CanPrimaryMessageFrameId id; /*!< Message carrying the source */
    uint32_t stale_ms;                /*!< Silence after which the dashboard shows "--" */
};

EAGLETRT_STATIC const struct CanCommunicationsRouterSource prv_router_api_sources[UI_DATA_SOURCE_COUNT] = {
    [UI_DATA_SOURCE_HV_SOC] = { CAN_PRIMARY_MESSAGE_FRAME_ID_TSACMAINBOARDESTIMATEDSOC, CAN_COMMUNICATIONS_ROUTER_STALE_MS(can_primary_cycle_time_tsacmainboardestimatedsoc) },
    [UI_DATA_SOURCE_HV_TEMP] = { CAN_PRIMARY_MESSAGE_FRAME_ID_TSACMAINBOARDTEMPERATUREINFO, CAN_COMMUNICATIONS_ROUTER_STALE_MS(can_primary_cycle_time_tsacmainboardtemperatureinfo) },
    [UI_DATA_SOURCE_LV_SOC] = { CAN_PRIMARY_MESSAGE_FRAME_ID_LVACESTIMATEDSOC, CAN_COMMUNICATIONS_ROUTER_STALE_MS(can_primary_cycle_time_lvacestimatedsoc) },
    [UI_DATA_SOURCE_SPEED] = { CAN_PRIMARY_MESSAGE_FRAME_ID_ESTIMATEDVEHICLEVELOCITY, CAN_COMMUNICATIONS_ROUTER_STALE_MS(can_primary_cycle_time_estimatedvehiclevelocity) },
    [UI_DATA_SOURCE_SHUTDOWN] = { CAN_PRIMARY_MESSAGE_FRAME_ID_TSACMAINBOARDFEEDBACKSHUTDOWN, CAN_COMMUNICATIONS_ROUTER_STALE_MS(can_primary_cycle_time_tsacmainboardfeedbackshutdown) },
    [UI_DATA_SOURCE_INVERTER] = { CAN_PRIMARY_MESSAGE_FRAME_ID_INVERTERTEMPERATURE, CAN_COMMUNICATIONS_ROUTER_STALE_MS(can_primary_cycle_time_invertertemperature) },
    [UI_DATA_SOURCE_MOTOR] = { CAN_PRIMARY_MESSAGE_FRAME_ID_MOTORTEMPERATURE, CAN_COMMUNICATIONS_ROUTER_STALE_MS(can_primary_cycle_time_motortemperature) },
    [UI_DATA_SOURCE_THROTTLE] = { CAN_PRIMARY_MESSAGE_FRAME_ID_PEDALSTHROTTLE, CAN_COMMUNICATIONS_ROUTER_STALE_MS(can_primary_cycle_time_pedalsthrottle) },
    [UI_DATA_SOURCE_BRAKE] = { CAN_PRIMARY_MESSAGE_FRAME_ID_PEDALSBRAKE, CAN_COMMUNICATIONS_ROUTER_STALE_MS(can_primary_cycle_time_pedalsbrake) },
    [UI_DATA_SOURCE_LAP] = { CAN_PRIMARY_MESSAGE_FRAME_ID_CURRENTLAP, CAN_COMMUNICATIONS_ROUTER_STALE_MS(can_primary_cycle_time_currentlap) },
};

/*!
 * \brief One stale timeout per source, indexed by UIDataSource.
 */
EAGLETRT_STATIC timeout_id prv_router_api_source_timeouts[UI_DATA_SOURCE_COUNT];

/*!
 * \brief A source went silent: the dashboard shows "--" for it.
 *
 * \param context The UIDataSource, as an integer.
 */
EAGLETRT_STATIC void prv_router_api_on_source_stale(void *context) {
    ui_data_api_get()->stale |= UI_DATA_STALE_BIT((uintptr_t)context);
}

/*!
 * \brief A frame arrived: its source is fresh again until its stale time passes.
 *
 * \param id Identifier of the frame.
 */
EAGLETRT_STATIC void prv_router_api_mark_fresh(uint32_t id) {
    for (uint8_t source = 0U; source < UI_DATA_SOURCE_COUNT; source++) {
        if ((uint32_t)prv_router_api_sources[source].id == id) {
            ui_data_api_get()->stale &= (uint16_t)~UI_DATA_STALE_BIT(source);
            EAGLETRT_API_UNUSED(timeout_api_start(prv_router_api_source_timeouts[source]));
        }
    }
}

/*!
 * \brief The ECU went silent: its state is unknown again (A4, the menu locks).
 *
 * \param context Unused.
 */
EAGLETRT_STATIC void prv_router_api_on_ecu_stale(void *context) {
    EAGLETRT_API_UNUSED(context);
    ui_data_api_get()->vehicle_state = (uint8_t)UI_DATA_VEHICLE_STATE_COUNT;
    ui_data_api_get()->autonomous = 0U; // stand-in; TODO: no page without a reported mission (D57)
}

enum CanCommunicationReturnCode can_communications_router_api_init(void) {
    if (timeout_api_create(CAN_COMMUNICATIONS_ROUTER_ECU_STALE_MS, prv_router_api_on_ecu_stale, NULL, &prv_router_api_ecu_timeout) != TIMEOUT_RC_OK) {
        return CAN_COMMUNICATION_RC_ERROR;
    }
    for (uint8_t source = 0U; source < UI_DATA_SOURCE_COUNT; source++) {
        // source is what is used to have a single callback for all sources to index the timeout array into the bitmask (not a function and never accessed, so it's safe)
        if (timeout_api_create(prv_router_api_sources[source].stale_ms, prv_router_api_on_source_stale, (void *)(uintptr_t)source, &prv_router_api_source_timeouts[source]) != TIMEOUT_RC_OK) {
            return CAN_COMMUNICATION_RC_ERROR;
        }
    }
    return CAN_COMMUNICATION_RC_OK;
}

/*!
 * \brief Scale from the seconds libcan reports to the milliseconds the UI shows.
 */
#define CAN_COMMUNICATIONS_ROUTER_MS_PER_SECOND (1000.0F)

/*!
 * \brief Serialize one primary message and queue it for transmission.
 *
 * \param frame_id The primary frame ID to serialize and send.
 * \param message The message union holding the payload.
 * \param length The on-bus length of the message, in bytes.
 *
 * \retval CAN_COMMUNICATION_RC_OK on success.
 * \retval CAN_COMMUNICATION_RC_ERROR if libcan failed to serialize.
 */
EAGLETRT_STATIC enum CanCommunicationReturnCode prv_router_api_queue_primary(
    enum CanPrimaryMessageFrameId frame_id,
    union CanPrimaryMessages *message,
    uint8_t length) {
    struct CanCommunicationFrame frame;
    if (can_primary_api_serialize_from_id(frame_id, message, frame.data) == -1) {
        return CAN_COMMUNICATION_RC_ERROR;
    }
    frame.id = (uint32_t)frame_id;
    frame.length = length;
    return can_communications_api_add_to_tx_buffer(CAN_COMMUNICATION_NETWORK_PRIMARY, &frame);
}

enum CanCommunicationReturnCode can_communications_router_api_send_button_status(void) {
    union CanPrimaryMessages message;
    message.steeringwheelbuttonstatus = (struct CanPrimarySteeringwheelbuttonstatus){
        .tson = parameters_api_get(INPUTS_SHARED_PARAMETER_ID_TS_ON),
        .ptt = parameters_api_get(INPUTS_SHARED_PARAMETER_ID_PTT),
    };
    return prv_router_api_queue_primary(
        CAN_PRIMARY_MESSAGE_FRAME_ID_STEERINGWHEELBUTTONSTATUS,
        &message,
        can_primary_byte_size_steeringwheelbuttonstatus);
}

enum CanCommunicationReturnCode can_communications_router_api_send_control_maps(void) {
    union CanPrimaryMessages message;
    message.controlmapsset = (struct CanPrimaryControlmapsset){
        .power = (float)parameters_api_get(INPUTS_SHARED_PARAMETER_ID_POWER) / (float)INPUTS_SHARED_PARAMETER_NUMERIC_MAX,
        .torquevectoring = (float)parameters_api_get(INPUTS_SHARED_PARAMETER_ID_TORQUE_VECTORING) / (float)INPUTS_SHARED_PARAMETER_NUMERIC_MAX,
        .regen = (float)parameters_api_get(INPUTS_SHARED_PARAMETER_ID_REGEN) / (float)INPUTS_SHARED_PARAMETER_NUMERIC_MAX,
        .launchcontrol = parameters_api_get(INPUTS_SHARED_PARAMETER_ID_LAUNCH_CONTROL),
    };
    return prv_router_api_queue_primary(
        CAN_PRIMARY_MESSAGE_FRAME_ID_CONTROLMAPSSET,
        &message,
        can_primary_byte_size_controlmapsset);
}

/*!
 * \brief Build and queue DriverAction from the current parameters.
 *
 * \details Event-based message (no cycle time in libcan), sent on every
 *     change of one of its signals: the telemetry-log toggle and the driver
 *     actions OK and REPEAT (1 while their button is held). The marker has
 *     no button on this wheel.
 *
 * \retval CAN_COMMUNICATION_RC_OK on success.
 * \retval CAN_COMMUNICATION_RC_ERROR if libcan failed to serialize.
 */
EAGLETRT_STATIC enum CanCommunicationReturnCode prv_router_api_send_driver_action(void) {
    union CanPrimaryMessages message;
    message.driveraction = (struct CanPrimaryDriveraction){
        .log = parameters_api_get(INPUTS_SHARED_PARAMETER_ID_TELEMETRY_LOG),
        .marker = 0U,
        .ok = parameters_api_get(INPUTS_SHARED_PARAMETER_ID_DRIVER_OK),
        .repeat = parameters_api_get(INPUTS_SHARED_PARAMETER_ID_DRIVER_REPEAT),
    };
    return prv_router_api_queue_primary(
        CAN_PRIMARY_MESSAGE_FRAME_ID_DRIVERACTION,
        &message,
        can_primary_byte_size_driveraction);
}

enum CanCommunicationReturnCode can_communications_router_api_on_parameter_change(const enum InputsSharedParameterID parameter_id, uint8_t value) {
    EAGLETRT_API_UNUSED(value);

    switch (parameter_id) {
        case INPUTS_SHARED_PARAMETER_ID_TS_ON:
        case INPUTS_SHARED_PARAMETER_ID_PTT:
            return can_communications_router_api_send_button_status();

        case INPUTS_SHARED_PARAMETER_ID_POWER:
        case INPUTS_SHARED_PARAMETER_ID_REGEN:
        case INPUTS_SHARED_PARAMETER_ID_TORQUE_VECTORING:
        case INPUTS_SHARED_PARAMETER_ID_LAUNCH_CONTROL:
            return can_communications_router_api_send_control_maps();

        case INPUTS_SHARED_PARAMETER_ID_TELEMETRY_LOG:
        case INPUTS_SHARED_PARAMETER_ID_DRIVER_OK:
        case INPUTS_SHARED_PARAMETER_ID_DRIVER_REPEAT:
            return prv_router_api_send_driver_action();

        case INPUTS_SHARED_PARAMETER_ID_COUNT:
        default:
            return CAN_COMMUNICATION_RC_OK;
    }
}

/*!
 * \brief Map an EcuFsm vehicle status onto the dashboard's state label.
 *
 * \param vehicle_status The raw CanPrimaryEcufsmVehiclestatus value.
 *
 * \return The matching UIDataVehicleState.
 */
EAGLETRT_STATIC enum UIDataVehicleState prv_router_api_map_vehicle_state(uint8_t vehicle_status) {
    switch ((enum CanPrimaryEcufsmVehiclestatus)vehicle_status) {
        case CAN_PRIMARY_ECUFSM_VEHICLESTATUS_PRECHARGE:
            return UI_DATA_VEHICLE_STATE_PRECHARGE;

        case CAN_PRIMARY_ECUFSM_VEHICLESTATUS_TSON:
            return UI_DATA_VEHICLE_STATE_TSON;

        case CAN_PRIMARY_ECUFSM_VEHICLESTATUS_R2D:
            return UI_DATA_VEHICLE_STATE_DRIVE;

        case CAN_PRIMARY_ECUFSM_VEHICLESTATUS_ASOFF:
        case CAN_PRIMARY_ECUFSM_VEHICLESTATUS_AS_READY:
        case CAN_PRIMARY_ECUFSM_VEHICLESTATUS_AS_DRIVING:
        case CAN_PRIMARY_ECUFSM_VEHICLESTATUS_AS_R2D:
        case CAN_PRIMARY_ECUFSM_VEHICLESTATUS_AS_FINISHED:
            return UI_DATA_VEHICLE_STATE_AUTONOMOUS;

        case CAN_PRIMARY_ECUFSM_VEHICLESTATUS_ERROR:
        case CAN_PRIMARY_ECUFSM_VEHICLESTATUS_AS_EMERGENCY:
            return UI_DATA_VEHICLE_STATE_ERROR;

        case CAN_PRIMARY_ECUFSM_VEHICLESTATUS_DISCHARGE:
            return UI_DATA_VEHICLE_STATE_DISCHARGE;

        case CAN_PRIMARY_ECUFSM_VEHICLESTATUS_INIT:
        case CAN_PRIMARY_ECUFSM_VEHICLESTATUS_IDLE:
        default:
            return UI_DATA_VEHICLE_STATE_IDLE;
    }
}

/*!
 * \brief Largest of four temperatures, rounded to the nearest degree.
 *
 * \details The dashboard shows a single number per group, and the one that
 *     matters to the driver is the hottest inverter.
 *
 * \param front_left  First temperature, in °C.
 * \param front_right Second temperature, in °C.
 * \param rear_left   Third temperature, in °C.
 * \param rear_right  Fourth temperature, in °C.
 *
 * \return The largest of the four, as a whole number of °C.
 */
EAGLETRT_STATIC int16_t prv_router_api_hottest(float front_left, float front_right, float rear_left, float rear_right) {
    float max = EAGLETRT_API_MAX(EAGLETRT_API_MAX(front_left, front_right), EAGLETRT_API_MAX(rear_left, rear_right));
    return (int16_t)max;
}

enum CanCommunicationReturnCode can_communications_router_api_receive_primary(const struct CanCommunicationFrame *frame) {
    if (frame == NULL) {
        return CAN_COMMUNICATION_RC_NULL_POINTER;
    }

    // libcan takes a mutable buffer; the queue hands us a const frame.
    uint8_t payload[CAN_COMMUNICATIONS_FRAME_DATA_SIZE];
    memcpy(payload, frame->data, sizeof(payload));

    union CanPrimaryMessages decoded;
    struct UIData *ui_data = ui_data_api_get();

    // Every frame that gets here is decoded: the FDCAN filter
    // (can_communications_router_api_get_filters) keeps the rest out.
    if (can_primary_api_deserialize_from_id((enum CanPrimaryMessageFrameId)frame->id, payload, &decoded) < 0) {
        return CAN_COMMUNICATION_RC_RECEIVE_HANDLER_ERROR;
    }

    prv_router_api_mark_fresh(frame->id);

    switch ((enum CanPrimaryMessageFrameId)frame->id) {
        case CAN_PRIMARY_MESSAGE_FRAME_ID_ECUFSM:
            ui_data->vehicle_state = (uint8_t)prv_router_api_map_vehicle_state(decoded.ecufsm.vehiclestatus);
            ui_data->autonomous = prv_router_api_is_autonomous(decoded.ecufsm.vehiclestatus) ? 1U : 0U;
            EAGLETRT_API_UNUSED(timeout_api_start(prv_router_api_ecu_timeout));
            break;

        case CAN_PRIMARY_MESSAGE_FRAME_ID_TSACMAINBOARDESTIMATEDSOC:
            // libcan hands back a 0..1 fraction, the dashboard shows a percentage.
            ui_data->soc = (uint8_t)EAGLETRT_API_CLAMP(decoded.tsacmainboardestimatedsoc.soc * CAN_COMMUNICATIONS_ROUTER_SOC_PERCENT, 0.0F, CAN_COMMUNICATIONS_ROUTER_SOC_PERCENT);
            break;

        case CAN_PRIMARY_MESSAGE_FRAME_ID_TSACMAINBOARDTEMPERATUREINFO:
            ui_data->hv_temp = (int16_t)decoded.tsacmainboardtemperatureinfo.max;
            break;

        case CAN_PRIMARY_MESSAGE_FRAME_ID_INVERTERTEMPERATURE:
            ui_data->inverter_temp = prv_router_api_hottest(
                decoded.invertertemperature.temperaturefl,
                decoded.invertertemperature.temperaturefr,
                decoded.invertertemperature.temperaturerl,
                decoded.invertertemperature.temperaturerr);
            break;

        case CAN_PRIMARY_MESSAGE_FRAME_ID_MOTORTEMPERATURE:
            ui_data->motor_fl_temp = (int16_t)decoded.motortemperature.temperaturefl;
            ui_data->motor_fr_temp = (int16_t)decoded.motortemperature.temperaturefr;
            ui_data->motor_rl_temp = (int16_t)decoded.motortemperature.temperaturerl;
            ui_data->motor_rr_temp = (int16_t)decoded.motortemperature.temperaturerr;
            break;

        case CAN_PRIMARY_MESSAGE_FRAME_ID_CURRENTLAP:
            ui_data->lap_current = decoded.currentlap.number;
            // libcan decodes the delta in seconds, the dashboard shows ms.
            ui_data->lap_delta_ms = (int32_t)(decoded.currentlap.deltabestlap * CAN_COMMUNICATIONS_ROUTER_MS_PER_SECOND);
            break;

        case CAN_PRIMARY_MESSAGE_FRAME_ID_LVACESTIMATEDSOC:
            ui_data->lv_soc = (uint8_t)EAGLETRT_API_CLAMP(decoded.lvacestimatedsoc.soc * CAN_COMMUNICATIONS_ROUTER_SOC_PERCENT, 0.0F, CAN_COMMUNICATIONS_ROUTER_SOC_PERCENT);
            break;

        case CAN_PRIMARY_MESSAGE_FRAME_ID_ESTIMATEDVEHICLEVELOCITY:
            // Longitudinal speed in m/s, rounded to whole km/h.
            ui_data->speed_kmh = (uint16_t)EAGLETRT_API_CLAMP(decoded.estimatedvehiclevelocity.u * CAN_COMMUNICATIONS_ROUTER_KMH_PER_MPS + 0.5F, 0.0F, CAN_COMMUNICATIONS_ROUTER_SPEED_MAX_KMH);
            break;

        case CAN_PRIMARY_MESSAGE_FRAME_ID_PEDALSTHROTTLE:
            ui_data->throttle_percent = (uint8_t)EAGLETRT_API_CLAMP(decoded.pedalsthrottle.travel * CAN_COMMUNICATIONS_ROUTER_SOC_PERCENT, 0.0F, CAN_COMMUNICATIONS_ROUTER_SOC_PERCENT);
            break;

        case CAN_PRIMARY_MESSAGE_FRAME_ID_PEDALSBRAKE: {
            // No brake travel sensor yet: the highest pressure, 1 % per bar (0-1 bar = 0 %, 100 bar = 100 %).
            const int16_t bar = prv_router_api_hottest(decoded.pedalsbrake.pressurefl, decoded.pedalsbrake.pressurefr, decoded.pedalsbrake.pressurerl, decoded.pedalsbrake.pressurerr);
            ui_data->brake_percent = (uint8_t)EAGLETRT_API_CLAMP(bar, 0, 100);
            break;
        }

        case CAN_PRIMARY_MESSAGE_FRAME_ID_TSACMAINBOARDFEEDBACKSHUTDOWN:
            ui_data->shutdown_closed = (decoded.tsacmainboardfeedbackshutdown.sdend >= CAN_COMMUNICATIONS_ROUTER_SHUTDOWN_CLOSED_V) ? 1U : 0U;
            break;

        default:
            // Flashing traffic (the wheel's own resets it), or nothing for us.
            EAGLETRT_API_UNUSED(flash_monitor_api_on_frame(frame->id));
            break;
    }

    return CAN_COMMUNICATION_RC_OK;
}

enum CanCommunicationReturnCode can_communications_router_api_receive_secondary(const struct CanCommunicationFrame *frame) {
    if (frame == NULL) {
        return CAN_COMMUNICATION_RC_NULL_POINTER;
    }

    // Nothing the dashboard renders lives on the secondary network yet.

    return CAN_COMMUNICATION_RC_OK;
}

const struct CanCommunicationsRouterFilter *can_communications_router_api_get_filters(enum CanCommunicationNetwork network, size_t *count) {
    if (count == NULL) {
        return NULL;
    }
    // Nothing is decoded on the secondary network: it accepts no frame.
    if (network != CAN_COMMUNICATION_NETWORK_PRIMARY) {
        *count = 0U;
        return NULL;
    }
    *count = CAN_COMMUNICATIONS_ROUTER_PRIMARY_FILTER_COUNT;
    return prv_router_api_primary_filters;
}

bool can_communications_router_api_accepts(enum CanCommunicationNetwork network, uint32_t id) {
    size_t count = 0U;
    const struct CanCommunicationsRouterFilter *filters = can_communications_router_api_get_filters(network, &count);
    for (size_t i = 0U; i < count; i++) {
        if (id >= filters[i].first_id && id <= filters[i].last_id) {
            return true;
        }
    }
    return false;
}
