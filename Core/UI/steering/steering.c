#include "steering.h"
#include "steering_config.h"

lv_obj_t *lb_estimated_velocity;
lv_obj_t *lb_apps;
lv_obj_t *lb_bse;

extern bool steering_initialized;

#define SNPRINTF_BUFFER_SIZE (64u)
char snprintf_buffer[SNPRINTF_BUFFER_SIZE];

uint8_t primary_messages_last_state[primary_MESSAGE_COUNT]
                                   [primary_MAX_STRUCT_SIZE_CONVERSION];
uint8_t secondary_messages_last_state[secondary_MESSAGE_COUNT]
                                     [secondary_MAX_STRUCT_SIZE_CONVERSION];
uint8_t inverters_messages_last_state[inverters_MESSAGE_COUNT]
                                     [inverters_MAX_STRUCT_SIZE_CONVERSION];

// extern int torque_vectoring_last_state;
// extern int slip_control_last_state;
extern int set_pumps_speed_last_state;
extern int set_radiators_last_state;
extern int pork_fans_status_last_state;
// extern int power_map_last_state;

primary_lv_cells_voltage_converted_t lv_voltages_stock_1 = {0};
primary_lv_cells_voltage_converted_t lv_voltages_stock_2 = {0};
primary_lv_cells_temp_converted_t lv_temps_stock_1 = {0};
primary_lv_cells_temp_converted_t lv_temps_stock_2 = {0};

uint32_t timestamp_start_lap = 0;

void set_lb_estimated_velocity(const char *s) {
  lv_label_set_text(lb_estimated_velocity, s);
}

void set_lb_apps(const char *s) { lv_label_set_text(lb_apps, s); }

void set_lb_bse(const char *s) { lv_label_set_text(lb_bse, s); }

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

void lv_pumps_actual_value_update() {}

void lv_radiators_actual_value_update() {}

void tlm_status_update() {
  GET_LAST_STATE(primary, tlm_status, PRIMARY, TLM_STATUS);
  if (primary_tlm_status_last_state->status == primary_tlm_status_status_on) {
    snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "ON");
    all_leds_green();
  } else {
    snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "OFF");
    all_leds_red();
  }
  set_tab_sensors_label_text(snprintf_buffer, tab_sensors_lb_tlm_status);
}

void speed_update(void) {
  GET_LAST_STATE(secondary, angular_velocity, SECONDARY, ANGULAR_VELOCITY);
  GET_LAST_STATE(primary, ecu_status, PRIMARY, ECU_STATUS);
  if (primary_ecu_status_last_state->status !=
      primary_ecu_status_status_drive) {
    return;
  }
  float speed = fabs((secondary_angular_velocity_last_state->fl +
                      secondary_angular_velocity_last_state->fr) /
                     2.0f) *
                3.6f * 0.203f;
  set_tab_racing_label_text("KM/H", tab_rac_bottom_status_idx);
  snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "%.0f", speed);
  set_tab_racing_label_text(snprintf_buffer, tab_rac_status_idx);
  set_tab_track_test_lb_speed(snprintf_buffer);
}

void hv_debug_signals_update(void) {
  GET_LAST_STATE(primary, hv_debug_signals, PRIMARY, HV_DEBUG_SIGNALS);
  // primary_hv_debug_signals_last_state
  // tab_hv_set_error_status(debug_signal_error_cell_low_voltage, primary_hv_debug_signals_last_state->errors_cell_low_voltage);
  tab_hv_set_error_status(debug_signal_error_cell_under_voltage, primary_hv_debug_signals_last_state->errors_cell_under_voltage);
  tab_hv_set_error_status(debug_signal_error_cell_over_voltage, primary_hv_debug_signals_last_state->errors_cell_over_voltage);
  // tab_hv_set_error_status(debug_signal_error_cell_high_temperature, primary_hv_debug_sigdnals_last_state->errors_cell_high_temperature);
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

void hv_cell_voltage_update(void) {
  GET_LAST_STATE(primary, hv_cells_voltage_stats, PRIMARY,
                 HV_CELLS_VOLTAGE_STATS);
  snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "%.1f",
           primary_hv_cells_voltage_stats_last_state->min);

  set_tab_sensors_label_text(snprintf_buffer, tab_sensors_lb_min_cell_voltage);
  set_tab_hv_label_text(snprintf_buffer, tab_hv_lb_voltage_min);

  snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "%.1f",
           primary_hv_cells_voltage_stats_last_state->max);
  set_tab_hv_label_text(snprintf_buffer, tab_hv_lb_voltage_max);

  /*
  float delta = primary_hv_cells_voltage_stats_last_state->max -
                primary_hv_cells_voltage_stats_last_state->min;
  snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "%d", (int)(delta * 1000.0f));
  set_tab_sensors_label_text(snprintf_buffer, tab_sensors_lb_hv_delta);
  set_tab_hv_label_text(snprintf_buffer, tab_hv_lb_voltage_delta);
  */

  snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "%d",
           (int)(primary_hv_cells_voltage_stats_last_state->delta * 1000.0f));
  set_tab_sensors_label_text(snprintf_buffer, tab_sensors_lb_hv_delta);
  set_tab_hv_label_text(snprintf_buffer, tab_hv_lb_voltage_delta);
}

