/***
 * TODO: desburing Varano
 * ptt
 * riquadri
 * allineamenti (pulire tipo MOT etc)
 * 
 */

#include "steering.h"

uint8_t primary_messages_last_state[primary_MESSAGE_COUNT][primary_MAX_STRUCT_SIZE_CONVERSION];
uint8_t secondary_messages_last_state[secondary_MESSAGE_COUNT][secondary_MAX_STRUCT_SIZE_CONVERSION];
uint8_t inverters_messages_last_state[inverters_MESSAGE_COUNT][inverters_MAX_STRUCT_SIZE_CONVERSION];

bool is_pmsg_valid[primary_MESSAGE_COUNT];
bool is_smsg_valid[secondary_MESSAGE_COUNT];
bool is_imsg_valid[inverters_MESSAGE_COUNT];

bool is_pmsg_new[primary_MESSAGE_COUNT];
bool is_smsg_new[secondary_MESSAGE_COUNT];
bool is_imsg_new[inverters_MESSAGE_COUNT];

bool ptt_button_pressed         = false;
ptt_status_t current_ptt_status = ptt_status_OFF;

void set_ptt_button_pressed(bool val) {
    ptt_button_pressed = val;
}

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

/* static const char *debug_signal_error_labels[] = {
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
    "eeprom write error"}; */

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

void update_shutdown_circuit_component(UI_t *screen, shutdown_circuit_indexes_t idx, bool is_close) {
    screen->components[swoc_sd].swoc_elem_was_updated = true;
    shutdown_status_lb_array[idx]                     = is_close ? SC_CLOSE : SC_OPEN;
    shutdown_circuit_indexes_t last_opend_index;
    if ((last_opend_index = last_shutdown_element_unknown()) != shutdown_circuit_no_element_index) {
        // devices are missing
        screen->components[swoc_sd].swoc_elem_lb_color = OLIVEC_COLOR_WHITE;
        strcpy(screen->components[swoc_sd].swoc_elem_label, "not avail");
        return;
    }
    if ((last_opend_index = last_open_shutdown_circuit()) != shutdown_circuit_no_element_index) {
        if (global_shutdown_status == SC_CLOSE) {
            // shutdown was opened
        }
        strcpy(screen->components[swoc_sd].swoc_elem_label, shutdown_labels[last_opend_index]);
        if (strlen(shutdown_labels[last_opend_index]) > 10) {
            screen->components[swoc_sd].swoc_elem_font_size = 0.2;
        } else {
            screen->components[swoc_sd].swoc_elem_font_size = 0.5;
        }
        screen->components[swoc_sd].swoc_elem_lb_color = OLIVEC_COLOR_BLACK;
        screen->components[swoc_sd].swoc_elem_bg_color = OLIVEC_COLOR_RED;
        global_shutdown_status                         = SC_OPEN;
        return;
    }
    if (global_shutdown_status != SC_CLOSE) {
        // shutdown was closed
    }
    strcpy(screen->components[swoc_sd].swoc_elem_label, "OK");
    screen->components[swoc_sd].swoc_elem_font_size = 0.5;
    screen->components[swoc_sd].swoc_elem_lb_color  = OLIVEC_COLOR_BLACK;
    screen->components[swoc_sd].swoc_elem_bg_color  = OLIVEC_COLOR_GREEN;
    global_shutdown_status                          = SC_CLOSE;
}

