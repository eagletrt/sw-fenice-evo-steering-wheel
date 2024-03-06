#include "controls.h"

bool tson_button_pressed = false;
bool calibration_min_sent_request[CALBOX_N];
bool calibration_max_sent_request[CALBOX_N];
uint32_t calibration_min_request_timestamp[CALBOX_N];
uint32_t calibration_max_request_timestamp[CALBOX_N];

int torque_vectoring_last_state = 0;
int slip_control_last_state = 0;
int set_pumps_speed_last_state = 0;
int set_radiators_last_state = 0;
int pork_fans_status_last_state = 0;
int power_map_last_state = 0;

#if CANSNIFFER_ENABLED == 1
int primary_cansniffer_start_index = 0;
int secondary_cansniffer_start_index = 0;
#endif

#if STEER_TAB_CALIBRATION_ENABLED == 1
extern lv_obj_t *set_min_btn;
extern lv_obj_t *set_max_btn;
#endif
char sprintf_buffer_controls[BUFSIZ];

primary_steer_status_converted_t steer_status_last_state = {
    .map_pw = 0.0f, .map_sc = 0.0f, .map_tv = 0.0f};
primary_cooling_status_converted_t steering_cooling_settings = {
    .pumps_speed = -1.0f, .radiators_speed = -1.0f};
primary_hv_fans_override_converted_t hv_fans_override_settings = {
    .fans_override = primary_hv_fans_override_fans_override_OFF,
    .fans_speed = 0.0f};

int imin(int x, int y) { return x > y ? y : x; }
int imax(int x, int y) { return x > y ? x : y; }

void set_dmt_steering_angle_target(void) {
  GET_LAST_STATE(secondary, steering_angle, SECONDARY, STEERING_ANGLE);
  set_tab_track_test_dmt_steering_angle_target(
      secondary_steering_angle_last_state->angle);
}

void turn_telemetry_on_off(void) {
  GET_LAST_STATE(primary, tlm_status, PRIMARY, TLM_STATUS);
  primary_set_tlm_status_converted_t converted = {0};
  if (primary_tlm_status_last_state->tlm_status ==
      (primary_set_tlm_status_tlm_status)primary_set_tlm_status_tlm_status_ON) {
    display_notification("Sending Telemetry OFF", 800);
    converted.tlm_status = primary_set_tlm_status_tlm_status_OFF;
  } else {
    display_notification("Sending Telemetry ON", 800);
    converted.tlm_status = primary_set_tlm_status_tlm_status_ON;
  }
  STEER_CAN_PACK(primary, PRIMARY, set_tlm_status, SET_TLM_STATUS);
  can_send(&msg, true);
}

void manettino_right_actions(int dsteps) {
  if (engineer_mode) {

  } else {
    switch (current_racing_tab) {

    case NOT_SCREEN:
      break;
    case TAB_RACING:
      torque_vectoring_last_state += dsteps * 10;
      torque_vectoring_last_state =
          imin(torque_vectoring_last_state, TORQUE_VECTORING_MAX);
      torque_vectoring_last_state =
          imax(torque_vectoring_last_state, TORQUE_VECTORING_MIN);
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
          imin(pork_fans_status_last_state, PORK_HIGH_FANS_SPEED);
      pork_fans_status_last_state =
          imax(pork_fans_status_last_state, PORK_LOW_FANS_SPEED);
      send_pork_fans_status((float)pork_fans_status_last_state / 100.0f);
      break;
    case TAB_LV:
      set_radiators_last_state += dsteps * 10;
      set_radiators_last_state =
          imin(set_radiators_last_state, SET_RADIATORS_MAX);
      set_radiators_last_state =
          imax(set_radiators_last_state, SET_RADIATORS_MIN);
      manettino_send_set_radiators((float)set_radiators_last_state / 100.f);
      break;
    default:
      break;
    }
  }
}

void manettino_center_actions(int dsteps) {
  if (engineer_mode) {

  } else {
    switch (current_racing_tab) {
      power_map_last_state += (dsteps * 10);
      power_map_last_state = imin(power_map_last_state, POWER_MAP_MAX);
      power_map_last_state = imax(power_map_last_state, POWER_MAP_MIN);
    case NOT_SCREEN:
      break;
    case TAB_RACING:
      manettino_send_power_map((float)power_map_last_state / 100.0f);
      break;
    case TAB_TRACK_TEST:
      manettino_send_power_map((float)power_map_last_state / 100.0f);
      break;
    case TAB_SENSORS:
      manettino_send_power_map((float)power_map_last_state / 100.0f);
      break;
    case TAB_HV:
      manettino_send_power_map((float)power_map_last_state / 100.0f);
      break;
    case TAB_LV:
      manettino_send_power_map((float)power_map_last_state / 100.0f);
      break;
    default:
      break;
    }
  }
}

