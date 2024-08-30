#include "controls.h"

#if STEERING_WHEEL_MODE == STEERING_WHEEL_OLIVEC_MODE

void manettino_right_actions(int dsteps) {
    steering_wheel_set_fans_status.fans_speed += (dsteps * 0.1f);
    steering_wheel_set_fans_status.fans_speed = fminf(steering_wheel_set_fans_status.fans_speed, 1.05f);
    steering_wheel_set_fans_status.fans_speed = fmaxf(steering_wheel_set_fans_status.fans_speed, -0.15f);
    if (steering_wheel_set_fans_status.fans_speed < 0.05f) {
        steering_wheel_set_fans_status.fans_override = 0;
    } else {
        steering_wheel_set_fans_status.fans_override = 1;
    }

    primary_hv_set_fans_status_converted_t converted = steering_wheel_set_fans_status;
    STEER_CAN_PACK(primary, PRIMARY, hv_set_fans_status, HV_SET_FANS_STATUS);
}

int slip_preset = 3;
void manettino_center_actions(int dsteps) {
    slip_preset += dsteps;

    primary_hv_set_fans_status_converted_t converted = steering_wheel_set_fans_status;
    STEER_CAN_PACK(primary, PRIMARY, hv_set_fans_status, HV_SET_FANS_STATUS);
}

void manettino_left_actions(int dsteps) {
    steering_wheel_set_fans_status.fans_speed += (dsteps * 0.1f);
    steering_wheel_set_fans_status.fans_speed = fminf(steering_wheel_set_fans_status.fans_speed, 1.05f);
    steering_wheel_set_fans_status.fans_speed = fmaxf(steering_wheel_set_fans_status.fans_speed, -0.15f);
    if (steering_wheel_set_fans_status.fans_speed < 0.05f) {
        steering_wheel_set_fans_status.fans_override = 0;
    } else {
        steering_wheel_set_fans_status.fans_override = 1;
    }

    primary_hv_set_fans_status_converted_t converted = steering_wheel_set_fans_status;
    STEER_CAN_PACK(primary, PRIMARY, hv_set_fans_status, HV_SET_FANS_STATUS);
}

void buttons_pressed_actions(uint8_t button) {
    switch (button) {
        case PADDLE_TOP_LEFT:
        case PADDLE_TOP_RIGHT:
            set_ptt_button_pressed(1);
            break;
        default:
            break;
    }
}

void buttons_released_actions(uint8_t button) {
    switch (button) {
        case PADDLE_TOP_LEFT:
        case PADDLE_TOP_RIGHT:
            set_ptt_button_pressed(0);
            break;
        default:
            break;
    }
}

void buttons_long_pressed_actions(uint8_t button) {
    switch (button) {
        case BUTTON_BOTTOM_LEFT: {
            ecu_set_power_maps_last_state.reg_state = 0;
            break;
        }
        case BUTTON_BOTTOM_RIGHT: {
            ecu_set_power_maps_last_state.tv_state = 0;
            break;
        }
        case BUTTON_TOP_LEFT: {
            ecu_set_power_maps_last_state.reg_state = 1;
            break;
        }
        case BUTTON_TOP_RIGHT: {
            ecu_set_power_maps_last_state.tv_state = 1;
            break;
        }
        default:
            break;
    }
}

#endif  // STEERING_WHEEL_MODE == STEERING_WHEEL_OLIVEC_MODE
