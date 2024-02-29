#include "steering.h"

lv_obj_t *lb_estimated_velocity;
lv_obj_t *lb_apps;
lv_obj_t *lb_bse;

extern bool steering_initialized;

#define SPRINTF_BUFFER_SIZE 64
char sprintf_buffer[SPRINTF_BUFFER_SIZE];

uint8_t primary_messages_last_state[primary_MESSAGE_COUNT]
                                   [primary_MAX_STRUCT_SIZE_CONVERSION];
uint8_t secondary_messages_last_state[secondary_MESSAGE_COUNT]
                                     [secondary_MAX_STRUCT_SIZE_CONVERSION];
uint8_t inverters_messages_last_state[inverters_MESSAGE_COUNT]
                                     [inverters_MAX_STRUCT_SIZE_CONVERSION];

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
  GET_LAST_STATE(primary, car_status, PRIMARY, CAR_STATUS);
  switch (primary_car_status_last_state->car_status) {
  case primary_car_status_car_status_INIT:
  case primary_car_status_car_status_ENABLE_INV_UPDATES:
  case primary_car_status_car_status_CHECK_INV_SETTINGS: {
    set_tab_racing_label_text("-", tab_rac_bottom_status_idx);
    set_tab_racing_label_text("INIT", tab_rac_status_idx);
    set_tab_track_test_lb_speed("INIT");
    break;
  }
  case primary_car_status_car_status_IDLE: {
    set_tab_racing_label_text("-", tab_rac_bottom_status_idx);
    set_tab_racing_label_text("IDLE", tab_rac_status_idx);
    set_tab_track_test_lb_speed("IDLE");
    break;
  }
  case primary_car_status_car_status_START_TS_PRECHARGE:
  case primary_car_status_car_status_WAIT_TS_PRECHARGE: {
    set_tab_racing_label_text("-", tab_rac_bottom_status_idx);
    set_tab_racing_label_text("PRCHG", tab_rac_status_idx);
    set_tab_track_test_lb_speed("PRCHG");
    break;
  }
  case primary_car_status_car_status_WAIT_DRIVER: {
    set_tab_racing_label_text("-", tab_rac_bottom_status_idx);
    set_tab_racing_label_text("SETUP", tab_rac_status_idx);
    set_tab_track_test_lb_speed("SETUP");
    break;
  }
  case primary_car_status_car_status_ENABLE_INV_DRIVE: {
    set_tab_racing_label_text("-", tab_rac_bottom_status_idx);
    set_tab_racing_label_text("ENINV", tab_rac_status_idx);
    set_tab_track_test_lb_speed("ENINV");
    break;
  }
  case primary_car_status_car_status_DRIVE: {
    set_tab_racing_label_text("-", tab_rac_bottom_status_idx);
    set_tab_racing_label_text("DRIVE", tab_rac_status_idx);
    set_tab_track_test_lb_speed("DRIVE");
    break;
  }
  case primary_car_status_car_status_DISABLE_INV_DRIVE:
  case primary_car_status_car_status_START_TS_DISCHARGE:
  case primary_car_status_car_status_WAIT_TS_DISCHARGE: {
    set_tab_racing_label_text("-", tab_rac_bottom_status_idx);
    set_tab_racing_label_text("TSOFF", tab_rac_status_idx);
    set_tab_track_test_lb_speed("TSOFF");
    break;
  }
  case primary_car_status_car_status_FATAL_ERROR: {
    set_tab_racing_label_text("-", tab_rac_bottom_status_idx);
    set_tab_racing_label_text("FATAL", tab_rac_status_idx);
    set_tab_track_test_lb_speed("FATAL");
    break;
  }
  default:
    break;
  }
}

