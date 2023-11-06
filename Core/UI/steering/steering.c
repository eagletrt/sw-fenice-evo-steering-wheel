#include "steering.h"

lv_obj_t *lb_estimated_velocity;
lv_obj_t *lb_apps;
lv_obj_t *lb_bse;

extern lv_obj_t *hv_feedbacks_status[20];
extern lv_obj_t *hv_errors[16];
extern lv_obj_t *das_errors[9];
extern lv_obj_t *lv_errors[17];

// steering_tabs_t steering = {0};
extern bool steering_initialized;

#define SPRINTF_BUFFER_SIZE 64
char sprintf_buffer[SPRINTF_BUFFER_SIZE];

primary_car_status_converted_t primary_car_status_last_state = {0};
primary_control_output_converted_t primary_control_output_last_state = {0};

primary_ambient_temperature_converted_t secondary_ambient_temperature_last_state = {0};
primary_tlm_status_converted_t primary_tlm_status_last_state = {.tlm_status = 2};
primary_speed_converted_t primary_speed_last_state = {0};
primary_ptt_status_converted_t primary_ptt_status_last_state= {0};

primary_hv_voltage_converted_t primary_hv_voltage_last_state = {0};
primary_hv_current_converted_t primary_hv_current_last_state = {0};
primary_hv_temp_converted_t primary_hv_temp_last_state = {0};
primary_hv_errors_converted_t primary_hv_errors_last_state = {0};
primary_hv_feedbacks_status_converted_t primary_hv_feedbacks_status_last_state = {0};
float hv_delta_last_state = 0.0f;

primary_das_errors_converted_t primary_das_errors_last_state;

primary_lv_currents_converted_t primary_lv_currents_last_state = {0};
primary_lv_cells_voltage_converted_t primary_lv_cells_voltage_last_state_1 = {0};
primary_lv_cells_voltage_converted_t primary_lv_cells_voltage_last_state_2 = {0};
primary_lv_cells_temp_converted_t primary_lv_cells_temp_last_state_1 = {0};
primary_lv_cells_temp_converted_t primary_lv_cells_temp_last_state_2 = {0};
primary_lv_cells_temp_converted_t primary_lv_cells_temp_last_state_3 = {0};
primary_lv_cells_temp_converted_t primary_lv_cells_temp_last_state_4 = {0};
float lv_cells_temp_mean_last_state = 0;
primary_lv_total_voltage_converted_t primary_lv_total_voltage_last_state = {0};
primary_lv_errors_converted_t primary_lv_errors_last_state = {0};
primary_cooling_status_converted_t primary_cooling_status_last_state = {0};
primary_hv_fans_override_status_converted_t primary_hv_fans_override_status_last_state = {0};
primary_lv_feedbacks_converted_t primary_lv_feedbacks_last_state = {0};
primary_hv_cell_balancing_status_converted_t primary_hv_cell_balancing_status_last_state = {0};
primary_pedal_calibration_ack_converted_t primary_pedal_calibration_ack_last_state = {0};

secondary_steering_angle_converted_t secondary_steering_angle_last_state = {0};

secondary_pedals_output_converted_t secondary_pedals_output_last_state = {0};
secondary_imu_acceleration_converted_t secondary_imu_acceleration_last_state = {0};
secondary_lap_count_converted_t secondary_lap_count_last_state = {0};
secondary_lc_status_converted_t secondary_lc_status_last_state = {0};
float current_delta_state = 0.0f;
secondary_timestamp_converted_t secondary_timestamp_last_state = {0};
uint32_t timestamp_start_lap = 0;

inverters_inv_r_rcv_converted_t inverters_inv_r_last_state = {0};
inverters_inv_l_rcv_converted_t inverters_inv_l_last_state = {0};

void set_lb_estimated_velocity(const char *s) {
  lv_label_set_text(lb_estimated_velocity, s);
}

void set_lb_apps(const char *s) { lv_label_set_text(lb_apps, s); }

void set_lb_bse(const char *s) { lv_label_set_text(lb_bse, s); }

