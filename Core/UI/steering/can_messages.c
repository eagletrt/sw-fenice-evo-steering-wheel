#include "can_messages.h"
#define _XOPEN_SOURCE
#include <time.h>

extern bool steering_initialized;
extern primary_steer_status_converted_t steer_status_last_state;

extern primary_watchdog m_primary_watchdog;
extern secondary_watchdog m_secondary_watchdog;
extern inverters_watchdog m_inverters_watchdog;

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
  struct tm timeinfo;
  strptime(__DATE__ " " __TIME__, "%b %d %Y %H:%M:%S", &timeinfo);
  primary_steer_version_converted_t converted = {
      .canlib_build_time = CANLIB_BUILD_TIME,
      .component_build_time = mktime(&timeinfo)};
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
#if ENGINEERING_TAB_ENABLED == 1
#if CANSNIFFER_ENABLED == 1
  if (cansniffer_initialized) {
    cansniffer_primary_new_message(msg);
  }
#endif // CANSNIFFER_ENABLED
#endif // ENGINEERING_TAB_ENABLED
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
#if 0
  case PRIMARY_PEDAL_CALIBRATION_ACK_FRAME_ID: {
    STEER_CAN_UNPACK(primary, PRIMARY, pedal_calibration_ack,
                     PEDAL_CALIBRATION_ACK, is_pmsg_new);
    pedal_calibration_ack();
    break;
  }
#endif
  case PRIMARY_COOLING_STATUS_FRAME_ID: {
    STEER_CAN_UNPACK(primary, PRIMARY, cooling_status, COOLING_STATUS,
                     is_pmsg_new);
    break;
  }
  case PRIMARY_PTT_STATUS_FRAME_ID: {
    STEER_CAN_UNPACK(primary, PRIMARY, ptt_status, PTT_STATUS, is_pmsg_new);
    handle_ptt_message();
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
    STEER_CAN_UNPACK(primary, PRIMARY, lv_cells_temp, LV_CELLS_TEMP,
                     is_pmsg_new);
    switch (last_state->start_index) {
    case 0:
      memcpy(&lv_temps_stock_1, last_state,
             sizeof(primary_lv_cells_temp_converted_t));
      break;
    case 3:
      memcpy(&lv_temps_stock_2, last_state,
             sizeof(primary_lv_cells_temp_converted_t));
      break;
    default:
      break;
    }
    break;
  }
  case PRIMARY_LV_CELLS_VOLTAGE_FRAME_ID: {
    STEER_CAN_UNPACK(primary, PRIMARY, lv_cells_voltage, LV_CELLS_VOLTAGE,
                     is_pmsg_new);
    switch (last_state->start_index) {
    case 0:
      memcpy(&lv_voltages_stock_1, last_state,
             sizeof(primary_lv_cells_voltage_converted_t));
      break;
    case 3:
      memcpy(&lv_voltages_stock_2, last_state,
             sizeof(primary_lv_cells_voltage_converted_t));
      break;
    default:
      break;
    }
    break;
  }
  case PRIMARY_LV_TOTAL_VOLTAGE_FRAME_ID: {
    STEER_CAN_UNPACK(primary, PRIMARY, lv_total_voltage, LV_TOTAL_VOLTAGE,
                     is_pmsg_new);
    break;
  }
  case PRIMARY_LV_ERRORS_FRAME_ID: {
    STEER_CAN_UNPACK(primary, PRIMARY, lv_errors, LV_ERRORS, is_pmsg_new);
    break;
  }
  case PRIMARY_HV_FEEDBACK_STATUS_FRAME_ID: {
    STEER_CAN_UNPACK(primary, PRIMARY, hv_feedback_status, HV_FEEDBACK_STATUS,
                     is_pmsg_new);
    break;
  }
  case PRIMARY_CONTROL_OUTPUT_FRAME_ID: {
    STEER_CAN_UNPACK(primary, PRIMARY, control_output, CONTROL_OUTPUT,
                     is_pmsg_new);
    break;
  }
  case INVERTERS_INV_L_RCV_FRAME_ID: {
    STEER_CAN_UNPACK(inverters, INVERTERS, inv_l_rcv, INV_L_RCV, is_imsg_new);
    break;
  }
  case INVERTERS_INV_R_RCV_FRAME_ID: {
    STEER_CAN_UNPACK(inverters, INVERTERS, inv_r_rcv, INV_R_RCV, is_imsg_new);
    break;
  }
  default:
    break;
  }
}