void ecu_status_update(UI_t *screen, bool valid) {
    GET_LAST_STATE(primary, ecu_status, PRIMARY, ECU_STATUS);
    if (primary_ecu_status_last_state->status == primary_ecu_status_status_drive) {
        return;
    }

    screen->components[swoc_lap_time].swoc_elem_was_updated = 1;

    switch (primary_ecu_status_last_state->status) {
        case primary_ecu_status_status_init:
        case primary_ecu_status_status_enable_inv_updates:
        case primary_ecu_status_status_check_inv_settings: {
            snprintf(screen->components[swoc_lap_time].swoc_elem_label, SWOC_STRING_LEN, "INIT");
            break;
        }
        case primary_ecu_status_status_idle: {
            snprintf(screen->components[swoc_lap_time].swoc_elem_label, SWOC_STRING_LEN, "IDLE");
            break;
        }
        case primary_ecu_status_status_start_ts_precharge:
        case primary_ecu_status_status_wait_ts_precharge: {
            snprintf(screen->components[swoc_lap_time].swoc_elem_label, SWOC_STRING_LEN, "PRCH");
            break;
        }
        case primary_ecu_status_status_wait_driver: {
            snprintf(screen->components[swoc_lap_time].swoc_elem_label, SWOC_STRING_LEN, "SETUP");
            break;
        }
        case primary_ecu_status_status_enable_inv_drive: {
            snprintf(screen->components[swoc_lap_time].swoc_elem_label, SWOC_STRING_LEN, "ENINV");
            break;
        }
        case primary_ecu_status_status_re_enable_inverter_drive: {
            snprintf(screen->components[swoc_lap_time].swoc_elem_label, SWOC_STRING_LEN, "REINV");
            break;
        }
        case primary_ecu_status_status_drive: {
            snprintf(screen->components[swoc_lap_time].swoc_elem_label, SWOC_STRING_LEN, "DRIVE");
            break;
        }
        case primary_ecu_status_status_disable_inv_drive:
        case primary_ecu_status_status_start_ts_discharge:
        case primary_ecu_status_status_wait_ts_discharge: {
            snprintf(screen->components[swoc_lap_time].swoc_elem_label, SWOC_STRING_LEN, "TSOFF");
            break;
        }
        case primary_ecu_status_status_fatal_error: {
            snprintf(screen->components[swoc_lap_time].swoc_elem_label, SWOC_STRING_LEN, "FATAL");
            break;
        }
    }
}

void ecu_power_maps_update(UI_t *screen, bool valid) {
    GET_LAST_STATE(primary, ecu_power_maps, PRIMARY, ECU_POWER_MAPS);
    /* GET_LAST_STATE(primary, ecu_control_status, PRIMARY, ECU_CONTROL_STATUS); */
    screen->components[swoc_torque].swoc_elem_was_updated = 1;
    screen->components[swoc_regen].swoc_elem_was_updated  = 1;
    screen->components[swoc_slip].swoc_elem_was_updated   = 1;
    snprintf(screen->components[swoc_torque].swoc_elem_label, SWOC_STRING_LEN, "T");
    snprintf(screen->components[swoc_regen].swoc_elem_label, SWOC_STRING_LEN, "R");
    snprintf(screen->components[swoc_slip].swoc_elem_label, SWOC_STRING_LEN, "S");

    if (primary_ecu_power_maps_last_state->reg_state /* && primary_ecu_control_status_last_state->control_enabled */) {
        screen->components[swoc_regen].swoc_elem_bg_color = OLIVEC_COLOR_GREEN;
        screen->components[swoc_regen].swoc_elem_lb_color = OLIVEC_COLOR_BLACK;
    } else {
        screen->components[swoc_regen].swoc_elem_bg_color = OLIVEC_COLOR_BLACK;
        screen->components[swoc_regen].swoc_elem_lb_color = OLIVEC_COLOR_WHITE;
    }
    if (primary_ecu_power_maps_last_state->sc_state /* && primary_ecu_control_status_last_state->control_enabled */) {
        screen->components[swoc_slip].swoc_elem_bg_color = OLIVEC_COLOR_GREEN;
        screen->components[swoc_slip].swoc_elem_lb_color = OLIVEC_COLOR_BLACK;
    } else {
        screen->components[swoc_slip].swoc_elem_bg_color = OLIVEC_COLOR_BLACK;
        screen->components[swoc_slip].swoc_elem_lb_color = OLIVEC_COLOR_WHITE;
    }
    if (primary_ecu_power_maps_last_state->tv_state /* && primary_ecu_control_status_last_state->control_enabled */) {
        screen->components[swoc_torque].swoc_elem_bg_color = OLIVEC_COLOR_GREEN;
        screen->components[swoc_torque].swoc_elem_lb_color = OLIVEC_COLOR_BLACK;
    } else {
        screen->components[swoc_torque].swoc_elem_bg_color = OLIVEC_COLOR_BLACK;
        screen->components[swoc_torque].swoc_elem_lb_color = OLIVEC_COLOR_WHITE;
    }
}

void angular_velocity_update(UI_t *screen, bool valid) {
}

void vehicle_speed_update(UI_t *screen, bool valid) {
}

void hv_feedback_misc_voltage_update(UI_t *screen, bool valid) {
}