void car_status_update() {
  switch (primary_car_status_last_state.car_status) {
  case primary_car_status_car_status_INIT:
  case primary_car_status_car_status_ENABLE_INV_UPDATES:
  case primary_car_status_car_status_CHECK_INV_SETTINGS: {
    lv_label_set_text_fmt(get_tab_racing_bottom_lb_speed(), "-");
    set_tab_racing_lb_speed("INIT");
    set_tab_track_test_lb_speed("INIT");
    break;
  }
  case primary_car_status_car_status_IDLE: {
    lv_label_set_text_fmt(get_tab_racing_bottom_lb_speed(), "-");
    set_tab_racing_lb_speed("IDLE");
    set_tab_track_test_lb_speed("IDLE");
    break;
  }
  case primary_car_status_car_status_START_TS_PRECHARGE:
  case primary_car_status_car_status_WAIT_TS_PRECHARGE: {
    lv_label_set_text_fmt(get_tab_racing_bottom_lb_speed(), "-");
    set_tab_racing_lb_speed("PRCHG");
    set_tab_track_test_lb_speed("PRCHG");
    break;
  }
  case primary_car_status_car_status_WAIT_DRIVER: {
    lv_label_set_text_fmt(get_tab_racing_bottom_lb_speed(), "-");
    set_tab_racing_lb_speed("SETUP");
    set_tab_track_test_lb_speed("SETUP");
    break;
  }
  case primary_car_status_car_status_ENABLE_INV_DRIVE: {
    lv_label_set_text_fmt(get_tab_racing_bottom_lb_speed(), "-");
    set_tab_racing_lb_speed("ENINV");
    set_tab_track_test_lb_speed("ENINV");
    break;
  }
  case primary_car_status_car_status_DRIVE: {
    lv_label_set_text_fmt(get_tab_racing_bottom_lb_speed(), "-");
    set_tab_racing_lb_speed("DRIVE");
    set_tab_track_test_lb_speed("DRIVE");
    break;
  }
  case primary_car_status_car_status_DISABLE_INV_DRIVE:
  case primary_car_status_car_status_START_TS_DISCHARGE:
  case primary_car_status_car_status_WAIT_TS_DISCHARGE: {
    lv_label_set_text_fmt(get_tab_racing_bottom_lb_speed(), "-");
    set_tab_racing_lb_speed("TSOFF");
    set_tab_track_test_lb_speed("TSOFF");
    break;
  }
  case primary_car_status_car_status_FATAL_ERROR: {
    lv_label_set_text_fmt(get_tab_racing_bottom_lb_speed(), "-");
    set_tab_racing_lb_speed("FATAL");
    set_tab_track_test_lb_speed("FATAL");
    break;
  }
  default:
    break;
  }
}

void control_output_update(primary_control_output_converted_t *data) {
  if (data->estimated_velocity !=
      primary_control_output_last_state.estimated_velocity) {
    primary_control_output_last_state.estimated_velocity = data->estimated_velocity;
    sprintf(sprintf_buffer, "%.1f", data->estimated_velocity);
  }
}

void cooling_status_update(primary_cooling_status_converted_t *data) {
  if (data->pumps_speed != primary_cooling_status_last_state.pumps_speed) {
    primary_cooling_status_last_state.pumps_speed = data->pumps_speed;
    set_pumps_speed_bar(data->pumps_speed * 100);
    set_pumps_speed_value_label(data->pumps_speed);
  }
  if (data->radiators_speed != primary_cooling_status_last_state.radiators_speed) {
    primary_cooling_status_last_state.radiators_speed = data->radiators_speed;
    set_radiators_speed_bar(data->radiators_speed * 100);
    set_radiators_speed_value_label(data->radiators_speed);
  }
}

void tlm_status_update() {

    if (primary_tlm_status_last_state.tlm_status == primary_tlm_status_tlm_status_ON) {
      sprintf(sprintf_buffer, "ON");

      set_tab_sensors_lb_tlm_status(sprintf_buffer);

      all_leds_green();
    } else {
      sprintf(sprintf_buffer, "OFF");

      set_tab_sensors_lb_tlm_status(sprintf_buffer);

      all_leds_red();
    }
}


void speed_update() {
  if (primary_car_status_last_state.car_status !=
      primary_car_status_car_status_DRIVE)
    return;

    primary_speed_last_state.inverter_l = primary_speed_last_state.inverter_l;
    primary_speed_last_state.inverter_r = primary_speed_last_state.inverter_r;
    primary_speed_last_state.encoder_l = primary_speed_last_state.encoder_l;
    primary_speed_last_state.encoder_r = primary_speed_last_state.encoder_r;
}

