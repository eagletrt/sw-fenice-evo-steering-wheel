#ifndef CAN_MESSAGES_H
#define CAN_MESSAGES_H

#include <stdint.h>
#include <stdbool.h>
#include "lvgl.h"
#include "steering.h"
#include "messages.h"
#include "primary/primary_network.h"
#include "primary/primary_watchdog.h"
#include "secondary/secondary_network.h"
#include "secondary/secondary_watchdog.h"
#include "inverters/inverters_network.h"
#include "inverters/inverters_watchdog.h"

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

bool can_send(can_message_t *msg, bool to_primary_network);
void openblt_reset(void);

void send_steer_version(lv_timer_t *);
void send_steer_status(lv_timer_t *);
void pedal_calibration_ack(primary_pedal_calibration_ack_converted_t *data);
void handle_ptt_message(primary_ptt_status_status val);

void handle_primary(can_message_t *msg);
void handle_secondary(can_message_t *msg);

uint32_t get_current_time_ms(void);

#endif // CAN_MESSAGES_H
