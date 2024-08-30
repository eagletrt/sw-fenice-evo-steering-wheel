#include "steering.h"

#if STEERING_WHEEL_MODE == STEERING_WHEEL_OLIVEC_MODE

shutdown_circuit_component_state_t global_shutdown_status = SC_UNKNOWN;

static char shutdown_labels[SHUTDOWN_COMPONENT_SIZE][21] = {
    "SD START",
    "TSAC LV IN",
    "TSAC LV OUT",
    "TSAC HV",
    "MUSHROOM L R",
    "MUSHROOM COCK",
    "INERTIAL SWITCH",
    "BOTS",
    "INVC LID",
    "HVD",
    "BSPD",
    "INVC MOT",
    "TSMS LV",
    "TSMS HV"};

static shutdown_circuit_component_state_t shutdown_status_lb_array[SHUTDOWN_COMPONENT_SIZE] = {SC_UNKNOWN};

static const char *debug_signal_error_labels[] = {
    "cell low voltage",
    "cell under voltage",
    "cell over voltage",
    "cell high temperature",
    "cell over temperature",
    "over current",
    "can error",
    "internal voltage mismatch",
    "cellboard communication",
    "cellboard internal",
    "connector disconnected",
    "fans disconnected",
    "feedback error",
    "feedback circuitry error",
    "eeprom communication error",
    "eeprom write error"};

shutdown_circuit_indexes_t last_open_shutdown_circuit(void) {
    for (int sdi = 0; sdi < SHUTDOWN_COMPONENT_SIZE; sdi++) {
        if (shutdown_status_lb_array[sdi] == SC_OPEN)
            return sdi;
    }
    return shutdown_circuit_no_element_index;
}

shutdown_circuit_indexes_t last_shutdown_element_unknown(void) {
    for (int sdi = 0; sdi < SHUTDOWN_COMPONENT_SIZE; sdi++) {
        if (shutdown_status_lb_array[sdi] == SC_UNKNOWN)
            return sdi;
    }
    return shutdown_circuit_no_element_index;
}

void update_shutdown_circuit_component(shutdown_circuit_indexes_t idx, bool is_close) {
    swoc_elem_was_updated[swoc_sd] = true;
    shutdown_status_lb_array[idx]  = is_close ? SC_CLOSE : SC_OPEN;
    shutdown_circuit_indexes_t last_opend_index;
    if ((last_opend_index = last_shutdown_element_unknown()) != shutdown_circuit_no_element_index) {
        // devices are missing
        swoc_elem_lb_color[swoc_sd] = OLIVEC_COLOR_WHITE;
        strcpy(swoc_elem_label[swoc_sd], "not avail");
        return;
    }
    if ((last_opend_index = last_open_shutdown_circuit()) != shutdown_circuit_no_element_index) {
        if (global_shutdown_status == SC_CLOSE) {
            // shutdown was opened
        }
        strcpy(swoc_elem_label[swoc_sd], shutdown_labels[last_opend_index]);
        swoc_elem_lb_color[swoc_sd] = OLIVEC_COLOR_BLACK;
        swoc_elem_bg_color[swoc_sd] = OLIVEC_COLOR_RED;
        global_shutdown_status      = SC_OPEN;
        return;
    }
    if (global_shutdown_status != SC_CLOSE) {
        // shutdown was closed
    }
    strcpy(swoc_elem_label[swoc_sd], "OK");
    swoc_elem_lb_color[swoc_sd] = OLIVEC_COLOR_BLACK;
    swoc_elem_bg_color[swoc_sd] = OLIVEC_COLOR_GREEN;
    global_shutdown_status      = SC_CLOSE;
}