void hv_voltage_update() {

    sprintf(sprintf_buffer, "%.1f",  primary_hv_voltage_last_state.min_cell_voltage);

    set_tab_sensors_lb_min_cell_voltage(sprintf_buffer);


    primary_hv_voltage_last_state.pack_voltage = primary_hv_voltage_last_state.pack_voltage;
    sprintf(sprintf_buffer, "%.0f", primary_hv_voltage_last_state.pack_voltage);

    set_tab_racing_lb_pack_voltage(sprintf_buffer);
    set_tab_sensors_lb_pack_voltage(sprintf_buffer);

    lv_bar_set_value(get_tab_racing_racing_lv_bar(), primary_hv_voltage_last_state.pack_voltage,
                     LV_ANIM_OFF);


    float percentage = (primary_hv_voltage_last_state.bus_voltage) / (primary_hv_voltage_last_state.pack_voltage) * 100;
    percentage = fmin(fmax(percentage, 0), 100);
    lv_meter_set_indicator_value(get_tab_racing_custom_meter(),
                                 get_tab_racing_indicator_blue(), percentage);

  float delta = primary_hv_voltage_last_state.max_cell_voltage - primary_hv_voltage_last_state.min_cell_voltage;
  if (delta != hv_delta_last_state) {
    hv_delta_last_state = delta;
    sprintf(sprintf_buffer, "%.3f", delta);

    set_tab_sensors_lb_hv_delta(sprintf_buffer);
  }
}

void hv_current_update() {
    sprintf(sprintf_buffer, "%.1f", primary_hv_current_last_state.current);

    set_tab_racing_lb_hv_current(sprintf_buffer);
    set_tab_sensors_lb_hv_current(sprintf_buffer);

    lv_bar_set_value(get_tab_racing_racing_hv_bar(),
                     primary_hv_current_last_state.current, LV_ANIM_OFF);
}

void hv_temp_update() {
    sprintf(sprintf_buffer, "%0.f", primary_hv_temp_last_state.average_temp);

    set_tab_racing_lb_average_temperature(sprintf_buffer);
    set_tab_sensors_lb_average_temperature(sprintf_buffer);
}

void hv_errors_update() {
#if STEER_TAB_DEBUG_ENABLED == 1
  STEER_ERROR_UPDATE(hv_errors, errors_cell_low_voltage, 0)
  STEER_ERROR_UPDATE(hv_errors, errors_cell_under_voltage, 1)
  STEER_ERROR_UPDATE(hv_errors, errors_cell_over_voltage, 2)
  STEER_ERROR_UPDATE(hv_errors, errors_cell_high_temperature, 3)
  STEER_ERROR_UPDATE(hv_errors, errors_cell_over_temperature, 4)
  STEER_ERROR_UPDATE(hv_errors, errors_over_current, 5)
  STEER_ERROR_UPDATE(hv_errors, errors_can, 6)
  STEER_ERROR_UPDATE(hv_errors, errors_int_voltage_mismatch, 7)
  STEER_ERROR_UPDATE(hv_errors, errors_cellboard_comm, 8)
  STEER_ERROR_UPDATE(hv_errors, errors_cellboard_internal, 9)
  STEER_ERROR_UPDATE(hv_errors, errors_connector_disconnected, 10)
  STEER_ERROR_UPDATE(hv_errors, errors_fans_disconnected, 11)
  STEER_ERROR_UPDATE(hv_errors, errors_feedback, 12)
  STEER_ERROR_UPDATE(hv_errors, errors_feedback_circuitry, 13)
  STEER_ERROR_UPDATE(hv_errors, errors_eeprom_comm, 14)
  STEER_ERROR_UPDATE(hv_errors, errors_eeprom_write, 15)
#endif
}

#define N_PORK_CELLBOARD 6
bool cellboard_bal[N_PORK_CELLBOARD] = {0};

void set_bal_status_label_text(char *text);

void hv_cell_balancing_status_update() {

  uint8_t cellboard_id = (uint8_t)primary_hv_cell_balancing_status_last_state.cellboard_id;
  if (cellboard_id < 0 || cellboard_id >= N_PORK_CELLBOARD) {
    return;
  }
  primary_hv_cell_balancing_status_balancing_status status =
          primary_hv_cell_balancing_status_last_state.balancing_status;
  cellboard_bal[cellboard_id] =
      status == primary_hv_cell_balancing_status_balancing_status_OFF ? false
                                                                      : true;
  bool is_bal = false;
  for (uint8_t i = 0; i < N_PORK_CELLBOARD; i++) {
    if (cellboard_bal[i]) {
      is_bal = true;
      break;
    }
  }
  char buf[BUFSIZ] = {0};
  sprintf(buf, "BAL STATUS: %s", is_bal ? "ON" : "OFF");
  set_bal_status_label_text(buf);
}

