#include "steering.h"

#include "steering_config.h"

extern bool steering_initialized;

#define SNPRINTF_BUFFER_SIZE (64u)
char snprintf_buffer[SNPRINTF_BUFFER_SIZE];

uint8_t primary_messages_last_state[primary_MESSAGE_COUNT][primary_MAX_STRUCT_SIZE_CONVERSION];
uint8_t secondary_messages_last_state[secondary_MESSAGE_COUNT][secondary_MAX_STRUCT_SIZE_CONVERSION];
uint8_t inverters_messages_last_state[inverters_MESSAGE_COUNT][inverters_MAX_STRUCT_SIZE_CONVERSION];

// extern int torque_vectoring_last_state;
// extern int slip_control_last_state;
extern int set_pumps_speed_last_state;
extern int set_radiators_last_state;
extern int pork_fans_status_last_state;
// extern int power_map_last_state;

primary_lv_cells_voltage_converted_t lv_voltages_stock_1 = {0};
primary_lv_cells_voltage_converted_t lv_voltages_stock_2 = {0};
primary_lv_cells_temp_converted_t lv_temps_stock_1       = {0};
primary_lv_cells_temp_converted_t lv_temps_stock_2       = {0};

size_t tlm_ntw_interfaces_current_size               = 0;
uint32_t tlm_ntw_interfaces[TLM_NTW_INTERFACE_MAX_N] = {0};
uint32_t tlm_ntw_ips[TLM_NTW_INTERFACE_MAX_N]        = {0};

uint32_t timestamp_start_lap = 0;

void car_status_update() {
    GET_LAST_STATE(primary, ecu_status, PRIMARY, ECU_STATUS);
    switch (primary_ecu_status_last_state->status) {
        case primary_ecu_status_status_init:
        case primary_ecu_status_status_enable_inv_updates:
        case primary_ecu_status_status_check_inv_settings: {
            set_tab_racing_label_text("-", tab_rac_bottom_status_idx);
            set_tab_racing_label_text("INIT", tab_rac_status_idx);
            set_tab_track_test_lb_speed("INIT");
            break;
        }
        case primary_ecu_status_status_idle: {
            set_tab_racing_label_text("-", tab_rac_bottom_status_idx);
            set_tab_racing_label_text("IDLE", tab_rac_status_idx);
            set_tab_track_test_lb_speed("IDLE");
            break;
        }
        case primary_ecu_status_status_start_ts_precharge:
        case primary_ecu_status_status_wait_ts_precharge: {
            set_tab_racing_label_text("-", tab_rac_bottom_status_idx);
            set_tab_racing_label_text("PRCHG", tab_rac_status_idx);
            set_tab_track_test_lb_speed("PRCHG");
            break;
        }
        case primary_ecu_status_status_wait_driver: {
            set_tab_racing_label_text("-", tab_rac_bottom_status_idx);
            set_tab_racing_label_text("SETUP", tab_rac_status_idx);
            set_tab_track_test_lb_speed("SETUP");
            break;
        }
        case primary_ecu_status_status_enable_inv_drive: {
            set_tab_racing_label_text("-", tab_rac_bottom_status_idx);
            set_tab_racing_label_text("ENINV", tab_rac_status_idx);
            set_tab_track_test_lb_speed("ENINV");
            break;
        }
        case primary_ecu_status_status_drive: {
            set_tab_racing_label_text("km/h", tab_rac_bottom_status_idx);
            // set_tab_racing_label_text("DRIVE", tab_rac_status_idx);
            // set_tab_track_test_lb_speed("DRIVE");
            break;
        }
        case primary_ecu_status_status_disable_inv_drive:
        case primary_ecu_status_status_start_ts_discharge:
        case primary_ecu_status_status_wait_ts_discharge: {
            set_tab_racing_label_text("-", tab_rac_bottom_status_idx);
            set_tab_racing_label_text("TSOFF", tab_rac_status_idx);
            set_tab_track_test_lb_speed("TSOFF");
            break;
        }
        case primary_ecu_status_status_fatal_error: {
            set_tab_racing_label_text("-", tab_rac_bottom_status_idx);
            set_tab_racing_label_text("FATAL", tab_rac_status_idx);
            set_tab_track_test_lb_speed("FATAL");
            break;
        }
        default:
            break;
    }
}

void lv_pumps_actual_value_update() {
}

void lv_radiators_actual_value_update() {
}

void tlm_status_update() {
    GET_LAST_STATE(primary, tlm_status, PRIMARY, TLM_STATUS);
    if (primary_tlm_status_last_state->status == primary_tlm_status_status_on) {
        snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "ON");
        // all_leds_green();
    } else {
        snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "OFF");
        // all_leds_red();
    }
}

void angular_velocity_update(void) {
    GET_LAST_STATE(secondary, angular_velocity, SECONDARY, ANGULAR_VELOCITY);
    GET_LAST_STATE(primary, ecu_status, PRIMARY, ECU_STATUS);
    if (primary_ecu_status_last_state->status != primary_ecu_status_status_drive) {
        return;
    }
    float speed = fabs((secondary_angular_velocity_last_state->fl + secondary_angular_velocity_last_state->fr) / 2.0f) * 3.6f * 0.203f;
    set_tab_racing_label_text("KM/H", tab_rac_bottom_status_idx);
    snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "%.0f", speed);
    set_tab_racing_label_text(snprintf_buffer, tab_rac_status_idx);
    set_tab_track_test_lb_speed(snprintf_buffer);
}

#define ERROR_COUNTER_CHECKER(error_def, error_def_string)                                                                                   \
    if (primary_hv_errors_last_state->error_def != 0) {                                                                                      \
        if (error_counter == 3) {                                                                                                            \
            hv_errors_buffer_size += snprintf(hv_errors_buffer + hv_errors_buffer_size, BUFSIZ - hv_errors_buffer_size, "and others");       \
            goto display_notification_jmp;                                                                                                   \
        }                                                                                                                                    \
        error_counter++;                                                                                                                     \
        hv_errors_buffer_size += snprintf(hv_errors_buffer + hv_errors_buffer_size, BUFSIZ - hv_errors_buffer_size, error_def_string " \n"); \
    }

