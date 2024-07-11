#include "controls.h"

bool calibration_min_sent_request[CALBOX_N];
bool calibration_max_sent_request[CALBOX_N];
uint32_t calibration_min_request_timestamp[CALBOX_N];
uint32_t calibration_max_request_timestamp[CALBOX_N];

int torque_vectoring_last_state = 0;
int slip_control_last_state     = 0;
int pork_fans_status_last_state = 0;
int power_map_last_state        = 0;

int left_manettino_selection = 0;

#if CANSNIFFER_ENABLED == 1
int primary_cansniffer_start_index   = 0;
int secondary_cansniffer_start_index = 0;
int inverters_cansniffer_start_index = 0;
#endif

#if STEER_TAB_CALIBRATION_ENABLED == 1
extern lv_obj_t *set_min_btn;
extern lv_obj_t *set_max_btn;
#endif
char sprintf_buffer_controls[BUFSIZ];

primary_ecu_set_power_maps_converted_t ecu_set_power_maps_last_state = {.map_pw = 0.0f, .map_sc = 0.0f, .map_tv = 0.0f};

primary_lv_radiator_speed_converted_t steering_wheel_state_radiator_speed = {.radiator_speed = 0.0f, .status = primary_lv_radiator_speed_status_off};
primary_lv_pumps_speed_converted_t steering_wheel_state_pumps_speed       = {.pumps_speed = 0.0f, .status = primary_lv_radiator_speed_status_off};

uint32_t steering_wheel_lv_pumps_speed_sent_timestamp                  = 0;
uint32_t steering_wheel_lv_radiators_speed_sent_timestamp              = 0;
steering_wheel_cooling_status_t steering_wheel_lv_pumps_speed_state    = STEERING_WHEEL_COOLING_STATUS_SYNC;
steering_wheel_cooling_status_t steering_wheel_lv_radiator_speed_state = STEERING_WHEEL_COOLING_STATUS_SYNC;

void manettino_set_radiators_speed(void);
void manettino_set_pumps_speed(void);

void set_dmt_steering_angle_target(void) {
    GET_LAST_STATE(secondary, steer_angle, SECONDARY, STEER_ANGLE);
    set_tab_track_test_dmt_steering_angle_target(secondary_steer_angle_last_state->angle);
}

void turn_telemetry_on_off(void) {
    GET_LAST_STATE(primary, tlm_status, PRIMARY, TLM_STATUS);
    primary_tlm_set_status_converted_t converted = {0};
    if (primary_tlm_status_last_state->status == primary_tlm_status_status_on) {
        display_notification("Sending\nTelemetry\nOFF", 800, 0xff9245U, COLOR_PRIMARY_HEX);
        converted.status = primary_tlm_set_status_status_off;
    } else {
        display_notification("Sending\nTelemetry\nON", 800, COLOR_GREEN_STATUS_HEX, COLOR_PRIMARY_HEX);
        converted.status = primary_tlm_set_status_status_on;
    }
    STEER_CAN_PACK(primary, PRIMARY, tlm_set_status, TLM_SET_STATUS);
    can_send(&msg, true);
}

#define HELP_NOTIFICATION_SCREEN_BACKGROUND (0xf1ff73U)