void hv_feedbacks_status_update() {
#if STEER_TAB_DEBUG_ENABLED == 1
  STEER_ERROR_UPDATE(hv_feedbacks_status,
                     feedbacks_status_feedback_implausibility_detected, 0)
  STEER_ERROR_UPDATE(hv_feedbacks_status, feedbacks_status_feedback_imd_cockpit,
                     1)
  STEER_ERROR_UPDATE(hv_feedbacks_status,
                     feedbacks_status_feedback_tsal_green_fault_latched, 2)
  STEER_ERROR_UPDATE(hv_feedbacks_status, feedbacks_status_feedback_bms_cockpit,
                     3)
  STEER_ERROR_UPDATE(hv_feedbacks_status, feedbacks_status_feedback_ext_latched,
                     4)
  STEER_ERROR_UPDATE(hv_feedbacks_status, feedbacks_status_feedback_tsal_green,
                     5)
  STEER_ERROR_UPDATE(hv_feedbacks_status,
                     feedbacks_status_feedback_ts_over_60v_status, 6)
  STEER_ERROR_UPDATE(hv_feedbacks_status, feedbacks_status_feedback_airn_status,
                     7)
  STEER_ERROR_UPDATE(hv_feedbacks_status, feedbacks_status_feedback_airp_status,
                     8)
  STEER_ERROR_UPDATE(hv_feedbacks_status, feedbacks_status_feedback_airp_gate,
                     9)
  STEER_ERROR_UPDATE(hv_feedbacks_status, feedbacks_status_feedback_airn_gate,
                     10)
  STEER_ERROR_UPDATE(hv_feedbacks_status,
                     feedbacks_status_feedback_precharge_status, 11)
  STEER_ERROR_UPDATE(hv_feedbacks_status,
                     feedbacks_status_feedback_tsp_over_60v_status, 12)
  STEER_ERROR_UPDATE(hv_feedbacks_status, feedbacks_status_feedback_imd_fault,
                     13)
  STEER_ERROR_UPDATE(hv_feedbacks_status, feedbacks_status_feedback_check_mux,
                     14)
  STEER_ERROR_UPDATE(hv_feedbacks_status, feedbacks_status_feedback_sd_end, 15)
  STEER_ERROR_UPDATE(hv_feedbacks_status, feedbacks_status_feedback_sd_out, 16)
  STEER_ERROR_UPDATE(hv_feedbacks_status, feedbacks_status_feedback_sd_in, 17)
  STEER_ERROR_UPDATE(hv_feedbacks_status, feedbacks_status_feedback_sd_bms, 18)
  STEER_ERROR_UPDATE(hv_feedbacks_status, feedbacks_status_feedback_sd_imd, 19)

  shutdown_component_state_t comp[SHUTDOWN_COMPONENT_SIZE] = {
      SC_UNKNOWN,
      data->feedbacks_status_feedback_sd_in + 1,
      data->feedbacks_status_feedback_sd_out + 1,
      SC_UNKNOWN,
      SC_UNKNOWN,
      SC_UNKNOWN,
      SC_UNKNOWN,
      SC_UNKNOWN,
      SC_UNKNOWN,
      SC_UNKNOWN,
      SC_UNKNOWN,
      SC_UNKNOWN,
      SC_UNKNOWN,
      data->feedbacks_status_feedback_sd_end + 1,
      data->feedbacks_status_feedback_precharge_status + 1,
      data->feedbacks_status_feedback_airp_gate + 1,
      data->feedbacks_status_feedback_airn_gate + 1};
  update_shutdown_circuit(comp);
#endif
}

void lv_feedbacks_update() {
#if 0
  if (memcmp(data, &primary_lv_feedbacks_last_state,
             sizeof(primary_lv_feedbacks_converted_t)) != 0) {
    memcpy(&primary_lv_feedbacks_last_state, data,
           sizeof(primary_lv_feedbacks_converted_t));
    shutdown_component_state_t comp[SHUTDOWN_COMPONENT_SIZE] = {
        data->sd_start + 1,
        SC_UNKNOWN,
        SC_UNKNOWN,
        data->feedbacks_interlock_fb + 1,
        SC_UNKNOWN,
        SC_UNKNOWN,
        SC_UNKNOWN,
        SC_UNKNOWN,
        data->feedbacks_invc_lid_fb + 1,
        data->feedbacks_hvd_fb + 1,
        data->feedbacks_bspd_fb + 1,
        data->feedbacks_invc_interlock_fb + 1,
        data->sd_end + 1,
        SC_UNKNOWN,
        SC_UNKNOWN,
        SC_UNKNOWN,
        SC_UNKNOWN};
    update_shutdown_circuit(comp);
  }
#endif
}

primary_ecu_feedbacks_converted_t ecu_feedbacks_last_state = {0};