void hv_errors_update() {
    GET_LAST_STATE(primary, hv_errors, PRIMARY, HV_ERRORS);
    size_t error_counter          = 0;
    char hv_errors_buffer[BUFSIZ] = {0};
    size_t hv_errors_buffer_size  = 0;
    hv_errors_buffer_size += snprintf(hv_errors_buffer + hv_errors_buffer_size, BUFSIZ - hv_errors_buffer_size, "HV BMS ERRORS: \n");
    ERROR_COUNTER_CHECKER(errors_cell_under_voltage, "errors_cell_under_voltage");
    ERROR_COUNTER_CHECKER(errors_cell_over_voltage, "errors_cell_over_voltage");
    ERROR_COUNTER_CHECKER(errors_cell_under_temperature, "errors_cell_under_temperature");
    ERROR_COUNTER_CHECKER(errors_cell_over_temperature, "errors_cell_over_temperature");
    ERROR_COUNTER_CHECKER(errors_over_current, "errors_over_current");
    ERROR_COUNTER_CHECKER(errors_can, "errors_can");
    ERROR_COUNTER_CHECKER(errors_int_voltage_mismatch, "errors_int_voltage_mismatch");
    ERROR_COUNTER_CHECKER(errors_cellboard_comm, "errors_cellboard_comm");
    ERROR_COUNTER_CHECKER(errors_cellboard_internal, "errors_cellboard_internal");
    ERROR_COUNTER_CHECKER(errors_connector_disconnected, "errors_connector_disconnected");
    ERROR_COUNTER_CHECKER(errors_fans_disconnected, "errors_fans_disconnected");
    ERROR_COUNTER_CHECKER(errors_feedback, "errors_feedback");
    ERROR_COUNTER_CHECKER(errors_feedback_circuitry, "errors_feedback_circuitry");
    ERROR_COUNTER_CHECKER(errors_eeprom_comm, "errors_eeprom_comm");
    ERROR_COUNTER_CHECKER(errors_eeprom_write, "errors_eeprom_write");
display_notification_jmp:
    if (error_counter != 0)
        display_notification(hv_errors_buffer, 2500, COLOR_SECONDARY_HEX, COLOR_PRIMARY_HEX);
}

void hv_debug_signals_update(void) {
    GET_LAST_STATE(primary, hv_debug_signals, PRIMARY, HV_DEBUG_SIGNALS);
    tab_hv_set_error_status(debug_signal_error_cell_under_voltage, primary_hv_debug_signals_last_state->errors_cell_under_voltage);
    tab_hv_set_error_status(debug_signal_error_cell_over_voltage, primary_hv_debug_signals_last_state->errors_cell_over_voltage);
    tab_hv_set_error_status(debug_signal_error_cell_over_temperature, primary_hv_debug_signals_last_state->errors_cell_over_temperature);
    tab_hv_set_error_status(debug_signal_error_over_current, primary_hv_debug_signals_last_state->errors_over_current);
    tab_hv_set_error_status(debug_signal_error_can, primary_hv_debug_signals_last_state->errors_can);
    tab_hv_set_error_status(debug_signal_error_int_voltage_mismatch, primary_hv_debug_signals_last_state->errors_int_voltage_mismatch);
    tab_hv_set_error_status(debug_signal_error_cellboard_comm, primary_hv_debug_signals_last_state->errors_cellboard_comm);
    tab_hv_set_error_status(debug_signal_error_cellboard_internal, primary_hv_debug_signals_last_state->errors_cellboard_internal);
    tab_hv_set_error_status(debug_signal_error_connector_disconnected, primary_hv_debug_signals_last_state->errors_connector_disconnected);
    tab_hv_set_error_status(debug_signal_error_fans_disconnected, primary_hv_debug_signals_last_state->errors_fans_disconnected);
    tab_hv_set_error_status(debug_signal_error_feedback, primary_hv_debug_signals_last_state->errors_feedback);
    tab_hv_set_error_status(debug_signal_error_feedback_circuitry, primary_hv_debug_signals_last_state->errors_feedback_circuitry);
    tab_hv_set_error_status(debug_signal_error_eeprom_comm, primary_hv_debug_signals_last_state->errors_eeprom_comm);
    tab_hv_set_error_status(debug_signal_error_eeprom_write, primary_hv_debug_signals_last_state->errors_eeprom_write);

    tab_hv_update_error_label();
}

void hv_cells_voltage_stats_update(void) {
    GET_LAST_STATE(primary, hv_cells_voltage_stats, PRIMARY, HV_CELLS_VOLTAGE_STATS);

    snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "%.2f", primary_hv_cells_voltage_stats_last_state->min);
    set_tab_hv_label_text(snprintf_buffer, tab_hv_lb_voltage_min);

    snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "%.2f", primary_hv_cells_voltage_stats_last_state->max);
    set_tab_hv_label_text(snprintf_buffer, tab_hv_lb_voltage_max);

    snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "%d", (int)(primary_hv_cells_voltage_stats_last_state->delta * 1000.0f));
    set_tab_hv_label_text(snprintf_buffer, tab_hv_lb_voltage_delta);
}

void hv_total_voltage_update(void) {
    GET_LAST_STATE(primary, hv_total_voltage, PRIMARY, HV_TOTAL_VOLTAGE);

    precharge_bar_update((int32_t)primary_hv_total_voltage_last_state->bus);
    snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "%.0f", primary_hv_total_voltage_last_state->pack);
    set_tab_hv_label_text(snprintf_buffer, tab_hv_lb_pack_voltage);

    snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "%.0f", primary_hv_total_voltage_last_state->sum_cell);
    set_tab_hv_label_text(snprintf_buffer, tab_hv_lb_pack_voltage_2);
}