void car_status_update(bool valid) {
    GET_LAST_STATE(primary, ecu_status, PRIMARY, ECU_STATUS);
    swoc_elem_was_updated[swoc_lap_time] = 1;

    switch (primary_ecu_status_last_state->status) {
        case primary_ecu_status_status_init:
        case primary_ecu_status_status_enable_inv_updates:
        case primary_ecu_status_status_check_inv_settings: {
            snprintf(swoc_elem_label[swoc_lap_time], SWOC_STRING_LEN, "INIT");
            break;
        }
        case primary_ecu_status_status_idle: {
            snprintf(swoc_elem_label[swoc_lap_time], SWOC_STRING_LEN, "IDLE");
            break;
        }
        case primary_ecu_status_status_start_ts_precharge:
        case primary_ecu_status_status_wait_ts_precharge: {
            snprintf(swoc_elem_label[swoc_lap_time], SWOC_STRING_LEN, "PRCH");
            break;
        }
        case primary_ecu_status_status_wait_driver: {
            snprintf(swoc_elem_label[swoc_lap_time], SWOC_STRING_LEN, "SETUP");
            break;
        }
        case primary_ecu_status_status_enable_inv_drive: {
            snprintf(swoc_elem_label[swoc_lap_time], SWOC_STRING_LEN, "ENINV");
            break;
        }
        case primary_ecu_status_status_re_enable_inverter_drive: {
            snprintf(swoc_elem_label[swoc_lap_time], SWOC_STRING_LEN, "REINV");
            break;
        }
        case primary_ecu_status_status_drive: {
            snprintf(swoc_elem_label[swoc_lap_time], SWOC_STRING_LEN, "DRIVE");
            break;
        }
        case primary_ecu_status_status_disable_inv_drive:
        case primary_ecu_status_status_start_ts_discharge:
        case primary_ecu_status_status_wait_ts_discharge: {
            snprintf(swoc_elem_label[swoc_lap_time], SWOC_STRING_LEN, "TSOFF");
            break;
        }
        case primary_ecu_status_status_fatal_error: {
            snprintf(swoc_elem_label[swoc_lap_time], SWOC_STRING_LEN, "FATAL");
            break;
        }
    }
}

void ecu_power_maps_update(bool valid) {
    GET_LAST_STATE(primary, ecu_power_maps, PRIMARY, ECU_POWER_MAPS);
    GET_LAST_STATE(primary, ecu_control_status, PRIMARY, ECU_CONTROL_STATUS);
    swoc_elem_was_updated[swoc_torque] = 1;
    swoc_elem_was_updated[swoc_regen]  = 1;
    swoc_elem_was_updated[swoc_slip]   = 1;
    snprintf(swoc_elem_label[swoc_torque], SWOC_STRING_LEN, "T");
    snprintf(swoc_elem_label[swoc_regen], SWOC_STRING_LEN, "R");
    snprintf(swoc_elem_label[swoc_slip], SWOC_STRING_LEN, "L");

    if (primary_ecu_power_maps_last_state->reg_state && primary_ecu_control_status_last_state->control_enabled) {
        swoc_elem_bg_color[swoc_regen] = OLIVEC_COLOR_GREEN;
        swoc_elem_lb_color[swoc_regen] = OLIVEC_COLOR_BLACK;
    } else {
        swoc_elem_bg_color[swoc_regen] = OLIVEC_COLOR_BLACK;
        swoc_elem_lb_color[swoc_regen] = OLIVEC_COLOR_WHITE;
    }
    if (primary_ecu_power_maps_last_state->sc_state && primary_ecu_control_status_last_state->control_enabled) {
        swoc_elem_bg_color[swoc_slip] = OLIVEC_COLOR_GREEN;
        swoc_elem_lb_color[swoc_slip] = OLIVEC_COLOR_BLACK;
    } else {
        swoc_elem_bg_color[swoc_slip] = OLIVEC_COLOR_BLACK;
        swoc_elem_lb_color[swoc_slip] = OLIVEC_COLOR_WHITE;
    }
    if (primary_ecu_power_maps_last_state->tv_state && primary_ecu_control_status_last_state->control_enabled) {
        swoc_elem_bg_color[swoc_torque] = OLIVEC_COLOR_GREEN;
        swoc_elem_lb_color[swoc_torque] = OLIVEC_COLOR_BLACK;
    } else {
        swoc_elem_bg_color[swoc_torque] = OLIVEC_COLOR_BLACK;
        swoc_elem_lb_color[swoc_torque] = OLIVEC_COLOR_WHITE;
    }
}