void handle_secondary(can_message_t *msg) {
  if (!steering_initialized)
    return;
#if ENGINEERING_TAB_ENABLED == 1
#if CANSNIFFER_ENABLED == 1
  if (cansniffer_initialized) {
    cansniffer_secondary_new_message(msg);
  }
#endif // CANSNIFFER_ENABLED
#endif // ENGINEERING_TAB_ENABLED
#if CAN_LOG_ENABLED
  secondary_message_name_from_id(msg->id, name_buffer);
  print("Secondary network - message id %s\n", name_buffer);
#endif
  can_id_t id = msg->id;
  switch (id) {
  case SECONDARY_STEERING_ANGLE_FRAME_ID: {
    STEER_CAN_UNPACK(secondary, SECONDARY, steering_angle, STEERING_ANGLE,
                     is_smsg_new);
    break;
  }
  case SECONDARY_PEDALS_OUTPUT_FRAME_ID: {
    STEER_CAN_UNPACK(secondary, SECONDARY, pedals_output, PEDALS_OUTPUT,
                     is_smsg_new);
    break;
  }
#if 0
  case SECONDARY_IMU_ACCELERATION_FRAME_ID: {
    STEER_CAN_UNPACK(secondary, SECONDARY, imu_acceleration, IMU_ACCELERATION,
                     is_smsg_new);
    break;
  }
#endif
#if 0
  case SECONDARY_LAP_COUNT_FRAME_ID: {
    STEER_CAN_UNPACK(secondary, SECONDARY, lap_count, LAP_COUNT, is_smsg_new);
    break;
  }
#endif
#if 0
  case SECONDARY_LC_STATUS_FRAME_ID: {
    STEER_CAN_UNPACK(secondary, SECONDARY, lc_status, LC_STATUS, is_smsg_new);
    break;
  }
#endif
#if 0
  case SECONDARY_TIMESTAMP_FRAME_ID: {
    STEER_CAN_UNPACK(secondary, SECONDARY, timestamp, TIMESTAMP, is_smsg_new);
    break;
  }
#endif
#if 0
  case SECONDARY_IRTS_FL_0_FRAME_ID: {
    STEER_CAN_UNPACK(secondary, SECONDARY, irts_fl_0, IRTS_FL_0, is_smsg_new);
    break;
  }
  case SECONDARY_IRTS_FL_1_FRAME_ID: {
    STEER_CAN_UNPACK(secondary, SECONDARY, irts_fl_1, IRTS_FL_1, is_smsg_new);
    break;
  }
  case SECONDARY_IRTS_FL_2_FRAME_ID: {
    STEER_CAN_UNPACK(secondary, SECONDARY, irts_fl_2, IRTS_FL_2, is_smsg_new);
    break;
  }
  case SECONDARY_IRTS_FL_3_FRAME_ID: {
    STEER_CAN_UNPACK(secondary, SECONDARY, irts_fl_3, IRTS_FL_3, is_smsg_new);
    break;
  }
  case SECONDARY_IRTS_FR_0_FRAME_ID: {
    STEER_CAN_UNPACK(secondary, SECONDARY, irts_fr_0, IRTS_FR_0, is_smsg_new);
    break;
  }
  case SECONDARY_IRTS_FR_1_FRAME_ID: {
    STEER_CAN_UNPACK(secondary, SECONDARY, irts_fr_1, IRTS_FR_1, is_smsg_new);
    break;
  }
  case SECONDARY_IRTS_FR_2_FRAME_ID: {
    STEER_CAN_UNPACK(secondary, SECONDARY, irts_fr_2, IRTS_FR_2, is_smsg_new);
    break;
  }
  case SECONDARY_IRTS_FR_3_FRAME_ID: {
    STEER_CAN_UNPACK(secondary, SECONDARY, irts_fr_3, IRTS_FR_3, is_smsg_new);
    break;
  }
  case SECONDARY_IRTS_RL_0_FRAME_ID: {
    STEER_CAN_UNPACK(secondary, SECONDARY, irts_rl_0, IRTS_RL_0, is_smsg_new);
    break;
  }
  case SECONDARY_IRTS_RL_1_FRAME_ID: {
    STEER_CAN_UNPACK(secondary, SECONDARY, irts_rl_1, IRTS_RL_1, is_smsg_new);
    break;
  }
  case SECONDARY_IRTS_RL_2_FRAME_ID: {
    STEER_CAN_UNPACK(secondary, SECONDARY, irts_rl_2, IRTS_RL_2, is_smsg_new);
    break;
  }
  case SECONDARY_IRTS_RL_3_FRAME_ID: {
    STEER_CAN_UNPACK(secondary, SECONDARY, irts_rl_3, IRTS_RL_3, is_smsg_new);
    break;
  }
  case SECONDARY_IRTS_RR_0_FRAME_ID: {
    STEER_CAN_UNPACK(secondary, SECONDARY, irts_rr_0, IRTS_RR_0, is_smsg_new);
    break;
  }
  case SECONDARY_IRTS_RR_1_FRAME_ID: {
    STEER_CAN_UNPACK(secondary, SECONDARY, irts_rr_1, IRTS_RR_1, is_smsg_new);
    break;
  }
  case SECONDARY_IRTS_RR_2_FRAME_ID: {
    STEER_CAN_UNPACK(secondary, SECONDARY, irts_rr_2, IRTS_RR_2, is_smsg_new);
    break;
  }
  case SECONDARY_IRTS_RR_3_FRAME_ID: {
    STEER_CAN_UNPACK(secondary, SECONDARY, irts_rr_3, IRTS_RR_3, is_smsg_new);
    break;
  }
#endif
  default:
    break;
  }
}

#define SERIAL_TO_CAN_INTERFACE_PRIMARY 0
#define SERIAL_TO_CAN_INTERFACE_SECONDARY 1

void message_parser(uint8_t *msg, size_t msg_siz) {
  msg[msg_siz] = 0;
  uint32_t msgi = 0, msgl = 0;
  uint8_t msg_data[8] = {0};
  msgi = strtol((char *)msg, NULL, 16);
  msgl = strtol((char *)msg + 2, NULL, 16);
  for (size_t i = 0; i < 8; ++i) {
    char m[3] = {msg[6 + i * 2], msg[6 + i * 2 + 1], 0};
    msg_data[i] = strtol(m, NULL, 16);
  }
  can_message_t parsed_msg = {
      .id = msgl,
      .size = 0, // unused
  };
  memcpy(&parsed_msg.data, msg_data, 8);
  if (msgi == 0) {
    handle_primary(&parsed_msg);
  } else if (msgi == 1) {
    handle_secondary(&parsed_msg);
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