void hv_voltage_update(void) {
  GET_LAST_STATE(primary, hv_total_voltage, PRIMARY, HV_TOTAL_VOLTAGE);

  precharge_bar_update((int32_t) primary_hv_total_voltage_last_state->pack);
  snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "%.0f",
           primary_hv_total_voltage_last_state->pack);
  set_tab_racing_label_text(snprintf_buffer, tab_rac_pack_voltage_idx);
  set_tab_sensors_label_text(snprintf_buffer, tab_sensors_lb_pack_voltage);
  set_tab_racing_hv_pack_voltage_bar(primary_hv_total_voltage_last_state->pack);

  set_tab_hv_label_text(snprintf_buffer, tab_hv_lb_pack_voltage);
}

void hv_current_update() {
  GET_LAST_STATE(primary, hv_current, PRIMARY, HV_CURRENT);
  snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "%.1f",
           primary_hv_current_last_state->current);
  set_tab_sensors_label_text(snprintf_buffer, tab_sensors_lb_hv_current);
  set_tab_racing_label_text(snprintf_buffer, tab_rac_hv_curr_idx);
  set_tab_racing_hv_current_bar(primary_hv_current_last_state->current);
}

void hv_temp_update() {
  GET_LAST_STATE(primary, hv_cells_temp_stats, PRIMARY, HV_CELLS_TEMP_STATS);

  snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "%0.f",
           primary_hv_cells_temp_stats_last_state->avg);
  set_tab_racing_label_text(snprintf_buffer, tab_rac_hv_avg_temp_idx);
  set_tab_sensors_label_text(snprintf_buffer,
                             tab_sensors_lb_average_temperature);
  set_tab_hv_label_text(snprintf_buffer, tab_hv_lb_temp_avg);

  snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "%0.f",
           primary_hv_cells_temp_stats_last_state->max);
  set_tab_hv_label_text(snprintf_buffer, tab_hv_lb_temp_max);

  snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "%0.f",
           primary_hv_cells_temp_stats_last_state->min);
  set_tab_hv_label_text(snprintf_buffer, tab_hv_lb_temp_min);
}

bool cellboard_bal[N_PORK_CELLBOARD] = {0};

void set_bal_status_label_text(char *text);

void hv_cell_balancing_status_update() {
  GET_LAST_STATE(primary, hv_balancing_status, PRIMARY, HV_BALANCING_STATUS);
  uint8_t cellboard_id =
      (uint8_t)primary_hv_balancing_status_last_state->cellboard_id;
  if (cellboard_id < 0 || cellboard_id >= N_PORK_CELLBOARD) {
    return;
  }
  primary_hv_balancing_status_balancing_status status =
      primary_hv_balancing_status_last_state->balancing_status;
  cellboard_bal[cellboard_id] =
      status == primary_hv_balancing_status_balancing_status_off ? false : true;
  bool is_bal = false;
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

void hv_feedbacks_status_update() {
  GET_LAST_STATE(primary, hv_feedback_status, PRIMARY, HV_FEEDBACK_STATUS);
  update_shutdown_circuit_component(
      feedbacks_status_feedback_sd_in_index,
      primary_hv_feedback_status_last_state->feedback_sd_in);
  update_shutdown_circuit_component(
      feedbacks_status_feedback_sd_out_index,
      primary_hv_feedback_status_last_state->feedback_sd_out);
  update_shutdown_circuit_component(
      feedbacks_status_feedback_sd_end_index,
      primary_hv_feedback_status_last_state->feedback_sd_end);
  update_shutdown_circuit_component(
      feedbacks_status_feedback_precharge_status_index,
      primary_hv_feedback_status_last_state->feedback_precharge_status);
  update_shutdown_circuit_component(
      feedbacks_status_feedback_airp_gate_index,
      primary_hv_feedback_status_last_state->feedback_airp_status);
  update_shutdown_circuit_component(
      feedbacks_status_feedback_airn_gate_index,
      primary_hv_feedback_status_last_state->feedback_airn_status);

}

primary_ecu_feedbacks_converted_t ecu_feedbacks_last_state = {0};

void ecu_feedbacks_update(void) {
  // TODO RICKY
  // GET_LAST_STATE(primary, ecu_feedbacks, PRIMARY, ECU_FEEDBACKS);
  // update_shutdown_circuit_component(ecu_feedbacks_sd_in_index,
  // primary_ecu_feedbacks_last_state->ecu_feedbacks_sd_in);
  // update_shutdown_circuit_component(ecu_feedbacks_sd_cock_fb_index,
  // primary_ecu_feedbacks_last_state->ecu_feedbacks_sd_cock_fb);
  // update_shutdown_circuit_component(ecu_feedbacks_sd_interial_fb_index,
  // primary_ecu_feedbacks_last_state->ecu_feedbacks_sd_interial_fb);
  // update_shutdown_circuit_component(ecu_feedbacks_sd_bots_fb_index,
  // primary_ecu_feedbacks_last_state->ecu_feedbacks_sd_bots_fb);
}

void ts_status_update() {
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
  float cval = primary_hv_fans_status_last_state->fans_speed;
  pork_fans_status_last_state = (int)(cval * 100.0f);
  if (cval < 0) {
    snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "AUTO");
    tab_hv_set_pork_speed_bar(0, true);
  } else {
    snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "%0.1f",
             primary_hv_fans_status_last_state->fans_speed);
    tab_hv_set_pork_speed_bar((int32_t)(cval * 100), false);
  }
  set_tab_hv_label_text(snprintf_buffer, tab_hv_pork_speed_value);
}

