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
#include "can-primary.h"
#include "eagletrt-api.h"
#include "parameters-api.h"
#include "ui-data-api.h"

#include <string.h>

/*!
 * \brief Cycle-time bookkeeping for the wheel's outbound messages.
 */
struct CanCommunicationsRouterHandler {
    uint32_t last_tick_ms_button_status; /*!< Tick of the last SteeringWheelButtonStatus. */
    uint32_t last_tick_ms_control_maps;  /*!< Tick of the last ControlMapsSet. */
    bool reset_asked;                    /*!< True if reset has been asked for bootloader. */
};

EAGLETRT_STATIC struct CanCommunicationsRouterHandler can_communications_router_handler;

/*!
 * \brief Scale from the 0..1 state-of-charge libcan reports to a percentage.
 */
#define CAN_COMMUNICATIONS_ROUTER_SOC_PERCENT (100.0F)

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

/*!
 * \brief Build and queue SteeringWheelButtonStatus from the current parameters.
 *
 * \details Carries the two momentary requests the ECU acts on: TS-on and
 *     push-to-talk.
 *
 * \retval CAN_COMMUNICATION_RC_OK on success.
 * \retval CAN_COMMUNICATION_RC_ERROR if libcan failed to serialize.
 */
EAGLETRT_STATIC enum CanCommunicationReturnCode prv_router_api_send_button_status(void) {
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

/*!
 * \brief Build and queue ControlMapsSet from the current parameters.
 *
 * \details The three maps are 0..INPUTS_SHARED_PARAMETER_NUMERIC_MAX on the
 *     wheel but normalized 0..1 floats in libcan, which re-quantizes them
 *     to the same ten steps on the wire.
 *
 * \retval CAN_COMMUNICATION_RC_OK on success.
 * \retval CAN_COMMUNICATION_RC_ERROR if libcan failed to serialize.
 */
EAGLETRT_STATIC enum CanCommunicationReturnCode prv_router_api_send_control_maps(void) {
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
 * \details Event-based message (no cycle time in libcan): only the
 *     telemetry-log toggle is wired today, the other three signals are
 *     reserved for buttons that do not exist on this wheel yet.
 *
 * \retval CAN_COMMUNICATION_RC_OK on success.
 * \retval CAN_COMMUNICATION_RC_ERROR if libcan failed to serialize.
 */
EAGLETRT_STATIC enum CanCommunicationReturnCode prv_router_api_send_driver_action(void) {
    union CanPrimaryMessages message;
    message.driveraction = (struct CanPrimaryDriveraction){
        .log = parameters_api_get(INPUTS_SHARED_PARAMETER_ID_TELEMETRY_LOG),
        .marker = 0U,
        .ok = 0U,
        .repeat = 0U,
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
            return prv_router_api_send_button_status();

        case INPUTS_SHARED_PARAMETER_ID_POWER:
        case INPUTS_SHARED_PARAMETER_ID_REGEN:
        case INPUTS_SHARED_PARAMETER_ID_TORQUE_VECTORING:
        case INPUTS_SHARED_PARAMETER_ID_LAUNCH_CONTROL:
            return prv_router_api_send_control_maps();

        case INPUTS_SHARED_PARAMETER_ID_TELEMETRY_LOG:
            return prv_router_api_send_driver_action();

        case INPUTS_SHARED_PARAMETER_ID_COUNT:
        default:
            return CAN_COMMUNICATION_RC_OK;
    }
}

enum CanCommunicationReturnCode can_communications_router_api_process_tx_periodic(uint32_t tick_ms) {
    enum CanCommunicationReturnCode return_code = CAN_COMMUNICATION_RC_OK;

    if (tick_ms - can_communications_router_handler.last_tick_ms_button_status >= (uint32_t)can_primary_cycle_time_steeringwheelbuttonstatus) {
        can_communications_router_handler.last_tick_ms_button_status = tick_ms;
        if (prv_router_api_send_button_status() != CAN_COMMUNICATION_RC_OK) {
            return_code = CAN_COMMUNICATION_RC_ERROR;
        }
    }

    if (tick_ms - can_communications_router_handler.last_tick_ms_control_maps >= (uint32_t)can_primary_cycle_time_controlmapsset) {
        can_communications_router_handler.last_tick_ms_control_maps = tick_ms;
        if (prv_router_api_send_control_maps() != CAN_COMMUNICATION_RC_OK) {
            return_code = CAN_COMMUNICATION_RC_ERROR;
        }
    }

    return return_code;
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
        case CAN_PRIMARY_ECUFSM_VEHICLESTATUS_TSON:
            return UI_DATA_VEHICLE_STATE_READY;

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

        case CAN_PRIMARY_ECUFSM_VEHICLESTATUS_INIT:
        case CAN_PRIMARY_ECUFSM_VEHICLESTATUS_IDLE:
        case CAN_PRIMARY_ECUFSM_VEHICLESTATUS_DISCHARGE:
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

    if (frame->id == 0x17 || frame->id == 0x18) {
        can_communications_router_handler.reset_asked = true;
        return CAN_COMMUNICATION_RC_OK;
    }

    switch ((enum CanPrimaryMessageFrameId)frame->id) {
        case CAN_PRIMARY_MESSAGE_FRAME_ID_ECUFSM:
        case CAN_PRIMARY_MESSAGE_FRAME_ID_TSACMAINBOARDESTIMATEDSOC:
        case CAN_PRIMARY_MESSAGE_FRAME_ID_TSACMAINBOARDTEMPERATUREINFO:
        case CAN_PRIMARY_MESSAGE_FRAME_ID_INVERTERTEMPERATURE:
        case CAN_PRIMARY_MESSAGE_FRAME_ID_MOTORTEMPERATURE:
        case CAN_PRIMARY_MESSAGE_FRAME_ID_TYRETEMPERATUREFRONT:
        case CAN_PRIMARY_MESSAGE_FRAME_ID_TYRETEMPERATUREREAR:
        case CAN_PRIMARY_MESSAGE_FRAME_ID_CURRENTLAP:
        case CAN_PRIMARY_MESSAGE_FRAME_ID_LAPS:
            break;

        default:
            // Not a message the dashboard renders.
            return CAN_COMMUNICATION_RC_OK;
    }

    if (can_primary_api_deserialize_from_id((enum CanPrimaryMessageFrameId)frame->id, payload, &decoded) < 0) {
        return CAN_COMMUNICATION_RC_RECEIVE_HANDLER_ERROR;
    }

    switch ((enum CanPrimaryMessageFrameId)frame->id) {
        case CAN_PRIMARY_MESSAGE_FRAME_ID_ECUFSM:
            ui_data->vehicle_state = (uint8_t)prv_router_api_map_vehicle_state(decoded.ecufsm.vehiclestatus);
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

        case CAN_PRIMARY_MESSAGE_FRAME_ID_TYRETEMPERATUREFRONT:
            ui_data->tire_fl_temp = (int16_t)decoded.tyretemperaturefront.frontleft;
            ui_data->tire_fr_temp = (int16_t)decoded.tyretemperaturefront.frontright;
            break;

        case CAN_PRIMARY_MESSAGE_FRAME_ID_TYRETEMPERATUREREAR:
            ui_data->tire_rl_temp = (int16_t)decoded.tyretemperaturerear.rearleft;
            ui_data->tire_rr_temp = (int16_t)decoded.tyretemperaturerear.rearright;
            break;

        case CAN_PRIMARY_MESSAGE_FRAME_ID_CURRENTLAP:
            ui_data->lap_current = decoded.currentlap.number;
            // libcan decodes the delta in seconds, the dashboard shows ms.
            ui_data->lap_delta_ms = (int32_t)(decoded.currentlap.deltabestlap * CAN_COMMUNICATIONS_ROUTER_MS_PER_SECOND);
            break;

        case CAN_PRIMARY_MESSAGE_FRAME_ID_LAPS:
            ui_data->lap_total = decoded.laps.totalnumber;
            break;

        default:
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

bool can_communications_router_api_reset_asked(void) {
    return can_communications_router_handler.reset_asked;
}