void help(void) {
    // TODO: try the lvgl messagebox instead of a full screen popup or use a full screen popup with references to steering wheel
    // lv_msgbox_t msgbox;
    if (is_on_help_animation()) {
        restore_previous_screen(NULL);
        return;
    }
    if (engineer_mode) {
    } else {
        switch (current_racing_tab) {
            case STEERING_WHEEL_TAB_RACING: {
                set_on_help_animation();
                display_notification(
                    "telemetry | -\n"
                    "help      | -\n"
                    "torque | power | slip",
                    4000,
                    HELP_NOTIFICATION_SCREEN_BACKGROUND,
                    COLOR_PRIMARY_HEX);
                break;
            }
            case STEERING_WHEEL_TAB_TRACK_TEST: {
                set_on_help_animation();
                display_notification(
                    "  set target  | -\n"
                    "         help | -\n\n"
                    "       - | - | -",
                    2000,
                    HELP_NOTIFICATION_SCREEN_BACKGROUND,
                    COLOR_PRIMARY_HEX);
                break;
            }
            case STEERING_WHEEL_TAB_SENSORS: {
                set_on_help_animation();
                display_notification(
                    "  -  | -\n"
                    "  help | -\n\n"
                    "- | - | -",
                    1000,
                    HELP_NOTIFICATION_SCREEN_BACKGROUND,
                    COLOR_PRIMARY_HEX);
                break;
            }
            case STEERING_WHEEL_TAB_HV: {
                set_on_help_animation();
                display_notification(
                    "balancing | -\n"
                    "   help   | -\n\n"
                    "bal thres  | - | pork fans",
                    4000,
                    HELP_NOTIFICATION_SCREEN_BACKGROUND,
                    COLOR_PRIMARY_HEX);
                break;
            }
            case STEERING_WHEEL_TAB_LV: {
                set_on_help_animation();
                display_notification(
                    "     - | -\n"
                    "     - | -\n\n"
                    "pumps | - | radiators",
                    1000,
                    HELP_NOTIFICATION_SCREEN_BACKGROUND,
                    COLOR_PRIMARY_HEX);
                break;
            }
            default: {
                break;
            }
        }
    }
}

void manettino_right_actions(int dsteps) {
    if (dsteps == 0) {
        return;
    }
    char tbuf[100];
    snprintf(tbuf, 100, "manettino right %d", dsteps);
    if (engineer_mode) {
        tab_terminal_new_message(tbuf);
    } else {
        switch (current_racing_tab) {
            case NOT_SCREEN:
                break;
            case STEERING_WHEEL_TAB_RACING:
                torque_vectoring_last_state += dsteps * 10;
                torque_vectoring_last_state = fminf(torque_vectoring_last_state, TORQUE_VECTORING_MAX);
                torque_vectoring_last_state = fmaxf(torque_vectoring_last_state, TORQUE_VECTORING_MIN);
                manettino_send_torque_vectoring((float)torque_vectoring_last_state / 100.0f);
                break;
            case STEERING_WHEEL_TAB_TRACK_TEST:
                break;
            case STEERING_WHEEL_TAB_SENSORS:
                break;
            case STEERING_WHEEL_TAB_HV:
                pork_fans_status_last_state += dsteps * 10;
                pork_fans_status_last_state = LV_CLAMP(PORK_LOW_FANS_SPEED, pork_fans_status_last_state, PORK_HIGH_FANS_SPEED);
                send_pork_fans_status((float)pork_fans_status_last_state / 100.0f);
                break;
            case STEERING_WHEEL_TAB_LV: {
                steering_wheel_lv_radiator_speed_state             = STEERING_WHEEL_COOLING_STATUS_SET;
                float new_radspeed_val                             = steering_wheel_state_radiator_speed.radiator_speed + ((float)dsteps / 10.0f);
                new_radspeed_val                                   = fminf(new_radspeed_val, SET_RADIATORS_MAX);
                new_radspeed_val                                   = fmaxf(new_radspeed_val, SET_RADIATORS_MIN);
                steering_wheel_state_radiator_speed.radiator_speed = new_radspeed_val;
                manettino_set_radiators_speed();
                break;
            }
            default:
                break;
        }
    }
}

