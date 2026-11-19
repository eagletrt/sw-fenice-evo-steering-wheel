/*!
 * \file can-communications-router-api.c
 * \date 2026-08-27
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief Implementation of the steering-wheel CAN routing layer.
 */

#include "can/router/can-communications-router-api.h"

#include "can/bus/can-communications-api.h"
#include "can/can-placeholders.h"
#include "can-primary-api.h"
#include "eagletrt-api.h"
#include "can/flash-monitor/flash-monitor-api.h"
#include "inputs/parameters/parameters-api.h"
#include "util/timeout/timeout-api.h"
#include "ui/data/ui-data-api.h"

#include <math.h>
#include <string.h>

/*!
 * \brief Primary identifiers the FDCAN lets through, in programming order.
 *
 * \details Flashing traffic comes first, so that if the peripheral has fewer
 *     filter slots than entries the wheel can still be reset into its
 *     bootloader and reflashed. Then every message the dashboard decodes.
 */
// Ranges may hold messages nobody decodes (deserialized, then ignored):
// fewer slots, cheap at their rates. Fast messages the screen does not use
// stay out: SuspensionRear 0x4CD, SteeringEncoder 0x4DF, SuspensionFront 0x4E2.
#define CAN_COMMUNICATIONS_ROUTER_ID(name) CAN_PRIMARY_MESSAGE_FRAME_ID_##name
EAGLETRT_STATIC const struct CanCommunicationsRouterFilter prv_router_api_primary_filters[] = {
    // Every <Board>FlashingTx/Rx pair, the wheel's own included (flash monitor).
    { CAN_COMMUNICATIONS_ROUTER_ID(ECUFLASHINGTX), CAN_COMMUNICATIONS_ROUTER_ID(STEERINGBOARDFLASHINGRX) },
    { CAN_COMMUNICATIONS_ROUTER_ID(LVACESTIMATEDSOC), CAN_COMMUNICATIONS_ROUTER_ID(TSACMAINBOARDESTIMATEDSOC) },                        // 0x000-0x006: EcuFsm, pedals throttle, EBS
    { CAN_COMMUNICATIONS_ROUTER_ID(PIPOSHUTDOWN), CAN_COMMUNICATIONS_ROUTER_ID(TSACMAINBOARDFEEDBACKSHUTDOWN) },                        // 0x199-0x1A2: shutdown nodes, TSAC status, current, voltage
    { CAN_COMMUNICATIONS_ROUTER_ID(DASREARFSM), CAN_COMMUNICATIONS_ROUTER_ID(TYRETEMPERATUREREAR) },                                    // 0x4CB-0x4CC
    { CAN_COMMUNICATIONS_ROUTER_ID(PIPOFSM), CAN_COMMUNICATIONS_ROUTER_ID(STEERINGFSM) },                                               // 0x4CE-0x4DE: PIPO, LVAC, cooling, steering board
    { CAN_COMMUNICATIONS_ROUTER_ID(DASFRONTFSM), CAN_COMMUNICATIONS_ROUTER_ID(TYRETEMPERATUREFRONT) },                                  // 0x4E0-0x4E1
    { CAN_COMMUNICATIONS_ROUTER_ID(INVERTERTEMPERATURE), CAN_COMMUNICATIONS_ROUTER_ID(IMUACCELERATION) },                               // 0x4E4-0x4ED: ECU, telemetry, IMU
    { CAN_COMMUNICATIONS_ROUTER_ID(PEDALSBRAKE), CAN_COMMUNICATIONS_ROUTER_ID(PEDALSBRAKE) },                                           // 0x4F2
    { CAN_COMMUNICATIONS_ROUTER_ID(TSACMAINBOARDTEMPERATUREINFO), CAN_COMMUNICATIONS_ROUTER_ID(TSACMAINBOARDTEMPERATUREINFO) },         // 0x4F9
    { CAN_COMMUNICATIONS_ROUTER_ID(TSACMAINBOARDCOOLINGTEMPERATURE1), CAN_COMMUNICATIONS_ROUTER_ID(TSACMAINBOARDCOOLINGTEMPERATURE2) }, // 0x500-0x501
    { CAN_COMMUNICATIONS_ROUTER_ID(TSACMAINBOARDFEEDBACK), CAN_COMMUNICATIONS_ROUTER_ID(TSACMAINBOARDFEEDBACK) },                       // 0x50E
    { CAN_COMMUNICATIONS_ROUTER_ID(DASREARVERSION), CAN_COMMUNICATIONS_ROUTER_ID(ECULIBCANVERSIONINFO) },                               // 0x664-0x684: board versions
    { CAN_COMMUNICATIONS_ROUTER_ID(TSACMAINBOARDVERSION), CAN_COMMUNICATIONS_ROUTER_ID(TSACMAINBOARDLIBCANVERSIONINFO) },               // 0x691-0x694
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

#define CAN_COMMUNICATIONS_ROUTER_KMH_PER_MPS (3.6F)     /*!< m/s to km/h */
#define CAN_COMMUNICATIONS_ROUTER_SPEED_MAX_KMH (999.0F) /*!< Clamp: three digits on the dashboard */
#define CAN_COMMUNICATIONS_ROUTER_STALE_MIN_MS (500U)    /*!< Shortest stale time: fast messages (5 ms) must not flicker to "--" on a late routine */

/*!
 * \brief Stale time of a message: three missed frames, at least CAN_COMMUNICATIONS_ROUTER_STALE_MIN_MS.
 *     Messages without a cycle time (libcan can_oneshot, e.g. the IMU) get the minimum.
 */
#define CAN_COMMUNICATIONS_ROUTER_STALE_MS(cycle_ms) (((cycle_ms) <= 0) ? CAN_COMMUNICATIONS_ROUTER_STALE_MIN_MS : EAGLETRT_API_MAX(3U * (uint32_t)(cycle_ms), CAN_COMMUNICATIONS_ROUTER_STALE_MIN_MS))

/*!
 * \brief The message each UIDataSource comes from, and how long it stays fresh.
 */
struct CanCommunicationsRouterSource {
    uint32_t id;       /*!< Message carrying the source, CAN_PLACEHOLDER_MESSAGE_ID until libcan has it */
    uint32_t stale_ms; /*!< Silence after which the pages show "--" */
};

#define CAN_COMMUNICATIONS_ROUTER_SOURCE(cycle, name) { (uint32_t)CAN_PRIMARY_MESSAGE_FRAME_ID_##name, CAN_COMMUNICATIONS_ROUTER_STALE_MS(can_primary_cycle_time_##cycle) }

// clang-format off
EAGLETRT_STATIC const struct CanCommunicationsRouterSource prv_router_api_sources[UI_DATA_SOURCE_COUNT] = {
    [UI_DATA_SOURCE_HV_SOC] = CAN_COMMUNICATIONS_ROUTER_SOURCE(tsacmainboardestimatedsoc, TSACMAINBOARDESTIMATEDSOC),
    [UI_DATA_SOURCE_HV_TEMP] = CAN_COMMUNICATIONS_ROUTER_SOURCE(tsacmainboardtemperatureinfo, TSACMAINBOARDTEMPERATUREINFO),
    [UI_DATA_SOURCE_LV_SOC] = CAN_COMMUNICATIONS_ROUTER_SOURCE(lvacestimatedsoc, LVACESTIMATEDSOC),
    [UI_DATA_SOURCE_SPEED] = CAN_COMMUNICATIONS_ROUTER_SOURCE(estimatedvehiclevelocity, ESTIMATEDVEHICLEVELOCITY),
    [UI_DATA_SOURCE_SHUTDOWN] = CAN_COMMUNICATIONS_ROUTER_SOURCE(tsacmainboardfeedbackshutdown, TSACMAINBOARDFEEDBACKSHUTDOWN),
    [UI_DATA_SOURCE_INVERTER] = CAN_COMMUNICATIONS_ROUTER_SOURCE(invertertemperature, INVERTERTEMPERATURE),
    [UI_DATA_SOURCE_MOTOR] = CAN_COMMUNICATIONS_ROUTER_SOURCE(motortemperature, MOTORTEMPERATURE),
    [UI_DATA_SOURCE_THROTTLE] = CAN_COMMUNICATIONS_ROUTER_SOURCE(pedalsthrottle, PEDALSTHROTTLE),
    [UI_DATA_SOURCE_BRAKE] = CAN_COMMUNICATIONS_ROUTER_SOURCE(pedalsbrake, PEDALSBRAKE),
    [UI_DATA_SOURCE_LAP] = CAN_COMMUNICATIONS_ROUTER_SOURCE(currentlap, CURRENTLAP),
    [UI_DATA_SOURCE_HV_VOLTAGE] = CAN_COMMUNICATIONS_ROUTER_SOURCE(tsacmainboardvoltageinfo, TSACMAINBOARDVOLTAGEINFO),
    [UI_DATA_SOURCE_HV_CURRENT] = CAN_COMMUNICATIONS_ROUTER_SOURCE(tsacmainboardcurrentinfo, TSACMAINBOARDCURRENTINFO),
    [UI_DATA_SOURCE_HV_FEEDBACK] = CAN_COMMUNICATIONS_ROUTER_SOURCE(tsacmainboardfeedback, TSACMAINBOARDFEEDBACK),
    [UI_DATA_SOURCE_HV_STATUS] = CAN_COMMUNICATIONS_ROUTER_SOURCE(tsacstatus, TSACSTATUS),
    [UI_DATA_SOURCE_HV_SHUTDOWN] = CAN_COMMUNICATIONS_ROUTER_SOURCE(tsacmainboardshutdown, TSACMAINBOARDSHUTDOWN),
    [UI_DATA_SOURCE_HV_COOLING_1] = CAN_COMMUNICATIONS_ROUTER_SOURCE(tsacmainboardcoolingtemperature1, TSACMAINBOARDCOOLINGTEMPERATURE1),
    [UI_DATA_SOURCE_HV_COOLING_2] = CAN_COMMUNICATIONS_ROUTER_SOURCE(tsacmainboardcoolingtemperature2, TSACMAINBOARDCOOLINGTEMPERATURE2),
    [UI_DATA_SOURCE_TORQUE] = CAN_COMMUNICATIONS_ROUTER_SOURCE(motortorque, MOTORTORQUE),
    [UI_DATA_SOURCE_COOLING_MOTOR] = CAN_COMMUNICATIONS_ROUTER_SOURCE(coolingmotortemperature, COOLINGMOTORTEMPERATURE),
    [UI_DATA_SOURCE_COOLING_CIRCUIT] = CAN_COMMUNICATIONS_ROUTER_SOURCE(coolingcircuittemperature, COOLINGCIRCUITTEMPERATURE),
    [UI_DATA_SOURCE_COOLING_OUT] = CAN_COMMUNICATIONS_ROUTER_SOURCE(coolingout, COOLINGOUT),
    [UI_DATA_SOURCE_TYRE_FRONT] = CAN_COMMUNICATIONS_ROUTER_SOURCE(tyretemperaturefront, TYRETEMPERATUREFRONT),
    [UI_DATA_SOURCE_TYRE_REAR] = CAN_COMMUNICATIONS_ROUTER_SOURCE(tyretemperaturerear, TYRETEMPERATUREREAR),
    [UI_DATA_SOURCE_DAS_SHUTDOWN] = CAN_COMMUNICATIONS_ROUTER_SOURCE(dasshutdown, DASSHUTDOWN),
    [UI_DATA_SOURCE_PIPO_SHUTDOWN] = CAN_COMMUNICATIONS_ROUTER_SOURCE(piposhutdown, PIPOSHUTDOWN),
    [UI_DATA_SOURCE_EBS] = CAN_COMMUNICATIONS_ROUTER_SOURCE(brakesebs, BRAKESEBS),
    [UI_DATA_SOURCE_LV_VOLTAGE] = CAN_COMMUNICATIONS_ROUTER_SOURCE(lvacvoltageinfo, LVACVOLTAGEINFO),
    [UI_DATA_SOURCE_LV_TEMP] = CAN_COMMUNICATIONS_ROUTER_SOURCE(lvactemperatureinfo, LVACTEMPERATUREINFO),
    [UI_DATA_SOURCE_LV_STATUS] = CAN_COMMUNICATIONS_ROUTER_SOURCE(lvacfsm, LVACFSM),
    [UI_DATA_SOURCE_PIPO_SUPPLY] = CAN_COMMUNICATIONS_ROUTER_SOURCE(powersupply, POWERSUPPLY),
    [UI_DATA_SOURCE_PIPO_LINES_LOW] = CAN_COMMUNICATIONS_ROUTER_SOURCE(linecurrentlowpower, LINECURRENTLOWPOWER),
    [UI_DATA_SOURCE_PIPO_LINES_HIGH] = CAN_COMMUNICATIONS_ROUTER_SOURCE(linecurrenthighpower, LINECURRENTHIGHPOWER),
    [UI_DATA_SOURCE_PIPO_LINES_AS] = CAN_COMMUNICATIONS_ROUTER_SOURCE(linecurrenthighpoweras, LINECURRENTHIGHPOWERAS),
    [UI_DATA_SOURCE_PIPO_STATUS] = CAN_COMMUNICATIONS_ROUTER_SOURCE(pipofsm, PIPOFSM),
    [UI_DATA_SOURCE_IMU_ACC] = CAN_COMMUNICATIONS_ROUTER_SOURCE(imuacceleration, IMUACCELERATION),
    [UI_DATA_SOURCE_IMU_GYRO] = CAN_COMMUNICATIONS_ROUTER_SOURCE(imuangularrate, IMUANGULARRATE),
    [UI_DATA_SOURCE_POSITION] = CAN_COMMUNICATIONS_ROUTER_SOURCE(estimatedvehicleposition, ESTIMATEDVEHICLEPOSITION),
    [UI_DATA_SOURCE_STEERING_STATUS] = CAN_COMMUNICATIONS_ROUTER_SOURCE(steeringfsm, STEERINGFSM),
    // TODO(libcan): no message yet (can-placeholders.h), they stay stale.
    [UI_DATA_SOURCE_INVERTER_STATUS] = { CAN_PLACEHOLDER_MESSAGE_ID, 0U },
    [UI_DATA_SOURCE_MOTOR_SPEED] = { CAN_PLACEHOLDER_MESSAGE_ID, 0U },
    [UI_DATA_SOURCE_TORQUE_ACTUAL] = { CAN_PLACEHOLDER_MESSAGE_ID, 0U },
    [UI_DATA_SOURCE_TYRE_PRESSURE] = { CAN_PLACEHOLDER_MESSAGE_ID, 0U },
    [UI_DATA_SOURCE_DISC_TEMP] = { CAN_PLACEHOLDER_MESSAGE_ID, 0U },
    [UI_DATA_SOURCE_COOLING_TANK] = { CAN_PLACEHOLDER_MESSAGE_ID, 0U },
    [UI_DATA_SOURCE_BRAKE_BIAS_SET] = { CAN_PLACEHOLDER_MESSAGE_ID, 0U },
    [UI_DATA_SOURCE_PEDALS_VOLTAGE] = { CAN_PLACEHOLDER_MESSAGE_ID, 0U },
    [UI_DATA_SOURCE_GPS] = { CAN_PLACEHOLDER_MESSAGE_ID, 0U },
    [UI_DATA_SOURCE_PC_AS] = { CAN_PLACEHOLDER_MESSAGE_ID, 0U },
    [UI_DATA_SOURCE_MISSION] = { CAN_PLACEHOLDER_MESSAGE_ID, 0U },
    [UI_DATA_SOURCE_EBS_STATE] = { CAN_PLACEHOLDER_MESSAGE_ID, 0U },
    [UI_DATA_SOURCE_PIPO_FUSES] = { CAN_PLACEHOLDER_MESSAGE_ID, 0U },
    [UI_DATA_SOURCE_PIPO_CHARGER] = { CAN_PLACEHOLDER_MESSAGE_ID, 0U },
    [UI_DATA_SOURCE_SHUTDOWN_EXTRA] = { CAN_PLACEHOLDER_MESSAGE_ID, 0U },
    [UI_DATA_SOURCE_SUSPENSION] = { CAN_PLACEHOLDER_MESSAGE_ID, 0U },
    [UI_DATA_SOURCE_LV_REASON] = { CAN_PLACEHOLDER_MESSAGE_ID, 0U },
};
// clang-format on

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
    for (uint8_t source = 0U; source < UI_DATA_SOURCE_FIRST_PLACEHOLDER; source++) {
        if (prv_router_api_sources[source].id == id) {
            ui_data_api_get()->stale &= ~UI_DATA_STALE_BIT(source);
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
    for (uint8_t source = 0U; source < UI_DATA_SOURCE_FIRST_PLACEHOLDER; source++) {
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

/*!
 * \brief One value per wheel from a libcan message whose signals end in fl, fr, rl, rr.
 */
#define CAN_COMMUNICATIONS_ROUTER_CORNERS(message, prefix) ((struct UIDataCorners){ (message).prefix##fl, (message).prefix##fr, (message).prefix##rl, (message).prefix##rr })

#define CAN_COMMUNICATIONS_ROUTER_DEG_PER_RAD (57.29578F) /*!< 180 / pi */
#define CAN_COMMUNICATIONS_ROUTER_FULL_TURN_DEG (360.0F)

/*!
 * \brief Heading in degrees, 0..360, from the estimated yaw in radians.
 */
EAGLETRT_STATIC float prv_router_api_heading(float psi_rad) {
    const float deg = fmodf(psi_rad * CAN_COMMUNICATIONS_ROUTER_DEG_PER_RAD, CAN_COMMUNICATIONS_ROUTER_FULL_TURN_DEG);
    return (deg < 0.0F) ? deg + CAN_COMMUNICATIONS_ROUTER_FULL_TURN_DEG : deg;
}

/*!
 * \brief A board's commit hash arrived (SW tab).
 */
// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
EAGLETRT_STATIC void prv_router_api_board_commit(enum UIDataBoard board, uint32_t commit) {
    struct UIDataBoardInfo *info = &ui_data_api_get()->boards[board];
    info->commit = commit;
    info->known |= UI_DATA_BOARD_KNOWN_COMMIT;
}

/*!
 * \brief A board's libcan version arrived (SW tab).
 */
// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
EAGLETRT_STATIC void prv_router_api_board_libcan(enum UIDataBoard board, uint16_t major, uint16_t minor, uint16_t patch) {
    struct UIDataBoardInfo *info = &ui_data_api_get()->boards[board];
    info->libcan[0] = major;
    info->libcan[1] = minor;
    info->libcan[2] = patch;
    info->known |= UI_DATA_BOARD_KNOWN_LIBCAN;
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
            ui_data->ecu_status = decoded.ecufsm.vehiclestatus;
            ui_data->boards[UI_DATA_BOARD_ECU].status = decoded.ecufsm.vehiclestatus;
            EAGLETRT_API_UNUSED(timeout_api_start(prv_router_api_ecu_timeout));
            break;

        case CAN_PRIMARY_MESSAGE_FRAME_ID_TSACMAINBOARDESTIMATEDSOC:
            // libcan hands back a 0..1 fraction, the dashboard shows a percentage.
            ui_data->soc = (uint8_t)EAGLETRT_API_CLAMP(decoded.tsacmainboardestimatedsoc.soc * CAN_COMMUNICATIONS_ROUTER_SOC_PERCENT, 0.0F, CAN_COMMUNICATIONS_ROUTER_SOC_PERCENT);
            break;

        case CAN_PRIMARY_MESSAGE_FRAME_ID_TSACMAINBOARDTEMPERATUREINFO:
            ui_data->hv_temp = (int16_t)decoded.tsacmainboardtemperatureinfo.max;
            ui_data->hv.cell_t_max = decoded.tsacmainboardtemperatureinfo.max;
            ui_data->hv.cell_t_min = decoded.tsacmainboardtemperatureinfo.min;
            break;

        case CAN_PRIMARY_MESSAGE_FRAME_ID_INVERTERTEMPERATURE:
            ui_data->inverter_temp = prv_router_api_hottest(
                decoded.invertertemperature.temperaturefl,
                decoded.invertertemperature.temperaturefr,
                decoded.invertertemperature.temperaturerl,
                decoded.invertertemperature.temperaturerr);
            ui_data->powertrain.inverter_temp = CAN_COMMUNICATIONS_ROUTER_CORNERS(decoded.invertertemperature, temperature);
            break;

        case CAN_PRIMARY_MESSAGE_FRAME_ID_MOTORTEMPERATURE:
            ui_data->motor_fl_temp = (int16_t)decoded.motortemperature.temperaturefl;
            ui_data->motor_fr_temp = (int16_t)decoded.motortemperature.temperaturefr;
            ui_data->motor_rl_temp = (int16_t)decoded.motortemperature.temperaturerl;
            ui_data->motor_rr_temp = (int16_t)decoded.motortemperature.temperaturerr;
            ui_data->powertrain.motor_temp = CAN_COMMUNICATIONS_ROUTER_CORNERS(decoded.motortemperature, temperature);
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
            ui_data->pedals.apps_percent[0] = decoded.pedalsthrottle.apps1 * CAN_COMMUNICATIONS_ROUTER_SOC_PERCENT;
            ui_data->pedals.apps_percent[1] = decoded.pedalsthrottle.apps2 * CAN_COMMUNICATIONS_ROUTER_SOC_PERCENT;
            ui_data->pedals.apps_percent[2] = decoded.pedalsthrottle.apps3 * CAN_COMMUNICATIONS_ROUTER_SOC_PERCENT;
            ui_data->pedals.plausibility = decoded.pedalsthrottle.plausibility;
            break;

        case CAN_PRIMARY_MESSAGE_FRAME_ID_PEDALSBRAKE: {
            // No brake travel sensor yet: the highest pressure, 1 % per bar (0-1 bar = 0 %, 100 bar = 100 %).
            const int16_t bar = prv_router_api_hottest(decoded.pedalsbrake.pressurefl, decoded.pedalsbrake.pressurefr, decoded.pedalsbrake.pressurerl, decoded.pedalsbrake.pressurerr);
            ui_data->brake_percent = (uint8_t)EAGLETRT_API_CLAMP(bar, 0, 100);
            ui_data->pedals.pressure_bar = CAN_COMMUNICATIONS_ROUTER_CORNERS(decoded.pedalsbrake, pressure);
            ui_data->pedals.brake_travel = decoded.pedalsbrake.travel * CAN_COMMUNICATIONS_ROUTER_SOC_PERCENT;
            ui_data->pedals.bots_v = decoded.pedalsbrake.botsvoltage;
            ui_data->shutdown_v[SHUTDOWN_NODE_BOTS] = decoded.pedalsbrake.botsvoltage;
            break;
        }

        case CAN_PRIMARY_MESSAGE_FRAME_ID_TSACMAINBOARDFEEDBACKSHUTDOWN:
            ui_data->shutdown_closed = (decoded.tsacmainboardfeedbackshutdown.sdend >= SHUTDOWN_CLOSED_V) ? 1U : 0U;
            break;

        case CAN_PRIMARY_MESSAGE_FRAME_ID_TSACMAINBOARDVOLTAGEINFO:
            ui_data->hv.ts_v = decoded.tsacmainboardvoltageinfo.ts;
            ui_data->hv.pack_v = decoded.tsacmainboardvoltageinfo.total;
            ui_data->hv.cell_v_min = decoded.tsacmainboardvoltageinfo.min;
            ui_data->hv.cell_v_max = decoded.tsacmainboardvoltageinfo.max;
            break;

        case CAN_PRIMARY_MESSAGE_FRAME_ID_TSACMAINBOARDCURRENTINFO:
            ui_data->hv.current_a = decoded.tsacmainboardcurrentinfo.current;
            ui_data->hv.power_kw = decoded.tsacmainboardcurrentinfo.power;
            break;

        case CAN_PRIMARY_MESSAGE_FRAME_ID_TSACMAINBOARDFEEDBACK:
            // "Open command" bits: an AIR is commanded closed when its bit is clear.
            ui_data->hv.airp_closed = (decoded.tsacmainboardfeedback.airpopencom == 0U) ? 1U : 0U;
            ui_data->hv.airn_closed = (decoded.tsacmainboardfeedback.airnopencom == 0U) ? 1U : 0U;
            ui_data->hv.airp_mec_v = decoded.tsacmainboardfeedback.airpopenmec;
            ui_data->hv.airn_mec_v = decoded.tsacmainboardfeedback.airnopenmec;
            ui_data->hv.tsal_green_v = decoded.tsacmainboardfeedback.tsalgreen;
            ui_data->hv.plausible = decoded.tsacmainboardfeedback.plausiblestate;
            ui_data->hv.ams_fault = decoded.tsacmainboardfeedback.bmsfaultcockpitled;
            ui_data->hv.imd_fault = decoded.tsacmainboardfeedback.imdfaultcockpitled;
            break;

        case CAN_PRIMARY_MESSAGE_FRAME_ID_TSACSTATUS:
            ui_data->boards[UI_DATA_BOARD_BMS_HV].status = decoded.tsacstatus.mainboardstatus;
            break;

        case CAN_PRIMARY_MESSAGE_FRAME_ID_TSACMAINBOARDSHUTDOWN:
            ui_data->shutdown_v[SHUTDOWN_NODE_AMS] = decoded.tsacmainboardshutdown.amsvoltage;
            ui_data->shutdown_v[SHUTDOWN_NODE_IMD] = decoded.tsacmainboardshutdown.imdvoltage;
            ui_data->shutdown_v[SHUTDOWN_NODE_TSAC_INTERLOCK] = decoded.tsacmainboardshutdown.interlocktsacvoltage;
            break;

        case CAN_PRIMARY_MESSAGE_FRAME_ID_TSACMAINBOARDCOOLINGTEMPERATURE1:
            ui_data->hv.cooling[0] = decoded.tsacmainboardcoolingtemperature1.inlet;
            ui_data->hv.cooling[1] = decoded.tsacmainboardcoolingtemperature1.outlet1;
            ui_data->hv.cooling[2] = decoded.tsacmainboardcoolingtemperature1.outlet2;
            ui_data->hv.cooling[3] = decoded.tsacmainboardcoolingtemperature1.outlet3;
            break;

        case CAN_PRIMARY_MESSAGE_FRAME_ID_TSACMAINBOARDCOOLINGTEMPERATURE2:
            ui_data->hv.cooling[4] = decoded.tsacmainboardcoolingtemperature2.outlet4;
            ui_data->hv.cooling[5] = decoded.tsacmainboardcoolingtemperature2.outlet5;
            ui_data->hv.cooling[6] = decoded.tsacmainboardcoolingtemperature2.outlet6;
            break;

        case CAN_PRIMARY_MESSAGE_FRAME_ID_MOTORTORQUE:
            ui_data->powertrain.torque = CAN_COMMUNICATIONS_ROUTER_CORNERS(decoded.motortorque, torque);
            break;

        case CAN_PRIMARY_MESSAGE_FRAME_ID_COOLINGMOTORTEMPERATURE:
            ui_data->cooling.motor = (struct UIDataCorners){
                decoded.coolingmotortemperature.frontleft,
                decoded.coolingmotortemperature.frontright,
                decoded.coolingmotortemperature.rearleft,
                decoded.coolingmotortemperature.rearright,
            };
            break;

        case CAN_PRIMARY_MESSAGE_FRAME_ID_COOLINGCIRCUITTEMPERATURE:
            ui_data->cooling.radiator[0] = decoded.coolingcircuittemperature.radiatorleft;
            ui_data->cooling.radiator[1] = decoded.coolingcircuittemperature.radiatorright;
            break;

        case CAN_PRIMARY_MESSAGE_FRAME_ID_COOLINGOUT:
            // 0..1 duty cycles, shown as percentages.
            ui_data->cooling.pump_percent[0] = decoded.coolingout.pumpleft * CAN_COMMUNICATIONS_ROUTER_SOC_PERCENT;
            ui_data->cooling.pump_percent[1] = decoded.coolingout.pumpright * CAN_COMMUNICATIONS_ROUTER_SOC_PERCENT;
            ui_data->cooling.fan_percent[0] = decoded.coolingout.fanleft * CAN_COMMUNICATIONS_ROUTER_SOC_PERCENT;
            ui_data->cooling.fan_percent[1] = decoded.coolingout.fanright * CAN_COMMUNICATIONS_ROUTER_SOC_PERCENT;
            break;

        case CAN_PRIMARY_MESSAGE_FRAME_ID_TYRETEMPERATUREFRONT:
            ui_data->chassis.tyre_temp.fl = decoded.tyretemperaturefront.frontleft;
            ui_data->chassis.tyre_temp.fr = decoded.tyretemperaturefront.frontright;
            break;

        case CAN_PRIMARY_MESSAGE_FRAME_ID_TYRETEMPERATUREREAR:
            ui_data->chassis.tyre_temp.rl = decoded.tyretemperaturerear.rearleft;
            ui_data->chassis.tyre_temp.rr = decoded.tyretemperaturerear.rearright;
            break;

        case CAN_PRIMARY_MESSAGE_FRAME_ID_DASSHUTDOWN:
            ui_data->shutdown_v[SHUTDOWN_NODE_INERTIA] = decoded.dasshutdown.inertia;
            ui_data->shutdown_v[SHUTDOWN_NODE_COCKPIT_MUSHROOM] = decoded.dasshutdown.mushroomcockpit;
            ui_data->shutdown_v[SHUTDOWN_NODE_LEFT_MUSHROOM] = decoded.dasshutdown.mushroomleft;
            ui_data->shutdown_v[SHUTDOWN_NODE_RIGHT_MUSHROOM] = decoded.dasshutdown.mushroomright;
            break;

        case CAN_PRIMARY_MESSAGE_FRAME_ID_PIPOSHUTDOWN:
            ui_data->shutdown_v[SHUTDOWN_NODE_TSMS] = decoded.piposhutdown.tsms;
            ui_data->shutdown_v[SHUTDOWN_NODE_BSPD] = decoded.piposhutdown.bspd;
            ui_data->shutdown_v[SHUTDOWN_NODE_HVD] = decoded.piposhutdown.hvd;
            ui_data->shutdown_v[SHUTDOWN_NODE_ASMS] = decoded.piposhutdown.asms;
            ui_data->shutdown_v[SHUTDOWN_NODE_RES] = decoded.piposhutdown.res;
            break;

        case CAN_PRIMARY_MESSAGE_FRAME_ID_BRAKESEBS:
            ui_data->pedals.ebs_tank_bar[0] = decoded.brakesebs.pressuretank1;
            ui_data->pedals.ebs_tank_bar[1] = decoded.brakesebs.pressuretank2;
            ui_data->pedals.ebs_line_bar[0] = decoded.brakesebs.pressureline1;
            ui_data->pedals.ebs_line_bar[1] = decoded.brakesebs.pressureline2;
            ui_data->pedals.ebs_load_n[0] = decoded.brakesebs.loadline1;
            ui_data->pedals.ebs_load_n[1] = decoded.brakesebs.loadline2;
            ui_data->shutdown_v[SHUTDOWN_NODE_EBS] = decoded.brakesebs.shutdownvoltage;
            break;

        case CAN_PRIMARY_MESSAGE_FRAME_ID_LVACVOLTAGEINFO:
            ui_data->lv.total_v = decoded.lvacvoltageinfo.total;
            ui_data->lv.cell_v_min = decoded.lvacvoltageinfo.min;
            ui_data->lv.cell_v_max = decoded.lvacvoltageinfo.max;
            ui_data->lv.current_a = decoded.lvacvoltageinfo.current;
            break;

        case CAN_PRIMARY_MESSAGE_FRAME_ID_LVACTEMPERATUREINFO:
            ui_data->lv.t_max = decoded.lvactemperatureinfo.max;
            break;

        case CAN_PRIMARY_MESSAGE_FRAME_ID_POWERSUPPLY:
            ui_data->lv.input_v = decoded.powersupply.voltage;
            ui_data->lv.out_a = decoded.powersupply.current;
            ui_data->lv.out_w = decoded.powersupply.power;
            break;

        case CAN_PRIMARY_MESSAGE_FRAME_ID_LINECURRENTLOWPOWER:
            ui_data->lv.line_a[UI_DATA_PIPO_LINE_TELEMETRY] = decoded.linecurrentlowpower.tlm;
            ui_data->lv.line_a[UI_DATA_PIPO_LINE_POWERTRAIN] = decoded.linecurrentlowpower.powertrain;
            ui_data->lv.line_a[UI_DATA_PIPO_LINE_LIGHTS] = decoded.linecurrentlowpower.lights;
            ui_data->lv.line_a[UI_DATA_PIPO_LINE_DRIVER] = decoded.linecurrentlowpower.driver;
            ui_data->lv.line_a[UI_DATA_PIPO_LINE_SHUTDOWN] = decoded.linecurrentlowpower.shutdown;
            break;

        case CAN_PRIMARY_MESSAGE_FRAME_ID_LINECURRENTHIGHPOWER:
            ui_data->lv.line_a[UI_DATA_PIPO_LINE_COOLING_LEFT] = decoded.linecurrenthighpower.coolingleft;
            ui_data->lv.line_a[UI_DATA_PIPO_LINE_COOLING_RIGHT] = decoded.linecurrenthighpower.coolingright;
            break;

        case CAN_PRIMARY_MESSAGE_FRAME_ID_LINECURRENTHIGHPOWERAS:
            ui_data->lv.line_a[UI_DATA_PIPO_LINE_VISION] = decoded.linecurrenthighpoweras.vision;
            ui_data->lv.line_a[UI_DATA_PIPO_LINE_PC_AS] = decoded.linecurrenthighpoweras.pcas;
            ui_data->lv.line_a[UI_DATA_PIPO_LINE_STEERING] = decoded.linecurrenthighpoweras.steering;
            ui_data->lv.line_a[UI_DATA_PIPO_LINE_EBS] = decoded.linecurrenthighpoweras.ebs;
            break;

        case CAN_PRIMARY_MESSAGE_FRAME_ID_IMUACCELERATION:
            ui_data->chassis.acc_g[0] = decoded.imuacceleration.x;
            ui_data->chassis.acc_g[1] = decoded.imuacceleration.y;
            ui_data->chassis.acc_g[2] = decoded.imuacceleration.z;
            ui_data->chassis.imu_temp = decoded.imuacceleration.temperature;
            break;

        case CAN_PRIMARY_MESSAGE_FRAME_ID_IMUANGULARRATE:
            ui_data->chassis.gyro_dps[0] = decoded.imuangularrate.x;
            ui_data->chassis.gyro_dps[1] = decoded.imuangularrate.y;
            ui_data->chassis.gyro_dps[2] = decoded.imuangularrate.z;
            break;

        case CAN_PRIMARY_MESSAGE_FRAME_ID_ESTIMATEDVEHICLEPOSITION:
            ui_data->chassis.heading_deg = prv_router_api_heading(decoded.estimatedvehicleposition.psi);
            break;

        case CAN_PRIMARY_MESSAGE_FRAME_ID_LVACFSM:
            ui_data->boards[UI_DATA_BOARD_PORCHETTO].status = decoded.lvacfsm.status;
            break;

        case CAN_PRIMARY_MESSAGE_FRAME_ID_PIPOFSM:
            ui_data->boards[UI_DATA_BOARD_PIPO].status = decoded.pipofsm.status;
            break;

        case CAN_PRIMARY_MESSAGE_FRAME_ID_STEERINGFSM:
            ui_data->boards[UI_DATA_BOARD_STEERING].status = decoded.steeringfsm.status;
            break;

        case CAN_PRIMARY_MESSAGE_FRAME_ID_ECUVERSIONINFO:
            prv_router_api_board_commit(UI_DATA_BOARD_ECU, decoded.ecuversioninfo.commithash);
            break;
        case CAN_PRIMARY_MESSAGE_FRAME_ID_ECULIBCANVERSION:
            prv_router_api_board_libcan(UI_DATA_BOARD_ECU, decoded.eculibcanversion.major, decoded.eculibcanversion.minor, decoded.eculibcanversion.patch);
            break;
        case CAN_PRIMARY_MESSAGE_FRAME_ID_TSACMAINBOARDVERSIONINFO:
            prv_router_api_board_commit(UI_DATA_BOARD_BMS_HV, decoded.tsacmainboardversioninfo.commithash);
            break;
        case CAN_PRIMARY_MESSAGE_FRAME_ID_TSACMAINBOARDLIBCANVERSION:
            prv_router_api_board_libcan(UI_DATA_BOARD_BMS_HV, decoded.tsacmainboardlibcanversion.major, decoded.tsacmainboardlibcanversion.minor, decoded.tsacmainboardlibcanversion.patch);
            break;
        case CAN_PRIMARY_MESSAGE_FRAME_ID_LVACVERSIONINFO:
            prv_router_api_board_commit(UI_DATA_BOARD_PORCHETTO, decoded.lvacversioninfo.commithash);
            break;
        case CAN_PRIMARY_MESSAGE_FRAME_ID_LVACLIBCANVERSION:
            prv_router_api_board_libcan(UI_DATA_BOARD_PORCHETTO, decoded.lvaclibcanversion.major, decoded.lvaclibcanversion.minor, decoded.lvaclibcanversion.patch);
            break;
        case CAN_PRIMARY_MESSAGE_FRAME_ID_PIPOVERSIONINFO:
            prv_router_api_board_commit(UI_DATA_BOARD_PIPO, decoded.pipoversioninfo.commithash);
            break;
        case CAN_PRIMARY_MESSAGE_FRAME_ID_PIPOLIBCANVERSION:
            prv_router_api_board_libcan(UI_DATA_BOARD_PIPO, decoded.pipolibcanversion.major, decoded.pipolibcanversion.minor, decoded.pipolibcanversion.patch);
            break;
        case CAN_PRIMARY_MESSAGE_FRAME_ID_STEERINGVERSIONINFO:
            prv_router_api_board_commit(UI_DATA_BOARD_STEERING, decoded.steeringversioninfo.commithash);
            break;
        case CAN_PRIMARY_MESSAGE_FRAME_ID_STEERINGLIBCANVERSION:
            prv_router_api_board_libcan(UI_DATA_BOARD_STEERING, decoded.steeringlibcanversion.major, decoded.steeringlibcanversion.minor, decoded.steeringlibcanversion.patch);
            break;

            // TODO(libcan): inverters, motor speed (push into rpm_window), delivered torque (push into torque_window), tyre pressure, disc
            // temperature, cooling tanks, brake bias set, pedal voltages, GPS, AS computer,
            // mission, EBS state, fuses, charger, inverter interlock / SDC relay, suspension
            // per corner, LV reason (can-placeholders.h).

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