void hv_current_update() {
    GET_LAST_STATE(primary, hv_current, PRIMARY, HV_CURRENT);
    snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "%.1f", primary_hv_current_last_state->current);
    set_tab_racing_label_text(snprintf_buffer, tab_rac_hv_curr_idx);
    set_tab_racing_hv_current_bar((int32_t)(primary_hv_current_last_state->current));
}

void hv_soc_estimation_update() {
    GET_LAST_STATE(secondary, hv_soc_estimation_state, SECONDARY, HV_SOC_ESTIMATION_STATE);
    snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "%.0f", secondary_hv_soc_estimation_state_last_state->soc * 100.0f);
    set_tab_racing_label_text(snprintf_buffer, tab_rac_hv_soc_idx);
    set_tab_racing_hv_pack_voltage_bar((int32_t)(secondary_hv_soc_estimation_state_last_state->soc * 100.0f));
}

void hv_cells_temp_stats_update() {
    GET_LAST_STATE(primary, hv_cells_temp_stats, PRIMARY, HV_CELLS_TEMP_STATS);

    snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "%0.f", primary_hv_cells_temp_stats_last_state->avg);
    set_tab_racing_label_text(snprintf_buffer, tab_rac_hv_avg_temp_idx);
    set_tab_hv_label_text(snprintf_buffer, tab_hv_lb_temp_avg);

    snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "%0.f", primary_hv_cells_temp_stats_last_state->max);
    set_tab_hv_label_text(snprintf_buffer, tab_hv_lb_temp_max);

    snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "%0.f", primary_hv_cells_temp_stats_last_state->min);
    set_tab_hv_label_text(snprintf_buffer, tab_hv_lb_temp_min);
}

bool cellboard_bal[N_PORK_CELLBOARD] = {0};

void set_bal_status_label_text(char *text);

void hv_cell_balancing_status_update() {
    GET_LAST_STATE(primary, hv_balancing_status, PRIMARY, HV_BALANCING_STATUS);
    uint8_t cellboard_id = (uint8_t)primary_hv_balancing_status_last_state->cellboard_id;
    if (cellboard_id < 0 || cellboard_id >= N_PORK_CELLBOARD) {
        return;
    }
    primary_hv_balancing_status_balancing_status status = primary_hv_balancing_status_last_state->balancing_status;
    cellboard_bal[cellboard_id]                         = status == primary_hv_balancing_status_balancing_status_off ? false : true;
    bool is_bal                                         = false;
    for (uint8_t i = 0; i < N_PORK_CELLBOARD; i++) {
        if (cellboard_bal[i]) {
            is_bal = true;
            break;
        }
    }
    char buf[BUFSIZ] = {0};
    snprintf(buf, BUFSIZ, "BAL STATUS: %s", is_bal ? "ON" : "OFF");
    set_bal_status_label_text(buf);

    set_balancing_column(cellboard_bal[cellboard_id], cellboard_id);
    // TODO: if bal is off, write bal off on the screen
}

void hv_feedback_misc_voltage_update(void) {
}

void hv_feedbacks_status_update(void) {
}

void hv_feedback_ts_voltage_update(void) {
    // GET_LAST_STATE(primary, hv_feedback_ts_voltage, PRIMARY, HV_FEEDBACK_TS_VOLTAGE);
    // update_shutdown_circuit_component(shutdown_circuit_feedbacks_status_feedback_precharge_status_index, true);
    // update_shutdown_circuit_component(shutdown_circuit_feedbacks_status_feedback_airp_gate_index, primary_hv_feedback_ts_voltage_last_state->airp_gate > 2.5f);
    // update_shutdown_circuit_component(shutdown_circuit_feedbacks_status_feedback_airn_gate_index, primary_hv_feedback_ts_voltage_last_state->airn_gate > 2.5f);
}

void hv_feedback_sd_voltage_update(void) {
    GET_LAST_STATE(primary, hv_feedback_sd_voltage, PRIMARY, HV_FEEDBACK_SD_VOLTAGE);
    update_shutdown_circuit_component(shutdown_circuit_feedbacks_status_feedback_sd_in_index, primary_hv_feedback_sd_voltage_last_state->sd_in > 3.0f);
    update_shutdown_circuit_component(shutdown_circuit_feedbacks_status_feedback_sd_end_index, primary_hv_feedback_sd_voltage_last_state->sd_end > 2.5f);
    update_shutdown_circuit_component(shutdown_circuit_feedbacks_status_feedback_sd_out_index, primary_hv_feedback_sd_voltage_last_state->sd_out > 3.0f);
}

void ecu_feedbacks_update(void) {
    GET_LAST_STATE(primary, ecu_feedbacks, PRIMARY, ECU_FEEDBACKS);
    update_shutdown_circuit_component(shutdown_circuit_ecu_feedbacks_sd_in_index, primary_ecu_feedbacks_last_state->feedbacks_sd_in);
    update_shutdown_circuit_component(shutdown_circuit_ecu_feedbacks_sd_cock_fb_index, primary_ecu_feedbacks_last_state->feedbacks_sd_cock_fb);
    update_shutdown_circuit_component(shutdown_circuit_ecu_feedbacks_sd_inertial_fb_index, primary_ecu_feedbacks_last_state->feedbacks_sd_interial_fb);
    update_shutdown_circuit_component(shutdown_circuit_ecu_feedbacks_sd_bots_fb_index, primary_ecu_feedbacks_last_state->feedbacks_sd_bots_fb);
}

void lv_feedback_sd_voltage_update() {
    GET_LAST_STATE(primary, lv_feedback_sd_voltage, PRIMARY, LV_FEEDBACK_SD_VOLTAGE);
    update_shutdown_circuit_component(shutdown_circuit_sd_start_index, primary_lv_feedback_sd_voltage_last_state->sd_start > 12.0f);
    update_shutdown_circuit_component(shutdown_circuit_feedbacks_interlock_fb_index, primary_lv_feedback_sd_voltage_last_state->interlock > 12.0f);
    update_shutdown_circuit_component(shutdown_circuit_sd_end_index, primary_lv_feedback_sd_voltage_last_state->sd_end > 12.0f);
}

