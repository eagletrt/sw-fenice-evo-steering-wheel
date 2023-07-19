#include "can_messages.h"

bool primary_messages_updated[PRIMARY_MONITORED_MESSAGES_SIZE] = {false};
bool secondary_messages_updated[SECONDARY_MONITORED_MESSAGES_SIZE] = {false};

extern steering_t steering;
extern bool steering_initialized;
primary_steer_status_converted_t steer_status_last_state = {
    .map_pw = 0.0f, .map_sc = 0.0f, .map_tv = 0.0f};
primary_cooling_status_converted_t cooling_status_last_state = {
    .pumps_speed = -1.0f, .radiators_speed = -1.0f};



void send_steer_version(lv_timer_t *main_timer) {
  primary_steer_version_converted_t converted = {
      .canlib_build_time = CANLIB_BUILD_TIME, .component_version = 1};
  STEER_CAN_PACK(primary, PRIMARY, steer_version, STEER_VERSION)
  can_send(&msg, true);
}

void send_steer_status(lv_timer_t *main_timer) {
  primary_steer_status_converted_t converted = {
      .map_pw = steer_status_last_state.map_pw,
      .map_sc = steer_status_last_state.map_sc,
      .map_tv = steer_status_last_state.map_tv,
  };
  STEER_CAN_PACK(primary, PRIMARY, steer_status, STEER_STATUS)
  can_send(&msg, true);
}



void handle_primary(can_message_t *msg) {
  if (!steering_initialized)
    return;
#if CAN_LOG_ENABLED
  char name_buffer[BUFSIZ];
  primary_message_name_from_id(msg->id, name_buffer);
  print("Primary network - message id %s\n", name_buffer);
#endif
  can_id_t id = msg->id;
  switch (id) {
  case PRIMARY_CAR_STATUS_FRAME_ID: {
    STEER_CAN_UNPACK(primary, PRIMARY, car_status, CAR_STATUS);
    car_status_update(&converted);
    break;
  }
  case PRIMARY_PEDAL_CALIBRATION_ACK_FRAME_ID: {
    STEER_CAN_UNPACK(primary, PRIMARY, pedal_calibration_ack,
                     PEDAL_CALIBRATION_ACK);
    pedal_calibration_ack(&converted);
    break;
  }
  case PRIMARY_STEERING_JMP_TO_BLT_FRAME_ID:
    openblt_reset();
    break;
  case PRIMARY_PTT_STATUS_FRAME_ID: {
    STEER_CAN_UNPACK(primary, PRIMARY, ptt_status, PTT_STATUS);
    handle_ptt_message(converted.status);
    break;
  }
  case PRIMARY_TLM_STATUS_FRAME_ID: {
    STEER_CAN_UNPACK(primary, PRIMARY, tlm_status, TLM_STATUS);
    // tlm_status_update(&converted);
    break;
  }
  case PRIMARY_AMBIENT_TEMPERATURE_FRAME_ID: {
    STEER_CAN_UNPACK(primary, PRIMARY, ambient_temperature,
                     AMBIENT_TEMPERATURE);
    // ambient_temperature_update(&converted);
    break;
  }
  case PRIMARY_SPEED_FRAME_ID: {
    STEER_CAN_UNPACK(primary, PRIMARY, speed, SPEED);
    speed_update(&converted);
    break;
  }
  case PRIMARY_HV_VOLTAGE_FRAME_ID: {
    STEER_CAN_UNPACK(primary, PRIMARY, hv_voltage, HV_VOLTAGE);
    hv_voltage_update(&converted);
    break;
  }
  case PRIMARY_HV_CURRENT_FRAME_ID: {
    STEER_CAN_UNPACK(primary, PRIMARY, hv_current, HV_CURRENT);
    hv_current_update(&converted);
    break;
  }
  case PRIMARY_HV_TEMP_FRAME_ID: {
    STEER_CAN_UNPACK(primary, PRIMARY, hv_temp, HV_TEMP);
    hv_temp_update(&converted);
    break;
  }
  case PRIMARY_HV_ERRORS_FRAME_ID: {
    STEER_CAN_UNPACK(primary, PRIMARY, hv_errors, HV_ERRORS);
    hv_errors_update(&converted);
    break;
  }
  case PRIMARY_HV_FEEDBACKS_STATUS_FRAME_ID: {
    STEER_CAN_UNPACK(primary, PRIMARY, hv_feedbacks_status,
                     HV_FEEDBACKS_STATUS);
    hv_feedbacks_status_update(&converted);
    break;
  }
  case PRIMARY_DAS_ERRORS_FRAME_ID: {
    STEER_CAN_UNPACK(primary, PRIMARY, das_errors, DAS_ERRORS);
    das_errors_update(&converted);
    break;
  }
  case PRIMARY_LV_CURRENTS_FRAME_ID: {
    STEER_CAN_UNPACK(primary, PRIMARY, lv_currents, LV_CURRENTS);
    lv_currents_update(&converted);
    break;
  }
  case PRIMARY_LV_CELLS_TEMP_FRAME_ID: {
    STEER_CAN_UNPACK(primary, PRIMARY, lv_cells_temp, LV_CELLS_TEMP);
    lv_cells_temp_update(&converted);
    break;
  }
  case PRIMARY_LV_TOTAL_VOLTAGE_FRAME_ID: {
    STEER_CAN_UNPACK(primary, PRIMARY, lv_total_voltage, LV_TOTAL_VOLTAGE);
    lv_total_voltage_update(&converted);
    break;
  }
  case PRIMARY_LV_ERRORS_FRAME_ID: {
    STEER_CAN_UNPACK(primary, PRIMARY, lv_errors, LV_ERRORS);
    lv_errors_update(&converted);
    break;
  }
  case INVERTERS_INV_L_RCV_FRAME_ID: {
    STEER_CAN_UNPACK(inverters, INVERTERS, inv_l_rcv, INV_L_RCV);
    inv_l_rcv_update(&converted);
    break;
  }
  case INVERTERS_INV_R_RCV_FRAME_ID: {
    STEER_CAN_UNPACK(inverters, INVERTERS, inv_r_rcv, INV_R_RCV);
    inv_r_rcv_update(&converted);
    break;
  }
  case PRIMARY_CONTROL_OUTPUT_FRAME_ID: {
    STEER_CAN_UNPACK(primary, PRIMARY, control_output, CONTROL_OUTPUT);
    control_output_update(&converted);
  }
  }
}

void handle_secondary(can_message_t *msg) {
  if (!steering_initialized)
    return;
#if CAN_LOG_ENABLED
  char name_buffer[BUFSIZ];
  secondary_message_name_from_id(msg->id, name_buffer);
  print("Secondary network - message id %s\n", name_buffer);
#endif
  can_id_t id = msg->id;
  switch (id) {
  case SECONDARY_STEERING_ANGLE_FRAME_ID: {
    STEER_CAN_UNPACK(secondary, SECONDARY, steering_angle, STEERING_ANGLE);
    steering_angle_update(&converted);
    break;
  }
  case SECONDARY_PEDALS_OUTPUT_FRAME_ID: {
    STEER_CAN_UNPACK(secondary, SECONDARY, pedals_output, PEDALS_OUTPUT);
    pedals_output_update(&converted);
    break;
  }
  default:
    break;
  }
}