void manettino_center_actions(int dsteps) {
    if (dsteps == 0) {
        return;
    }
    char tbuf[100];
    snprintf(tbuf, 100, "manettino center %d", dsteps);
    if (engineer_mode) {
        tab_terminal_new_message(tbuf);
    } else {
        power_map_last_state += (dsteps * 10);
        power_map_last_state = fminf(power_map_last_state, POWER_MAP_MAX);
        power_map_last_state = fmaxf(power_map_last_state, POWER_MAP_MIN);
        manettino_send_power_map((float)power_map_last_state / 100.0f);
        switch (current_racing_tab) {
            case NOT_SCREEN:
                break;
            case STEERING_WHEEL_TAB_RACING:
                break;
            case STEERING_WHEEL_TAB_TRACK_TEST:
                break;
            case STEERING_WHEEL_TAB_SENSORS:
                break;
            case STEERING_WHEEL_TAB_HV:
                break;
            case STEERING_WHEEL_TAB_LV:
                break;
            default:
                break;
        }
    }
}

void manettino_left_actions(int dsteps) {
    if (dsteps == 0) {
        return;
    }
    char tbuf[100];
    snprintf(tbuf, 100, "manettino left %d", dsteps);
    if (engineer_mode) {
        tab_terminal_new_message(tbuf);
    } else {
        switch (current_racing_tab) {
            case NOT_SCREEN:
                break;
            case STEERING_WHEEL_TAB_RACING:
                slip_control_last_state += dsteps * 10;
                slip_control_last_state = fminf(slip_control_last_state, SLIP_CONTROL_MAX);
                slip_control_last_state = fmaxf(slip_control_last_state, SLIP_CONTROL_MIN);
                manettino_send_slip_control((float)slip_control_last_state / 100.0f);
                break;
            case STEERING_WHEEL_TAB_TRACK_TEST:
                break;
            case STEERING_WHEEL_TAB_SENSORS:
                break;
            case STEERING_WHEEL_TAB_HV:
                // TODO: implement Balancing threshold as defined by the new steering
                // wheel map (see images)
                break;
            case STEERING_WHEEL_TAB_LV: {
                steering_wheel_lv_pumps_speed_state          = STEERING_WHEEL_COOLING_STATUS_SET;
                float new_pumps_speed_val                    = steering_wheel_state_pumps_speed.pumps_speed + ((float)dsteps / 10.0f);
                new_pumps_speed_val                          = fminf(new_pumps_speed_val, SET_PUMP_SPEED_MAX);
                new_pumps_speed_val                          = fmaxf(new_pumps_speed_val, SET_PUMP_SPEED_MIN);
                steering_wheel_state_pumps_speed.pumps_speed = new_pumps_speed_val;
                manettino_set_pumps_speed();
                break;
            }
            default:
                break;
        }
    }
}

void manettino_send_slip_control(float val) {
    ecu_set_power_maps_last_state.map_sc = val;
    int map_val                          = (int)(ecu_set_power_maps_last_state.map_sc * 100.0f);
    snprintf(sprintf_buffer_controls, BUFSIZ, "%u", map_val);
    set_tab_racing_label_text(sprintf_buffer_controls, tab_rac_slip_idx);
}

void manettino_send_torque_vectoring(float val) {
    ecu_set_power_maps_last_state.map_tv = val;
    int map_val                          = (int)(ecu_set_power_maps_last_state.map_tv * 100.0f);
    snprintf(sprintf_buffer_controls, BUFSIZ, "%u", map_val);
    set_tab_racing_label_text(sprintf_buffer_controls, tab_rac_torque_idx);
}

void manettino_send_power_map(float val) {
    ecu_set_power_maps_last_state.map_pw = val;
    float map_val                        = (float)(ecu_set_power_maps_last_state.map_pw * 100.0f);
    snprintf(sprintf_buffer_controls, BUFSIZ, "%.0f", map_val);
    set_tab_racing_label_text(sprintf_buffer_controls, tab_rac_pow_idx);
}

/***
 * RADIATORS
 */

void manettino_send_radiators_speed(void) {
    primary_lv_set_radiator_speed_converted_t converted = {0};
    converted.status                                    = steering_wheel_state_radiator_speed.status;
    converted.radiator_speed                            = steering_wheel_state_radiator_speed.radiator_speed;
    STEER_CAN_PACK(primary, PRIMARY, lv_set_radiator_speed, LV_SET_RADIATOR_SPEED);
    can_send(&msg, true);
}