void ecu_errors_update(UI_t *screen, bool valid) {
}

void hv_debug_signals_update(UI_t *screen, bool valid) {
}

void hv_cells_voltage_stats_update(UI_t *screen, bool valid) {
}

void hv_ts_voltage_update(UI_t *screen, bool valid) {
}

void hv_current_update(UI_t *screen, bool valid) {
}

void hv_soc_estimation_update(UI_t *screen, bool valid) {
    GET_LAST_STATE(secondary, hv_soc_estimation_state, SECONDARY, HV_SOC_ESTIMATION_STATE);
    float soc = secondary_hv_soc_estimation_state_last_state->soc;
    snprintf(screen->components[swoc_soc_hv].swoc_elem_label, SWOC_STRING_LEN, "%.0f", soc * 100.0f);
    screen->components[swoc_soc_hv].swoc_elem_was_updated = true;

    if (soc < SOC_HV_CRITICAL_THRESHOLD) {
        screen->components[swoc_soc_hv].swoc_elem_bg_color      = OLIVEC_COLOR_RED;
        screen->components[swoc_soc_hv].swoc_elem_lb_color      = OLIVEC_COLOR_BLACK;
        screen->components[swoc_soc_hv_name].swoc_elem_bg_color = OLIVEC_COLOR_RED;
        screen->components[swoc_soc_hv_name].swoc_elem_lb_color = OLIVEC_COLOR_BLACK;
    } else if (soc < SOC_HV_ALERT_THRESHOLD) {
        screen->components[swoc_soc_hv].swoc_elem_bg_color      = OLIVEC_COLOR_YELLOW;
        screen->components[swoc_soc_hv].swoc_elem_lb_color      = OLIVEC_COLOR_BLACK;
        screen->components[swoc_soc_hv_name].swoc_elem_bg_color = OLIVEC_COLOR_YELLOW;
        screen->components[swoc_soc_hv_name].swoc_elem_lb_color = OLIVEC_COLOR_BLACK;
    } else {
        screen->components[swoc_soc_hv].swoc_elem_bg_color      = OLIVEC_COLOR_GREEN;
        screen->components[swoc_soc_hv].swoc_elem_lb_color      = OLIVEC_COLOR_BLACK;
        screen->components[swoc_soc_hv_name].swoc_elem_bg_color = OLIVEC_COLOR_GREEN;
        screen->components[swoc_soc_hv_name].swoc_elem_lb_color = OLIVEC_COLOR_BLACK;
    }
}

void lv_soc_estimation_update(UI_t *screen, bool valid) {
    GET_LAST_STATE(secondary, lv_soc_estimation_state, SECONDARY, LV_SOC_ESTIMATION_STATE);
    float soc                                                  = secondary_lv_soc_estimation_state_last_state->soc;
    screen->components[swoc_soc_lv].swoc_elem_was_updated      = true;
    screen->components[swoc_soc_lv_name].swoc_elem_was_updated = true;
    snprintf(screen->components[swoc_soc_lv].swoc_elem_label, SWOC_STRING_LEN, "%.0f", soc * 100.0f);

    if (soc < SOC_LV_CRITICAL_THRESHOLD) {
        screen->components[swoc_soc_lv].swoc_elem_bg_color = OLIVEC_COLOR_RED;
        screen->components[swoc_soc_lv].swoc_elem_lb_color = OLIVEC_COLOR_BLACK;
    } else if (soc < SOC_LV_ALERT_THRESHOLD) {
        screen->components[swoc_soc_lv].swoc_elem_bg_color = OLIVEC_COLOR_YELLOW;
        screen->components[swoc_soc_lv].swoc_elem_lb_color = OLIVEC_COLOR_BLACK;
    } else {
        screen->components[swoc_soc_lv].swoc_elem_bg_color = OLIVEC_COLOR_GREEN;
        screen->components[swoc_soc_lv].swoc_elem_lb_color = OLIVEC_COLOR_BLACK;
    }
    screen->components[swoc_soc_lv_name].swoc_elem_bg_color = screen->components[swoc_soc_lv].swoc_elem_bg_color;
    screen->components[swoc_soc_lv_name].swoc_elem_lb_color = screen->components[swoc_soc_lv].swoc_elem_lb_color;
}

