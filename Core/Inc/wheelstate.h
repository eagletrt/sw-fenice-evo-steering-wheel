#ifndef WHEEL_STATE_H
#define WHEEL_STATE_H

#include "can-primary.h"

enum WheelStateReturnCode {
    WHEEL_STATE_RC_OK,
    WHEEL_STATE_RC_ERROR
};

struct WheelState {
    enum CanPrimarySteeringWheelSetEcuStatusTargetstatus target_status;
    bool request_reset;
    uint32_t build_time;
};

enum CanPrimarySteeringWheelSetEcuStatusTargetstatus wheel_state_get_tson();

void wheel_state_set_tson(enum CanPrimarySteeringWheelSetEcuStatusTargetstatus target_status);

void wheel_state_set_request_reset();

bool wheel_state_get_request_reset();

enum WheelStateReturnCode wheel_state_periodically_send_identity(uint32_t tick);

#endif // WHEEL_STATE_H
