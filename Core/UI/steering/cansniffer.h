#ifndef CANSNIFFER_H
#define CANSNIFFER_H

#include "lvgl.h"
#include "maxheap.h"
#include "messages.h"

#define CAN_POSSIBLE_IDS 2048
#define CANSNIFFER_ELEM_T_SIZE 20

#define TAB_CANSNIFFER_N_MESSAGES_SHOWN 12

#define PRIMARY_BUFSIZE 40
#define SECONDARY_BUFSIZE 10

typedef struct {
  uint32_t timestamp;
  uint32_t delta;
  can_id_t id;
  uint8_t len;
  uint8_t data[8];
} cansniffer_elem_t;

void cansniffer_buffer_init(void);

void cansniffer_secondary_new_message(can_message_t *msg);
void switch_secondary_cansniffer();
void update_secondary_cansniffer_ui(lv_timer_t *unused_tim);
void cansniffer_primary_new_message(can_message_t *msg);
void switch_primary_cansniffer();
void update_primary_cansniffer_ui(lv_timer_t *unused_tim);

#endif // CANSNIFFER_H