void hv_cells_temp_stats_update(UI_t *screen, bool valid) {
    GET_LAST_STATE(primary, hv_cells_temp_stats, PRIMARY, HV_CELLS_TEMP_STATS);
    float tmax = primary_hv_cells_temp_stats_last_state->max;
    snprintf(screen->components[swoc_temp_hv].swoc_elem_label, SWOC_STRING_LEN, "%.0f", tmax);
    screen->components[swoc_temp_hv].swoc_elem_was_updated      = true;
    screen->components[swoc_temp_hv_name].swoc_elem_was_updated = true;

    if (tmax > HV_TEMP_CRITICAL_THRESHOLD) {
        screen->components[swoc_temp_hv].swoc_elem_bg_color = OLIVEC_COLOR_RED;
        screen->components[swoc_temp_hv].swoc_elem_lb_color = OLIVEC_COLOR_BLACK;
    } else if (tmax > HV_TEMP_ALERT_THRESHOLD) {
        screen->components[swoc_temp_hv].swoc_elem_bg_color = OLIVEC_COLOR_YELLOW;
        screen->components[swoc_temp_hv].swoc_elem_lb_color = OLIVEC_COLOR_BLACK;
    } else {
        screen->components[swoc_temp_hv].swoc_elem_bg_color = OLIVEC_COLOR_GREEN;
        screen->components[swoc_temp_hv].swoc_elem_lb_color = OLIVEC_COLOR_BLACK;
    }
    screen->components[swoc_temp_hv_name].swoc_elem_bg_color = screen->components[swoc_temp_hv].swoc_elem_bg_color;
    screen->components[swoc_temp_hv_name].swoc_elem_lb_color = screen->components[swoc_temp_hv].swoc_elem_lb_color;
}

void hv_errors_update(UI_t *screen, bool valid) {
}

void hv_cell_balancing_status_update(UI_t *screen, bool valid) {
}

void hv_feedbacks_status_update(UI_t *screen, bool valid) {
}

void debug_hv_feedbacks_status_update(UI_t *screen, bool valid) {
}

void das_errors_update(UI_t *screen, bool valid) {
}

void hv_feedback_ts_voltage_update(UI_t *screen, bool valid) {
    // GET_LAST_STATE(primary, hv_feedback_ts_voltage, PRIMARY, HV_FEEDBACK_TS_VOLTAGE);
    // update_shutdown_circuit_component(screen, shutdown_circuit_feedbacks_status_feedback_precharge_status_index, true);
    // update_shutdown_circuit_component(screen, shutdown_circuit_feedbacks_status_feedback_airp_gate_index, primary_hv_feedback_ts_voltage_last_state->airp_gate > 2.5f);
    // update_shutdown_circuit_component(screen, shutdown_circuit_feedbacks_status_feedback_airn_gate_index, primary_hv_feedback_ts_voltage_last_state->airn_gate > 2.5f);
}

void hv_feedback_analog_sd_update(UI_t *screen, bool valid) {
    GET_LAST_STATE(primary, hv_feedback_analog_sd, PRIMARY, HV_FEEDBACK_ANALOG_SD);
    update_shutdown_circuit_component(screen, shutdown_circuit_feedbacks_status_feedback_sd_in_index, primary_hv_feedback_analog_sd_last_state->sd_in > 1.5f);
    update_shutdown_circuit_component(screen, shutdown_circuit_feedbacks_status_feedback_sd_end_index, primary_hv_feedback_analog_sd_last_state->sd_end > 1.5f);
    update_shutdown_circuit_component(screen, shutdown_circuit_feedbacks_status_feedback_sd_out_index, primary_hv_feedback_analog_sd_last_state->sd_out > 1.5f);
}

void ecu_feedbacks_update(UI_t *screen, bool valid) {
    GET_LAST_STATE(primary, ecu_feedbacks, PRIMARY, ECU_FEEDBACKS);
    update_shutdown_circuit_component(screen, shutdown_circuit_ecu_feedbacks_sd_in_index, primary_ecu_feedbacks_last_state->feedbacks_sd_in);
    update_shutdown_circuit_component(screen, shutdown_circuit_ecu_feedbacks_sd_cock_fb_index, primary_ecu_feedbacks_last_state->feedbacks_sd_cock_fb);
    update_shutdown_circuit_component(screen, shutdown_circuit_ecu_feedbacks_sd_inertial_fb_index, primary_ecu_feedbacks_last_state->feedbacks_sd_interial_fb);
    update_shutdown_circuit_component(screen, shutdown_circuit_ecu_feedbacks_sd_bots_fb_index, primary_ecu_feedbacks_last_state->feedbacks_sd_bots_fb);
}

