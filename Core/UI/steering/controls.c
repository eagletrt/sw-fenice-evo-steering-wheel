#include "controls.h"

bool tson_button_pressed = false;
bool calibration_min_sent_request[CALBOX_N];
bool calibration_max_sent_request[CALBOX_N];
uint32_t calibration_min_request_timestamp[CALBOX_N];
uint32_t calibration_max_request_timestamp[CALBOX_N];

int torque_vectoring_last_state = 0;
int slip_control_last_state = 0;
int pork_fans_status_last_state = 0;
int power_map_last_state = 0;

int left_manettino_selection = 0;

#if CANSNIFFER_ENABLED == 1
int primary_cansniffer_start_index = 0;
int secondary_cansniffer_start_index = 0;
#endif

#if STEER_TAB_CALIBRATION_ENABLED == 1
extern lv_obj_t *set_min_btn;
extern lv_obj_t *set_max_btn;
#endif
char sprintf_buffer_controls[BUFSIZ];

primary_ecu_set_power_maps_converted_t ecu_set_power_maps_last_state = {
    .map_pw = 0.0f, .map_sc = 0.0f, .map_tv = 0.0f};

primary_lv_radiator_speed_converted_t steering_wheel_state_radiator_speed = {
    .radiator_speed = 0.0f, .status = primary_lv_radiator_speed_status_off};
primary_lv_pumps_speed_converted_t steering_wheel_state_pumps_speed = {
    .pumps_speed = 0.0f, .status = primary_lv_radiator_speed_status_off};
primary_hv_fans_status_converted_t hv_fans_override_settings = {
    .fans_override = primary_hv_fans_status_fans_override_off,
    .fans_speed = 0.0f};

uint32_t steering_wheel_lv_pumps_speed_sent_timestamp = 0;
uint32_t steering_wheel_lv_radiators_speed_sent_timestamp = 0;
steering_wheel_cooling_status_t steering_wheel_lv_pumps_speed_state =
    STEERING_WHEEL_COOLING_STATUS_SYNC;
steering_wheel_cooling_status_t steering_wheel_lv_radiator_speed_state =
    STEERING_WHEEL_COOLING_STATUS_SYNC;

void set_dmt_steering_angle_target(void) {
  GET_LAST_STATE(secondary, steer_angle, SECONDARY, STEER_ANGLE);
  set_tab_track_test_dmt_steering_angle_target(
      secondary_steer_angle_last_state->angle);
}