void lv_feedback_ts_voltage_update() {
    GET_LAST_STATE(primary, lv_feedback_ts_voltage, PRIMARY, LV_FEEDBACK_TS_VOLTAGE);
    update_shutdown_circuit_component(shutdown_circuit_feedbacks_hvd_fb_index, true);  // mmm not available(?)
    update_shutdown_circuit_component(shutdown_circuit_feedbacks_bspd_fb_index, primary_lv_feedback_ts_voltage_last_state->bspd > 12.0f);
    update_shutdown_circuit_component(shutdown_circuit_feedbacks_invc_interlock_fb_index, primary_lv_feedback_ts_voltage_last_state->invc_interlock > 12.0f);
}

void lv_feedback_enclosure_voltage_update() {
    // GET_LAST_STATE(primary, lv_feedback_enclosure_voltage, PRIMARY, LV_FEEDBACK_ENCLOSURE_VOLTAGE);
    update_shutdown_circuit_component(shutdown_circuit_feedbacks_invc_lid_fb_index, true);  // not available
}

void hv_status_update() {
    GET_LAST_STATE(primary, hv_status, PRIMARY, HV_STATUS);

    switch (primary_hv_status_last_state->status) {
        case primary_hv_status_status_init:
            precharge_bar_insert(false);
            set_tab_hv_label_text("INIT", tab_hv_lb_current_state);
            break;
        case primary_hv_status_status_idle:
            precharge_bar_insert(false);
            set_tab_hv_label_text("IDLE", tab_hv_lb_current_state);
            break;
        case primary_hv_status_status_airn_close:
            precharge_bar_insert(false);
            set_tab_hv_label_text("AIRN CLOSE", tab_hv_lb_current_state);
            break;
        case primary_hv_status_status_precharge:
            precharge_bar_insert(true);
            set_tab_hv_label_text("PRECHARGE", tab_hv_lb_current_state);
            break;
        case primary_hv_status_status_airp_close:
            precharge_bar_insert(false);
            set_tab_hv_label_text("AIRP CLOSE", tab_hv_lb_current_state);
            break;
        case primary_hv_status_status_ts_on:
            precharge_bar_insert(false);
            set_tab_hv_label_text("TS ON", tab_hv_lb_current_state);
            break;
        case primary_hv_status_status_fatal_error:
            precharge_bar_insert(false);
            set_tab_hv_label_text("FATAL ERROR", tab_hv_lb_current_state);
            break;
    };
}

extern int pork_fans_status_last_state;

void hv_fans_override_status_update() {
    GET_LAST_STATE(primary, hv_fans_status, PRIMARY, HV_FANS_STATUS);

    if (primary_hv_fans_status_last_state->fans_override == primary_hv_fans_status_fans_override_off) {
        snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "AUTO");
        tab_hv_set_pork_speed_bar((int32_t)(primary_hv_fans_status_last_state->fans_speed * 100.0f), true);
    } else {
        snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "%0.1f", primary_hv_fans_status_last_state->fans_speed);
        tab_hv_set_pork_speed_bar((int32_t)(primary_hv_fans_status_last_state->fans_speed * 100.0f), false);
    }
    set_tab_hv_label_text(snprintf_buffer, tab_hv_pork_speed_value);
}

extern primary_lv_radiator_speed_converted_t steering_wheel_state_radiator_speed;
extern primary_lv_pumps_speed_converted_t steering_wheel_state_pumps_speed;
extern uint32_t steering_wheel_lv_pumps_speed_sent_timestamp;
extern uint32_t steering_wheel_lv_radiators_speed_sent_timestamp;
extern steering_wheel_cooling_status_t steering_wheel_lv_pumps_speed_state;
extern steering_wheel_cooling_status_t steering_wheel_lv_radiator_speed_state;

void lv_pumps_speed_update_all_graphics(primary_lv_pumps_speed_converted_t *msg) {
    if (msg->status == primary_lv_pumps_speed_status_off || msg->status == primary_lv_pumps_speed_status_auto) {
        snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "AUTO");
        set_tab_lv_label_text(snprintf_buffer, tab_lv_lb_pumps_local);
    } else {
        snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "%0.1f", msg->pumps_speed);
        set_tab_lv_label_text(snprintf_buffer, tab_lv_lb_pumps_local);
    }
    lv_set_pumps_speed_bar((int32_t)(msg->pumps_speed * 100.0f));
}

#define ALMOST_ERROR                (0.005f)
#define IS_ALMOST_EQUAL(val1, val2) ((fabs(val1 - val2) < ALMOST_ERROR))

void lv_pumps_speed_update(void) {
    GET_LAST_STATE(primary, lv_pumps_speed, PRIMARY, LV_PUMPS_SPEED);
    float actual_speed  = primary_lv_pumps_speed_last_state->pumps_speed;
    float actual_status = primary_lv_pumps_speed_last_state->status;

    switch (steering_wheel_lv_pumps_speed_state) {
        case STEERING_WHEEL_COOLING_STATUS_SYNC: {
            steering_wheel_state_pumps_speed.pumps_speed = actual_speed;
            steering_wheel_state_pumps_speed.status      = actual_status;
            lv_pumps_speed_update_all_graphics(&steering_wheel_state_pumps_speed);
            break;
        }
        case STEERING_WHEEL_COOLING_STATUS_SET: {
            if ((get_current_time_ms() - steering_wheel_lv_pumps_speed_sent_timestamp) < COOLING_STATE_SYNC_TIMEOUT)
                break;
            if ((!IS_ALMOST_EQUAL(steering_wheel_state_pumps_speed.pumps_speed, actual_speed)) &&
                !IS_ALMOST_EQUAL(steering_wheel_state_pumps_speed.status, actual_status)) {
                display_notification("BMS LV does not respond on pumps settings", 500, COLOR_SECONDARY_HEX, COLOR_PRIMARY_HEX);
                steering_wheel_state_pumps_speed.pumps_speed = actual_speed;
                steering_wheel_state_pumps_speed.status      = actual_status;
                steering_wheel_lv_pumps_speed_state          = STEERING_WHEEL_COOLING_STATUS_SYNC;
                lv_pumps_speed_update_all_graphics(&steering_wheel_state_pumps_speed);
            } else {
                lv_pumps_speed_update_all_graphics(&steering_wheel_state_pumps_speed);
                steering_wheel_lv_pumps_speed_state = STEERING_WHEEL_COOLING_STATUS_SYNC;
            }
            break;
        }
    }
}

