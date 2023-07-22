#ifndef CANSNIFFER_H
#define CANSNIFFER_H

#include "messages.h"

#define CANSNIFFER_MEMORY_POOL_ADDRESS                                         \
  0xC0600000 // ends memory usage in address 0xC0610000
#define CAN_POSSIBLE_IDS 2048
#define CANSNIFFER_ELEM_T_SIZE 16

#define PRIMARY_BUFSIZE 40
#define SECONDARY_BUFSIZE 10

typedef struct {
  uint32_t timestamp;
  can_id_t id;
  uint8_t len;
  uint8_t data[8];
} cansniffer_elem_t;

void cansniffer_buffer_init(void);
void cansniffer_primary_new_message(can_message_t *msg);
void cansniffer_secondary_new_message(can_message_t *msg);

#endif // CANSNIFFER_H