void cooling_status_update() {
#if ENGINEERING_TAB_ENABLED == 1
  GET_LAST_STATE(primary, cooling_status, PRIMARY, COOLING_STATUS);
  set_pumps_speed_bar(primary_cooling_status_last_state->pumps_speed * 100);
  set_pumps_speed_value_label(primary_cooling_status_last_state->pumps_speed);
  set_radiators_speed_bar(primary_cooling_status_last_state->radiators_speed *
                          100);
  set_radiators_speed_value_label(
      primary_cooling_status_last_state->radiators_speed);
#endif
}

void tlm_status_update() {
  GET_LAST_STATE(primary, tlm_status, PRIMARY, TLM_STATUS);
  if (primary_tlm_status_last_state->tlm_status ==
      primary_tlm_status_tlm_status_ON) {
    sprintf(sprintf_buffer, "ON");
    all_leds_green();
  } else {
    sprintf(sprintf_buffer, "OFF");
    all_leds_red();
  }
  set_tab_sensors_label_text(sprintf_buffer, tab_sensors_lb_tlm_status);
}

void speed_update(void) {
  GET_LAST_STATE(primary, speed, PRIMARY, SPEED);
  GET_LAST_STATE(primary, car_status, PRIMARY, CAR_STATUS);
  if (primary_car_status_last_state->car_status !=
      primary_car_status_car_status_DRIVE) {
    return;
  }
  float speed = fabs((primary_speed_last_state->encoder_l +
                      primary_speed_last_state->encoder_r) /
                     2.0f) *
                3.6 * 0.203;
  set_tab_racing_label_text("KM/H", tab_rac_bottom_status_idx);
  snprintf(sprintf_buffer, SPRINTF_BUFFER_SIZE, "%.0f", speed);
  set_tab_racing_label_text(sprintf_buffer, tab_rac_status_idx);
  set_tab_track_test_lb_speed(sprintf_buffer);
}

void hv_cell_voltage_update(void) {
  GET_LAST_STATE(primary, hv_cell_voltage, PRIMARY, HV_CELL_VOLTAGE);
  sprintf(sprintf_buffer, "%.1f",
          primary_hv_cell_voltage_last_state->min_cell_voltage);

  set_tab_sensors_label_text(sprintf_buffer, tab_sensors_lb_min_cell_voltage);

  float delta = primary_hv_cell_voltage_last_state->max_cell_voltage -
                primary_hv_cell_voltage_last_state->min_cell_voltage;
  sprintf(sprintf_buffer, "%d", (int)(delta * 1000.0f));
  set_tab_sensors_label_text(sprintf_buffer, tab_sensors_lb_hv_delta);
}

void hv_voltage_update(void) {
  GET_LAST_STATE(primary, hv_voltage, PRIMARY, HV_VOLTAGE);
  sprintf(sprintf_buffer, "%.0f", primary_hv_voltage_last_state->pack_voltage);
  set_tab_racing_label_text(sprintf_buffer, tab_rac_pack_voltage_idx);
  set_tab_sensors_label_text(sprintf_buffer, tab_sensors_lb_pack_voltage);
  set_tab_racing_hv_pack_voltage_bar(
      primary_hv_voltage_last_state->pack_voltage);
}

void hv_current_update() {
  GET_LAST_STATE(primary, hv_current, PRIMARY, HV_CURRENT);
  sprintf(sprintf_buffer, "%.1f", primary_hv_current_last_state->current);
  set_tab_sensors_label_text(sprintf_buffer, tab_sensors_lb_hv_current);
  set_tab_racing_label_text(sprintf_buffer, tab_rac_hv_curr_idx);
  set_tab_racing_hv_current_bar(primary_hv_current_last_state->current);
}

void hv_temp_update() {
  GET_LAST_STATE(primary, hv_temp, PRIMARY, HV_TEMP);
  sprintf(sprintf_buffer, "%0.f", primary_hv_temp_last_state->average_temp);
  set_tab_racing_label_text(sprintf_buffer, tab_rac_hv_avg_temp_idx);
  set_tab_sensors_label_text(sprintf_buffer,
                             tab_sensors_lb_average_temperature);
}

#define N_PORK_CELLBOARD 6
bool cellboard_bal[N_PORK_CELLBOARD] = {0};