void lv_feedback_sd_voltage_update(UI_t *screen, bool valid) {
    GET_LAST_STATE(primary, lv_feedback_sd_voltage, PRIMARY, LV_FEEDBACK_SD_VOLTAGE);
    update_shutdown_circuit_component(screen, shutdown_circuit_sd_start_index, primary_lv_feedback_sd_voltage_last_state->sd_start > 5.0f);
    update_shutdown_circuit_component(screen, shutdown_circuit_feedbacks_interlock_fb_index, primary_lv_feedback_sd_voltage_last_state->interlock > 5.0f);
    update_shutdown_circuit_component(screen, shutdown_circuit_sd_end_index, primary_lv_feedback_sd_voltage_last_state->sd_end > 5.0f);
}

void lv_feedback_ts_voltage_update(UI_t *screen, bool valid) {
    GET_LAST_STATE(primary, lv_feedback_ts_voltage, PRIMARY, LV_FEEDBACK_TS_VOLTAGE);
    update_shutdown_circuit_component(screen, shutdown_circuit_feedbacks_hvd_fb_index, primary_lv_feedback_ts_voltage_last_state->bspd > 5.0f);
    update_shutdown_circuit_component(screen, shutdown_circuit_feedbacks_bspd_fb_index, primary_lv_feedback_ts_voltage_last_state->bspd > 5.0f);
    update_shutdown_circuit_component(
        screen, shutdown_circuit_feedbacks_invc_interlock_fb_index, primary_lv_feedback_ts_voltage_last_state->invc_interlock > 5.0f);
}

void lv_feedback_enclosure_voltage_update(UI_t *screen, bool valid) {
    // GET_LAST_STATE(primary, lv_feedback_enclosure_voltage, PRIMARY, LV_FEEDBACK_ENCLOSURE_VOLTAGE);
    update_shutdown_circuit_component(screen, shutdown_circuit_feedbacks_invc_lid_fb_index, true);  // not available
}

void lv_errors_update(UI_t *screen, bool valid) {
}

void lv_currents_update(UI_t *screen, bool valid) {
}

void lv_total_voltage_update(UI_t *screen, bool valid) {
}

void lv_cells_voltage_update(UI_t *screen, bool valid) {
}

void lv_cells_voltage_stats_update(UI_t *screen, bool valid) {
}

void lv_cells_temp_update(UI_t *screen, bool valid) {
}

void lv_cells_temp_stats_update(UI_t *screen, bool valid) {
}

void hv_status_update(UI_t *screen, bool valid) {
}

// TODO: remove or reimplement
void hv_fans_override_status_update(UI_t *screen, bool valid) {
    GET_LAST_STATE(primary, hv_fans_status, PRIMARY, HV_FANS_STATUS);

    screen->components[swoc_hv_cooling].swoc_elem_was_updated      = 1;
    screen->components[swoc_hv_cooling_name].swoc_elem_was_updated = 1;
    if (!primary_hv_fans_status_last_state->fans_override) {
        snprintf(screen->components[swoc_hv_cooling].swoc_elem_label, SWOC_STRING_LEN, "A");
        screen->components[swoc_hv_cooling].swoc_elem_bg_color = OLIVEC_COLOR_GREEN;
        screen->components[swoc_hv_cooling].swoc_elem_lb_color = OLIVEC_COLOR_BLACK;
    } else {
        snprintf(screen->components[swoc_hv_cooling].swoc_elem_label, SWOC_STRING_LEN, "%.0f", primary_hv_fans_status_last_state->fans_speed * 10.0f);
        screen->components[swoc_hv_cooling].swoc_elem_bg_color = OLIVEC_COLOR_BLACK;
        screen->components[swoc_hv_cooling].swoc_elem_lb_color = OLIVEC_COLOR_WHITE;
    }
    screen->components[swoc_hv_cooling_name].swoc_elem_bg_color = screen->components[swoc_hv_cooling].swoc_elem_bg_color;
    screen->components[swoc_hv_cooling_name].swoc_elem_lb_color = screen->components[swoc_hv_cooling].swoc_elem_lb_color;
}

