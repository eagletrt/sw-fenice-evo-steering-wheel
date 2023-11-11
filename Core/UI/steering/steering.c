#include "steering.h"

lv_obj_t *lb_estimated_velocity;
lv_obj_t *lb_apps;
lv_obj_t *lb_bse;

extern lv_obj_t *hv_feedbacks_status[20];
extern lv_obj_t *hv_errors[16];
extern lv_obj_t *das_errors[9];
extern lv_obj_t *lv_errors[17];

extern bool steering_initialized;

#define SPRINTF_BUFFER_SIZE 64
char sprintf_buffer[SPRINTF_BUFFER_SIZE];

primary_car_status_converted_t primary_car_status_last_state = {0};
primary_control_output_converted_t primary_control_output_last_state = {0};

primary_ambient_temperature_converted_t
    secondary_ambient_temperature_last_state = {0};
primary_tlm_status_converted_t primary_tlm_status_last_state = {.tlm_status =
                                                                    2};
primary_speed_converted_t primary_speed_last_state = {0};
primary_ptt_status_converted_t primary_ptt_status_last_state = {0};

primary_hv_voltage_converted_t primary_hv_voltage_last_state = {0};
primary_hv_current_converted_t primary_hv_current_last_state = {0};
primary_hv_temp_converted_t primary_hv_temp_last_state = {0};
primary_hv_errors_converted_t primary_hv_errors_last_state = {0};
float hv_delta_last_state = 0.0f;

primary_das_errors_converted_t primary_das_errors_last_state;

primary_lv_currents_converted_t primary_lv_currents_last_state = {0};
primary_lv_cells_voltage_converted_t primary_lv_cells_voltage_last_state = {0};
primary_lv_cells_voltage_converted_t primary_lv_cells_voltage_last_state_1 = {
    0};
primary_lv_cells_voltage_converted_t primary_lv_cells_voltage_last_state_2 = {
    0};
primary_lv_cells_temp_converted_t primary_lv_cells_temp_last_state = {0};
primary_lv_cells_temp_converted_t primary_lv_cells_temp_last_state_1 = {0};
primary_lv_cells_temp_converted_t primary_lv_cells_temp_last_state_2 = {0};
primary_lv_total_voltage_converted_t primary_lv_total_voltage_last_state = {0};
primary_lv_errors_converted_t primary_lv_errors_last_state = {0};
primary_cooling_status_converted_t primary_cooling_status_last_state = {0};
primary_hv_fans_override_status_converted_t
    primary_hv_fans_override_status_last_state = {0};
primary_lv_feedbacks_converted_t primary_lv_feedbacks_last_state = {0};
primary_hv_cell_balancing_status_converted_t
    primary_hv_cell_balancing_status_last_state = {0};
primary_pedal_calibration_ack_converted_t
    primary_pedal_calibration_ack_last_state = {0};

secondary_steering_angle_converted_t secondary_steering_angle_last_state = {0};

secondary_pedals_output_converted_t secondary_pedals_output_last_state = {0};
secondary_imu_acceleration_converted_t secondary_imu_acceleration_last_state = {
    0};
secondary_lap_count_converted_t secondary_lap_count_last_state = {0};
secondary_lc_status_converted_t secondary_lc_status_last_state = {0};
float current_delta_state = 0.0f;
secondary_timestamp_converted_t secondary_timestamp_last_state = {0};
inverters_inv_l_rcv_converted_t inverters_inv_l_rcv_last_state = {0};
inverters_inv_r_rcv_converted_t inverters_inv_r_rcv_last_state = {0};
uint32_t timestamp_start_lap = 0;

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

void cooling_status_update() {
  set_pumps_speed_bar(primary_cooling_status_last_state.pumps_speed * 100);
  set_pumps_speed_value_label(primary_cooling_status_last_state.pumps_speed);
  set_radiators_speed_bar(primary_cooling_status_last_state.radiators_speed *
                          100);
  set_radiators_speed_value_label(
      primary_cooling_status_last_state.radiators_speed);
}

