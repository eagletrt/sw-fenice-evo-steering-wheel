#include "controls.h"

void manettino_right_actions(int dsteps) {
    GET_LAST_STATE(primary, hv_set_fans_status, PRIMARY, HV_SET_FANS_STATUS);
    primary_hv_set_fans_status_last_state->fans_speed += (dsteps * 0.1f);
    primary_hv_set_fans_status_last_state->fans_speed = fminf(primary_hv_set_fans_status_last_state->fans_speed, 1.05f);
    primary_hv_set_fans_status_last_state->fans_speed = fmaxf(primary_hv_set_fans_status_last_state->fans_speed, -0.15f);
    if (primary_hv_set_fans_status_last_state->fans_speed < 0.05f) {
        primary_hv_set_fans_status_last_state->fans_override = 0;
    } else {
        primary_hv_set_fans_status_last_state->fans_override = 1;
    }
}

void manettino_center_actions(int dsteps) {
    GET_LAST_STATE(primary, ecu_set_power_maps, PRIMARY, ECU_SET_POWER_MAPS);
    if (dsteps > 0) {
        primary_ecu_set_power_maps_last_state->sc_state = 1;
    } else {
        primary_ecu_set_power_maps_last_state->sc_state = 0;
    }

    /* GET_LAST_STATE(primary, lv_set_cooling_aggressiveness, PRIMARY, LV_SET_COOLING_AGGRESSIVENESS);
    primary_lv_set_cooling_aggressiveness_last_state->status += dsteps;
    primary_lv_set_cooling_aggressiveness_last_state->status = clamp(primary_lv_set_cooling_aggressiveness_last_state->status, 0, 2); */
}

void manettino_left_actions(int dsteps) {
    GET_LAST_STATE(primary, lv_set_pumps_speed, PRIMARY, LV_SET_PUMPS_SPEED);
    primary_lv_set_pumps_speed_last_state->pumps_speed += (dsteps * 0.1f);
    primary_lv_set_pumps_speed_last_state->pumps_speed = fminf(primary_lv_set_pumps_speed_last_state->pumps_speed, 1.05f);
    primary_lv_set_pumps_speed_last_state->pumps_speed = fmaxf(primary_lv_set_pumps_speed_last_state->pumps_speed, -0.15f);
    if (primary_lv_set_pumps_speed_last_state->pumps_speed < 0.05f) {
        primary_lv_set_pumps_speed_last_state->status = primary_lv_set_pumps_speed_status_auto;
    } else {
        primary_lv_set_pumps_speed_last_state->status = primary_lv_set_pumps_speed_status_manual;
    }

    GET_LAST_STATE(primary, lv_set_radiator_speed, PRIMARY, LV_SET_RADIATOR_SPEED);
    primary_lv_set_radiator_speed_last_state->radiator_speed += (dsteps * 0.1f);
    primary_lv_set_radiator_speed_last_state->radiator_speed = fminf(primary_lv_set_radiator_speed_last_state->radiator_speed, 1.05f);
    primary_lv_set_radiator_speed_last_state->radiator_speed = fmaxf(primary_lv_set_radiator_speed_last_state->radiator_speed, -0.15f);
    if (primary_lv_set_radiator_speed_last_state->radiator_speed < 0.05f) {
        primary_lv_set_radiator_speed_last_state->status = primary_lv_set_radiator_speed_status_auto;
    } else {
        primary_lv_set_radiator_speed_last_state->status = primary_lv_set_radiator_speed_status_manual;
    }
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
    GET_LAST_STATE(primary, ecu_set_power_maps, PRIMARY, ECU_SET_POWER_MAPS);
    switch (button) {
        case BUTTON_BOTTOM_LEFT: {
            primary_ecu_set_power_maps_last_state->reg_state = 0;
            break;
        }
        case BUTTON_BOTTOM_RIGHT: {
            primary_ecu_set_power_maps_last_state->tv_state = 0;
            break;
        }
        case BUTTON_TOP_LEFT: {
            primary_ecu_set_power_maps_last_state->reg_state = 1;
            break;
        }
        case BUTTON_TOP_RIGHT: {
            primary_ecu_set_power_maps_last_state->tv_state = 1;
            break;
        }
        case PADDLE_BOTTOM_LEFT: {
            primary_ecu_set_power_maps_last_state->sc_state = 0;
            break;
        }
        case PADDLE_BOTTOM_RIGHT: {
            primary_ecu_set_power_maps_last_state->sc_state = 1;
            break;
        }
        default:
            break;
    }
}

void prepare_set_car_status(void) {
    GET_LAST_STATE(primary, ecu_status, PRIMARY, ECU_STATUS);
    switch (primary_ecu_status_last_state->status) {
        case primary_ecu_status_status_init:
        case primary_ecu_status_status_enable_inv_updates:
        case primary_ecu_status_status_check_inv_settings: {
            send_set_car_status(primary_ecu_set_status_status_idle);
            break;
        }
        case primary_ecu_status_status_idle: {
            send_set_car_status(primary_ecu_set_status_status_ready);
            break;
        }
        case primary_ecu_status_status_start_ts_precharge:
        case primary_ecu_status_status_wait_ts_precharge: {
            break;
        }
        case primary_ecu_status_status_wait_driver: {
            send_set_car_status(primary_ecu_set_status_status_drive);
            break;
        }
        case primary_ecu_status_status_enable_inv_drive:
        case primary_ecu_status_status_drive:
        case primary_ecu_status_status_disable_inv_drive:
        case primary_ecu_status_status_start_ts_discharge:
        case primary_ecu_status_status_re_enable_inverter_drive:
        case primary_ecu_status_status_wait_ts_discharge: {
            break;
        }
        case primary_ecu_status_status_fatal_error: {
            send_set_car_status(primary_ecu_set_status_status_idle);
            break;
        }
    }
}

bool send_set_car_status_directly(void) {
    GET_LAST_STATE(primary, ecu_status, PRIMARY, ECU_STATUS);
    bool retval = true;
    switch (primary_ecu_status_last_state->status) {
        case primary_ecu_status_status_idle:
        case primary_ecu_status_status_wait_driver: {
            retval = false;
            break;
        }
        default:
            break;
    }
    return retval;
}