void set_bal_status_label_text(char *text);

void hv_cell_balancing_status_update() {
#if ENGINEERING_TAB_ENABLED == 1
  GET_LAST_STATE(primary, hv_cell_balancing_status, PRIMARY,
                 HV_CELL_BALANCING_STATUS);
  uint8_t cellboard_id =
      (uint8_t)primary_hv_cell_balancing_status_last_state->cellboard_id;
  if (cellboard_id < 0 || cellboard_id >= N_PORK_CELLBOARD) {
    return;
  }
  primary_hv_cell_balancing_status_balancing_status status =
      primary_hv_cell_balancing_status_last_state->balancing_status;
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
#endif
}

void lv_feedbacks_update() {}

primary_ecu_feedbacks_converted_t ecu_feedbacks_last_state = {0};

void ecu_feedbacks_update() {}

void hv_fans_override_status_update() {
#if ENGINEERING_TAB_ENABLED == 1
  GET_LAST_STATE(primary, hv_fans_override_status, PRIMARY,
                 HV_FANS_OVERRIDE_STATUS);
  set_pork_speed_bar(primary_hv_fans_override_status_last_state->fans_speed *
                     100);
  set_pork_speed_value_label(
      primary_hv_fans_override_status_last_state->fans_speed);
#endif
}

void lv_currents_update() {
  GET_LAST_STATE(primary, lv_currents, PRIMARY, LV_CURRENTS);
  sprintf(sprintf_buffer, "%.1f",
          primary_lv_currents_last_state->current_lv_battery);
  set_tab_sensors_label_text(sprintf_buffer, tab_sensors_lb_lv_current);
}

void lv_cells_voltage_update(void) {
#define N_LV_CELLS 6
  float current_lv_temp[N_LV_CELLS] = {0};
  current_lv_temp[0] = lv_voltages_stock_1.voltage_0;
  current_lv_temp[1] = lv_voltages_stock_1.voltage_1;
  current_lv_temp[2] = lv_voltages_stock_1.voltage_2;
  current_lv_temp[3] = lv_voltages_stock_2.voltage_0;
  current_lv_temp[4] = lv_voltages_stock_2.voltage_1;
  current_lv_temp[5] = lv_voltages_stock_2.voltage_2;

  float sum = 0;
  for (uint8_t temp_index = 0; temp_index < N_LV_CELLS; temp_index++)
    sum += current_lv_temp[temp_index];

  float mean_voltage = (float)(sum / N_LV_CELLS);
  sprintf(sprintf_buffer, "%.0f", mean_voltage);
  set_tab_racing_label_text(sprintf_buffer, tab_rac_lv_temp_idx);
  set_tab_sensors_label_text(sprintf_buffer,
                             tab_sensors_lb_battery_temperature);
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

  float mean_temp = (float)(sum / N_LV_CELLS);
  sprintf(sprintf_buffer, "%.0f", mean_temp);
  set_tab_racing_label_text(sprintf_buffer, tab_rac_lv_temp_idx);
  set_tab_sensors_label_text(sprintf_buffer,
                             tab_sensors_lb_battery_temperature);
}

void lv_total_voltage_update() {
  GET_LAST_STATE(primary, lv_total_voltage, PRIMARY, LV_TOTAL_VOLTAGE);
  sprintf(sprintf_buffer, "%.1f",
          primary_lv_total_voltage_last_state->total_voltage);
  set_tab_sensors_label_text(sprintf_buffer, tab_sensors_lb_voltage);
}