void tlm_status_update(UI_t *screen, bool valid) {
}

void odometer_update(UI_t *screen, bool valid) {
}

void steer_angle_update(UI_t *screen, bool valid) {
    GET_LAST_STATE(secondary, steer_angle, SECONDARY, STEER_ANGLE);
    screen->components[18].swoc_elem_was_updated = 1;
    snprintf(screen->components[18].swoc_elem_label, SWOC_STRING_LEN, "%.1f", secondary_steer_angle_last_state->angle);
    screen->components[18].swoc_elem_lb_color = OLIVEC_COLOR_WHITE;
    screen->components[18].swoc_elem_bg_color = OLIVEC_COLOR_BLACK;

    screen->components[19].swoc_elem_was_updated = 1;
    screen->components[19].swoc_elem_lb_color    = OLIVEC_COLOR_WHITE;
    screen->components[19].swoc_elem_bg_color    = OLIVEC_COLOR_BLACK;
}

void tlm_network_interface_update(UI_t *screen, bool valid) {
}

void imu_acceleration_update(UI_t *screen, bool valid) {
}

void tlm_lap_time_update(UI_t *screen, bool valid) {
}

void tlm_laps_stats_update(UI_t *screen, bool valid) {
    GET_LAST_STATE(primary, ecu_status, PRIMARY, ECU_STATUS);
    if (primary_ecu_status_last_state->status != primary_ecu_status_status_drive) {
        return;
    }

    GET_LAST_STATE(secondary, tlm_laps_stats, SECONDARY, TLM_LAPS_STATS);

    screen->components[swoc_lap_time].swoc_elem_was_updated = 1;
    snprintf(screen->components[swoc_lap_time].swoc_elem_label, SWOC_STRING_LEN, "%.0f", secondary_tlm_laps_stats_last_state->last_time);

    if (secondary_tlm_laps_stats_last_state->last_time < secondary_tlm_laps_stats_last_state->best_time) {
        screen->components[swoc_lap_time].swoc_elem_lb_color = OLIVEC_COLOR_BLACK;
        screen->components[swoc_lap_time].swoc_elem_bg_color = OLIVEC_COLOR_PURPLE;
    } else {
        screen->components[swoc_lap_time].swoc_elem_lb_color = OLIVEC_COLOR_WHITE;
        screen->components[swoc_lap_time].swoc_elem_bg_color = OLIVEC_COLOR_BLACK;
    }
}

#define INVERTER_MESSAGE_UNINITIALIZED     (-100.0f)
#define INVERTER_TEMP_CONVERSION(raw_temp) (-43.23745 + 0.01073427 * raw_temp - 5.523417e-7 * pow(raw_temp, 2) + 1.330787e-11 * pow(raw_temp, 3));
float l_motor_temp = INVERTER_MESSAGE_UNINITIALIZED;
float l_igbt_temp  = INVERTER_MESSAGE_UNINITIALIZED;
float r_motor_temp = INVERTER_MESSAGE_UNINITIALIZED;
float r_igbt_temp  = INVERTER_MESSAGE_UNINITIALIZED;

#include "inverter_conversions.h"