void lv_feedback_sd_voltage_update() {
  GET_LAST_STATE(primary, lv_feedback_sd_voltage, PRIMARY,
                 LV_FEEDBACK_SD_VOLTAGE);

  update_shutdown_circuit_component(
      sd_start_index, primary_lv_feedback_sd_voltage_last_state->sd_start);
  update_shutdown_circuit_component(
      feedbacks_interlock_fb_index,
      primary_lv_feedback_sd_voltage_last_state->interlock);
  // update_shutdown_circuit_component(feedbacks_invc_lid_fb_index,
  // primary_lv_feedback_sd_voltage_last_state->feedbacks_invc_lid_fb);
  // update_shutdown_circuit_component(feedbacks_hvd_fb_index,
  // primary_lv_feedback_sd_voltage_last_state->feedbacks_hvd_fb);
  // update_shutdown_circuit_component(feedbacks_bspd_fb_index,
  // primary_lv_feedback_sd_voltage_last_state->feedbacks_bspd_fb);
  // update_shutdown_circuit_component(feedbacks_invc_interlock_fb_index,
  // primary_lv_feedback_sd_voltage_last_state->feedbacks_invc_interlock_fb);
  update_shutdown_circuit_component(
      sd_end_index, primary_lv_feedback_sd_voltage_last_state->sd_end);
}

extern primary_lv_radiator_speed_converted_t steering_wheel_state_radiator_speed;
extern primary_lv_pumps_speed_converted_t steering_wheel_state_pumps_speed;
extern uint32_t steering_wheel_lv_pumps_speed_sent_timestamp;
extern uint32_t steering_wheel_lv_radiators_speed_sent_timestamp;
extern steering_wheel_cooling_status_t steering_wheel_lv_pumps_speed_state;
extern steering_wheel_cooling_status_t steering_wheel_lv_radiator_speed_state;


void lv_pumps_speed_update_all_graphics(primary_lv_pumps_speed_converted_t* msg) {
  snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "%0.1f", msg->pumps_speed);
  set_tab_lv_label_text(snprintf_buffer, tab_lv_lb_pumps_actual);
  if (msg->status == primary_lv_pumps_speed_status_off || msg->status == primary_lv_pumps_speed_status_auto) {
    snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "AUTO");
    set_tab_lv_label_text(snprintf_buffer, tab_lv_lb_pumps_local);  
  } else {
    snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "%0.1f", msg->pumps_speed);
    set_tab_lv_label_text(snprintf_buffer, tab_lv_lb_pumps_local);
  }
  lv_set_pumps_speed_bar((int32_t) (msg->pumps_speed * 100.0f));
}

#define ALMOST_ERROR (0.05f)
#define IS_ALMOST_EQUAL(val1, val2) ((fabs(val1 - val2) < ALMOST_ERROR))
/*
  - valori diversi - timeout passato -> errore, aggiorno e cambio stato (1)
  - valori diversi - timeout non passato -> aggiorno solo i valori del porchetto (2)
  - valori uguali - timeout passato -> aggiorno e cambio stato (3)
  - valori uguali - timeout non passato -> aggiorno e cambio stato (3)
  */
void lv_pumps_speed_update(void) {
  return;
  GET_LAST_STATE(primary, lv_pumps_speed, PRIMARY, LV_PUMPS_SPEED);
  float actual_speed = primary_lv_pumps_speed_last_state->pumps_speed;
  float actual_status = primary_lv_pumps_speed_last_state->status;


  switch (steering_wheel_lv_pumps_speed_state) {
  case STEERING_WHEEL_COOLING_STATUS_SYNC: {
    steering_wheel_state_pumps_speed.pumps_speed = actual_speed;
    steering_wheel_state_pumps_speed.status = actual_status;
    lv_pumps_speed_update_all_graphics(&steering_wheel_state_pumps_speed);
    break;
  }
  case STEERING_WHEEL_COOLING_STATUS_SET: {
    if ((!IS_ALMOST_EQUAL (steering_wheel_state_pumps_speed.pumps_speed, actual_speed)) &&
        !IS_ALMOST_EQUAL(steering_wheel_state_pumps_speed.status, actual_status) &&
        ((get_current_time_ms() - steering_wheel_lv_pumps_speed_sent_timestamp) > COOLING_STATE_SYNC_TIMEOUT)) {

      display_notification("Porket does not respond on pumps settings", 500);
      steering_wheel_state_pumps_speed.pumps_speed = actual_speed;
      steering_wheel_state_pumps_speed.status = actual_status;
      steering_wheel_lv_pumps_speed_state = STEERING_WHEEL_COOLING_STATUS_SYNC;
      lv_pumps_speed_update_all_graphics(&steering_wheel_state_pumps_speed);
    } else if ((steering_wheel_state_pumps_speed.pumps_speed != actual_speed) &&
                (steering_wheel_state_pumps_speed.status != actual_status)){
    
      // snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "%0.1f", actual_speed);
      // set_tab_lv_label_text(snprintf_buffer, tab_lv_lb_pumps_actual);
      // lv_set_pumps_speed_bar((int32_t) (actual_speed * 100.0f));
    } else {
      lv_pumps_speed_update_all_graphics(&steering_wheel_state_pumps_speed);
      steering_wheel_lv_pumps_speed_state = STEERING_WHEEL_COOLING_STATUS_SYNC;
      }
    break;
  }
  }
}