void lv_radiator_speed_update_all_graphics(primary_lv_radiator_speed_converted_t *msg) {
    if (msg->status == primary_lv_radiator_speed_status_off || msg->status == primary_lv_radiator_speed_status_auto) {
        snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "AUTO");
    } else {
        snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "%0.1f", msg->radiator_speed);
    }
    set_tab_lv_label_text(snprintf_buffer, tab_lv_lb_radiators_local);
    lv_set_radiators_speed_bar((int32_t)(msg->radiator_speed * 100.0f), msg->status == primary_lv_radiator_speed_status_auto);
}

void lv_radiator_speed_update(void) {
    GET_LAST_STATE(primary, lv_radiator_speed, PRIMARY, LV_RADIATOR_SPEED);
    float actual_speed  = primary_lv_radiator_speed_last_state->radiator_speed;
    float actual_status = primary_lv_radiator_speed_last_state->status;

    switch (steering_wheel_lv_radiator_speed_state) {
        case STEERING_WHEEL_COOLING_STATUS_SYNC: {
            steering_wheel_state_radiator_speed.radiator_speed = actual_speed;
            steering_wheel_state_radiator_speed.status         = actual_status;
            lv_radiator_speed_update_all_graphics(&steering_wheel_state_radiator_speed);
            break;
        }
        case STEERING_WHEEL_COOLING_STATUS_SET: {
            if (((get_current_time_ms() - steering_wheel_lv_radiators_speed_sent_timestamp) < COOLING_STATE_SYNC_TIMEOUT))
                break;
            if (!IS_ALMOST_EQUAL(steering_wheel_state_radiator_speed.radiator_speed, actual_speed) &&
                !IS_ALMOST_EQUAL(steering_wheel_state_radiator_speed.status, actual_status)) {
                display_notification("BMS LV does not respond on radiator settings", 500, COLOR_SECONDARY_HEX, COLOR_PRIMARY_HEX);
                steering_wheel_state_radiator_speed.radiator_speed = actual_speed;
                steering_wheel_state_radiator_speed.status         = actual_status;
                steering_wheel_lv_radiator_speed_state             = STEERING_WHEEL_COOLING_STATUS_SYNC;
                lv_radiator_speed_update_all_graphics(&steering_wheel_state_radiator_speed);
            } else {
                steering_wheel_lv_radiator_speed_state = STEERING_WHEEL_COOLING_STATUS_SYNC;
            }
            break;
        }
    }
}

void lv_currents_update() {
    GET_LAST_STATE(primary, lv_current_battery, PRIMARY, LV_CURRENT_BATTERY);
    snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "%.1f", primary_lv_current_battery_last_state->lv_current);
    set_tab_lv_label_text(snprintf_buffer, tab_lv_lb_pack_voltage_2);
}

void lv_total_voltage_update() {
    GET_LAST_STATE(primary, lv_total_voltage, PRIMARY, LV_TOTAL_VOLTAGE);
    snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "%.1f", primary_lv_total_voltage_last_state->total);
    set_tab_lv_label_text(snprintf_buffer, tab_lv_lb_pack_voltage);
}

void lv_cells_voltage_update(void) {
#define N_LV_CELLS 6
    float current_lv_voltages[N_LV_CELLS] = {0};
    current_lv_voltages[0]                = lv_voltages_stock_1.voltage_0;
    current_lv_voltages[1]                = lv_voltages_stock_1.voltage_1;
    current_lv_voltages[2]                = lv_voltages_stock_1.voltage_2;
    current_lv_voltages[3]                = lv_voltages_stock_2.voltage_0;
    current_lv_voltages[4]                = lv_voltages_stock_2.voltage_1;
    current_lv_voltages[5]                = lv_voltages_stock_2.voltage_2;

    float sum = 0;
    for (uint8_t temp_index = 0; temp_index < N_LV_CELLS; temp_index++)
        sum += current_lv_voltages[temp_index];

    snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "%0.0f", sum);
    set_tab_lv_label_text(snprintf_buffer, tab_lv_lb_pack_voltage);

    float mean_voltage = (float)(sum / N_LV_CELLS);
    snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "%.0f", mean_voltage);
}

void lv_cells_voltage_stats_update() {
    GET_LAST_STATE(primary, lv_cells_voltage_stats, PRIMARY, LV_CELLS_VOLTAGE_STATS);
    snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "%.2f", primary_lv_cells_voltage_stats_last_state->max);
    set_tab_lv_label_text(snprintf_buffer, tab_lv_lb_voltage_max);

    snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "%.2f", primary_lv_cells_voltage_stats_last_state->min);
    set_tab_lv_label_text(snprintf_buffer, tab_lv_lb_voltage_min);

    snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "%.0f", primary_lv_cells_voltage_stats_last_state->delta * 1000);
    set_tab_lv_label_text(snprintf_buffer, tab_lv_lb_voltage_delta);
}