void manettino_left_actions(int dsteps) {
  if (engineer_mode) {

  } else {
    switch (current_racing_tab) {
    case NOT_SCREEN:
      break;
    case TAB_RACING:
      slip_control_last_state += dsteps * 10;
      slip_control_last_state = imin(slip_control_last_state, SLIP_CONTROL_MAX);
      slip_control_last_state = imax(slip_control_last_state, SLIP_CONTROL_MIN);
      manettino_send_slip_control((float)slip_control_last_state / 100.0f);
      //      if (steer_status_last_state.map_sc + (float)dsteps / 10.0f > 0.0f
      //      && steer_status_last_state.map_sc + (float)dsteps / 10.0f < 1.0f)
      //      {
      //        manettino_send_slip_control(steer_status_last_state.map_sc +
      //        (float)dsteps / 10.0f);
      //      }else if (steer_status_last_state.map_sc + (float)dsteps / 10.0f <
      //      0.0f) {
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
      break;
    case TAB_LV:
      set_pumps_speed_last_state += dsteps * 10;
      set_pumps_speed_last_state =
          imin(set_pumps_speed_last_state, SET_PUMP_SPEED_MAX);
      set_pumps_speed_last_state =
          imax(set_pumps_speed_last_state, SET_PUMP_SPEED_MIN);
      manettino_send_set_pumps_speed((float)set_pumps_speed_last_state /
                                     100.0f);
      break;
    default:
      break;
    }
  }
}

void manettino_send_slip_control(float val) {
  steer_status_last_state.map_sc = val;
  int map_val = (int)(steer_status_last_state.map_sc * 100.0f);
  sprintf(sprintf_buffer_controls, "%u", map_val);
  set_tab_racing_label_text(sprintf_buffer_controls, tab_rac_slip_idx);
}

void manettino_send_torque_vectoring(float val) {
  steer_status_last_state.map_tv = val;
  int map_val = (int)(steer_status_last_state.map_tv * 100.0f);
  sprintf(sprintf_buffer_controls, "%u", map_val);
  set_tab_racing_label_text(sprintf_buffer_controls, tab_rac_torque_idx);
}

void manettino_send_power_map(float val) {
  steer_status_last_state.map_pw = val;
  float map_val = (float)(steer_status_last_state.map_pw * 100.0f);
  sprintf(sprintf_buffer_controls, "%.0f", map_val);
  set_tab_racing_label_text(sprintf_buffer_controls, tab_rac_pow_idx);
}

void manettino_send_set_pumps_speed(float val) {
  steering_cooling_settings.pumps_speed = val;

  primary_set_pumps_speed_converted_t converted = {0};
  converted.pumps_speed = steering_cooling_settings.pumps_speed;
  STEER_CAN_PACK(primary, PRIMARY, set_pumps_speed, SET_PUMPS_SPEED);
  can_send(&msg, true);

  if (val < 0.0f) {
    lv_pumps_speed_bar_invalidate();
    set_tab_lv_label_text("0", tab_lv_lb_pumps_local);
  } else {
    lv_set_pumps_speed_bar((int32_t)val * 100);
    int map_val = (int)(steering_cooling_settings.pumps_speed * 100);
    sprintf(sprintf_buffer_controls, "%u", map_val);
    set_tab_lv_label_text(sprintf_buffer_controls, tab_lv_lb_pumps_local);
  }

#if 0
  int map_val = (int)(steering_cooling_settings.pumps_speed * 100.0f);

  if (steering_cooling_settings.pumps_speed < 0.0f) {
    sprintf(sprintf_buffer_controls, "PUMPS SPEED AUTO");
  } else {
    sprintf(sprintf_buffer_controls, "PUMPS SPEED %d", map_val);
  }
  display_notification(sprintf_buffer_controls, 750);
#endif
}

void manettino_send_set_radiators(float val) {
  steering_cooling_settings.radiators_speed = val;
  primary_set_radiator_speed_converted_t converted = {0};
  converted.radiators_speed = steering_cooling_settings.radiators_speed;
  STEER_CAN_PACK(primary, PRIMARY, set_radiator_speed, SET_RADIATOR_SPEED);
  can_send(&msg, true);

  if (val < 0.0f) {
    lv_radiators_speed_bar_invalidate();
    set_tab_lv_label_text("0", tab_lv_lb_radiators_local);
  } else {
    lv_set_radiators_speed_bar(
        (int32_t)steering_cooling_settings.radiators_speed * 100);
    int map_val = (int)(steering_cooling_settings.radiators_speed * 100);
    sprintf(sprintf_buffer_controls, "%u", map_val);
    set_tab_lv_label_text(sprintf_buffer_controls, tab_lv_lb_radiators_local);
  }

#if 0
  int map_val = (int)(steering_cooling_settings.radiators_speed * 100.0f);

  if (steering_cooling_settings.radiators_speed < 0.0f) {
    sprintf(sprintf_buffer_controls, "RADIATOR SPEED AUTO");
  } else {
    sprintf(sprintf_buffer_controls, "RADIATORS SPEED %d", map_val);
  }
  // display_notification(sprintf_buffer_controls, 750);
#endif
}