void manettino_set_radiators_speed(void) {
    steering_wheel_lv_radiators_speed_sent_timestamp = get_current_time_ms();
    steering_wheel_state_radiator_speed.status       = primary_lv_radiator_speed_status_manual;

    if (steering_wheel_state_radiator_speed.radiator_speed < 0.0f) {
        snprintf(sprintf_buffer_controls, BUFSIZ, "SET\nAUTO");
    } else {
        snprintf(sprintf_buffer_controls, BUFSIZ, "SET\n%0.1f", steering_wheel_state_radiator_speed.radiator_speed);
    }
    set_tab_lv_label_text(sprintf_buffer_controls, tab_lv_lb_radiators_value);
    // lv_set_radiators_speed_bar((int32_t)(steering_wheel_state_radiator_speed.radiator_speed * 100.0f));
    manettino_send_radiators_speed();
}

/***
 * PUMPS
 */

void manettino_send_pumps_speed(void) {
    primary_lv_set_pumps_speed_converted_t converted = {0};
    converted.status                                 = steering_wheel_state_pumps_speed.status;
    converted.pumps_speed                            = steering_wheel_state_pumps_speed.pumps_speed;
    STEER_CAN_PACK(primary, PRIMARY, lv_set_pumps_speed, LV_SET_PUMPS_SPEED);
    can_send(&msg, true);
}

void manettino_set_pumps_speed(void) {
    steering_wheel_lv_pumps_speed_sent_timestamp = get_current_time_ms();
    steering_wheel_state_pumps_speed.status      = primary_lv_pumps_speed_status_manual;

    if (steering_wheel_state_pumps_speed.pumps_speed < 0.0f) {
        steering_wheel_state_pumps_speed.pumps_speed = 0.0f;
        snprintf(sprintf_buffer_controls, BUFSIZ, "zero");
    } else {
        snprintf(sprintf_buffer_controls, BUFSIZ, "%0.1f", steering_wheel_state_pumps_speed.pumps_speed);
    }
    set_tab_lv_label_text(sprintf_buffer_controls, tab_lv_lb_pumps_value);
    manettino_send_pumps_speed();
}

/***
 * AUTO COOLING
 */

void set_cooling_auto_mode() {
    steering_wheel_state_pumps_speed.status      = primary_lv_pumps_speed_status_auto;
    steering_wheel_state_pumps_speed.pumps_speed = 0.0f;

    steering_wheel_state_radiator_speed.status         = primary_lv_radiator_speed_status_auto;
    steering_wheel_state_radiator_speed.radiator_speed = 0.0f;

    manettino_send_pumps_speed();
    manettino_send_radiators_speed();
}