void lv_radiator_speed_update_all_graphics(primary_lv_radiator_speed_converted_t* msg) {
  snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "%0.1f", msg->radiator_speed);
  set_tab_lv_label_text(snprintf_buffer, tab_lv_lb_radiators_actual);
  if (msg->status == primary_lv_radiator_speed_status_off || msg->status == primary_lv_radiator_speed_status_auto) {
    snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "AUTO");
    set_tab_lv_label_text(snprintf_buffer, tab_lv_lb_radiators_local);  
  } else {
    snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "%0.1f", msg->radiator_speed);
    set_tab_lv_label_text(snprintf_buffer, tab_lv_lb_radiators_local);
  }
  lv_set_radiators_speed_bar((int32_t) (msg->radiator_speed * 100.0f));
}

void lv_radiator_speed_update(void) {
  return;
  GET_LAST_STATE(primary, lv_radiator_speed, PRIMARY, LV_RADIATOR_SPEED);
  float actual_speed = primary_lv_radiator_speed_last_state->radiator_speed;
  float actual_status = primary_lv_radiator_speed_last_state->status;

  switch (steering_wheel_lv_radiator_speed_state) {
  case STEERING_WHEEL_COOLING_STATUS_SYNC: {
    steering_wheel_state_radiator_speed.radiator_speed = actual_speed;
    steering_wheel_state_radiator_speed.status = actual_status;
    lv_radiator_speed_update_all_graphics(&steering_wheel_state_radiator_speed);
    break;
  }
  case STEERING_WHEEL_COOLING_STATUS_SET: {
    if (!IS_ALMOST_EQUAL(steering_wheel_state_radiator_speed.radiator_speed, actual_speed) &&
        !IS_ALMOST_EQUAL(steering_wheel_state_radiator_speed.status, actual_status) &&
        ((get_current_time_ms() - steering_wheel_lv_radiators_speed_sent_timestamp) > COOLING_STATE_SYNC_TIMEOUT)) {

      display_notification("Porket does not respond on radiator settings", 500);
      steering_wheel_state_radiator_speed.radiator_speed = actual_speed;
      steering_wheel_state_radiator_speed.status = actual_status;
      steering_wheel_lv_radiator_speed_state = STEERING_WHEEL_COOLING_STATUS_SYNC;
      lv_radiator_speed_update_all_graphics(&steering_wheel_state_radiator_speed);
    } else if ((steering_wheel_state_radiator_speed.radiator_speed != actual_speed) &&
                (steering_wheel_state_radiator_speed.status != actual_status)){
    
    // snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "%0.1f", actual_speed);
    // set_tab_lv_label_text(snprintf_buffer, tab_lv_lb_radiators_actual);
    // lv_set_radiators_speed_bar((int32_t) (actual_speed * 100.0f));
    } else {
      lv_radiator_speed_update_all_graphics(&steering_wheel_state_radiator_speed);
      steering_wheel_lv_radiator_speed_state = STEERING_WHEEL_COOLING_STATUS_SYNC;
      }
    break;
  }
  }
}

void lv_currents_update() {
  GET_LAST_STATE(primary, lv_current_battery, PRIMARY, LV_CURRENT_BATTERY);
  snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "%.1f", primary_lv_current_battery_last_state->lv_current);
  set_tab_sensors_label_text(snprintf_buffer, tab_sensors_lb_lv_current);
  set_tab_lv_label_text(snprintf_buffer, tab_lv_lb_pack_voltage_2);
}

void lv_total_voltage_update() {
  GET_LAST_STATE(primary, lv_total_voltage, PRIMARY, LV_TOTAL_VOLTAGE);
  snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "%.1f", primary_lv_total_voltage_last_state->total);
  set_tab_sensors_label_text(snprintf_buffer, tab_sensors_lb_voltage);
}

void lv_cells_voltage_update(void) {
#define N_LV_CELLS 6
  float current_lv_voltages[N_LV_CELLS] = {0};
  current_lv_voltages[0] = lv_voltages_stock_1.voltage_0;
  current_lv_voltages[1] = lv_voltages_stock_1.voltage_1;
  current_lv_voltages[2] = lv_voltages_stock_1.voltage_2;
  current_lv_voltages[3] = lv_voltages_stock_2.voltage_0;
  current_lv_voltages[4] = lv_voltages_stock_2.voltage_1;
  current_lv_voltages[5] = lv_voltages_stock_2.voltage_2;

  float sum = 0;
  for (uint8_t temp_index = 0; temp_index < N_LV_CELLS; temp_index++)
    sum += current_lv_voltages[temp_index];

  snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "%0.0f", sum);
  set_tab_lv_label_text(snprintf_buffer, tab_lv_lb_pack_voltage);

  float mean_voltage = (float)(sum / N_LV_CELLS);
  snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "%.0f", mean_voltage);
  set_tab_racing_label_text(snprintf_buffer, tab_rac_lv_temp_idx);
  set_tab_sensors_label_text(snprintf_buffer,
                             tab_sensors_lb_battery_temperature);

  // set_tab_lv_label_text(snprintf_buffer, tab_lv_lb_);
}