void ecu_feedbacks_update() {
#if 0
  if (memcmp(data, &ecu_feedbacks_last_state,
             sizeof(primary_ecu_feedbacks_converted_t)) != 0) {
    memcpy(&primary_lv_feedbacks_last_state, data,
           sizeof(primary_ecu_feedbacks_converted_t));
    shutdown_component_state_t comp[SHUTDOWN_COMPONENT_SIZE] = {
        SC_UNKNOWN,
        SC_UNKNOWN,
        SC_UNKNOWN,
        SC_UNKNOWN,
        data->ecu_feedbacks_sd_in,
        data->ecu_feedbacks_sd_cock_fb,
        data->ecu_feedbacks_sd_interial_fb,
        data->ecu_feedbacks_sd_bots_fb,
        SC_UNKNOWN,
        SC_UNKNOWN,
        SC_UNKNOWN,
        SC_UNKNOWN,
        SC_UNKNOWN,
        SC_UNKNOWN,
        SC_UNKNOWN,
        SC_UNKNOWN,
        SC_UNKNOWN};
    update_shutdown_circuit(comp);
  }
#endif
}

void hv_fans_override_status_update() {
    set_pork_speed_bar(primary_hv_fans_override_status_last_state.fans_speed * 100);
    set_pork_speed_value_label(primary_hv_fans_override_status_last_state.fans_speed);
}

void das_errors_update() {
#if STEER_TAB_DEBUG_ENABLED == 1
  STEER_ERROR_UPDATE(das_errors, das_error_pedal_adc, 0)
  STEER_ERROR_UPDATE(das_errors, das_error_pedal_implausibility, 1)
  STEER_ERROR_UPDATE(das_errors, das_error_imu_tout, 2)
  STEER_ERROR_UPDATE(das_errors, das_error_irts_tout, 3)
  STEER_ERROR_UPDATE(das_errors, das_error_ts_tout, 4)
  STEER_ERROR_UPDATE(das_errors, das_error_invl_tout, 5)
  STEER_ERROR_UPDATE(das_errors, das_error_invr_tout, 6)
  STEER_ERROR_UPDATE(das_errors, das_error_steer_tout, 7)
  STEER_ERROR_UPDATE(das_errors, das_error_fsm, 8)
#endif
}

void lv_currents_update(primary_lv_currents_converted_t *data) {
    sprintf(sprintf_buffer, "%.1f", primary_lv_currents_last_state.current_lv_battery);

    set_tab_sensors_lb_lv_current(sprintf_buffer);
}

