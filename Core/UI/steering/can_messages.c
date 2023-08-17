#include "can_messages.h"

extern steering_tabs_t steering;
extern bool steering_initialized;
extern primary_steer_status_converted_t steer_status_last_state;

#if CANSNIFFER_ENABLED == 1
extern cansniffer_elem_t *primary_cansniffer_buffer;
extern cansniffer_elem_t *secondary_cansniffer_buffer;
extern bool cansniffer_initialized;
#endif

char name_buffer[BUFSIZ];

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

void send_bal(bool on) {
  primary_set_cell_balancing_status_converted_t converted = {
      .set_balancing_status =
          on ? primary_set_cell_balancing_status_set_balancing_status_ON
             : primary_set_cell_balancing_status_set_balancing_status_OFF};
  STEER_CAN_PACK(primary, PRIMARY, set_cell_balancing_status,
                 SET_CELL_BALANCING_STATUS)
  can_send(&msg, true);
}

void handle_primary(can_message_t *msg) {
  if (!steering_initialized)
    return;
#if CANSNIFFER_ENABLED == 1
  if (cansniffer_initialized) {
    cansniffer_primary_new_message(msg);
  }
#endif
#if CAN_LOG_ENABLED
  primary_message_name_from_id(msg->id, name_buffer);
  print("Primary network - message id %s\n", name_buffer);
#endif
  can_id_t id = msg->id;
  switch (id) {
  case PRIMARY_STEERING_JMP_TO_BLT_FRAME_ID:
    openblt_reset();
    break;
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
  case PRIMARY_COOLING_STATUS_FRAME_ID: {
    STEER_CAN_UNPACK(primary, PRIMARY, cooling_status, COOLING_STATUS);
    cooling_status_update(&converted);
    break;
  }
  case PRIMARY_PTT_STATUS_FRAME_ID: {
    STEER_CAN_UNPACK(primary, PRIMARY, ptt_status, PTT_STATUS);
    handle_ptt_message(converted.status);
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
  case PRIMARY_LV_FEEDBACKS_FRAME_ID: {
    STEER_CAN_UNPACK(primary, PRIMARY, lv_feedbacks, LV_FEEDBACKS);
    lv_feedbacks_update(&converted);
    break;
  }
  case PRIMARY_HV_FANS_OVERRIDE_STATUS_FRAME_ID: {
    STEER_CAN_UNPACK(primary, PRIMARY, hv_fans_override_status,
                     HV_FANS_OVERRIDE_STATUS);
    hv_fans_override_status_update(&converted);
    break;
  }
  case PRIMARY_TLM_STATUS_FRAME_ID: {
    STEER_CAN_UNPACK(primary, PRIMARY, tlm_status, TLM_STATUS);
    tlm_status_update(&converted);
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
  default:
    break;
  }
}

void handle_secondary(can_message_t *msg) {
#if 1
  if (!steering_initialized)
    return;
#if CANSNIFFER_ENABLED == 1
  if (cansniffer_initialized) {
    cansniffer_secondary_new_message(msg);
  }
#endif
#if CAN_LOG_ENABLED
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
  case SECONDARY_IMU_ACCELERATION_FRAME_ID: {
    STEER_CAN_UNPACK(secondary, SECONDARY, imu_acceleration, IMU_ACCELERATION);
    imu_acceleration_update(&converted);
    break;
  }
  case SECONDARY_LAP_COUNT_FRAME_ID: {
    STEER_CAN_UNPACK(secondary, SECONDARY, lap_count, LAP_COUNT);
    lap_count_update(&converted);
    break;
  }
  case SECONDARY_LC_STATUS_FRAME_ID: {
    STEER_CAN_UNPACK(secondary, SECONDARY, lc_status, LC_STATUS);
    lc_status_update(&converted);
    break;
  }
  case SECONDARY_TIMESTAMP_FRAME_ID: {
    STEER_CAN_UNPACK(secondary, SECONDARY, timestamp, TIMESTAMP);
    timestamp_update(&converted);
  }
  default:
    break;
  }
#endif
}