void inv_l_rcv_update(UI_t *screen, bool valid) {
    GET_LAST_STATE(inverters, inv_l_rcv, INVERTERS, INV_L_RCV);

    l_motor_temp          = convert_t_motor(inverters_inv_l_rcv_last_state->t_motor);
    float motor_temp_mean = (l_motor_temp + r_motor_temp) / 2.0f;
    if (r_motor_temp != INVERTER_MESSAGE_UNINITIALIZED) {
        screen->components[swoc_temp_mot].swoc_elem_was_updated      = 1;
        screen->components[swoc_temp_mot_name].swoc_elem_was_updated = 1;
        snprintf(screen->components[swoc_temp_mot].swoc_elem_label, SWOC_STRING_LEN, "%.0f", motor_temp_mean);

        if (motor_temp_mean > MOTOR_TEMP_CRITICAL_THRESHOLD) {
            screen->components[swoc_temp_mot].swoc_elem_lb_color = OLIVEC_COLOR_BLACK;
            screen->components[swoc_temp_mot].swoc_elem_bg_color = OLIVEC_COLOR_RED;
        } else if (motor_temp_mean > MOTOR_TEMP_ALERT_THRESHOLD) {
            screen->components[swoc_temp_mot].swoc_elem_lb_color = OLIVEC_COLOR_BLACK;
            screen->components[swoc_temp_mot].swoc_elem_bg_color = OLIVEC_COLOR_YELLOW;
        } else {
            screen->components[swoc_temp_mot].swoc_elem_lb_color = OLIVEC_COLOR_BLACK;
            screen->components[swoc_temp_mot].swoc_elem_bg_color = OLIVEC_COLOR_GREEN;
        }
        screen->components[swoc_temp_mot_name].swoc_elem_lb_color = screen->components[swoc_temp_mot].swoc_elem_lb_color;
        screen->components[swoc_temp_mot_name].swoc_elem_bg_color = screen->components[swoc_temp_mot].swoc_elem_bg_color;
    }

    l_igbt_temp          = convert_t_igbt(inverters_inv_l_rcv_last_state->t_igbt);
    float igbt_temp_mean = (l_igbt_temp + r_igbt_temp) / 2.0f;
    if (r_igbt_temp != INVERTER_MESSAGE_UNINITIALIZED) {
        screen->components[swoc_temp_inv].swoc_elem_was_updated      = 1;
        screen->components[swoc_temp_inv_name].swoc_elem_was_updated = 1;
        snprintf(screen->components[swoc_temp_inv].swoc_elem_label, SWOC_STRING_LEN, "%.0f", igbt_temp_mean);

        if (igbt_temp_mean > INVERTER_TEMP_CRITICAL_THRESHOLD) {
            screen->components[swoc_temp_inv].swoc_elem_lb_color = OLIVEC_COLOR_BLACK;
            screen->components[swoc_temp_inv].swoc_elem_bg_color = OLIVEC_COLOR_RED;
        } else if (igbt_temp_mean > INVERTER_TEMP_ALERT_THRESHOLD) {
            screen->components[swoc_temp_inv].swoc_elem_lb_color = OLIVEC_COLOR_BLACK;
            screen->components[swoc_temp_inv].swoc_elem_bg_color = OLIVEC_COLOR_YELLOW;
        } else {
            screen->components[swoc_temp_inv].swoc_elem_lb_color = OLIVEC_COLOR_BLACK;
            screen->components[swoc_temp_inv].swoc_elem_bg_color = OLIVEC_COLOR_GREEN;
        }
        screen->components[swoc_temp_inv_name].swoc_elem_lb_color = screen->components[swoc_temp_inv].swoc_elem_lb_color;
        screen->components[swoc_temp_inv_name].swoc_elem_bg_color = screen->components[swoc_temp_inv].swoc_elem_bg_color;
    }
}

void inv_r_rcv_update(UI_t *screen, bool valid) {
    GET_LAST_STATE(inverters, inv_r_rcv, INVERTERS, INV_R_RCV);
    r_motor_temp = convert_t_motor(inverters_inv_r_rcv_last_state->t_motor);
    r_igbt_temp  = convert_t_igbt(inverters_inv_r_rcv_last_state->t_igbt);
}

void irts_fl_update(UI_t *screen, bool valid) {
}

void irts_fr_update(UI_t *screen, bool valid) {
}

void irts_rl_update(UI_t *screen, bool valid) {
}

void irts_rr_update(UI_t *screen, bool valid) {
}

void pedal_throttle_update(UI_t *screen, bool valid) {
}

void pedal_brakes_pressure_update(UI_t *screen, bool valid) {
}

void ecu_version_update(UI_t *screen, bool valid) {
}

void lv_version_update(UI_t *screen, bool valid) {
}

void hv_cellboard_version_update(UI_t *screen, bool valid) {
}

void hv_mainboard_version_update(UI_t *screen, bool valid) {
}

void tlm_version_update(UI_t *screen, bool valid) {
}

void control_status_update(UI_t *screen, bool valid) {
}

void lv_radiator_speed_update(UI_t *screen, bool valid) {
}