void lv_cells_temp_update() {
    float current_lv_temp[N_LV_CELLS] = {0};
    current_lv_temp[0]                = lv_temps_stock_1.temp_0;
    current_lv_temp[1]                = lv_temps_stock_1.temp_1;
    current_lv_temp[2]                = lv_temps_stock_1.temp_2;
    current_lv_temp[3]                = lv_temps_stock_2.temp_0;
    current_lv_temp[4]                = lv_temps_stock_2.temp_1;
    current_lv_temp[5]                = lv_temps_stock_2.temp_2;

    float sum = 0;
    for (uint8_t temp_index = 0; temp_index < N_LV_CELLS; temp_index++)
        sum += current_lv_temp[temp_index];

    for (size_t idx = 0; idx < N_LV_CELLS; idx++) {
    }

    float mean_temp = (float)(sum / N_LV_CELLS);
    snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "%.0f", mean_temp);
    set_tab_lv_label_text(snprintf_buffer, tab_lv_lb_temp_avg);
}

void lv_cells_temp_stats_update() {
    GET_LAST_STATE(primary, lv_cells_temp_stats, PRIMARY, LV_CELLS_TEMP_STATS);

    snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "%.0f", primary_lv_cells_temp_stats_last_state->max);
    set_tab_lv_label_text(snprintf_buffer, tab_lv_lb_temp_max);
    set_tab_racing_label_text(snprintf_buffer, tab_rac_lv_temp_idx);

    snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "%.0f", primary_lv_cells_temp_stats_last_state->min);
    set_tab_lv_label_text(snprintf_buffer, tab_lv_lb_temp_min);

    snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "%.0f", primary_lv_cells_temp_stats_last_state->avg);
    set_tab_lv_label_text(snprintf_buffer, tab_lv_lb_temp_avg);
}

void steer_angle_update() {
    GET_LAST_STATE(secondary, steer_angle, SECONDARY, STEER_ANGLE);
    snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "%.1f", secondary_steer_angle_last_state->angle);
    set_tab_track_test_steering_angle_bar(secondary_steer_angle_last_state->angle);
}

void tlm_network_interface_update(void) {
    for (size_t i = 0; i < tlm_ntw_interfaces_current_size; i++) {
        snprintf(
            snprintf_buffer,
            SNPRINTF_BUFFER_SIZE,
            "%c%c%c%c %lu %lu %lu %lu",
            (char)(tlm_ntw_interfaces[i] >> 24),
            (char)(tlm_ntw_interfaces[i] >> 16),
            (char)(tlm_ntw_interfaces[i] >> 8),
            (char)(tlm_ntw_interfaces[i]),
            (tlm_ntw_ips[i] >> 24),
            (tlm_ntw_ips[i] >> 16),
            (tlm_ntw_ips[i] >> 8),
            (tlm_ntw_ips[i]));
        size_t index_to_update = LV_MIN(tab_sensors_lb_tlm_ntw_interface_0 + TLM_NTW_INTERFACE_MAX_N - 1, tab_sensors_lb_tlm_ntw_interface_0 + i);
        set_tab_sensors_label_text(snprintf_buffer, index_to_update);
    }
}

void pedal_throttle_update(void) {
    GET_LAST_STATE(secondary, pedal_throttle, SECONDARY, PEDAL_THROTTLE);
    set_tab_sensors_value_apps((int32_t)(secondary_pedal_throttle_last_state->throttle * 100.0f));
}

void pedal_brakes_pressure_update(void) {
    GET_LAST_STATE(secondary, pedal_brakes_pressure, SECONDARY, PEDAL_BRAKES_PRESSURE);
    set_tab_sensors_value_brake_f(secondary_pedal_brakes_pressure_last_state->front);
    set_tab_sensors_value_brake_r(secondary_pedal_brakes_pressure_last_state->rear);
}

void imu_acceleration_update() {
}

void tlm_lap_time_update(void) {
    GET_LAST_STATE(secondary, tlm_lap_time, SECONDARY, TLM_LAP_TIME);
    // secondary_tlm_lap_time_last_state->lap_count;
    // secondary_tlm_lap_time_last_state->lap_time;

    int minutes        = (int)(secondary_tlm_lap_time_last_state->lap_time / 60.0f);
    int seconds        = (int)(secondary_tlm_lap_time_last_state->lap_time - minutes * 60.0f);
    double seconds_dec = 0.0f;
    int milliseconds   = (int)(modf(secondary_tlm_lap_time_last_state->lap_time, &seconds_dec) * 1000.0f);
    snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "%01d:%02d:%01d", minutes, seconds, milliseconds);
    set_tab_racing_label_text(snprintf_buffer, tab_rac_curr_time_idx);
}

void tlm_laps_stats_update(void) {
    GET_LAST_STATE(secondary, tlm_laps_stats, SECONDARY, TLM_LAPS_STATS);
    // secondary_tlm_laps_stats_last_state->best_time;
    // secondary_tlm_laps_stats_last_state->last_time;
    // secondary_tlm_laps_stats_last_state->lap_number;
    int minutes        = (int)(secondary_tlm_laps_stats_last_state->last_time / 60.0f);
    int seconds        = (int)(secondary_tlm_laps_stats_last_state->last_time - minutes * 60.0f);
    double seconds_dec = 0.0f;
    int milliseconds   = (int)(modf(secondary_tlm_laps_stats_last_state->last_time, &seconds_dec) * 1000.0f);
    snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "%01d:%02d:%01d", minutes, seconds, milliseconds);
    set_tab_racing_label_text(snprintf_buffer, tab_rac_last_time_idx);

    minutes      = (int)(secondary_tlm_laps_stats_last_state->best_time / 60.0f);
    seconds      = (int)(secondary_tlm_laps_stats_last_state->best_time - minutes * 60.0f);
    milliseconds = (int)(modf(secondary_tlm_laps_stats_last_state->best_time, &seconds_dec) * 1000.0f);
    snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "%01d:%02d:%01d", minutes, seconds, milliseconds);
    set_tab_racing_label_text(snprintf_buffer, tab_rac_best_time_idx);
}