void lv_cells_voltage_stats_update() {
  GET_LAST_STATE(primary, lv_cells_voltage_stats, PRIMARY,
                 LV_CELLS_VOLTAGE_STATS);
  snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "%.1f",
           primary_lv_cells_voltage_stats_last_state->max);
  set_tab_lv_label_text(snprintf_buffer, tab_lv_lb_voltage_max);

  snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "%.1f",
           primary_lv_cells_voltage_stats_last_state->min);
  set_tab_lv_label_text(snprintf_buffer, tab_lv_lb_voltage_min);

  snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "%.1f",
           primary_lv_cells_voltage_stats_last_state->delta);
  set_tab_lv_label_text(snprintf_buffer, tab_lv_lb_voltage_delta);
}

void lv_cells_temp_update() {
  float current_lv_temp[N_LV_CELLS] = {0};
  current_lv_temp[0] = lv_temps_stock_1.temp_0;
  current_lv_temp[1] = lv_temps_stock_1.temp_1;
  current_lv_temp[2] = lv_temps_stock_1.temp_2;
  current_lv_temp[3] = lv_temps_stock_2.temp_0;
  current_lv_temp[4] = lv_temps_stock_2.temp_1;
  current_lv_temp[5] = lv_temps_stock_2.temp_2;

  float sum = 0;
  for (uint8_t temp_index = 0; temp_index < N_LV_CELLS; temp_index++)
    sum += current_lv_temp[temp_index];

  for (size_t idx = 0; idx < N_LV_CELLS; idx++) {
  }

  float mean_temp = (float)(sum / N_LV_CELLS);
  snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "%.0f", mean_temp);
  set_tab_racing_label_text(snprintf_buffer, tab_rac_lv_temp_idx);
  set_tab_sensors_label_text(snprintf_buffer,
                             tab_sensors_lb_battery_temperature);
  set_tab_lv_label_text(snprintf_buffer, tab_lv_lb_temp_avg);
}

void lv_cells_temp_stats_update() {
  GET_LAST_STATE(primary, lv_cells_temp_stats, PRIMARY, LV_CELLS_TEMP_STATS);

  snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "%.1f",
           primary_lv_cells_temp_stats_last_state->max);
  set_tab_lv_label_text(snprintf_buffer, tab_lv_lb_temp_max);

  snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "%.1f",
           primary_lv_cells_temp_stats_last_state->min);
  set_tab_lv_label_text(snprintf_buffer, tab_lv_lb_temp_min);

  snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "%.1f",
           primary_lv_cells_temp_stats_last_state->avg);
  set_tab_lv_label_text(snprintf_buffer, tab_lv_lb_temp_avg);
}

void lv_feedbacks_update() {
  // TODO RICKY
  // GET_LAST_STATE(primary, lv_feedbacks, PRIMARY, LV_FEEDBACKS);
  // update_shutdown_circuit_component(sd_start_index,
  // primary_lv_feedbacks_last_state->sd_start);
  // update_shutdown_circuit_component(feedbacks_interlock_fb_index,
  // primary_lv_feedbacks_last_state->feedbacks_interlock_fb);
  // update_shutdown_circuit_component(feedbacks_invc_lid_fb_index,
  // primary_lv_feedbacks_last_state->feedbacks_invc_lid_fb);
  // update_shutdown_circuit_component(feedbacks_hvd_fb_index,
  // primary_lv_feedbacks_last_state->feedbacks_hvd_fb);
  // update_shutdown_circuit_component(feedbacks_bspd_fb_index,
  // primary_lv_feedbacks_last_state->feedbacks_bspd_fb);
  // update_shutdown_circuit_component(feedbacks_invc_interlock_fb_index,
  // primary_lv_feedbacks_last_state->feedbacks_invc_interlock_fb);
  // update_shutdown_circuit_component(sd_end_index,
  // primary_lv_feedbacks_last_state->sd_end);
}

void steering_angle_update() {
  GET_LAST_STATE(secondary, steer_angle, SECONDARY, STEER_ANGLE);
  snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "%.1f",
           secondary_steer_angle_last_state->angle);
  set_tab_track_test_steering_angle_bar(
      secondary_steer_angle_last_state->angle);
#if STEER_TAB_CALIBRATION_ENABLED == 1
  set_tab_calibration_lb_steering_angle(snprintf_buffer);
  calibration_box_t *curr_focus = get_tab_calibration_curr_focus();
  if (*curr_focus == STEER) {
    lv_slider_set_mode(get_tab_calibration_slider(), LV_BAR_MODE_SYMMETRICAL);
    lv_slider_set_range(
        get_tab_calibration_slider(), STEERING_ANGLE_RANGE_LOW,
        STEERING_ANGLE_RANGE_HIGH); // se range 45 e max value 180 ->
                                    // set_value ( 0.25 * gradi_inclinazione )
    lv_slider_set_value(get_tab_calibration_slider(),
                        secondary_steering_angle_last_state.angle, LV_ANIM_OFF);
  }
#endif
}

