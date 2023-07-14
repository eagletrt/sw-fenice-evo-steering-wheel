#include "can_data.h"

#define DESERIALIZE(network, message)                                          \
  network##_message_##message data;                                            \
  network##_deserialize_##message(&data, raw);

#define DESERIALIZE_CONVERSION(network, message)                               \
  DESERIALIZE(network, message)                                                \
  network##_message_##message##_conversion conversion;                         \
  network##_raw_to_conversion_struct_##message(&conversion, &data);

void update_car_status(uint8_t val);

void can_handle_primary(struct can_frame frame) {
  
#define CAL_LOG_ENABLED 0
  int16_t id = frame.can_id;
  int length = frame.can_dlc;
  
  switch (id) {
    #if 0
  case PRIMARY_CAR_STATUS_FRAME_ID: {
    STEER_CAN_UNPACK(primary, PRIMARY, car_status, CAR_STATUS);
    car_status_update(&converted);
    break;
  }
  #endif
  

  case PRIMARY_PEDAL_CALIBRATION_ACK_FRAME_ID: {
    STEER_CAN_UNPACK(primary, PRIMARY, pedal_calibration_ack,
                     PEDAL_CALIBRATION_ACK);
    // pedal_calibration_ack(&converted);
    break;
  }
  
  case PRIMARY_STEERING_JMP_TO_BLT_FRAME_ID:
    // print("Resetting for open blt\n");
    // HAL_NVIC_SystemReset();
    break;
  case PRIMARY_PTT_STATUS_FRAME_ID: {
    // STEER_CAN_UNPACK(primary, PRIMARY, ptt_status, PTT_STATUS);
    // handle_ptt_message(converted.status);
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
  }
}

void can_handle_secondary(struct can_frame frame) {
  int16_t id = frame.can_id;
  int length = frame.can_dlc;
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

