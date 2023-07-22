#include "cansniffer.h"

static_assert(
    sizeof(cansniffer_elem_t) == CANSNIFFER_ELEM_T_SIZE,
    "Please set CANSNIFFER_ELEM_T_SIZE accordingly to cansniffer_elem_t size");

/*
primary_cansniffer_buffer -> 32768 Bytes
secondary_cansniffer_buffer -> 32768 Bytes
*/
extern cansniffer_elem_t *primary_cansniffer_buffer;
extern cansniffer_elem_t *secondary_cansniffer_buffer;

void cansniffer_buffer_init(void) {
  memset(primary_cansniffer_buffer, 0,
         CAN_POSSIBLE_IDS * CANSNIFFER_ELEM_T_SIZE);
  memset(secondary_cansniffer_buffer, 0,
         CAN_POSSIBLE_IDS * CANSNIFFER_ELEM_T_SIZE);
}

void cansniffer_primary_new_message(can_message_t *msg) {
  uint32_t id = msg->id;
  primary_cansniffer_buffer[id].timestamp = get_current_time_ms();
  primary_cansniffer_buffer[id].id = msg->id;
  primary_cansniffer_buffer[id].len = msg->size;
  memcpy(primary_cansniffer_buffer[id].data, msg->data, msg->size);
}

void cansniffer_secondary_new_message(can_message_t *msg) {
  uint32_t id = msg->id;
  secondary_cansniffer_buffer[id].timestamp = get_current_time_ms();
  secondary_cansniffer_buffer[id].id = msg->id;
  secondary_cansniffer_buffer[id].len = msg->size;
  memcpy(secondary_cansniffer_buffer[id].data, msg->data, msg->size);
}
