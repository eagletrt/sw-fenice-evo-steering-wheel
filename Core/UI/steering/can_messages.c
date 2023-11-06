#include "can_messages.h"

extern bool steering_initialized;
extern primary_steer_status_converted_t steer_status_last_state;

extern primary_car_status_converted_t primary_car_status_last_state;
extern primary_control_output_converted_t primary_control_output_last_state;
extern primary_ambient_temperature_converted_t secondary_ambient_temperature_last_state;
extern primary_tlm_status_converted_t primary_tlm_status_last_state;
extern primary_speed_converted_t primary_speed_last_state;
extern primary_ptt_status_converted_t primary_ptt_status_last_state;
extern primary_hv_voltage_converted_t primary_hv_voltage_last_state;
extern primary_hv_current_converted_t primary_hv_current_last_state;
extern primary_hv_temp_converted_t primary_hv_temp_last_state;
extern primary_hv_errors_converted_t primary_hv_errors_last_state;
extern primary_hv_feedbacks_status_converted_t primary_hv_feedbacks_status_last_state;
extern primary_das_errors_converted_t primary_das_errors_last_state;
extern primary_lv_currents_converted_t primary_lv_currents_last_state;
extern primary_lv_cells_voltage_converted_t primary_lv_cells_voltage_last_state_1;
extern primary_lv_cells_voltage_converted_t primary_lv_cells_voltage_last_state_2;
extern primary_lv_cells_temp_converted_t primary_lv_cells_temp_last_state_1;
extern primary_lv_cells_temp_converted_t primary_lv_cells_temp_last_state_2;
extern primary_lv_cells_temp_converted_t primary_lv_cells_temp_last_state_3;
extern primary_lv_cells_temp_converted_t primary_lv_cells_temp_last_state_4;
extern primary_lv_total_voltage_converted_t primary_lv_total_voltage_last_state;
extern primary_lv_errors_converted_t primary_lv_errors_last_state;
extern primary_cooling_status_converted_t primary_cooling_status_last_state;
extern primary_hv_fans_override_status_converted_t primary_hv_fans_override_status_last_state;
extern primary_lv_feedbacks_converted_t primary_lv_feedbacks_last_state;
extern primary_hv_cell_balancing_status_converted_t primary_hv_cell_balancing_status_last_state;
extern primary_pedal_calibration_ack_converted_t primary_pedal_calibration_ack_last_state;
extern secondary_steering_angle_converted_t secondary_steering_angle_last_state;
extern secondary_pedals_output_converted_t secondary_pedals_output_last_state;
extern secondary_imu_acceleration_converted_t secondary_imu_acceleration_last_state;
extern secondary_lap_count_converted_t secondary_lap_count_last_state;
extern secondary_lc_status_converted_t secondary_lc_status_last_state;
extern secondary_timestamp_converted_t secondary_timestamp_last_state;
extern inverters_inv_r_rcv_converted_t inverters_inv_r_last_state;
extern inverters_inv_l_rcv_converted_t inverters_inv_l_last_state;

extern bool is_pmsg_new[primary_MESSAGE_COUNT];
extern bool is_smsg_new[secondary_MESSAGE_COUNT];
extern bool is_imsg_new[inverters_MESSAGE_COUNT];