void angular_velocity_update(bool valid) {
}

void vehicle_speed_update(bool valid) {
}

void hv_feedback_misc_voltage_update(bool valid) {
}

void ecu_errors_update(bool valid) {
}

void hv_debug_signals_update(bool valid) {
}

void hv_cells_voltage_stats_update(bool valid) {
}

void hv_total_voltage_update(bool valid) {
}

void hv_current_update(bool valid) {
}

void hv_soc_estimation_update(bool valid) {
    GET_LAST_STATE(secondary, hv_soc_estimation_state, SECONDARY, HV_SOC_ESTIMATION_STATE);
    float soc = secondary_hv_soc_estimation_state_last_state->soc;
    snprintf(swoc_elem_label[swoc_soc_hv], SWOC_STRING_LEN, "%.0f", soc * 100.0f);
    swoc_elem_was_updated[swoc_soc_hv] = true;

    if (soc < SOC_CRITICAL_THRESHOLD) {
        swoc_elem_bg_color[swoc_soc_hv] = OLIVEC_COLOR_RED;
        swoc_elem_lb_color[swoc_soc_hv] = OLIVEC_COLOR_BLACK;
    } else if (soc < SOC_ALERT_THRESHOLD) {
        swoc_elem_bg_color[swoc_soc_hv] = OLIVEC_COLOR_YELLOW;
        swoc_elem_lb_color[swoc_soc_hv] = OLIVEC_COLOR_BLACK;
    } else {
        swoc_elem_bg_color[swoc_soc_hv] = OLIVEC_COLOR_GREEN;
        swoc_elem_lb_color[swoc_soc_hv] = OLIVEC_COLOR_BLACK;
    }
}

void lv_soc_estimation_update(bool valid) {
    GET_LAST_STATE(secondary, lv_soc_estimation_state, SECONDARY, LV_SOC_ESTIMATION_STATE);
    float soc                               = secondary_lv_soc_estimation_state_last_state->soc;
    swoc_elem_was_updated[swoc_soc_lv]      = true;
    swoc_elem_was_updated[swoc_soc_lv_name] = true;
    snprintf(swoc_elem_label[swoc_soc_lv], SWOC_STRING_LEN, "%.0f", soc * 100.0f);

    if (soc < SOC_CRITICAL_THRESHOLD) {
        swoc_elem_bg_color[swoc_soc_lv] = OLIVEC_COLOR_RED;
        swoc_elem_lb_color[swoc_soc_lv] = OLIVEC_COLOR_BLACK;
    } else if (soc < SOC_ALERT_THRESHOLD) {
        swoc_elem_bg_color[swoc_soc_lv] = OLIVEC_COLOR_YELLOW;
        swoc_elem_lb_color[swoc_soc_lv] = OLIVEC_COLOR_BLACK;
    } else {
        swoc_elem_bg_color[swoc_soc_lv] = OLIVEC_COLOR_GREEN;
        swoc_elem_lb_color[swoc_soc_lv] = OLIVEC_COLOR_BLACK;
    }
    swoc_elem_bg_color[swoc_soc_lv_name] = swoc_elem_bg_color[swoc_soc_lv];
    swoc_elem_lb_color[swoc_soc_lv_name] = swoc_elem_lb_color[swoc_soc_lv];
}