void lv_cells_temp_update() {
  //TODO
#if 0
  uint8_t start_index = data->start_index;
  uint8_t offset = 20;
  float current_lv_temp[12];
  current_lv_temp[0] = 27.0f;
  current_lv_temp[1] = 27.0f;
  current_lv_temp[2] = 27.0f;
  current_lv_temp[3] = 27.0f;
  current_lv_temp[4] = 27.0f;
  current_lv_temp[5] = 27.0f;
  current_lv_temp[6] = 27.0f;
  current_lv_temp[7] = 27.0f;
  current_lv_temp[8] = 27.0f;
  current_lv_temp[9] = 27.0f;
  current_lv_temp[10] = 27.0f;
  current_lv_temp[11] = 27.0f;
  if (start_index == 0) {
    current_lv_temp[0] = data->temp_0 + offset;
    current_lv_temp[1] = data->temp_1 + offset;
    current_lv_temp[2] = data->temp_2 + offset;
    current_lv_temp[3] = primary_lv_cells_temp_last_state_2.temp_0 + offset;
    current_lv_temp[4] = primary_lv_cells_temp_last_state_2.temp_1 + offset;
    current_lv_temp[5] = primary_lv_cells_temp_last_state_2.temp_2 + offset;
    current_lv_temp[6] = primary_lv_cells_temp_last_state_3.temp_0 + offset;
    current_lv_temp[7] = primary_lv_cells_temp_last_state_3.temp_1 + offset;
    current_lv_temp[8] = primary_lv_cells_temp_last_state_3.temp_2 + offset;
    current_lv_temp[9] = primary_lv_cells_temp_last_state_4.temp_0 + offset;
    current_lv_temp[10] = primary_lv_cells_temp_last_state_4.temp_1 + offset;
    current_lv_temp[11] = primary_lv_cells_temp_last_state_4.temp_2 + offset;
  } else if (start_index == 3) {
    current_lv_temp[0] = primary_lv_cells_temp_last_state_1.temp_0 + offset;
    current_lv_temp[1] = primary_lv_cells_temp_last_state_1.temp_1 + offset;
    current_lv_temp[2] = primary_lv_cells_temp_last_state_1.temp_2 + offset;
    current_lv_temp[3] = data->temp_0 + offset;
    current_lv_temp[4] = data->temp_1 + offset;
    current_lv_temp[5] = data->temp_2 + offset;
    current_lv_temp[6] = primary_lv_cells_temp_last_state_3.temp_0 + offset;
    current_lv_temp[7] = primary_lv_cells_temp_last_state_3.temp_1 + offset;
    current_lv_temp[8] = primary_lv_cells_temp_last_state_3.temp_2 + offset;
    current_lv_temp[9] = primary_lv_cells_temp_last_state_4.temp_0 + offset;
    current_lv_temp[10] = primary_lv_cells_temp_last_state_4.temp_1 + offset;
    current_lv_temp[11] = primary_lv_cells_temp_last_state_4.temp_2 + offset;
  } else if (start_index == 6) {
    current_lv_temp[0] = primary_lv_cells_temp_last_state_1.temp_0 + offset;
    current_lv_temp[1] = primary_lv_cells_temp_last_state_1.temp_1 + offset;
    current_lv_temp[2] = primary_lv_cells_temp_last_state_1.temp_2 + offset;
    current_lv_temp[3] = primary_lv_cells_temp_last_state_2.temp_0 + offset;
    current_lv_temp[4] = primary_lv_cells_temp_last_state_2.temp_1 + offset;
    current_lv_temp[5] = primary_lv_cells_temp_last_state_2.temp_2 + offset;
    current_lv_temp[6] = data->temp_0 + offset;
    current_lv_temp[7] = data->temp_1 + offset;
    current_lv_temp[8] = data->temp_2 + offset;
    current_lv_temp[9] = primary_lv_cells_temp_last_state_4.temp_0 + offset;
    current_lv_temp[10] = primary_lv_cells_temp_last_state_4.temp_1 + offset;
    current_lv_temp[11] = primary_lv_cells_temp_last_state_4.temp_2 + offset;
  } else if (start_index == 9) {
    current_lv_temp[0] = primary_lv_cells_temp_last_state_1.temp_0 + offset;
    current_lv_temp[1] = primary_lv_cells_temp_last_state_1.temp_1 + offset;
    current_lv_temp[2] = primary_lv_cells_temp_last_state_1.temp_2 + offset;
    current_lv_temp[3] = primary_lv_cells_temp_last_state_2.temp_0 + offset;
    current_lv_temp[4] = primary_lv_cells_temp_last_state_2.temp_1 + offset;
    current_lv_temp[5] = primary_lv_cells_temp_last_state_2.temp_2 + offset;
    current_lv_temp[6] = primary_lv_cells_temp_last_state_3.temp_0 + offset;
    current_lv_temp[7] = primary_lv_cells_temp_last_state_3.temp_1 + offset;
    current_lv_temp[8] = primary_lv_cells_temp_last_state_3.temp_2 + offset;
    current_lv_temp[9] = data->temp_0 + offset;
    current_lv_temp[10] = data->temp_1 + offset;
    current_lv_temp[11] = data->temp_2 + offset;
  }

  size_t temp_size = 0;
  float sum = 0;
  for (uint8_t temp_index = 0; temp_index < 12; temp_index++) {
    if (current_lv_temp[temp_index] != -1) {
      sum += current_lv_temp[temp_index];
      temp_size++;
    }
  }

  float mean_temp = (float)(sum / temp_size);
  if (mean_temp != lv_cells_temp_mean_last_state) {
    lv_cells_temp_mean_last_state = mean_temp;
    sprintf(sprintf_buffer, "%.0f", mean_temp);

    set_tab_racing_lb_battery_temperature(sprintf_buffer);
    set_tab_sensors_lb_battery_temperature(sprintf_buffer);
  }
#endif
}

void lv_total_voltage_update() {
    sprintf(sprintf_buffer, "%.1f", primary_lv_total_voltage_last_state.total_voltage);

    set_tab_sensors_lb_voltage(sprintf_buffer);
}