lv_timer_t *steer_status_task;
lv_timer_t *steer_version_task;

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
    STEER_CAN_UNPACK(primary, PRIMARY, car_status, CAR_STATUS, is_pmsg_new);
    break;
  }
  case PRIMARY_PEDAL_CALIBRATION_ACK_FRAME_ID: {
    STEER_CAN_UNPACK(primary, PRIMARY, pedal_calibration_ack, PEDAL_CALIBRATION_ACK, is_pmsg_new);
    break;
  }
  case PRIMARY_COOLING_STATUS_FRAME_ID: {
    STEER_CAN_UNPACK(primary, PRIMARY, cooling_status, COOLING_STATUS, is_pmsg_new);
    break;
  }
  case PRIMARY_PTT_STATUS_FRAME_ID: {
    STEER_CAN_UNPACK(primary, PRIMARY, ptt_status, PTT_STATUS, is_pmsg_new);
    break;
  }
  case PRIMARY_SPEED_FRAME_ID: {
    STEER_CAN_UNPACK(primary, PRIMARY, speed, SPEED, is_pmsg_new);
    break;
  }
  case PRIMARY_HV_VOLTAGE_FRAME_ID: {
    STEER_CAN_UNPACK(primary, PRIMARY, hv_voltage, HV_VOLTAGE, is_pmsg_new);
    break;
  }
  case PRIMARY_HV_CURRENT_FRAME_ID: {
    STEER_CAN_UNPACK(primary, PRIMARY, hv_current, HV_CURRENT, is_pmsg_new);
    break;
  }
  case PRIMARY_HV_TEMP_FRAME_ID: {
    STEER_CAN_UNPACK(primary, PRIMARY, hv_temp, HV_TEMP, is_pmsg_new);
    break;
  }
  case PRIMARY_HV_ERRORS_FRAME_ID: {
    STEER_CAN_UNPACK(primary, PRIMARY, hv_errors, HV_ERRORS, is_pmsg_new);
    break;
  }
  case PRIMARY_HV_CELL_BALANCING_STATUS_FRAME_ID: {
    STEER_CAN_UNPACK(primary, PRIMARY, hv_cell_balancing_status,
                     HV_CELL_BALANCING_STATUS, is_pmsg_new);
    break;
  }
  case PRIMARY_HV_FEEDBACKS_STATUS_FRAME_ID: {
    STEER_CAN_UNPACK(primary, PRIMARY, hv_feedbacks_status,
                     HV_FEEDBACKS_STATUS, is_pmsg_new);
    break;
  }
  case PRIMARY_LV_FEEDBACKS_FRAME_ID: {
    STEER_CAN_UNPACK(primary, PRIMARY, lv_feedbacks, LV_FEEDBACKS, is_pmsg_new);
    break;
  }
  case PRIMARY_HV_FANS_OVERRIDE_STATUS_FRAME_ID: {
    STEER_CAN_UNPACK(primary, PRIMARY, hv_fans_override_status,
                     HV_FANS_OVERRIDE_STATUS, is_pmsg_new);
    break;
  }
  case PRIMARY_TLM_STATUS_FRAME_ID: {
    STEER_CAN_UNPACK(primary, PRIMARY, tlm_status, TLM_STATUS, is_pmsg_new);
    break;
  }
  case PRIMARY_DAS_ERRORS_FRAME_ID: {
    STEER_CAN_UNPACK(primary, PRIMARY, das_errors, DAS_ERRORS, is_pmsg_new);
    break;
  }
  case PRIMARY_LV_CURRENTS_FRAME_ID: {
    STEER_CAN_UNPACK(primary, PRIMARY, lv_currents, LV_CURRENTS, is_pmsg_new);
    break;
  }
  case PRIMARY_LV_CELLS_TEMP_FRAME_ID: {
    // TODO: message index!!!
    // STEER_CAN_UNPACK(primary, PRIMARY, lv_cells_temp, LV_CELLS_TEMP, is_pmsg_new);
    break;
  }
  case PRIMARY_LV_TOTAL_VOLTAGE_FRAME_ID: {
    STEER_CAN_UNPACK(primary, PRIMARY, lv_total_voltage, LV_TOTAL_VOLTAGE, is_pmsg_new);
    break;
  }
  case PRIMARY_LV_ERRORS_FRAME_ID: {
    STEER_CAN_UNPACK(primary, PRIMARY, lv_errors, LV_ERRORS, is_pmsg_new);
    break;
  }
  case INVERTERS_INV_L_RCV_FRAME_ID: {
    //STEER_CAN_UNPACK(inverters, INVERTERS, inv_l_rcv, INV_L_RCV, is_imsg_new);
    break;
  }
  case INVERTERS_INV_R_RCV_FRAME_ID: {
    //STEER_CAN_UNPACK(inverters, INVERTERS, inv_r_rcv, INV_R_RCV, is_imsg_new);
    break;
  }
  case PRIMARY_CONTROL_OUTPUT_FRAME_ID: {
    STEER_CAN_UNPACK(primary, PRIMARY, control_output, CONTROL_OUTPUT, is_pmsg_new);
    break;
  }
  default:
    break;
  }
}

void handle_secondary(can_message_t *msg) {
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
    STEER_CAN_UNPACK(secondary, SECONDARY, steering_angle, STEERING_ANGLE, is_smsg_new);
    break;
  }
  case SECONDARY_PEDALS_OUTPUT_FRAME_ID: {
    STEER_CAN_UNPACK(secondary, SECONDARY, pedals_output, PEDALS_OUTPUT, is_smsg_new);
    break;
  }
  case SECONDARY_IMU_ACCELERATION_FRAME_ID: {
    STEER_CAN_UNPACK(secondary, SECONDARY, imu_acceleration, IMU_ACCELERATION, is_smsg_new);
    break;
  }
  case SECONDARY_LAP_COUNT_FRAME_ID: {
    STEER_CAN_UNPACK(secondary, SECONDARY, lap_count, LAP_COUNT, is_smsg_new);
    break;
  }
  case SECONDARY_LC_STATUS_FRAME_ID: {
    STEER_CAN_UNPACK(secondary, SECONDARY, lc_status, LC_STATUS, is_smsg_new);
    break;
  }
  case SECONDARY_TIMESTAMP_FRAME_ID: {
    STEER_CAN_UNPACK(secondary, SECONDARY, timestamp, TIMESTAMP, is_smsg_new);
  }
  default:
    break;
  }
}

void init_periodic_can_messages_timers(void) {
  steer_status_task =
      lv_timer_create(send_steer_status, PRIMARY_INTERVAL_STEER_STATUS, NULL);
  lv_timer_set_repeat_count(steer_status_task, -1);
  lv_timer_reset(steer_status_task);

  steer_version_task =
      lv_timer_create(send_steer_version, PRIMARY_INTERVAL_STEER_VERSION, NULL);
  lv_timer_set_repeat_count(steer_version_task, -1);
  lv_timer_reset(steer_version_task);
}