void buttons_pressed_actions(uint8_t button) {
    switch (button) {
        case PADDLE_TOP_RIGHT: {
            if (engineer_mode)
                change_cansniffer_index(true);
            else
                set_ptt_button_pressed(true);
            break;
        }
        case PADDLE_TOP_LEFT: {
            if (engineer_mode)
                change_cansniffer_index(false);
            else
                set_ptt_button_pressed(true);
            break;
        }
        case PADDLE_BOTTOM_RIGHT: {
            steering_change_tab(true);
            break;
        }
        case PADDLE_BOTTOM_LEFT: {
            steering_change_tab(false);
            break;
        }
        case BUTTON_TOP_RIGHT:
            if (engineer_mode) {
            } else {
                switch (current_racing_tab) {
                    case STEERING_WHEEL_TAB_HV:
                        send_bal(true);
                        display_notification("BAL ON", 500, COLOR_SECONDARY_HEX, COLOR_PRIMARY_HEX);
                        break;
                    default:
                        break;
                }
            }
            break;
        case BUTTON_TOP_LEFT:
            if (engineer_mode) {
                // already activated
            } else {
                switch (current_racing_tab) {
                    case STEERING_WHEEL_TAB_RACING:
                        // long pressed // turn_telemetry_on_off();
                        break;
                    case STEERING_WHEEL_TAB_TRACK_TEST:
                        set_dmt_steering_angle_target();
                        break;
                    case STEERING_WHEEL_TAB_SENSORS:
                        break;
                    case STEERING_WHEEL_TAB_HV:
                        send_bal(false);
                        display_notification("BAL OFF", 500, COLOR_SECONDARY_HEX, COLOR_PRIMARY_HEX);
                        break;
                    case STEERING_WHEEL_TAB_LV:
                        break;
                    default:
                        break;
                }
            }
            break;
        case BUTTON_BOTTOM_RIGHT:
            system_reset();
            break;
        case BUTTON_BOTTOM_LEFT: {
            help();
#if 0
                if (current_racing_tab == STEERING_WHEEL_TAB_RACING) {
                    switch (left_manettino_selection) {
                        case LEFT_MANETTINO_SLIP_OPTION: {
                            left_manettino_selection = LEFT_MANETTINO_POWER_MAP_OPTION;
                            break;
                        }
                        case LEFT_MANETTINO_POWER_MAP_OPTION: {
                            left_manettino_selection = LEFT_MANETTINO_TORQUE_OPTION;
                            break;
                        }
                        case LEFT_MANETTINO_TORQUE_OPTION: {
                            left_manettino_selection = LEFT_MANETTINO_SLIP_OPTION;
                            break;
                        }
                        default:
                            break;
                    }
                } else if (current_racing_tab == STEERING_WHEEL_TAB_LV) {
                    set_cooling_auto_mode();
                }
#endif
            break;
        }

        break;
    }
}

void buttons_released_actions(uint8_t button) {
    switch (button) {
        case PADDLE_TOP_RIGHT:
            set_ptt_button_pressed(false);
            break;
        case PADDLE_TOP_LEFT:
            set_ptt_button_pressed(false);
            break;
        case PADDLE_BOTTOM_RIGHT:
            break;
        case PADDLE_BOTTOM_LEFT:
            break;
        case BUTTON_TOP_RIGHT:
            break;
        case BUTTON_TOP_LEFT:
            break;
        case BUTTON_BOTTOM_RIGHT:
            break;
        case BUTTON_BOTTOM_LEFT:
            break;
    }
}

void buttons_long_pressed_actions(uint8_t button) {
    switch (button) {
        case PADDLE_TOP_RIGHT:
            break;
        case PADDLE_TOP_LEFT:
            break;
        case PADDLE_BOTTOM_RIGHT:
            switch_mode();
            break;
        case PADDLE_BOTTOM_LEFT:
            switch_mode();
            break;
        case BUTTON_TOP_RIGHT:
            break;
        case BUTTON_TOP_LEFT: {
            if (engineer_mode) {
            } else {
                turn_telemetry_on_off();
            }
            break;
        }
        case BUTTON_BOTTOM_RIGHT:
            break;
        case BUTTON_BOTTOM_LEFT:
            break;
    }
}