void tlm_status_update() {
  if (primary_tlm_status_last_state.tlm_status ==
      primary_tlm_status_tlm_status_ON) {
    sprintf(sprintf_buffer, "ON");
    set_tab_sensors_lb_tlm_status(sprintf_buffer);
    all_leds_green();
  } else {
    sprintf(sprintf_buffer, "OFF");
    set_tab_sensors_lb_tlm_status(sprintf_buffer);
    all_leds_red();
  }
}

void speed_update(void) {
  if (primary_car_status_last_state.car_status !=
      primary_car_status_car_status_DRIVE) {
    return;
  }
  float speed = fabs((primary_speed_last_state.encoder_l +
                      primary_speed_last_state.encoder_r) /
                     2.0f) *
                3.6 * 0.203;
  lv_label_set_text_fmt(get_tab_racing_bottom_lb_speed(), "KM/H");
  snprintf(sprintf_buffer, SPRINTF_BUFFER_SIZE, "%.0f", speed);
  set_tab_racing_lb_speed(sprintf_buffer);
  set_tab_track_test_lb_speed(sprintf_buffer);
}

void hv_voltage_update(void) {

  sprintf(sprintf_buffer, "%.1f",
          primary_hv_voltage_last_state.min_cell_voltage);
  set_tab_sensors_lb_min_cell_voltage(sprintf_buffer);

  primary_hv_voltage_last_state.pack_voltage =
      primary_hv_voltage_last_state.pack_voltage;
  sprintf(sprintf_buffer, "%.0f", primary_hv_voltage_last_state.pack_voltage);

  set_tab_racing_lb_pack_voltage(sprintf_buffer);
  set_tab_sensors_lb_pack_voltage(sprintf_buffer);

  lv_bar_set_value(get_tab_racing_racing_lv_bar(),
                   primary_hv_voltage_last_state.pack_voltage, LV_ANIM_OFF);

  float percentage = (primary_hv_voltage_last_state.bus_voltage) /
                     (primary_hv_voltage_last_state.pack_voltage) * 100;
  percentage = fmin(fmax(percentage, 0), 100);
  lv_meter_set_indicator_value(get_tab_racing_custom_meter(),
                               get_tab_racing_indicator_blue(), percentage);

  float delta = primary_hv_voltage_last_state.max_cell_voltage -
                primary_hv_voltage_last_state.min_cell_voltage;
  if (delta != hv_delta_last_state) {
    hv_delta_last_state = delta;
    sprintf(sprintf_buffer, "%d", (int)(delta * 1000.0f));
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

void hv_errors_update() {}

#define N_PORK_CELLBOARD 6
bool cellboard_bal[N_PORK_CELLBOARD] = {0};

void set_bal_status_label_text(char *text);

void hv_cell_balancing_status_update() {

  uint8_t cellboard_id =
      (uint8_t)primary_hv_cell_balancing_status_last_state.cellboard_id;
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

void hv_feedbacks_status_update() {}

void lv_feedbacks_update() {}

primary_ecu_feedbacks_converted_t ecu_feedbacks_last_state = {0};

void ecu_feedbacks_update() {}

void hv_fans_override_status_update() {
  set_pork_speed_bar(primary_hv_fans_override_status_last_state.fans_speed *
                     100);
  set_pork_speed_value_label(
      primary_hv_fans_override_status_last_state.fans_speed);
}

void das_errors_update() {}

void lv_currents_update() {
  sprintf(sprintf_buffer, "%.1f",
          primary_lv_currents_last_state.current_lv_battery);

  set_tab_sensors_lb_lv_current(sprintf_buffer);
}

void lv_cells_voltage_update(void) {
#define N_LV_CELLS 6
  float current_lv_temp[N_LV_CELLS] = {0};
  current_lv_temp[0] = primary_lv_cells_voltage_last_state_1.voltage_0;
  current_lv_temp[1] = primary_lv_cells_voltage_last_state_1.voltage_1;
  current_lv_temp[2] = primary_lv_cells_voltage_last_state_1.voltage_2;
  current_lv_temp[3] = primary_lv_cells_voltage_last_state_2.voltage_0;
  current_lv_temp[4] = primary_lv_cells_voltage_last_state_2.voltage_1;
  current_lv_temp[5] = primary_lv_cells_voltage_last_state_2.voltage_2;

  float sum = 0;
  for (uint8_t temp_index = 0; temp_index < N_LV_CELLS; temp_index++)
    sum += current_lv_temp[temp_index];

  float mean_voltage = (float)(sum / N_LV_CELLS);
  sprintf(sprintf_buffer, "%.0f", mean_voltage);
  set_tab_racing_lb_battery_temperature(sprintf_buffer);
  set_tab_sensors_lb_battery_temperature(sprintf_buffer);
}

void lv_cells_temp_update() {
  float current_lv_temp[N_LV_CELLS] = {0};
  current_lv_temp[0] = primary_lv_cells_temp_last_state_1.temp_0;
  current_lv_temp[1] = primary_lv_cells_temp_last_state_1.temp_1;
  current_lv_temp[2] = primary_lv_cells_temp_last_state_1.temp_2;
  current_lv_temp[3] = primary_lv_cells_temp_last_state_2.temp_0;
  current_lv_temp[4] = primary_lv_cells_temp_last_state_2.temp_1;
  current_lv_temp[5] = primary_lv_cells_temp_last_state_2.temp_2;

  float sum = 0;
  for (uint8_t temp_index = 0; temp_index < N_LV_CELLS; temp_index++)
    sum += current_lv_temp[temp_index];

  float mean_temp = (float)(sum / N_LV_CELLS);
  sprintf(sprintf_buffer, "%.0f", mean_temp);
  set_tab_racing_lb_battery_temperature(sprintf_buffer);
  set_tab_sensors_lb_battery_temperature(sprintf_buffer);
}

void lv_total_voltage_update() {
  sprintf(sprintf_buffer, "%.1f",
          primary_lv_total_voltage_last_state.total_voltage);
  set_tab_sensors_lb_voltage(sprintf_buffer);
}

void lv_errors_update() {}

void steering_angle_update() {
  sprintf(sprintf_buffer, "%.1f", secondary_steering_angle_last_state.angle);
  set_tab_track_test_steering_angle_bar(
      secondary_steering_angle_last_state.angle);
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
  sprintf(sprintf_buffer, "%d", (int)secondary_pedals_output_last_state.apps);
  calibration_box_t *curr_focus = get_tab_calibration_curr_focus();

  if (*curr_focus == APPS) {
    lv_slider_set_mode(get_tab_calibration_slider(), LV_BAR_MODE_RANGE);
    lv_slider_set_range(get_tab_calibration_slider(), APPS_RANGE_LOW,
                        APPS_RANGE_HIGH);
    lv_slider_set_value(get_tab_calibration_slider(),
                        secondary_pedals_output_last_state.apps, LV_ANIM_OFF);
  }
  sprintf(sprintf_buffer, "%.1f", secondary_pedals_output_last_state.bse_front);
  if (*curr_focus == BSE) {
    lv_slider_set_mode(get_tab_calibration_slider(), LV_BAR_MODE_RANGE);
    lv_slider_set_range(get_tab_calibration_slider(), BRAKE_RANGE_LOW,
                        BRAKE_RANGE_HIGH);
    lv_slider_set_value(get_tab_calibration_slider(),
                        secondary_pedals_output_last_state.bse_front,
                        LV_ANIM_OFF);
  }
#endif
}

void imu_acceleration_update() {
  sprintf(sprintf_buffer, "%.4f",
          secondary_imu_acceleration_last_state.accel_x);
  set_tab_track_test_lb_inverter_speed_x(sprintf_buffer);

  sprintf(sprintf_buffer, "%.4f",
          secondary_imu_acceleration_last_state.accel_y);
  set_tab_track_test_lb_inverter_speed_y(sprintf_buffer);
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
  int seconds =
      (int)(secondary_lc_status_last_state.best_time - minutes * 60.0f);
  sprintf(sprintf_buffer, "%02d:%02d", minutes, seconds);

  set_tab_racing_lb_best_time(sprintf_buffer);

  float delta = secondary_lc_status_last_state.last_time -
                secondary_lc_status_last_state.best_time;
  if (delta != current_delta_state) {
    sprintf(sprintf_buffer, "%+.2f", delta);

    set_tab_racing_lb_delta_time(sprintf_buffer);
  }

  if (secondary_lc_status_last_state.last_time !=
          secondary_lc_status_last_state.last_time &&
      !on_lap_keep) {
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

#define INVERTER_MESSAGE_UNINITIALIZED -100.0f
#define INVERTER_TEMP_CONVERSION(raw_temp)                                     \
  -43.23745 + 0.01073427 * raw_temp - 5.523417e-7 * pow(raw_temp, 2) +         \
      1.330787e-11 * pow(raw_temp, 3);
float l_motor_temp = INVERTER_MESSAGE_UNINITIALIZED;
float l_igbt_temp = INVERTER_MESSAGE_UNINITIALIZED;
float r_motor_temp = INVERTER_MESSAGE_UNINITIALIZED;
float r_igbt_temp = INVERTER_MESSAGE_UNINITIALIZED;

void inv_l_rcv_update(void) {
  if (inverters_inv_l_rcv_last_state.rcv_mux ==
      INVERTERS_INV_L_RCV_RCV_MUX_ID_49_T_MOTOR_CHOICE) {
    l_motor_temp = (inverters_inv_l_rcv_last_state.t_motor - 9393.9f) / 55.1f;
    sprintf(sprintf_buffer, "%.0f", l_motor_temp);
    set_tab_racing_lb_motor_temp(sprintf_buffer);

    if (r_motor_temp != INVERTER_MESSAGE_UNINITIALIZED) {
      sprintf(sprintf_buffer, "%.0f", (l_motor_temp + r_motor_temp) / 2.0f);
      set_tab_sensors_lb_left_motor_temp(sprintf_buffer);
    }
  }

  if (inverters_inv_l_rcv_last_state.rcv_mux ==
      INVERTERS_INV_L_RCV_RCV_MUX_ID_4A_T_IGBT_CHOICE) {
    l_igbt_temp =
        INVERTER_TEMP_CONVERSION(inverters_inv_l_rcv_last_state.t_igbt);
    sprintf(sprintf_buffer, "%.0f", l_igbt_temp);
    set_tab_sensors_lb_left_inverter_temp(sprintf_buffer);

    if (r_igbt_temp != INVERTER_MESSAGE_UNINITIALIZED) {
      sprintf(sprintf_buffer, "%.0f", (l_igbt_temp + r_igbt_temp) / 2.0f);
      set_tab_racing_lb_inverter_temp(sprintf_buffer);
    }
  }
}

void inv_r_rcv_update() {
  if (inverters_inv_r_rcv_last_state.rcv_mux ==
      INVERTERS_INV_R_RCV_RCV_MUX_ID_49_T_MOTOR_CHOICE) {
    r_motor_temp = (inverters_inv_r_rcv_last_state.t_motor - 9393.9f) / 55.1f;
    sprintf(sprintf_buffer, "%.0f", r_motor_temp);
    set_tab_sensors_lb_right_motor_temp(sprintf_buffer);
  }

  if (inverters_inv_r_rcv_last_state.rcv_mux ==
      INVERTERS_INV_R_RCV_RCV_MUX_ID_4A_T_IGBT_CHOICE) {
    r_igbt_temp =
        INVERTER_TEMP_CONVERSION(inverters_inv_r_rcv_last_state.t_igbt);
    sprintf(sprintf_buffer, "%.0f", r_igbt_temp);
    set_tab_sensors_lb_right_inverter_temp(sprintf_buffer);
  }
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

void handle_ptt_message(void) {
  primary_ptt_status_status val = primary_ptt_status_last_state.status;
  if (val == primary_ptt_status_status_OFF) {
    ecu_ack = false;
  } else if (val == primary_ptt_status_status_ON) {
    ecu_ack = true;
  }
}
