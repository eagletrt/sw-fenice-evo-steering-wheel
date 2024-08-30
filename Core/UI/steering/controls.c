#include "controls.h"

primary_ecu_set_power_maps_converted_t ecu_set_power_maps_last_state      = {.map_power = 1.0f, .reg_state = 1, .sc_state = 1, .tv_state = 1};
primary_lv_radiator_speed_converted_t steering_wheel_state_radiator_speed = {.radiator_speed = 0.0f, .status = primary_lv_radiator_speed_status_off};
primary_lv_pumps_speed_converted_t steering_wheel_state_pumps_speed       = {.pumps_speed = 0.0f, .status = primary_lv_radiator_speed_status_off};
primary_hv_set_fans_status_converted_t steering_wheel_set_fans_status     = {.fans_override = 0, .fans_speed = 0.0f};

void send_set_car_status(primary_ecu_set_status_status val) {
    primary_ecu_set_status_converted_t converted = {0};
    converted.status                             = val;
    STEER_CAN_PACK(primary, PRIMARY, ecu_set_status, ECU_SET_STATUS);
    can_send(&msg, true);
    can_send(&msg, true);
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