void send_pork_fans_status(float val) {
    primary_hv_fans_status_converted_t hv_fans_override_settings = {.fans_override = primary_hv_fans_status_fans_override_off, .fans_speed = 0.0f};
    if (val < 0.0f) {
        hv_fans_override_settings.fans_override = primary_hv_fans_status_fans_override_off;
        hv_fans_override_settings.fans_speed    = 0.0f;  // ignored
    } else {
        hv_fans_override_settings.fans_override = primary_hv_fans_status_fans_override_on;
        hv_fans_override_settings.fans_speed    = val;  // fmax(0.0f, fmin(1.0f, val));
    }
    primary_hv_set_fans_status_converted_t converted = {0};
    converted.fans_override                          = hv_fans_override_settings.fans_override;
    converted.fans_speed                             = hv_fans_override_settings.fans_speed;
    STEER_CAN_PACK(primary, PRIMARY, hv_set_fans_status, HV_SET_FANS_STATUS);
    can_send(&msg, true);

    if (hv_fans_override_settings.fans_override == primary_hv_fans_status_fans_override_off) {
        snprintf(sprintf_buffer_controls, BUFSIZ, "AUTO SET");
    } else {
        snprintf(sprintf_buffer_controls, BUFSIZ, "%0.1f SET", hv_fans_override_settings.fans_speed);
    }
    set_tab_hv_label_text(sprintf_buffer_controls, tab_hv_pork_speed_value);
}

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
            display_notification("Sending\nIDLE\nanyway", 1500, COLOR_SECONDARY_HEX, COLOR_PRIMARY_HEX);
            break;
        }
        case primary_ecu_status_status_idle: {
            send_set_car_status(primary_ecu_set_status_status_ready);
            display_notification("TSON", 1500, COLOR_SECONDARY_HEX, COLOR_PRIMARY_HEX);
            break;
        }
        case primary_ecu_status_status_start_ts_precharge:
        case primary_ecu_status_status_wait_ts_precharge: {
            display_notification("Precharge\nnot finished\nyet", 1500, COLOR_SECONDARY_HEX, COLOR_PRIMARY_HEX);
            break;
        }
        case primary_ecu_status_status_wait_driver: {
            send_set_car_status(primary_ecu_set_status_status_drive);
            display_notification("DRIVE", 1500, COLOR_SECONDARY_HEX, COLOR_PRIMARY_HEX);
            break;
        }
        case primary_ecu_status_status_enable_inv_drive:
        case primary_ecu_status_status_drive:
        case primary_ecu_status_status_disable_inv_drive:
        case primary_ecu_status_status_start_ts_discharge:
        case primary_ecu_status_status_wait_ts_discharge: {
            send_set_car_status(primary_ecu_set_status_status_idle);
            display_notification("IDLE", 1500, COLOR_SECONDARY_HEX, COLOR_PRIMARY_HEX);
            break;
        }
        case primary_ecu_status_status_fatal_error: {
            send_set_car_status(primary_ecu_set_status_status_idle);
            display_notification("ECU in FATAL ERROR,\nsending IDLE anyway", 1500, COLOR_SECONDARY_HEX, COLOR_PRIMARY_HEX);
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

#if CANSNIFFER_ENABLED == 1

void change_cansniffer_index(bool plus) {
    if (current_engineer_tab == STEERING_WHEEL_TAB_PRIMARY_CANSNIFFER) {
        if (plus) {
            primary_cansniffer_start_index++;
            update_primary_cansniffer_ui();
        } else {
            primary_cansniffer_start_index--;
            primary_cansniffer_start_index = fmax(primary_cansniffer_start_index, 0);
            update_primary_cansniffer_ui();
        }
    } else if (current_engineer_tab == STEERING_WHEEL_TAB_SECONDARY_CANSNIFFER) {
        if (plus) {
            secondary_cansniffer_start_index++;
            update_secondary_cansniffer_ui();
        } else {
            secondary_cansniffer_start_index--;
            secondary_cansniffer_start_index = fmax(secondary_cansniffer_start_index, 0);
            update_secondary_cansniffer_ui();
        }
    } else if (current_engineer_tab == STEERING_WHEEL_TAB_INVERTERS_CANSNIFFER) {
        if (plus) {
            inverters_cansniffer_start_index++;
            update_inverters_cansniffer_ui();
        } else {
            inverters_cansniffer_start_index--;
            inverters_cansniffer_start_index = fmax(inverters_cansniffer_start_index, 0);
            update_inverters_cansniffer_ui();
        }
    }
}
#endif  // CANSNIFFER_ENABLED