void hv_cells_temp_stats_update(bool valid) {
    GET_LAST_STATE(primary, hv_cells_temp_stats, PRIMARY, HV_CELLS_TEMP_STATS);
    float tmax = primary_hv_cells_temp_stats_last_state->max;
    snprintf(swoc_elem_label[swoc_temp_hv], SWOC_STRING_LEN, "%.0f", tmax);
    swoc_elem_was_updated[swoc_temp_hv]      = true;
    swoc_elem_was_updated[swoc_temp_hv_name] = true;

    if (tmax > HV_TEMP_CRITICAL_THRESHOLD) {
        swoc_elem_bg_color[swoc_temp_hv] = OLIVEC_COLOR_RED;
        swoc_elem_lb_color[swoc_temp_hv] = OLIVEC_COLOR_BLACK;
    } else if (tmax > HV_TEMP_ALERT_THRESHOLD) {
        swoc_elem_bg_color[swoc_temp_hv] = OLIVEC_COLOR_YELLOW;
        swoc_elem_lb_color[swoc_temp_hv] = OLIVEC_COLOR_BLACK;
    } else {
        swoc_elem_bg_color[swoc_temp_hv] = OLIVEC_COLOR_GREEN;
        swoc_elem_lb_color[swoc_temp_hv] = OLIVEC_COLOR_BLACK;
    }
    swoc_elem_bg_color[swoc_temp_hv_name] = swoc_elem_bg_color[swoc_temp_hv];
    swoc_elem_lb_color[swoc_temp_hv_name] = swoc_elem_lb_color[swoc_temp_hv];
}

void hv_errors_update(bool valid) {
}

void hv_cell_balancing_status_update(bool valid) {
}

void hv_feedbacks_status_update(bool valid) {
}

void debug_hv_feedbacks_status_update(bool valid) {
}

void das_errors_update(bool valid) {
}

void hv_feedback_ts_voltage_update(bool valid) {
    // GET_LAST_STATE(primary, hv_feedback_ts_voltage, PRIMARY, HV_FEEDBACK_TS_VOLTAGE);
    // update_shutdown_circuit_component(shutdown_circuit_feedbacks_status_feedback_precharge_status_index, true);
    // update_shutdown_circuit_component(shutdown_circuit_feedbacks_status_feedback_airp_gate_index, primary_hv_feedback_ts_voltage_last_state->airp_gate > 2.5f);
    // update_shutdown_circuit_component(shutdown_circuit_feedbacks_status_feedback_airn_gate_index, primary_hv_feedback_ts_voltage_last_state->airn_gate > 2.5f);
}

void hv_feedback_sd_voltage_update(bool valid) {
    GET_LAST_STATE(primary, hv_feedback_sd_voltage, PRIMARY, HV_FEEDBACK_SD_VOLTAGE);
    update_shutdown_circuit_component(shutdown_circuit_feedbacks_status_feedback_sd_in_index, primary_hv_feedback_sd_voltage_last_state->sd_in > 1.5f);
    update_shutdown_circuit_component(shutdown_circuit_feedbacks_status_feedback_sd_end_index, primary_hv_feedback_sd_voltage_last_state->sd_end > 1.5f);
    update_shutdown_circuit_component(shutdown_circuit_feedbacks_status_feedback_sd_out_index, primary_hv_feedback_sd_voltage_last_state->sd_out > 1.5f);
}

void ecu_feedbacks_update(bool valid) {
    GET_LAST_STATE(primary, ecu_feedbacks, PRIMARY, ECU_FEEDBACKS);
    update_shutdown_circuit_component(shutdown_circuit_ecu_feedbacks_sd_in_index, primary_ecu_feedbacks_last_state->feedbacks_sd_in);
    update_shutdown_circuit_component(shutdown_circuit_ecu_feedbacks_sd_cock_fb_index, primary_ecu_feedbacks_last_state->feedbacks_sd_cock_fb);
    update_shutdown_circuit_component(shutdown_circuit_ecu_feedbacks_sd_inertial_fb_index, primary_ecu_feedbacks_last_state->feedbacks_sd_interial_fb);
    update_shutdown_circuit_component(shutdown_circuit_ecu_feedbacks_sd_bots_fb_index, primary_ecu_feedbacks_last_state->feedbacks_sd_bots_fb);
}

void lv_feedback_sd_voltage_update(bool valid) {
    GET_LAST_STATE(primary, lv_feedback_sd_voltage, PRIMARY, LV_FEEDBACK_SD_VOLTAGE);
    update_shutdown_circuit_component(shutdown_circuit_sd_start_index, primary_lv_feedback_sd_voltage_last_state->sd_start > 5.0f);
    update_shutdown_circuit_component(shutdown_circuit_feedbacks_interlock_fb_index, primary_lv_feedback_sd_voltage_last_state->interlock > 5.0f);
    update_shutdown_circuit_component(shutdown_circuit_sd_end_index, primary_lv_feedback_sd_voltage_last_state->sd_end > 5.0f);
}