void lv_pumps_speed_update(UI_t *screen, bool valid) {
    GET_LAST_STATE(primary, lv_pumps_speed, PRIMARY, LV_PUMPS_SPEED);

    screen->components[swoc_pt_cooling].swoc_elem_was_updated      = 1;
    screen->components[swoc_pt_cooling_name].swoc_elem_was_updated = 1;
    if (primary_lv_pumps_speed_last_state->status == primary_lv_pumps_speed_status_auto) {
        snprintf(screen->components[swoc_pt_cooling].swoc_elem_label, SWOC_STRING_LEN, "A");
        screen->components[swoc_pt_cooling].swoc_elem_bg_color = OLIVEC_COLOR_GREEN;
        screen->components[swoc_pt_cooling].swoc_elem_lb_color = OLIVEC_COLOR_BLACK;
    } else {
        snprintf(screen->components[swoc_pt_cooling].swoc_elem_label, SWOC_STRING_LEN, "%.0f", primary_lv_pumps_speed_last_state->pumps_speed * 10.0f);
        screen->components[swoc_pt_cooling].swoc_elem_bg_color = OLIVEC_COLOR_BLACK;
        screen->components[swoc_pt_cooling].swoc_elem_lb_color = OLIVEC_COLOR_WHITE;
    }
    screen->components[swoc_pt_cooling_name].swoc_elem_bg_color = screen->components[swoc_pt_cooling].swoc_elem_bg_color;
    screen->components[swoc_pt_cooling_name].swoc_elem_lb_color = screen->components[swoc_pt_cooling].swoc_elem_lb_color;
}

void lv_cooling_aggressiveness_update(UI_t *screen, bool valid) {
}

void ptt_periodic_check(UI_t *screen) {
    GET_LAST_STATE(primary, ecu_ptt_status, PRIMARY, ECU_PTT_STATUS);
    GET_LAST_STATE(primary, ecu_set_ptt_status, PRIMARY, ECU_SET_PTT_STATUS);
    screen->components[swoc_ptt].swoc_elem_was_updated = 1;
    if ((primary_ecu_ptt_status_last_state->status == primary_ecu_ptt_status_status_off) && ptt_button_pressed) {
        current_ptt_status                            = ptt_status_SET_ON;
        primary_ecu_set_ptt_status_last_state->status = primary_ecu_set_ptt_status_status_on;
        snprintf(screen->components[swoc_ptt].swoc_elem_label, SWOC_STRING_LEN, "MUTE");
        screen->components[swoc_ptt].swoc_elem_bg_color = OLIVEC_COLOR_BLACK;
        screen->components[swoc_ptt].swoc_elem_lb_color = OLIVEC_COLOR_WHITE;
    } else if ((primary_ecu_ptt_status_last_state->status == primary_ecu_ptt_status_status_on) && !ptt_button_pressed) {
        current_ptt_status                            = ptt_status_SET_OFF;
        primary_ecu_set_ptt_status_last_state->status = primary_ecu_set_ptt_status_status_off;
        snprintf(screen->components[swoc_ptt].swoc_elem_label, SWOC_STRING_LEN, "MUTE");
        screen->components[swoc_ptt].swoc_elem_bg_color = OLIVEC_COLOR_BLACK;
        screen->components[swoc_ptt].swoc_elem_lb_color = OLIVEC_COLOR_WHITE;
    } else if ((primary_ecu_ptt_status_last_state->status == primary_ecu_ptt_status_status_on) && ptt_button_pressed) {
        current_ptt_status                            = ptt_status_ON;
        primary_ecu_set_ptt_status_last_state->status = primary_ecu_set_ptt_status_status_on;
        snprintf(screen->components[swoc_ptt].swoc_elem_label, SWOC_STRING_LEN, "TALK");
        screen->components[swoc_ptt].swoc_elem_bg_color = OLIVEC_COLOR_BLUE;
        screen->components[swoc_ptt].swoc_elem_lb_color = OLIVEC_COLOR_WHITE;
    } else if ((primary_ecu_ptt_status_last_state->status == primary_ecu_ptt_status_status_off) && !ptt_button_pressed) {
        current_ptt_status                            = ptt_status_OFF;
        primary_ecu_set_ptt_status_last_state->status = primary_ecu_set_ptt_status_status_off;
        snprintf(screen->components[swoc_ptt].swoc_elem_label, SWOC_STRING_LEN, "MUTE");
        screen->components[swoc_ptt].swoc_elem_bg_color = OLIVEC_COLOR_BLACK;
        screen->components[swoc_ptt].swoc_elem_lb_color = OLIVEC_COLOR_WHITE;
    }
}
