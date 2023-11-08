#ifndef MESSAGES_H
#define MESSAGES_H

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define STEER_CAN_UNPACK(ntw, NTW, msg_name, MSG_NAME, arr)                    \
  ntw##_##msg_name##_t raw;                                                    \
  ntw##_##msg_name##_converted_t converted;                                    \
  ntw##_##msg_name##_unpack(&raw, msg->data, NTW##_##MSG_NAME##_BYTE_SIZE);    \
  ntw##_##msg_name##_raw_to_conversion_struct(&converted, &raw);               \
  memcpy(&ntw##_##msg_name##_last_state, &converted,                           \
         sizeof(ntw##_##msg_name##_converted_t));                              \
  arr[ntw##_index_from_id(NTW##_##MSG_NAME##_FRAME_ID)] = true;

#define STEER_CAN_PACK(ntw, NTW, msg_name, MSG_NAME)                           \
  can_message_t msg = {0};                                                     \
  msg.id = NTW##_##MSG_NAME##_FRAME_ID;                                        \
  msg.size = NTW##_##MSG_NAME##_BYTE_SIZE;                                     \
  ntw##_##msg_name##_t raw = {0};                                              \
  ntw##_##msg_name##_conversion_to_raw_struct(&raw, &converted);               \
  ntw##_##msg_name##_pack(msg.data, &raw, PRIMARY_##MSG_NAME##_BYTE_SIZE);

typedef uint16_t can_id_t;

typedef struct {
  can_id_t id;
  uint8_t size;
  uint8_t data[8];
} can_message_t;

#endif