void lv_feedback_ts_voltage_update(bool valid) {
    GET_LAST_STATE(primary, lv_feedback_ts_voltage, PRIMARY, LV_FEEDBACK_TS_VOLTAGE);
    update_shutdown_circuit_component(shutdown_circuit_feedbacks_hvd_fb_index, primary_lv_feedback_ts_voltage_last_state->bspd > 5.0f);
    update_shutdown_circuit_component(shutdown_circuit_feedbacks_bspd_fb_index, primary_lv_feedback_ts_voltage_last_state->bspd > 5.0f);
    update_shutdown_circuit_component(shutdown_circuit_feedbacks_invc_interlock_fb_index, primary_lv_feedback_ts_voltage_last_state->invc_interlock > 5.0f);
}

void lv_feedback_enclosure_voltage_update(bool valid) {
    // GET_LAST_STATE(primary, lv_feedback_enclosure_voltage, PRIMARY, LV_FEEDBACK_ENCLOSURE_VOLTAGE);
    update_shutdown_circuit_component(shutdown_circuit_feedbacks_invc_lid_fb_index, true);  // not available
}

void lv_errors_update(bool valid) {
}

void lv_currents_update(bool valid) {
}

void lv_total_voltage_update(bool valid) {
}

void lv_cells_voltage_update(bool valid) {
}

void lv_cells_voltage_stats_update(bool valid) {
}

void lv_cells_temp_update(bool valid) {
}

void lv_cells_temp_stats_update(bool valid) {
}

void hv_status_update(bool valid) {
}

void hv_fans_override_status_update(bool valid) {
    GET_LAST_STATE(primary, hv_fans_status, PRIMARY, HV_FANS_STATUS);

    swoc_elem_was_updated[swoc_hv_cooling]      = 1;
    swoc_elem_was_updated[swoc_hv_cooling_name] = 1;
    if (primary_hv_fans_status_last_state->fans_override) {
        snprintf(swoc_elem_label[swoc_hv_cooling], SWOC_STRING_LEN, "A");
        swoc_elem_bg_color[swoc_hv_cooling] = OLIVEC_COLOR_GREEN;
        swoc_elem_lb_color[swoc_hv_cooling] = OLIVEC_COLOR_BLACK;
    } else {
        snprintf(swoc_elem_label[swoc_hv_cooling], SWOC_STRING_LEN, "%.0f", primary_hv_fans_status_last_state->fans_speed * 10.0f);
        swoc_elem_bg_color[swoc_hv_cooling] = OLIVEC_COLOR_BLACK;
        swoc_elem_lb_color[swoc_hv_cooling] = OLIVEC_COLOR_WHITE;
    }
    swoc_elem_bg_color[swoc_hv_cooling_name] = swoc_elem_bg_color[swoc_hv_cooling];
    swoc_elem_lb_color[swoc_hv_cooling_name] = swoc_elem_lb_color[swoc_hv_cooling];
}

void tlm_status_update(bool valid) {
}

void odometer_update(bool valid) {
}

void steer_angle_update(bool valid) {
}

void tlm_network_interface_update(bool valid) {
}

void imu_acceleration_update(bool valid) {
}

void tlm_lap_time_update(bool valid) {
}

void tlm_laps_stats_update(bool valid) {
}

#define INVERTER_MESSAGE_UNINITIALIZED     (-100.0f)
#define INVERTER_TEMP_CONVERSION(raw_temp) (-43.23745 + 0.01073427 * raw_temp - 5.523417e-7 * pow(raw_temp, 2) + 1.330787e-11 * pow(raw_temp, 3));
float l_motor_temp = INVERTER_MESSAGE_UNINITIALIZED;
float l_igbt_temp  = INVERTER_MESSAGE_UNINITIALIZED;
float r_motor_temp = INVERTER_MESSAGE_UNINITIALIZED;
float r_igbt_temp  = INVERTER_MESSAGE_UNINITIALIZED;