void pedals_output_update() {
#if STEER_TAB_CALIBRATION_ENABLED == 1
  GET_LAST_STATE(secondary, pedals_output, SECONDARY, PEDALS_OUTPUT);
  snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "%d",
           (int)secondary_pedals_output_last_state->apps);
  calibration_box_t *curr_focus = get_tab_calibration_curr_focus();

  if (*curr_focus == APPS) {
    lv_slider_set_mode(get_tab_calibration_slider(), LV_BAR_MODE_RANGE);
    lv_slider_set_range(get_tab_calibration_slider(), APPS_RANGE_LOW,
                        APPS_RANGE_HIGH);
    lv_slider_set_value(get_tab_calibration_slider(),
                        secondary_pedals_output_last_state->apps, LV_ANIM_OFF);
  }
  snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "%.1f",
           secondary_pedals_output_last_state->bse_front);
  if (*curr_focus == BSE) {
    lv_slider_set_mode(get_tab_calibration_slider(), LV_BAR_MODE_RANGE);
    lv_slider_set_range(get_tab_calibration_slider(), BRAKE_RANGE_LOW,
                        BRAKE_RANGE_HIGH);
    lv_slider_set_value(get_tab_calibration_slider(),
                        secondary_pedals_output_last_state->bse_front,
                        LV_ANIM_OFF);
  }
#endif
}

void imu_acceleration_update() {}

void lap_count_update() {
  // TODO RICKY
  /* GET_LAST_STATE(secondary, lap_count, SECONDARY, LAP_COUNT);
  GET_LAST_STATE(secondary, lc_status, SECONDARY, LC_STATUS);
  keep_lap_counter_value(2000);
  timestamp_start_lap = get_current_time_ms();
  float last_time_seconds = secondary_lap_count_last_state->lap_time;
  int minutes = (int)(last_time_seconds / 60.0f);
  int seconds = (int)(last_time_seconds - minutes * 60.0f);
  snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "%02d:%02d", minutes,
           seconds);

  set_tab_racing_label_text(snprintf_buffer, tab_rac_last_time_idx);

  float delta = last_time_seconds - secondary_lc_status_last_state->best_time;
  snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "%+.2f", delta);

  set_tab_racing_label_text(snprintf_buffer, tab_rac_dtime_idx); */
}

extern bool on_lap_keep;

// void lc_status_update(secondary_lc_status_converted_t *data) {
// TODO RICKY
/* GET_LAST_STATE(secondary, lc_status, SECONDARY, LC_STATUS);
int minutes = (int)(secondary_lc_status_last_state->best_time / 60.0f);
int seconds =
    (int)(secondary_lc_status_last_state->best_time - minutes * 60.0f);
snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "%02d:%02d", minutes,
         seconds);

set_tab_racing_label_text(snprintf_buffer, tab_rac_best_time_idx);

float delta = secondary_lc_status_last_state->last_time -
              secondary_lc_status_last_state->best_time;
snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "%+.2f", delta);
set_tab_racing_label_text(snprintf_buffer, tab_rac_dtime_idx);

if (secondary_lc_status_last_state->last_time !=
        secondary_lc_status_last_state->last_time &&
    !on_lap_keep) {
  float last_time_seconds = secondary_lc_status_last_state->last_time;
  secondary_lc_status_last_state->last_time = last_time_seconds;
  int minutes = (int)(last_time_seconds / 60);
  int seconds = (int)(last_time_seconds - minutes * 60);
  snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "%02d:%02d", minutes,
           seconds);

  set_tab_racing_label_text(snprintf_buffer, tab_rac_last_time_idx);
}
snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "%d",
         (int)secondary_lc_status_last_state->lap_number);
set_tab_racing_label_text(snprintf_buffer, tab_rac_lap_count_idx); */
// }

#define INVERTER_MESSAGE_UNINITIALIZED -100.0f
#define INVERTER_TEMP_CONVERSION(raw_temp)                                     \
  -43.23745 + 0.01073427 * raw_temp - 5.523417e-7 * pow(raw_temp, 2) +         \
      1.330787e-11 * pow(raw_temp, 3);
float l_motor_temp = INVERTER_MESSAGE_UNINITIALIZED;
float l_igbt_temp = INVERTER_MESSAGE_UNINITIALIZED;
float r_motor_temp = INVERTER_MESSAGE_UNINITIALIZED;
float r_igbt_temp = INVERTER_MESSAGE_UNINITIALIZED;


float IGBT_TEMP_COEFFICIENT[6] = { (float) 3.58282057e-18, (float) -4.14165530e-13, (float) 1.90916947e-08, (float) -4.38539758e-04, (float) 5.02717412e+00, (float) -2.30219791e+04 };

float convert_t_motor(float val) { return (val - 9393.9f) / 55.1f; }
float convert_t_igbt(float val) {
  return (powf((val), 5) * IGBT_TEMP_COEFFICIENT[0]) + (powf((val), 4) * IGBT_TEMP_COEFFICIENT[1]) + (powf((val), 3) * IGBT_TEMP_COEFFICIENT[2]) + (powf((val), 2) * IGBT_TEMP_COEFFICIENT[3]) + (val * IGBT_TEMP_COEFFICIENT[4]) + IGBT_TEMP_COEFFICIENT[5];
}