void turn_telemetry_on_off(void) {
  GET_LAST_STATE(primary, tlm_status, PRIMARY, TLM_STATUS);
  primary_tlm_set_status_converted_t converted = {0};
  if (primary_tlm_status_last_state->status == primary_tlm_status_status_on) {
    display_notification("Sending Telemetry OFF", 800);
    converted.status = primary_tlm_set_status_status_off;
  } else {
    display_notification("Sending Telemetry ON", 800);
    converted.status = primary_tlm_set_status_status_on;
  }
  STEER_CAN_PACK(primary, PRIMARY, tlm_set_status, TLM_SET_STATUS);
  can_send(&msg, true);
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
    case TAB_RACING:
      torque_vectoring_last_state += dsteps * 10;
      torque_vectoring_last_state =
          fminf(torque_vectoring_last_state, TORQUE_VECTORING_MAX);
      torque_vectoring_last_state =
          fmaxf(torque_vectoring_last_state, TORQUE_VECTORING_MIN);
      manettino_send_torque_vectoring((float)torque_vectoring_last_state /
                                      100.0f);
      break;
    case TAB_TRACK_TEST:
      break;
    case TAB_SENSORS:
      break;
    case TAB_HV:
      pork_fans_status_last_state += dsteps * 10;
      pork_fans_status_last_state =
          fminf(pork_fans_status_last_state, PORK_HIGH_FANS_SPEED);
      pork_fans_status_last_state =
          fmaxf(pork_fans_status_last_state, PORK_LOW_FANS_SPEED);
      send_pork_fans_status((float)pork_fans_status_last_state / 100.0f);
      break;
    case TAB_LV: {
      steering_wheel_lv_radiator_speed_state = STEERING_WHEEL_COOLING_STATUS_SET;
      float new_radspeed_val = steering_wheel_state_radiator_speed.radiator_speed + ((float)dsteps / 10.0f);
      new_radspeed_val = fminf(new_radspeed_val, SET_RADIATORS_MAX);
      new_radspeed_val = fmaxf(new_radspeed_val, SET_RADIATORS_MIN);
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
    case TAB_RACING:
      break;
    case TAB_TRACK_TEST:
      break;
    case TAB_SENSORS:
      break;
    case TAB_HV:
      break;
    case TAB_LV:
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
    case TAB_RACING:
      slip_control_last_state += dsteps * 10;
      slip_control_last_state =
          fminf(slip_control_last_state, SLIP_CONTROL_MAX);
      slip_control_last_state =
          fmaxf(slip_control_last_state, SLIP_CONTROL_MIN);
      manettino_send_slip_control((float)slip_control_last_state / 100.0f);
      //      if (ecu_set_power_maps_last_state.map_sc + (float)dsteps / 10.0f >
      //      0.0f
      //      && ecu_set_power_maps_last_state.map_sc + (float)dsteps / 10.0f
      //      < 1.0f)
      //      {
      //        manettino_send_slip_control(ecu_set_power_maps_last_state.map_sc
      //        + (float)dsteps / 10.0f);
      //      }else if (ecu_set_power_maps_last_state.map_sc + (float)dsteps
      //      / 10.0f < 0.0f) {
      //        manettino_send_slip_control(0.0f);
      //      }else {
      //          manettino_send_slip_control(1.0f);
      //      }
      break;
    case TAB_TRACK_TEST:
      break;
    case TAB_SENSORS:
      break;
    case TAB_HV:
      // TODO: implement Balancing threshold as defined by the new steering
      // wheel map (see images)
      break;
    case TAB_LV: {
      steering_wheel_lv_pumps_speed_state = STEERING_WHEEL_COOLING_STATUS_SET;
      float new_pumps_speed_val = steering_wheel_state_pumps_speed.pumps_speed + ((float)dsteps / 10.0f);
      new_pumps_speed_val = fminf(new_pumps_speed_val, SET_PUMP_SPEED_MAX);
      new_pumps_speed_val = fmaxf(new_pumps_speed_val, SET_PUMP_SPEED_MIN);
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
  int map_val = (int)(ecu_set_power_maps_last_state.map_sc * 100.0f);
  snprintf(sprintf_buffer_controls, BUFSIZ, "%u", map_val);
  set_tab_racing_label_text(sprintf_buffer_controls, tab_rac_slip_idx);
}

void manettino_send_torque_vectoring(float val) {
  ecu_set_power_maps_last_state.map_tv = val;
  int map_val = (int)(ecu_set_power_maps_last_state.map_tv * 100.0f);
  snprintf(sprintf_buffer_controls, BUFSIZ, "%u", map_val);
  set_tab_racing_label_text(sprintf_buffer_controls, tab_rac_torque_idx);
}

void manettino_send_power_map(float val) {
  ecu_set_power_maps_last_state.map_pw = val;
  float map_val = (float)(ecu_set_power_maps_last_state.map_pw * 100.0f);
  snprintf(sprintf_buffer_controls, BUFSIZ, "%.0f", map_val);
  set_tab_racing_label_text(sprintf_buffer_controls, tab_rac_pow_idx);
}

void manettino_send_radiators_speed() {
  primary_lv_set_radiator_speed_converted_t converted = {0};
  converted.status = steering_wheel_state_radiator_speed.status;
  converted.radiator_speed = steering_wheel_state_radiator_speed.radiator_speed;
  STEER_CAN_PACK(primary, PRIMARY, lv_set_radiator_speed, LV_SET_RADIATOR_SPEED);
  can_send(&msg, true);
}

void manettino_set_radiators_speed() {
  steering_wheel_lv_radiators_speed_sent_timestamp = get_current_time_ms();
  steering_wheel_state_radiator_speed.status = primary_lv_radiator_speed_status_manual;

  if (steering_wheel_state_radiator_speed.radiator_speed < 0.0f) {
    steering_wheel_state_radiator_speed.radiator_speed = 0.0f;
    snprintf(sprintf_buffer_controls, BUFSIZ, "zero");
  } else {
    snprintf(sprintf_buffer_controls, BUFSIZ, "%0.1f", steering_wheel_state_radiator_speed.radiator_speed);
  }
  set_tab_lv_label_text(sprintf_buffer_controls, tab_lv_lb_radiators_local);
  manettino_send_radiators_speed();
}

void manettino_send_pumps_speed() {
  primary_lv_set_pumps_speed_converted_t converted = {0};
  converted.status = steering_wheel_state_pumps_speed.status;
  converted.pumps_speed = steering_wheel_state_pumps_speed.pumps_speed;
  STEER_CAN_PACK(primary, PRIMARY, lv_set_pumps_speed, LV_SET_PUMPS_SPEED);
  can_send(&msg, true);
}

void manettino_set_pumps_speed() {
  steering_wheel_lv_pumps_speed_sent_timestamp = get_current_time_ms();
  steering_wheel_state_pumps_speed.status = primary_lv_pumps_speed_status_manual;

  if (steering_wheel_state_pumps_speed.pumps_speed < 0.0f) {
    steering_wheel_state_pumps_speed.pumps_speed = 0.0f;
    snprintf(sprintf_buffer_controls, BUFSIZ, "zero");
  } else {
    snprintf(sprintf_buffer_controls, BUFSIZ, "%0.1f", steering_wheel_state_pumps_speed.pumps_speed);
  }
  set_tab_lv_label_text(sprintf_buffer_controls, tab_lv_lb_pumps_local);
  manettino_send_pumps_speed();
}

void set_cooling_auto_mode() {
  steering_wheel_state_pumps_speed.status = primary_lv_pumps_speed_status_auto;
  steering_wheel_state_pumps_speed.pumps_speed = 0.0f;

  steering_wheel_state_radiator_speed.status = primary_lv_radiator_speed_status_auto;
  steering_wheel_state_radiator_speed.radiator_speed = 0.0f;

  manettino_send_pumps_speed();
  manettino_send_radiators_speed();
}

void buttons_pressed_actions(uint8_t button) {
  switch (button) {
  case PADDLE_TOP_RIGHT: {
    steering_change_tab(true);
    break;
  }
  case PADDLE_TOP_LEFT: {
    steering_change_tab(false);
    break;
  }
  case PADDLE_BOTTOM_RIGHT: {
    if (!engineer_mode) {
#if STEER_TAB_CALIBRATION_ENABLED == 1
      shift_box_focus(true);
#endif
#if STEER_TAB_DEBUG_ENABLED == 1
      change_errors_view(false);
#endif
    } else {
#if CANSNIFFER_ENABLED == 1
      change_cansniffer_index(true);
#endif
    }
    break;
  }
  case PADDLE_BOTTOM_LEFT: {
    if (!engineer_mode) {
#if STEER_TAB_CALIBRATION_ENABLED == 1
      shift_box_focus(false);
#endif
#if STEER_TAB_DEBUG_ENABLED == 1
      change_errors_view(true);
#endif
    } else {
#if CANSNIFFER_ENABLED == 1
      change_cansniffer_index(false);
#endif
    }
    break;
  }
  case BUTTON_TOP_RIGHT:
    if (engineer_mode) {
#if CANSNIFFER_ENABLED == 1
      switch_cansniffer();
#endif
    } else {
      set_dmt_steering_angle_target();
    }
    break;
  case BUTTON_TOP_LEFT:
    if (engineer_mode) {
    } else {
      set_ptt_button_pressed(true);
    }
    break;
  case BUTTON_BOTTOM_RIGHT: {
    if (engineer_mode) {
      send_bal(true);
      display_notification("BAL ON", 500);
    } else {
      if (current_racing_tab == TAB_HV) {
        send_bal(true);
        display_notification("BAL ON", 500);
      }
    }
    break;
  }
  case BUTTON_BOTTOM_LEFT: {
    if (engineer_mode) {
      send_bal(false);
      display_notification("BAL OFF", 500);
    } else {
      if (current_racing_tab == TAB_HV) {
        send_bal(false);
        display_notification("BAL OFF", 500);
      }

      if (current_racing_tab == TAB_RACING) {
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
      } else if (current_racing_tab == TAB_LV) {
        set_cooling_auto_mode();
      }
    }
    break;
  }

  break;
  }
}

void buttons_released_actions(uint8_t button) {
  switch (button) {
  case PADDLE_TOP_RIGHT:
    break;
  case PADDLE_TOP_LEFT:
    break;
  case PADDLE_BOTTOM_RIGHT:
    break;
  case PADDLE_BOTTOM_LEFT:
    break;
  case BUTTON_TOP_RIGHT:
    break;
  case BUTTON_TOP_LEFT:
    set_ptt_button_pressed(false);
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
    switch_mode();
    break;
  case PADDLE_TOP_LEFT:
    break;
  case PADDLE_BOTTOM_RIGHT:
    break;
  case PADDLE_BOTTOM_LEFT:
    break;
  case BUTTON_TOP_RIGHT: {
    if (engineer_mode) {
    } else {
      turn_telemetry_on_off();
    }
    break;
  }
  case BUTTON_TOP_LEFT:
    break;
  case BUTTON_BOTTOM_RIGHT:
    break;
  case BUTTON_BOTTOM_LEFT:
    break;
  }
}

void send_pork_fans_status(float val) {
  if (val < 0.0f) {
    hv_fans_override_settings.fans_override =
        primary_hv_fans_status_fans_override_off;
  } else {
    hv_fans_override_settings.fans_override =
        primary_hv_fans_status_fans_override_on;
    hv_fans_override_settings.fans_speed = fmax(0.0f, fmin(1.0f, val));
  }
  primary_hv_fans_status_converted_t converted = {0};
  converted.fans_override = hv_fans_override_settings.fans_override;
  converted.fans_speed = hv_fans_override_settings.fans_speed;
  STEER_CAN_PACK(primary, PRIMARY, hv_set_fans_status, HV_SET_FANS_STATUS);
  can_send(&msg, true);

  bool auto_mode = hv_fans_override_settings.fans_override ==
                   primary_hv_fans_status_fans_override_off;

  if (val < 0.0f) { // TODO: move outside of cansend
  } else {
    tab_hv_set_pork_speed_bar((int32_t)(val * 100), auto_mode);
  }

#if 0
  int map_val = (int)(hv_fans_override_settings.fans_speed * 100.0f);
  if (hv_fans_override_settings.fans_override ==
      primary_hv_fans_override_fans_override_OFF) {
    snprintf(sprintf_buffer_controls, BUFSIZ, "FANS AUTO");
  } else {
    snprintf(sprintf_buffer_controls, BUFSIZ, "FANS %d", map_val);
  }
  display_notification(sprintf_buffer_controls, 750);
#endif
}

void send_set_car_status(primary_ecu_set_status_status val) {
  primary_ecu_set_status_converted_t converted = {0};
  converted.status = val;
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
    display_notification("ECU not in IDLE yet, sending IDLE anyway", 1500);
    break;
  }
  case primary_ecu_status_status_idle: {
    send_set_car_status(primary_ecu_set_status_status_ready);
    display_notification("TSON", 1500);
    break;
  }
  case primary_ecu_status_status_start_ts_precharge:
  case primary_ecu_status_status_wait_ts_precharge: {
    display_notification("Precharge not finished yet", 1500);
    break;
  }
  case primary_ecu_status_status_wait_driver: {
    send_set_car_status(primary_ecu_set_status_status_drive);
    display_notification("DRIVE", 1500);
    break;
  }
  case primary_ecu_status_status_enable_inv_drive:
  case primary_ecu_status_status_drive:
  case primary_ecu_status_status_disable_inv_drive:
  case primary_ecu_status_status_start_ts_discharge:
  case primary_ecu_status_status_wait_ts_discharge: {
    send_set_car_status(primary_ecu_set_status_status_idle);
    display_notification("IDLE", 1500);
    break;
  }
  case primary_ecu_status_status_fatal_error: {
    send_set_car_status(primary_ecu_set_status_status_idle);
    display_notification("ECU in FATAL ERROR, sending IDLE anyway", 1500);
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
void switch_cansniffer(void) {
  if (!engineer_mode)
    return;
  if (current_engineer_tab == TAB_PRIMARY_CANSNIFFER) {
    switch_primary_cansniffer();
  } else if (current_engineer_tab == TAB_SECONDARY_CANSNIFFER) {
    switch_secondary_cansniffer();
  }
}

void change_cansniffer_index(bool plus) {
  if (current_engineer_tab == TAB_PRIMARY_CANSNIFFER) {
    if (plus) {
      primary_cansniffer_start_index++;
      update_primary_cansniffer_ui(NULL);
    } else {
      primary_cansniffer_start_index--;
      primary_cansniffer_start_index = fmax(primary_cansniffer_start_index, 0);
      update_primary_cansniffer_ui(NULL);
    }
  } else if (current_engineer_tab == TAB_SECONDARY_CANSNIFFER) {
    if (plus) {
      secondary_cansniffer_start_index++;
      update_secondary_cansniffer_ui(NULL);
    } else {
      secondary_cansniffer_start_index--;
      secondary_cansniffer_start_index =
          fmax(secondary_cansniffer_start_index, 0);
      update_secondary_cansniffer_ui(NULL);
    }
  }
}
#endif // CANSNIFFER_ENABLED