#include "inverter_conversions.h"

void inv_l_rcv_update(bool valid) {
    GET_LAST_STATE(inverters, inv_l_rcv, INVERTERS, INV_L_RCV);

    l_motor_temp          = convert_t_motor(inverters_inv_l_rcv_last_state->t_motor);
    float motor_temp_mean = (l_motor_temp + r_motor_temp) / 2.0f;
    if (r_motor_temp != INVERTER_MESSAGE_UNINITIALIZED) {
        swoc_elem_was_updated[swoc_temp_mot]      = 1;
        swoc_elem_was_updated[swoc_temp_mot_name] = 1;
        snprintf(swoc_elem_label[swoc_temp_mot], SWOC_STRING_LEN, "%.0f", motor_temp_mean);

        if (motor_temp_mean > MOTOR_TEMP_CRITICAL_THRESHOLD) {
            swoc_elem_lb_color[swoc_temp_mot] = OLIVEC_COLOR_BLACK;
            swoc_elem_bg_color[swoc_temp_mot] = OLIVEC_COLOR_RED;
        } else if (motor_temp_mean > MOTOR_TEMP_ALERT_THRESHOLD) {
            swoc_elem_lb_color[swoc_temp_mot] = OLIVEC_COLOR_BLACK;
            swoc_elem_bg_color[swoc_temp_mot] = OLIVEC_COLOR_YELLOW;
        } else {
            swoc_elem_lb_color[swoc_temp_mot] = OLIVEC_COLOR_BLACK;
            swoc_elem_bg_color[swoc_temp_mot] = OLIVEC_COLOR_GREEN;
        }
        swoc_elem_lb_color[swoc_temp_mot_name] = swoc_elem_lb_color[swoc_temp_mot];
        swoc_elem_bg_color[swoc_temp_mot_name] = swoc_elem_bg_color[swoc_temp_mot];
    }

    l_igbt_temp          = convert_t_igbt(inverters_inv_l_rcv_last_state->t_igbt);
    float igbt_temp_mean = (l_igbt_temp + r_igbt_temp) / 2.0f;
    if (r_igbt_temp != INVERTER_MESSAGE_UNINITIALIZED) {
        swoc_elem_was_updated[swoc_temp_inv]      = 1;
        swoc_elem_was_updated[swoc_temp_inv_name] = 1;
        snprintf(swoc_elem_label[swoc_temp_inv], SWOC_STRING_LEN, "%.0f", igbt_temp_mean);

        if (motor_temp_mean > INVERTER_TEMP_CRITICAL_THRESHOLD) {
            swoc_elem_lb_color[swoc_temp_inv] = OLIVEC_COLOR_BLACK;
            swoc_elem_bg_color[swoc_temp_inv] = OLIVEC_COLOR_RED;
        } else if (motor_temp_mean > INVERTER_TEMP_ALERT_THRESHOLD) {
            swoc_elem_lb_color[swoc_temp_inv] = OLIVEC_COLOR_BLACK;
            swoc_elem_bg_color[swoc_temp_inv] = OLIVEC_COLOR_YELLOW;
        } else {
            swoc_elem_lb_color[swoc_temp_inv] = OLIVEC_COLOR_BLACK;
            swoc_elem_bg_color[swoc_temp_inv] = OLIVEC_COLOR_GREEN;
        }
        swoc_elem_lb_color[swoc_temp_inv_name] = swoc_elem_lb_color[swoc_temp_inv];
        swoc_elem_bg_color[swoc_temp_inv_name] = swoc_elem_bg_color[swoc_temp_inv];
    }
}

void inv_r_rcv_update(bool valid) {
    GET_LAST_STATE(inverters, inv_r_rcv, INVERTERS, INV_R_RCV);
    r_motor_temp = convert_t_motor(inverters_inv_r_rcv_last_state->t_motor);
    r_igbt_temp  = convert_t_igbt(inverters_inv_r_rcv_last_state->t_igbt);
}

void irts_fl_update(bool valid) {
}

void irts_fr_update(bool valid) {
}

void irts_rl_update(bool valid) {
}