void inv_l_rcv_update(void) {
  GET_LAST_STATE(inverters, inv_l_rcv, INVERTERS, INV_L_RCV);
  if (inverters_inv_l_rcv_last_state->rcv_mux ==
      INVERTERS_INV_L_RCV_RCV_MUX_ID_49_T_MOTOR_CHOICE) {
    l_motor_temp = convert_t_motor(inverters_inv_l_rcv_last_state->t_motor);
    snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "%.0f", l_motor_temp);
    set_tab_racing_label_text(snprintf_buffer, tab_rac_mot_idx);

    if (r_motor_temp != INVERTER_MESSAGE_UNINITIALIZED) {
      snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "%.0f",
               (l_motor_temp + r_motor_temp) / 2.0f);
      set_tab_sensors_label_text(snprintf_buffer,
                                 tab_sensors_lb_left_motor_temp);
    }
  }

  if (inverters_inv_l_rcv_last_state->rcv_mux ==
      INVERTERS_INV_L_RCV_RCV_MUX_ID_4A_T_IGBT_CHOICE) {
    l_igbt_temp = convert_t_igbt(inverters_inv_l_rcv_last_state->t_igbt);
    snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "%.0f", l_igbt_temp);
    set_tab_sensors_label_text(snprintf_buffer,
                               tab_sensors_lb_left_inverter_temp);

    if (r_igbt_temp != INVERTER_MESSAGE_UNINITIALIZED) {
      snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "%.0f",
               (l_igbt_temp + r_igbt_temp) / 2.0f);
      set_tab_racing_label_text(snprintf_buffer, tab_rac_inv_idx);
    }
  }
}

void inv_r_rcv_update() {
  GET_LAST_STATE(inverters, inv_r_rcv, INVERTERS, INV_R_RCV);
  if (inverters_inv_r_rcv_last_state->rcv_mux ==
      INVERTERS_INV_R_RCV_RCV_MUX_ID_49_T_MOTOR_CHOICE) {
    r_motor_temp = convert_t_motor(inverters_inv_r_rcv_last_state->t_motor);
    snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "%.0f", r_motor_temp);
    set_tab_sensors_label_text(snprintf_buffer,
                               tab_sensors_lb_right_motor_temp);
  }

  if (inverters_inv_r_rcv_last_state->rcv_mux ==
      INVERTERS_INV_R_RCV_RCV_MUX_ID_4A_T_IGBT_CHOICE) {
    r_igbt_temp = convert_t_igbt(inverters_inv_r_rcv_last_state->t_igbt);
    snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "%.0f", r_igbt_temp);
    set_tab_sensors_label_text(snprintf_buffer,
                               tab_sensors_lb_right_inverter_temp);
  }
}

void irts_fl_update() {
  GET_LAST_STATE(secondary, irts_fl_0, SECONDARY, IRTS_FL_0);
  GET_LAST_STATE(secondary, irts_fl_1, SECONDARY, IRTS_FL_1);
  GET_LAST_STATE(secondary, irts_fl_2, SECONDARY, IRTS_FL_2);
  GET_LAST_STATE(secondary, irts_fl_3, SECONDARY, IRTS_FL_3);
  float avg_temp = (secondary_irts_fl_0_last_state->channel2 +
                    secondary_irts_fl_0_last_state->channel3 +
                    secondary_irts_fl_0_last_state->channel4 +
                    secondary_irts_fl_1_last_state->channel5 +
                    secondary_irts_fl_1_last_state->channel6 +
                    secondary_irts_fl_1_last_state->channel7 +
                    secondary_irts_fl_1_last_state->channel8 +
                    secondary_irts_fl_2_last_state->channel9 +
                    secondary_irts_fl_2_last_state->channel10 +
                    secondary_irts_fl_2_last_state->channel11 +
                    secondary_irts_fl_2_last_state->channel12 +
                    secondary_irts_fl_3_last_state->channel13 +
                    secondary_irts_fl_3_last_state->channel14 +
                    secondary_irts_fl_3_last_state->channel15) /
                   14.0f;
  snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "%.0f", avg_temp);
  set_tab_sensors_label_text(snprintf_buffer, tab_sensors_lb_fl_temp);
}

void irts_fr_update() {
  GET_LAST_STATE(secondary, irts_fr_0, SECONDARY, IRTS_FR_0);
  GET_LAST_STATE(secondary, irts_fr_1, SECONDARY, IRTS_FR_1);
  GET_LAST_STATE(secondary, irts_fr_2, SECONDARY, IRTS_FR_2);
  GET_LAST_STATE(secondary, irts_fr_3, SECONDARY, IRTS_FR_3);
  float avg_temp = (secondary_irts_fr_0_last_state->channel2 +
                    secondary_irts_fr_0_last_state->channel3 +
                    secondary_irts_fr_0_last_state->channel4 +
                    secondary_irts_fr_1_last_state->channel5 +
                    secondary_irts_fr_1_last_state->channel6 +
                    secondary_irts_fr_1_last_state->channel7 +
                    secondary_irts_fr_1_last_state->channel8 +
                    secondary_irts_fr_2_last_state->channel9 +
                    secondary_irts_fr_2_last_state->channel10 +
                    secondary_irts_fr_2_last_state->channel11 +
                    secondary_irts_fr_2_last_state->channel12 +
                    secondary_irts_fr_3_last_state->channel13 +
                    secondary_irts_fr_3_last_state->channel14 +
                    secondary_irts_fr_3_last_state->channel15) /
                   14.0f;
  snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "%.0f", avg_temp);
  set_tab_sensors_label_text(snprintf_buffer, tab_sensors_lb_fr_temp);
}

