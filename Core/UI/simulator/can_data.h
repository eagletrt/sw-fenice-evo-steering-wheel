#ifndef CAN_DATA_H
#define CAN_DATA_H

#include <stdio.h>
#include <string.h>

#include <errno.h>
#include <inttypes.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

#include "../steering/steering.h"

#include <linux/can.h>
#include <linux/can/raw.h>

#include "primary/primary_network.h"
#include "secondary/secondary_network.h"
#include "inverters/inverters_network.h"


#define STEER_CAN_UNPACK(ntw, NTW, msg_name, MSG_NAME)                         \
  ntw##_##msg_name##_t raw;                                                    \
  ntw##_##msg_name##_converted_t converted;                                    \
  ntw##_##msg_name##_unpack(&raw, frame.data, NTW##_##MSG_NAME##_BYTE_SIZE);    \
  ntw##_##msg_name##_raw_to_conversion_struct(&converted, &raw);

#define STEER_CAN_PACK(ntw, NTW, msg_name, MSG_NAME)                           \
  can_message_t msg = {0};                                                     \
  msg.id = NTW##_##MSG_NAME##_FRAME_ID;                                        \
  msg.size = NTW##_##MSG_NAME##_BYTE_SIZE;                                     \
  ntw##_##msg_name##_t raw = {0};                                              \
  ntw##_##msg_name##_conversion_to_raw_struct(&raw, &converted);               \
  ntw##_##msg_name##_pack(msg.data, &raw, PRIMARY_##MSG_NAME##_BYTE_SIZE);

#include "can.h"
#include "queue.h"

void can_handle_primary(struct can_frame frame);

void update_lv_temp(uint8_t val);

void can_handle_secondary(struct can_frame frame);

#endif