void irts_rr_update(bool valid) {
}

void pedal_throttle_update(bool valid) {
}

void pedal_brakes_pressure_update(bool valid) {
}

void ecu_version_update(bool valid) {
}

void lv_version_update(bool valid) {
}

void hv_cellboard_version_update(bool valid) {
}

void hv_mainboard_version_update(bool valid) {
}

void tlm_version_update(bool valid) {
}

void control_status_update(bool valid) {
}

void lv_radiator_speed_update(bool valid) {
}

void lv_pumps_speed_update(bool valid) {
    GET_LAST_STATE(primary, lv_pumps_speed, PRIMARY, LV_PUMPS_SPEED);

    swoc_elem_was_updated[swoc_pt_cooling]      = 1;
    swoc_elem_was_updated[swoc_pt_cooling_name] = 1;
    if (primary_lv_pumps_speed_last_state->status == primary_lv_pumps_speed_status_auto) {
        snprintf(swoc_elem_label[swoc_pt_cooling], SWOC_STRING_LEN, "A");
        swoc_elem_bg_color[swoc_pt_cooling] = OLIVEC_COLOR_GREEN;
        swoc_elem_lb_color[swoc_pt_cooling] = OLIVEC_COLOR_BLACK;
    } else {
        snprintf(swoc_elem_label[swoc_pt_cooling], SWOC_STRING_LEN, "%.0f", primary_lv_pumps_speed_last_state->pumps_speed * 10.0f);
        swoc_elem_bg_color[swoc_pt_cooling] = OLIVEC_COLOR_BLACK;
        swoc_elem_lb_color[swoc_pt_cooling] = OLIVEC_COLOR_WHITE;
    }
    swoc_elem_bg_color[swoc_pt_cooling_name] = swoc_elem_bg_color[swoc_pt_cooling];
    swoc_elem_lb_color[swoc_pt_cooling_name] = swoc_elem_lb_color[swoc_pt_cooling];
}

void lv_cooling_aggressiveness_update(bool valid) {
}

void ptt_periodic_check(void *timer) {
    swoc_elem_was_updated[swoc_ptt] = 1;
    if (!ecu_ack && ptt_button_pressed) {
        ptt_status = ptt_status_SET_ON;
        send_ptt_status_message(true);
        snprintf(swoc_elem_label[swoc_ptt], SWOC_STRING_LEN, "MUTE");
        swoc_elem_bg_color[swoc_ptt] = OLIVEC_COLOR_BLACK;
        swoc_elem_lb_color[swoc_ptt] = OLIVEC_COLOR_WHITE;
    } else if (ecu_ack && !ptt_button_pressed) {
        ptt_status = ptt_status_SET_OFF;
        send_ptt_status_message(false);
        snprintf(swoc_elem_label[swoc_ptt], SWOC_STRING_LEN, "MUTE");
        swoc_elem_bg_color[swoc_ptt] = OLIVEC_COLOR_BLACK;
        swoc_elem_lb_color[swoc_ptt] = OLIVEC_COLOR_WHITE;
    } else if (ecu_ack && ptt_button_pressed) {
        ptt_status = ptt_status_ON;
        send_ptt_status_message(true);
        snprintf(swoc_elem_label[swoc_ptt], SWOC_STRING_LEN, "TALK");
        swoc_elem_bg_color[swoc_ptt] = OLIVEC_COLOR_BLUE;
        swoc_elem_lb_color[swoc_ptt] = OLIVEC_COLOR_BLACK;
    } else if (!ecu_ack && !ptt_button_pressed) {
        ptt_status = ptt_status_OFF;
        send_ptt_status_message(false);
        snprintf(swoc_elem_label[swoc_ptt], SWOC_STRING_LEN, "MUTE");
        swoc_elem_bg_color[swoc_ptt] = OLIVEC_COLOR_BLACK;
        swoc_elem_lb_color[swoc_ptt] = OLIVEC_COLOR_WHITE;
    }
}

#endif  // #if STEERING_WHEEL_MODE == STEERING_WHEEL_OLIVEC_MODE