void lv_errors_update() {
#if STEER_TAB_DEBUG_ENABLED == 1
  STEER_ERROR_UPDATE(lv_errors, errors_cell_undervoltage, 0)
  STEER_ERROR_UPDATE(lv_errors, errors_cell_overvoltage, 1)
  STEER_ERROR_UPDATE(lv_errors, errors_battery_open_wire, 2)
  STEER_ERROR_UPDATE(lv_errors, errors_can, 3)
  STEER_ERROR_UPDATE(lv_errors, errors_spi, 4)
  STEER_ERROR_UPDATE(lv_errors, errors_over_current, 5)
  STEER_ERROR_UPDATE(lv_errors, errors_cell_under_temperature, 6)
  STEER_ERROR_UPDATE(lv_errors, errors_cell_over_temperature, 7)
  STEER_ERROR_UPDATE(lv_errors, errors_relay, 8)
  STEER_ERROR_UPDATE(lv_errors, errors_bms_monitor, 9)
  STEER_ERROR_UPDATE(lv_errors, errors_voltages_not_ready, 10)
  STEER_ERROR_UPDATE(lv_errors, errors_mcp23017, 11)
  STEER_ERROR_UPDATE(lv_errors, errors_radiator, 12)
  STEER_ERROR_UPDATE(lv_errors, errors_fan, 13)
  STEER_ERROR_UPDATE(lv_errors, errors_pump, 14)
  STEER_ERROR_UPDATE(lv_errors, errors_adc_init, 15)
  STEER_ERROR_UPDATE(lv_errors, errors_mux, 16)
#endif
}

void steering_angle_update() {
    sprintf(sprintf_buffer, "%.1f", secondary_steering_angle_last_state.angle);

    set_tab_calibration_lb_steering_angle(sprintf_buffer);
    set_tab_track_test_lb_steering_angle(sprintf_buffer);

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
}

void pedals_output_update() {
    sprintf(sprintf_buffer, "%d", (int)secondary_pedals_output_last_state.apps);

    // set_lb_apps(sprintf_buffer);
    calibration_box_t *curr_focus = get_tab_calibration_curr_focus();

    if (*curr_focus == APPS) {
      lv_slider_set_mode(get_tab_calibration_slider(), LV_BAR_MODE_RANGE);
      lv_slider_set_range(get_tab_calibration_slider(), APPS_RANGE_LOW,
                          APPS_RANGE_HIGH);
      lv_slider_set_value(get_tab_calibration_slider(),
                          secondary_pedals_output_last_state.apps, LV_ANIM_OFF);
    }

    sprintf(sprintf_buffer, "%.1f", secondary_pedals_output_last_state.bse_front);

    // set_lb_bse(sprintf_buffer);


    if (*curr_focus == BSE) {
      lv_slider_set_mode(get_tab_calibration_slider(), LV_BAR_MODE_RANGE);
      lv_slider_set_range(get_tab_calibration_slider(), BRAKE_RANGE_LOW,
                          BRAKE_RANGE_HIGH);
      lv_slider_set_value(get_tab_calibration_slider(), secondary_pedals_output_last_state.bse_front,
                          LV_ANIM_OFF);
    }
}

void imu_acceleration_update() {
    sprintf(sprintf_buffer, "%.1f", secondary_imu_acceleration_last_state.accel_x);

    set_tab_track_test_lb_inverter_speed_x(sprintf_buffer);

  if (secondary_imu_acceleration_last_state.accel_y != secondary_imu_acceleration_last_state.accel_y) {
    secondary_imu_acceleration_last_state.accel_y = secondary_imu_acceleration_last_state.accel_y;
    sprintf(sprintf_buffer, "%.1f", secondary_imu_acceleration_last_state.accel_y);

    set_tab_track_test_lb_inverter_speed_y(sprintf_buffer);
  }
}

void lap_count_update() {
  keep_lap_counter_value(2000);
  timestamp_start_lap = get_current_time_ms();
  float last_time_seconds = secondary_lap_count_last_state.lap_time;
  int minutes = (int)(last_time_seconds / 60.0f);
  int seconds = (int)(last_time_seconds - minutes * 60.0f);
  sprintf(sprintf_buffer, "%02d:%02d", minutes, seconds);

  set_tab_racing_lb_last_time(sprintf_buffer);

  float delta = last_time_seconds - secondary_lc_status_last_state.best_time;
  sprintf(sprintf_buffer, "%+.2f", delta);

  set_tab_racing_lb_delta_time(sprintf_buffer);
}

extern bool on_lap_keep;