void send_pork_fans_status(float val) {
  if (val < 0.0f) {
    hv_fans_override_settings.fans_override =
        primary_hv_fans_override_fans_override_OFF;
  } else {
    hv_fans_override_settings.fans_override =
        primary_hv_fans_override_fans_override_ON;
    hv_fans_override_settings.fans_speed = fmax(0.0f, fmin(1.0f, val));
  }
  primary_hv_fans_override_converted_t converted = {0};
  converted.fans_override = hv_fans_override_settings.fans_override;
  converted.fans_speed = hv_fans_override_settings.fans_speed;
  STEER_CAN_PACK(primary, PRIMARY, hv_fans_override, HV_FANS_OVERRIDE);
  can_send(&msg, true);

  if (val < 0.0f) {
    tab_hv_pork_speed_bar_invalidate();
  } else {
    tab_hv_set_pork_speed_bar((int32_t)val * 100);
  }

#if 0
  int map_val = (int)(hv_fans_override_settings.fans_speed * 100.0f);
  if (hv_fans_override_settings.fans_override ==
      primary_hv_fans_override_fans_override_OFF) {
    sprintf(sprintf_buffer_controls, "FANS AUTO");
  } else {
    sprintf(sprintf_buffer_controls, "FANS %d", map_val);
  }
  display_notification(sprintf_buffer_controls, 750);
#endif
}

void send_set_car_status(primary_set_car_status_car_status_set val) {
  primary_set_car_status_converted_t converted = {0};
  converted.car_status_set = val;
  STEER_CAN_PACK(primary, PRIMARY, set_car_status, SET_CAR_STATUS);
  can_send(&msg, true);
  can_send(&msg, true);
}

void prepare_set_car_status(void) {
  GET_LAST_STATE(primary, car_status, PRIMARY, CAR_STATUS);
  switch (primary_car_status_last_state->car_status) {
  case primary_car_status_car_status_INIT:
  case primary_car_status_car_status_ENABLE_INV_UPDATES:
  case primary_car_status_car_status_CHECK_INV_SETTINGS: {
    send_set_car_status(primary_set_car_status_car_status_set_IDLE);
    display_notification("ECU not in IDLE yet, sending IDLE anyway", 1500);
    break;
  }
  case primary_car_status_car_status_IDLE: {
    send_set_car_status(primary_set_car_status_car_status_set_READY);
    display_notification("TSON", 1500);
    break;
  }
  case primary_car_status_car_status_START_TS_PRECHARGE:
  case primary_car_status_car_status_WAIT_TS_PRECHARGE: {
    display_notification("Precharge not finished yet", 1500);
    break;
  }
  case primary_car_status_car_status_WAIT_DRIVER: {
    send_set_car_status(primary_set_car_status_car_status_set_DRIVE);
    display_notification("DRIVE", 1500);
    break;
  }
  case primary_car_status_car_status_ENABLE_INV_DRIVE:
  case primary_car_status_car_status_DRIVE:
  case primary_car_status_car_status_DISABLE_INV_DRIVE:
  case primary_car_status_car_status_START_TS_DISCHARGE:
  case primary_car_status_car_status_WAIT_TS_DISCHARGE: {
    send_set_car_status(primary_set_car_status_car_status_set_IDLE);
    display_notification("IDLE", 1500);
    break;
  }
  case primary_car_status_car_status_FATAL_ERROR: {
    send_set_car_status(primary_set_car_status_car_status_set_IDLE);
    display_notification("ECU in FATAL ERROR, sending IDLE anyway", 1500);
    break;
  }
  }
}

bool send_set_car_status_directly(void) {
  GET_LAST_STATE(primary, car_status, PRIMARY, CAR_STATUS);
  bool retval = true;
  switch (primary_car_status_last_state->car_status) {
  case primary_car_status_car_status_IDLE:
  case primary_car_status_car_status_WAIT_DRIVER: {
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
#if ENGINEERING_TAB_ENABLED
  if (!engineer_mode)
    return;
  if (current_engineer_tab == TAB_PRIMARY_CANSNIFFER) {
    switch_primary_cansniffer();
  } else if (current_engineer_tab == TAB_SECONDARY_CANSNIFFER) {
    switch_secondary_cansniffer();
  }
#endif // ENGINEERING_TAB_ENABLED
}

void change_cansniffer_index(bool plus) {
#if ENGINEERING_TAB_ENABLED == 1
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
#endif // ENGINEERING_TAB_ENABLED
}
#endif // CANSNIFFER_ENABLED
