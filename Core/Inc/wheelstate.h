#ifndef WHEEL_STATE_H
#define WHEEL_STATE_H

#include "can-primary.h"

struct WheelState {
    enum CanPrimarySteeringWheelSetEcuStatusTargetstatus target_status;
};

enum CanPrimarySteeringWheelSetEcuStatusTargetstatus wheel_state_get_tson();

void wheel_state_set_tson(enum CanPrimarySteeringWheelSetEcuStatusTargetstatus target_status);

#endif // WHEEL_STATE_H
