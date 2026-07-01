#define _XOPEN_SOURCE
#include "wheelstate.h"

#include "can-communications-api.h"
#include "can-primary-api.h"
#include <time.h>

static struct WheelState wheel_state = { 0 };

enum WheelStateReturnCode wheel_state_init() {
    struct tm timeinfo;
    strptime(__DATE__ " " __TIME__, "%b %d %Y %H:%M:%S", &timeinfo);
    wheel_state.build_time = mktime(&timeinfo);

    return WHEEL_STATE_RC_OK;
}

enum CanPrimarySteeringWheelSetEcuStatusTargetstatus wheel_state_get_tson() {
    return wheel_state.target_status;
}

void wheel_state_set_tson(enum CanPrimarySteeringWheelSetEcuStatusTargetstatus target_status) {
    wheel_state.target_status = target_status;
}

void wheel_state_set_request_reset() {
    wheel_state.request_reset = true;
}

bool wheel_state_get_request_reset() {
    return wheel_state.request_reset;
}

enum WheelStateReturnCode wheel_state_periodically_send_identity(uint32_t tick) {
    EAGLETRT_STATIC uint32_t last_send_tick = 0;
    if (tick - last_send_tick >= 2000) {
        last_send_tick = tick;
        union CanPrimaryMessages message = { 0 };
        struct CanCommunicationFrame frame = { 0 };
        message.ecu_version.canlibbuildtime_s = can_generation_time;
        message.ecu_version.buildtime_s = wheel_state.build_time;
        frame.id = CAN_PRIMARY_MESSAGE_FRAME_ID_ECU_VERSION;
        frame.length = can_primary_byte_size_ecu_version;
        if (can_primary_api_serialize_from_id(frame.id, &message, frame.data) == -1) {
            return WHEEL_STATE_RC_ERROR;
        }
        if (can_communications_api_add_to_tx_buffer(CAN_COMMUNICATION_NETWORK_PRIMARY, &frame) != CAN_COMMUNICATION_RC_OK) {
            return WHEEL_STATE_RC_ERROR;
        }
    }
    return WHEEL_STATE_RC_OK;
}