void lc_status_update(secondary_lc_status_converted_t *data) {
    int minutes = (int)(secondary_lc_status_last_state.best_time / 60.0f);
    int seconds = (int)(secondary_lc_status_last_state.best_time - minutes * 60.0f);
    sprintf(sprintf_buffer, "%02d:%02d", minutes, seconds);

    set_tab_racing_lb_best_time(sprintf_buffer);

    float delta = secondary_lc_status_last_state.last_time - secondary_lc_status_last_state.best_time;
    if (delta != current_delta_state) {
      sprintf(sprintf_buffer, "%+.2f", delta);

      set_tab_racing_lb_delta_time(sprintf_buffer);
    }

  if (secondary_lc_status_last_state.last_time != secondary_lc_status_last_state.last_time && !on_lap_keep) {
    float last_time_seconds = secondary_lc_status_last_state.last_time;
    secondary_lc_status_last_state.last_time = last_time_seconds;
    int minutes = (int)(last_time_seconds / 60);
    int seconds = (int)(last_time_seconds - minutes * 60);
    sprintf(sprintf_buffer, "%02d:%02d", minutes, seconds);

    set_tab_racing_lb_last_time(sprintf_buffer);
  }
    sprintf(sprintf_buffer, "%d", (int)secondary_lc_status_last_state.lap_number);

    set_tab_racing_lb_lap_count(sprintf_buffer);

}


void inv_l_rcv_update() {
#if 0
  if (data->t_motor != inverters_inv_l_last_state.t_motor &&
      data->rcv_mux == INVERTERS_INV_L_RCV_RCV_MUX_ID_49_T_MOTOR_CHOICE) {
    float new_value = (data->t_motor - 9393.9f) / 55.1f;
    inverters_inv_l_last_state.t_motor = new_value;
    sprintf(sprintf_buffer, "%.0f", new_value);

    set_tab_racing_lb_left_motor_temp(sprintf_buffer);
    set_tab_sensors_lb_left_motor_temp(sprintf_buffer);
  }
  if (data->t_igbt != inverters_inv_l_last_state.t_igbt &&
      data->rcv_mux == INVERTERS_INV_L_RCV_RCV_MUX_ID_4A_T_IGBT_CHOICE) {
    float new_value = (data->t_igbt * 0.005f - 38.0f);
    inverters_inv_l_last_state.t_igbt = new_value;
    sprintf(sprintf_buffer, "%.0f", new_value);

    set_tab_racing_lb_left_inverter_temp(sprintf_buffer);
    set_tab_sensors_lb_left_inverter_temp(sprintf_buffer);
  }
#if 0
    if (data->n_actual_filt != inv_l_last_state.n_actual_filt && data->rcv_mux == INVERTERS_INV_L_RCV_RCV_MUX_ID_A8_N_ACTUAL_FILT_CHOICE) {
        float new_value = data->n_actual_filt * 0.1f;
        inv_l_last_state.n_actual_filt = new_value;
        sprintf(sprintf_buffer, "%.0f", new_value);
        set_tab_racing_lb_speed(sprintf_buffer);
        set_tab_track_test_lb_speed(sprintf_buffer);
    }
#endif
#endif
}

void inv_r_rcv_update() {
#if 0
  if (data->t_motor != inverters_inv_r_last_state.t_motor &&
      data->rcv_mux == INVERTERS_INV_R_RCV_RCV_MUX_ID_49_T_MOTOR_CHOICE) {
    float new_value = (data->t_motor - 9393.9f) / 55.1f;
    inverters_inv_r_last_state.t_motor = new_value;
    sprintf(sprintf_buffer, "%.0f", new_value);

    set_tab_sensors_lb_right_motor_temp(sprintf_buffer);
  }
  if (data->t_igbt != inverters_inv_r_last_state.t_igbt &&
      data->rcv_mux == INVERTERS_INV_R_RCV_RCV_MUX_ID_4A_T_IGBT_CHOICE) {
    float new_value = (data->t_igbt * 0.005f - 38.0f);
    inverters_inv_r_last_state.t_igbt = new_value;
    sprintf(sprintf_buffer, "%.0f", new_value);

    set_tab_sensors_lb_right_inverter_temp(sprintf_buffer);
  }
#endif
}

void update_sensors_extra_value(const char *buf, uint8_t extra_value) {
  switch (extra_value) {
  case 0: {
    lv_label_set_text_fmt(get_tab_sensors_extra_value0(), "%s", buf);
    break;
  }
  case 1: {
    lv_label_set_text_fmt(get_tab_sensors_extra_value1(), "%s", buf);
    break;
  }
  case 2: {
    lv_label_set_text_fmt(get_tab_sensors_extra_value2(), "%s", buf);
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
  primary_set_ptt_status_converted_t converted = {0};
  converted.status =
      on ? primary_set_ptt_status_status_ON : primary_set_ptt_status_status_OFF;
  STEER_CAN_PACK(primary, PRIMARY, set_ptt_status, SET_PTT_STATUS);
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

void handle_ptt_message(primary_ptt_status_status val) {
  if (val == primary_ptt_status_status_OFF) {
    ecu_ack = false;
  } else if (val == primary_ptt_status_status_ON) {
    ecu_ack = true;
  }
}
