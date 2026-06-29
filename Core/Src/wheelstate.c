#include "wheelstate.h"

static struct WheelState wheel_state = {0};

enum CanPrimarySteeringWheelSetEcuStatusTargetstatus wheel_state_get_tson() {
    return wheel_state.target_status;
}

void wheel_state_set_tson(enum CanPrimarySteeringWheelSetEcuStatusTargetstatus target_status) {
    wheel_state.target_status = target_status;
}
