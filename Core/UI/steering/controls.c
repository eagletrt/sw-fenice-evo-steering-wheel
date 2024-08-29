#include "controls.h"

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
            // display_notification("Sending\nIDLE\nanyway", 1500, COLOR_GREEN_STATUS_HEX, COLOR_PRIMARY_HEX);
            break;
        }
        case primary_ecu_status_status_idle: {
            send_set_car_status(primary_ecu_set_status_status_ready);
            // if (is_shutdown_closed()) {
            // display_notification("TSON", 1500, COLOR_RED_STATUS_HEX, COLOR_PRIMARY_HEX);
            // } else {
            // display_notification("Shutdown\nOpen\nSending TSON\nAnyway", 1500, COLOR_ORANGE_STATUS_HEX, COLOR_PRIMARY_HEX);
            // }
            break;
        }
        case primary_ecu_status_status_start_ts_precharge:
        case primary_ecu_status_status_wait_ts_precharge: {
            // display_notification("Precharge\nnot finished\nyet", 1500, COLOR_ORANGE_STATUS_HEX, COLOR_PRIMARY_HEX);
            break;
        }
        case primary_ecu_status_status_wait_driver: {
            GET_LAST_STATE(secondary, pedal_brakes_pressure, SECONDARY, PEDAL_BRAKES_PRESSURE);
            float brake_pressure = (secondary_pedal_brakes_pressure_last_state->front + secondary_pedal_brakes_pressure_last_state->rear) / 2.0f;
            if (brake_pressure > 0.89f) {
                // display_notification("DRIVE", 1500, COLOR_RED_STATUS_HEX, COLOR_PRIMARY_HEX);
            } else {
                // display_notification("Not Enough\nFreno\nSending DRIVE\nAnyway", 1500, COLOR_ORANGE_STATUS_HEX, COLOR_PRIMARY_HEX);
            }
            send_set_car_status(primary_ecu_set_status_status_drive);
            break;
        }
        case primary_ecu_status_status_enable_inv_drive:
        case primary_ecu_status_status_drive:
        case primary_ecu_status_status_disable_inv_drive:
        case primary_ecu_status_status_start_ts_discharge:
        case primary_ecu_status_status_re_enable_inverter_drive:
        case primary_ecu_status_status_wait_ts_discharge: {
            // disabled because the central buttons reads badly at the hardware level
            // send_set_car_status(primary_ecu_set_status_status_idle);
            // display_notification("IDLE", 1500, COLOR_GREEN_STATUS_HEX, COLOR_PRIMARY_HEX);
            break;
        }
        case primary_ecu_status_status_fatal_error: {
            send_set_car_status(primary_ecu_set_status_status_idle);
            // display_notification("ECU in FATAL ERROR,\nsending IDLE anyway", 1500, COLOR_GREEN_STATUS_HEX, COLOR_PRIMARY_HEX);
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