void irts_rl_update() {
  GET_LAST_STATE(secondary, irts_rl_0, SECONDARY, IRTS_RL_0);
  GET_LAST_STATE(secondary, irts_rl_1, SECONDARY, IRTS_RL_1);
  GET_LAST_STATE(secondary, irts_rl_2, SECONDARY, IRTS_RL_2);
  GET_LAST_STATE(secondary, irts_rl_3, SECONDARY, IRTS_RL_3);
  float avg_temp = (secondary_irts_rl_0_last_state->channel2 +
                    secondary_irts_rl_0_last_state->channel3 +
                    secondary_irts_rl_0_last_state->channel4 +
                    secondary_irts_rl_1_last_state->channel5 +
                    secondary_irts_rl_1_last_state->channel6 +
                    secondary_irts_rl_1_last_state->channel7 +
                    secondary_irts_rl_1_last_state->channel8 +
                    secondary_irts_rl_2_last_state->channel9 +
                    secondary_irts_rl_2_last_state->channel10 +
                    secondary_irts_rl_2_last_state->channel11 +
                    secondary_irts_rl_2_last_state->channel12 +
                    secondary_irts_rl_3_last_state->channel13 +
                    secondary_irts_rl_3_last_state->channel14 +
                    secondary_irts_rl_3_last_state->channel15) /
                   14.0f;

  snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "%.0f", avg_temp);
  set_tab_sensors_label_text(snprintf_buffer, tab_sensors_lb_rl_temp);
}

void irts_rr_update() {
  GET_LAST_STATE(secondary, irts_rr_0, SECONDARY, IRTS_RR_0);
  GET_LAST_STATE(secondary, irts_rr_1, SECONDARY, IRTS_RR_1);
  GET_LAST_STATE(secondary, irts_rr_2, SECONDARY, IRTS_RR_2);
  GET_LAST_STATE(secondary, irts_rr_3, SECONDARY, IRTS_RR_3);
  float avg_temp = (secondary_irts_rr_0_last_state->channel2 +
                    secondary_irts_rr_0_last_state->channel3 +
                    secondary_irts_rr_0_last_state->channel4 +
                    secondary_irts_rr_1_last_state->channel5 +
                    secondary_irts_rr_1_last_state->channel6 +
                    secondary_irts_rr_1_last_state->channel7 +
                    secondary_irts_rr_1_last_state->channel8 +
                    secondary_irts_rr_2_last_state->channel9 +
                    secondary_irts_rr_2_last_state->channel10 +
                    secondary_irts_rr_2_last_state->channel11 +
                    secondary_irts_rr_2_last_state->channel12 +
                    secondary_irts_rr_3_last_state->channel13 +
                    secondary_irts_rr_3_last_state->channel14 +
                    secondary_irts_rr_3_last_state->channel15) /
                   14.0f;
  snprintf(snprintf_buffer, SNPRINTF_BUFFER_SIZE, "%.0f", avg_temp);
  set_tab_sensors_label_text(snprintf_buffer, tab_sensors_lb_rr_temp);
}

void update_sensors_extra_value(const char *buf, uint8_t extra_value) {
  switch (extra_value) {
  case 0: {
    set_tab_sensors_label_text(buf, tab_sensors_extra_value0);
    break;
  }
  case 1: {
    set_tab_sensors_label_text(buf, tab_sensors_extra_value1);
    break;
  }
  case 2: {
    set_tab_sensors_label_text(buf, tab_sensors_extra_value2);
    break;
  }
  default: {
    break;
  }
  }
}

/***
 * PTT
 */

typedef enum {
  ptt_status_OFF = 0,
  ptt_status_SET_ON = 1,
  ptt_status_ON = 2,
  ptt_status_SET_OFF = 3,
} ptt_status_t;

bool ecu_ack = false;
bool ptt_button_pressed = false;
ptt_status_t ptt_status = ptt_status_OFF;

#include "can_messages.h"

void set_ptt_button_pressed(bool val) { ptt_button_pressed = val; }

void send_ptt_status_message(bool on) {
  primary_ecu_set_ptt_status_converted_t converted = {0};
  converted.status = on ? primary_ecu_set_ptt_status_status_on
                        : primary_ecu_set_ptt_status_status_off;
  STEER_CAN_PACK(primary, PRIMARY, ecu_set_ptt_status, ECU_SET_PTT_STATUS);
  can_send(&msg, true);
}

void ptt_tasks_fn(lv_timer_t *timer) {
  if (!ecu_ack && ptt_button_pressed) {
    ptt_status = ptt_status_SET_ON;
    send_ptt_status_message(true);
    update_sensors_extra_value("SO", 0);
  } else if (ecu_ack && !ptt_button_pressed) {
    ptt_status = ptt_status_SET_OFF;
    send_ptt_status_message(false);
    update_sensors_extra_value("SOF", 0);
  } else if (ecu_ack && ptt_button_pressed) {
    ptt_status = ptt_status_ON;
    send_ptt_status_message(true);
    update_sensors_extra_value("ON", 0);
  } else if (!ecu_ack && !ptt_button_pressed) {
    ptt_status = ptt_status_OFF;
    send_ptt_status_message(false);
    update_sensors_extra_value("OFF", 0);
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