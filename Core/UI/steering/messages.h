#ifndef MESSAGES_H
#define MESSAGES_H

#define STEER_CAN_UNPACK(ntw, NTW, msg_name, MSG_NAME)                         \
  ntw##_##msg_name##_t raw;                                                    \
  ntw##_##msg_name##_converted_t converted;                                    \
  ntw##_##msg_name##_unpack(&raw, msg->data, NTW##_##MSG_NAME##_BYTE_SIZE);    \
  ntw##_##msg_name##_raw_to_conversion_struct(&converted, &raw);

#define STEER_CAN_PACK(ntw, NTW, msg_name, MSG_NAME)                           \
  can_message_t msg = {0};                                                     \
  msg.id = NTW##_##MSG_NAME##_FRAME_ID;                                        \
  msg.size = NTW##_##MSG_NAME##_BYTE_SIZE;                                     \
  ntw##_##msg_name##_t raw = {0};                                              \
  ntw##_##msg_name##_conversion_to_raw_struct(&raw, &converted);               \
  ntw##_##msg_name##_pack(msg.data, &raw, PRIMARY_##MSG_NAME##_BYTE_SIZE);

#define CAN_LOG_ENABLED 0

typedef uint16_t can_id_t;

typedef struct {
  can_id_t id;
  uint8_t size;
  uint8_t data[8];
} can_message_t;

#define PRIMARY_MONITORED_MESSAGES                                             \
  {                                                                            \
    PRIMARY_CAR_STATUS_FRAME_ID, PRIMARY_PEDAL_CALIBRATION_ACK_FRAME_ID,       \
        PRIMARY_STEERING_JMP_TO_BLT_FRAME_ID, PRIMARY_PTT_STATUS_FRAME_ID,     \
        PRIMARY_TLM_STATUS_FRAME_ID, PRIMARY_AMBIENT_TEMPERATURE_FRAME_ID,     \
        PRIMARY_SPEED_FRAME_ID, PRIMARY_HV_VOLTAGE_FRAME_ID,                   \
        PRIMARY_HV_CURRENT_FRAME_ID, PRIMARY_HV_TEMP_FRAME_ID,                 \
        PRIMARY_HV_ERRORS_FRAME_ID, PRIMARY_HV_FEEDBACKS_STATUS_FRAME_ID,      \
        PRIMARY_HV_CELLS_VOLTAGE_FRAME_ID, PRIMARY_HV_CELLS_TEMP_FRAME_ID,     \
        PRIMARY_DAS_ERRORS_FRAME_ID, PRIMARY_LV_CURRENTS_FRAME_ID,             \
        PRIMARY_LV_CELLS_VOLTAGE_FRAME_ID, PRIMARY_LV_CELLS_TEMP_FRAME_ID,     \
        PRIMARY_LV_TOTAL_VOLTAGE_FRAME_ID, PRIMARY_LV_ERRORS_FRAME_ID          \
  }

#define SECONDARY_MONITORED_MESSAGES                                           \
  { SECONDARY_STEERING_ANGLE_FRAME_ID, SECONDARY_PEDALS_OUTPUT_FRAME_ID }

#endif