#define INVERTER_MESSAGE_UNINITIALIZED     (-100.0f)
#define INVERTER_TEMP_CONVERSION(raw_temp) (-43.23745 + 0.01073427 * raw_temp - 5.523417e-7 * pow(raw_temp, 2) + 1.330787e-11 * pow(raw_temp, 3));
float l_motor_temp = INVERTER_MESSAGE_UNINITIALIZED;
float l_igbt_temp  = INVERTER_MESSAGE_UNINITIALIZED;
float r_motor_temp = INVERTER_MESSAGE_UNINITIALIZED;
float r_igbt_temp  = INVERTER_MESSAGE_UNINITIALIZED;

float IGBT_TEMP_COEFFICIENT[6] =
    {(float)3.58282057e-18, (float)-4.14165530e-13, (float)1.90916947e-08, (float)-4.38539758e-04, (float)5.02717412e+00, (float)-2.30219791e+04};

float convert_t_motor(float val) {
    return (val - 9393.9f) / 55.1f;
}
float convert_t_igbt(float val) {
    return (powf((val), 5) * IGBT_TEMP_COEFFICIENT[0]) + (powf((val), 4) * IGBT_TEMP_COEFFICIENT[1]) + (powf((val), 3) * IGBT_TEMP_COEFFICIENT[2]) +
           (powf((val), 2) * IGBT_TEMP_COEFFICIENT[3]) + (val * IGBT_TEMP_COEFFICIENT[4]) + IGBT_TEMP_COEFFICIENT[5];
}

void inv_l_rcv_update(void) {
    GET_LAST_STATE(inverters, inv_l_rcv, INVERTERS, INV_L_RCV);
    l_motor_temp = convert_t_motor(inverters_inv_l_rcv_last_state->t_motor);
    snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "%.1f", l_motor_temp);
    set_tab_sensors_label_text(snprintf_buffer, tab_sensors_lb_left_motor_temp);
    if (r_motor_temp != INVERTER_MESSAGE_UNINITIALIZED) {
        snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "%.0f", (l_motor_temp + r_motor_temp) / 2.0f);
        set_tab_racing_label_text(snprintf_buffer, tab_rac_mot_idx);
    }
    l_igbt_temp = convert_t_igbt(inverters_inv_l_rcv_last_state->t_igbt);
    snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "%.1f", l_igbt_temp);
    set_tab_sensors_label_text(snprintf_buffer, tab_sensors_lb_left_inverter_temp);
    if (r_igbt_temp != INVERTER_MESSAGE_UNINITIALIZED) {
        snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "%.0f", (l_igbt_temp + r_igbt_temp) / 2.0f);
        set_tab_racing_label_text(snprintf_buffer, tab_rac_inv_idx);
    }
}

void inv_r_rcv_update(void) {
    GET_LAST_STATE(inverters, inv_r_rcv, INVERTERS, INV_R_RCV);
    r_motor_temp = convert_t_motor(inverters_inv_r_rcv_last_state->t_motor);
    snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "%.1f", r_motor_temp);
    set_tab_sensors_label_text(snprintf_buffer, tab_sensors_lb_right_motor_temp);
    r_igbt_temp = convert_t_igbt(inverters_inv_r_rcv_last_state->t_igbt);
    snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "%.1f", r_igbt_temp);
    set_tab_sensors_label_text(snprintf_buffer, tab_sensors_lb_right_inverter_temp);
}

void irts_fl_update() {
    GET_LAST_STATE(secondary, irts_fl_0, SECONDARY, IRTS_FL_0);
    GET_LAST_STATE(secondary, irts_fl_1, SECONDARY, IRTS_FL_1);
    GET_LAST_STATE(secondary, irts_fl_2, SECONDARY, IRTS_FL_2);
    GET_LAST_STATE(secondary, irts_fl_3, SECONDARY, IRTS_FL_3);
    float avg_temp = (secondary_irts_fl_0_last_state->channel2 + secondary_irts_fl_0_last_state->channel3 + secondary_irts_fl_0_last_state->channel4 +
                      secondary_irts_fl_1_last_state->channel5 + secondary_irts_fl_1_last_state->channel6 + secondary_irts_fl_1_last_state->channel7 +
                      secondary_irts_fl_1_last_state->channel8 + secondary_irts_fl_2_last_state->channel9 + secondary_irts_fl_2_last_state->channel10 +
                      secondary_irts_fl_2_last_state->channel11 + secondary_irts_fl_2_last_state->channel12 + secondary_irts_fl_3_last_state->channel13 +
                      secondary_irts_fl_3_last_state->channel14 + secondary_irts_fl_3_last_state->channel15) /
                     14.0f;
    snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "%.0f", avg_temp);
    set_tab_sensors_label_text(snprintf_buffer, tab_sensors_lb_fl_temp);
}

void irts_fr_update() {
    GET_LAST_STATE(secondary, irts_fr_0, SECONDARY, IRTS_FR_0);
    GET_LAST_STATE(secondary, irts_fr_1, SECONDARY, IRTS_FR_1);
    GET_LAST_STATE(secondary, irts_fr_2, SECONDARY, IRTS_FR_2);
    GET_LAST_STATE(secondary, irts_fr_3, SECONDARY, IRTS_FR_3);
    float avg_temp = (secondary_irts_fr_0_last_state->channel2 + secondary_irts_fr_0_last_state->channel3 + secondary_irts_fr_0_last_state->channel4 +
                      secondary_irts_fr_1_last_state->channel5 + secondary_irts_fr_1_last_state->channel6 + secondary_irts_fr_1_last_state->channel7 +
                      secondary_irts_fr_1_last_state->channel8 + secondary_irts_fr_2_last_state->channel9 + secondary_irts_fr_2_last_state->channel10 +
                      secondary_irts_fr_2_last_state->channel11 + secondary_irts_fr_2_last_state->channel12 + secondary_irts_fr_3_last_state->channel13 +
                      secondary_irts_fr_3_last_state->channel14 + secondary_irts_fr_3_last_state->channel15) /
                     14.0f;
    snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "%.0f", avg_temp);
    set_tab_sensors_label_text(snprintf_buffer, tab_sensors_lb_fr_temp);
}