void steering_angle_update() {
  GET_LAST_STATE(secondary, steering_angle, SECONDARY, STEERING_ANGLE);
  sprintf(sprintf_buffer, "%.1f", secondary_steering_angle_last_state->angle);
  set_tab_track_test_steering_angle_bar(
      secondary_steering_angle_last_state->angle);
#if STEER_TAB_CALIBRATION_ENABLED == 1
  set_tab_calibration_lb_steering_angle(sprintf_buffer);
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
  sprintf(sprintf_buffer, "%d", (int)secondary_pedals_output_last_state->apps);
  calibration_box_t *curr_focus = get_tab_calibration_curr_focus();

  if (*curr_focus == APPS) {
    lv_slider_set_mode(get_tab_calibration_slider(), LV_BAR_MODE_RANGE);
    lv_slider_set_range(get_tab_calibration_slider(), APPS_RANGE_LOW,
                        APPS_RANGE_HIGH);
    lv_slider_set_value(get_tab_calibration_slider(),
                        secondary_pedals_output_last_state->apps, LV_ANIM_OFF);
  }
  sprintf(sprintf_buffer, "%.1f",
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
  GET_LAST_STATE(secondary, lap_count, SECONDARY, LAP_COUNT);
  GET_LAST_STATE(secondary, lc_status, SECONDARY, LC_STATUS);
  keep_lap_counter_value(2000);
  timestamp_start_lap = get_current_time_ms();
  float last_time_seconds = secondary_lap_count_last_state->lap_time;
  int minutes = (int)(last_time_seconds / 60.0f);
  int seconds = (int)(last_time_seconds - minutes * 60.0f);
  sprintf(sprintf_buffer, "%02d:%02d", minutes, seconds);

  set_tab_racing_label_text(sprintf_buffer, tab_rac_last_time_idx);

  float delta = last_time_seconds - secondary_lc_status_last_state->best_time;
  sprintf(sprintf_buffer, "%+.2f", delta);

  set_tab_racing_label_text(sprintf_buffer, tab_rac_dtime_idx);
}

extern bool on_lap_keep;

void lc_status_update(secondary_lc_status_converted_t *data) {
  GET_LAST_STATE(secondary, lc_status, SECONDARY, LC_STATUS);
  int minutes = (int)(secondary_lc_status_last_state->best_time / 60.0f);
  int seconds =
      (int)(secondary_lc_status_last_state->best_time - minutes * 60.0f);
  sprintf(sprintf_buffer, "%02d:%02d", minutes, seconds);

  set_tab_racing_label_text(sprintf_buffer, tab_rac_best_time_idx);

  float delta = secondary_lc_status_last_state->last_time -
                secondary_lc_status_last_state->best_time;
  sprintf(sprintf_buffer, "%+.2f", delta);
  set_tab_racing_label_text(sprintf_buffer, tab_rac_dtime_idx);

  if (secondary_lc_status_last_state->last_time !=
          secondary_lc_status_last_state->last_time &&
      !on_lap_keep) {
    float last_time_seconds = secondary_lc_status_last_state->last_time;
    secondary_lc_status_last_state->last_time = last_time_seconds;
    int minutes = (int)(last_time_seconds / 60);
    int seconds = (int)(last_time_seconds - minutes * 60);
    sprintf(sprintf_buffer, "%02d:%02d", minutes, seconds);

    set_tab_racing_label_text(sprintf_buffer, tab_rac_last_time_idx);
  }
  sprintf(sprintf_buffer, "%d",
          (int)secondary_lc_status_last_state->lap_number);
  set_tab_racing_label_text(sprintf_buffer, tab_rac_lap_count_idx);
}

#define INVERTER_MESSAGE_UNINITIALIZED -100.0f
#define INVERTER_TEMP_CONVERSION(raw_temp)                                     \
  -43.23745 + 0.01073427 * raw_temp - 5.523417e-7 * pow(raw_temp, 2) +         \
      1.330787e-11 * pow(raw_temp, 3);
float l_motor_temp = INVERTER_MESSAGE_UNINITIALIZED;
float l_igbt_temp = INVERTER_MESSAGE_UNINITIALIZED;
float r_motor_temp = INVERTER_MESSAGE_UNINITIALIZED;
float r_igbt_temp = INVERTER_MESSAGE_UNINITIALIZED;

void inv_l_rcv_update(void) {
  GET_LAST_STATE(inverters, inv_l_rcv, INVERTERS, INV_L_RCV);
  if (inverters_inv_l_rcv_last_state->rcv_mux ==
      INVERTERS_INV_L_RCV_RCV_MUX_ID_49_T_MOTOR_CHOICE) {
    l_motor_temp = (inverters_inv_l_rcv_last_state->t_motor - 9393.9f) / 55.1f;
    sprintf(sprintf_buffer, "%.0f", l_motor_temp);
    set_tab_racing_label_text(sprintf_buffer, tab_rac_mot_idx);

    if (r_motor_temp != INVERTER_MESSAGE_UNINITIALIZED) {
      sprintf(sprintf_buffer, "%.0f", (l_motor_temp + r_motor_temp) / 2.0f);
      set_tab_sensors_label_text(sprintf_buffer,
                                 tab_sensors_lb_left_motor_temp);
    }
  }

  if (inverters_inv_l_rcv_last_state->rcv_mux ==
      INVERTERS_INV_L_RCV_RCV_MUX_ID_4A_T_IGBT_CHOICE) {
    l_igbt_temp =
        INVERTER_TEMP_CONVERSION(inverters_inv_l_rcv_last_state->t_igbt);
    sprintf(sprintf_buffer, "%.0f", l_igbt_temp);
    set_tab_sensors_label_text(sprintf_buffer,
                               tab_sensors_lb_left_inverter_temp);

    if (r_igbt_temp != INVERTER_MESSAGE_UNINITIALIZED) {
      sprintf(sprintf_buffer, "%.0f", (l_igbt_temp + r_igbt_temp) / 2.0f);
      set_tab_racing_label_text(sprintf_buffer, tab_rac_inv_idx);
    }
  }
}

void inv_r_rcv_update() {
  GET_LAST_STATE(inverters, inv_r_rcv, INVERTERS, INV_R_RCV);
  if (inverters_inv_r_rcv_last_state->rcv_mux ==
      INVERTERS_INV_R_RCV_RCV_MUX_ID_49_T_MOTOR_CHOICE) {
    r_motor_temp = (inverters_inv_r_rcv_last_state->t_motor - 9393.9f) / 55.1f;
    sprintf(sprintf_buffer, "%.0f", r_motor_temp);
    set_tab_sensors_label_text(sprintf_buffer, tab_sensors_lb_right_motor_temp);
  }

  if (inverters_inv_r_rcv_last_state->rcv_mux ==
      INVERTERS_INV_R_RCV_RCV_MUX_ID_4A_T_IGBT_CHOICE) {
    r_igbt_temp =
        INVERTER_TEMP_CONVERSION(inverters_inv_r_rcv_last_state->t_igbt);
    sprintf(sprintf_buffer, "%.0f", r_igbt_temp);
    set_tab_sensors_label_text(sprintf_buffer,
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
  sprintf(sprintf_buffer, "%.0f", avg_temp);
  set_tab_sensors_label_text(sprintf_buffer, tab_sensors_lb_fl_temp);
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
  sprintf(sprintf_buffer, "%.0f", avg_temp);
  set_tab_sensors_label_text(sprintf_buffer, tab_sensors_lb_fr_temp);
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

  sprintf(sprintf_buffer, "%.0f", avg_temp);
  set_tab_sensors_label_text(sprintf_buffer, tab_sensors_lb_rl_temp);
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
  sprintf(sprintf_buffer, "%.0f", avg_temp);
  set_tab_sensors_label_text(sprintf_buffer, tab_sensors_lb_rr_temp);
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

void handle_ptt_message(void) {
  GET_LAST_STATE(primary, ptt_status, PRIMARY, PTT_STATUS);
  primary_ptt_status_status val = primary_ptt_status_last_state->status;
  if (val == primary_ptt_status_status_OFF) {
    ecu_ack = false;
  } else if (val == primary_ptt_status_status_ON) {
    ecu_ack = true;
  }
}
