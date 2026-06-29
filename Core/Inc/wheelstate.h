#ifndef WHEEL_STATE_H
#define WHEEL_STATE_H

#include "can-primary.h"

struct WheelState {
    enum CanPrimarySteeringWheelSetEcuStatusTargetstatus target_status;
    bool request_reset;
};

enum CanPrimarySteeringWheelSetEcuStatusTargetstatus wheel_state_get_tson();

void wheel_state_set_tson(enum CanPrimarySteeringWheelSetEcuStatusTargetstatus target_status);

void wheel_state_set_request_reset();

bool wheel_state_get_request_reset();

#endif // WHEEL_STATE_H