void irts_rl_update() {
    GET_LAST_STATE(secondary, irts_rl_0, SECONDARY, IRTS_RL_0);
    GET_LAST_STATE(secondary, irts_rl_1, SECONDARY, IRTS_RL_1);
    GET_LAST_STATE(secondary, irts_rl_2, SECONDARY, IRTS_RL_2);
    GET_LAST_STATE(secondary, irts_rl_3, SECONDARY, IRTS_RL_3);
    float avg_temp = (secondary_irts_rl_0_last_state->channel2 + secondary_irts_rl_0_last_state->channel3 + secondary_irts_rl_0_last_state->channel4 +
                      secondary_irts_rl_1_last_state->channel5 + secondary_irts_rl_1_last_state->channel6 + secondary_irts_rl_1_last_state->channel7 +
                      secondary_irts_rl_1_last_state->channel8 + secondary_irts_rl_2_last_state->channel9 + secondary_irts_rl_2_last_state->channel10 +
                      secondary_irts_rl_2_last_state->channel11 + secondary_irts_rl_2_last_state->channel12 + secondary_irts_rl_3_last_state->channel13 +
                      secondary_irts_rl_3_last_state->channel14 + secondary_irts_rl_3_last_state->channel15) /
                     14.0f;

    snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "%.0f", avg_temp);
    set_tab_sensors_label_text(snprintf_buffer, tab_sensors_lb_rl_temp);
}

void irts_rr_update() {
    GET_LAST_STATE(secondary, irts_rr_0, SECONDARY, IRTS_RR_0);
    GET_LAST_STATE(secondary, irts_rr_1, SECONDARY, IRTS_RR_1);
    GET_LAST_STATE(secondary, irts_rr_2, SECONDARY, IRTS_RR_2);
    GET_LAST_STATE(secondary, irts_rr_3, SECONDARY, IRTS_RR_3);
    float avg_temp = (secondary_irts_rr_0_last_state->channel2 + secondary_irts_rr_0_last_state->channel3 + secondary_irts_rr_0_last_state->channel4 +
                      secondary_irts_rr_1_last_state->channel5 + secondary_irts_rr_1_last_state->channel6 + secondary_irts_rr_1_last_state->channel7 +
                      secondary_irts_rr_1_last_state->channel8 + secondary_irts_rr_2_last_state->channel9 + secondary_irts_rr_2_last_state->channel10 +
                      secondary_irts_rr_2_last_state->channel11 + secondary_irts_rr_2_last_state->channel12 + secondary_irts_rr_3_last_state->channel13 +
                      secondary_irts_rr_3_last_state->channel14 + secondary_irts_rr_3_last_state->channel15) /
                     14.0f;
    snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "%.0f", avg_temp);
    set_tab_sensors_label_text(snprintf_buffer, tab_sensors_lb_rr_temp);
}

void update_sensors_extra_value(const char *buf, uint8_t extra_value) {
    switch (extra_value) {
        case 0: {
            break;
        }
        case 1: {
            break;
        }
        case 2: {
            break;
        }
        default: {
            break;
        }
    }
}

void odometer_update(void) {
    GET_LAST_STATE(secondary, odometer, SECONDARY, ODOMETER);
    snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "%.1f", secondary_odometer_last_state->kilometers);
    set_tab_racing_label_text(snprintf_buffer, tab_rac_odometer_idx);
}

/***
 * PTT
 */

typedef enum {
    ptt_status_OFF     = 0,
    ptt_status_SET_ON  = 1,
    ptt_status_ON      = 2,
    ptt_status_SET_OFF = 3,
} ptt_status_t;

bool ecu_ack            = false;
bool ptt_button_pressed = false;
ptt_status_t ptt_status = ptt_status_OFF;

#include "can_messages.h"

void set_ptt_button_pressed(bool val) {
    ptt_button_pressed = val;
}

void send_ptt_status_message(bool on) {
    primary_ecu_set_ptt_status_converted_t converted = {0};
    converted.status                                 = on ? primary_ecu_set_ptt_status_status_on : primary_ecu_set_ptt_status_status_off;
    STEER_CAN_PACK(primary, PRIMARY, ecu_set_ptt_status, ECU_SET_PTT_STATUS);
    can_send(&msg, true);
}

void ptt_tasks_fn(lv_timer_t *timer) {
    if (!ecu_ack && ptt_button_pressed) {
        ptt_status = ptt_status_SET_ON;
        send_ptt_status_message(true);
        update_sensors_extra_value("X", 0);
        set_tab_racing_label_text("X", tab_rac_ptt_status_idx);
    } else if (ecu_ack && !ptt_button_pressed) {
        ptt_status = ptt_status_SET_OFF;
        send_ptt_status_message(false);
        update_sensors_extra_value("X", 0);
        set_tab_racing_label_text("X", tab_rac_ptt_status_idx);
    } else if (ecu_ack && ptt_button_pressed) {
        ptt_status = ptt_status_ON;
        send_ptt_status_message(true);
        update_sensors_extra_value("ON", 0);
        set_tab_racing_label_text("ON", tab_rac_ptt_status_idx);
    } else if (!ecu_ack && !ptt_button_pressed) {
        ptt_status = ptt_status_OFF;
        send_ptt_status_message(false);
        update_sensors_extra_value("OFF", 0);
        set_tab_racing_label_text("OFF", tab_rac_ptt_status_idx);
    }
}

void handle_ptt_message(void) {
    GET_LAST_STATE(primary, ecu_ptt_status, PRIMARY, ECU_PTT_STATUS);
    primary_ecu_ptt_status_status val = primary_ecu_ptt_status_last_state->status;
    if (val == primary_ecu_ptt_status_status_off) {
        ecu_ack = false;
    } else if (val == primary_ecu_ptt_status_status_on) {
        ecu_ack = true;
    }
}