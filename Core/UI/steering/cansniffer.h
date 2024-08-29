#ifndef CANSNIFFER_H
#define CANSNIFFER_H

#include "messages.h"
#include "steering_config.h"

#if CANSNIFFER_ENABLED == 1

#include "maxheap.h"
#include "min-heap.h"

#define CANSNIFFER_ELEM_T_SIZE 20

#define TAB_CANSNIFFER_N_MESSAGES_SHOWN 12

#define PRIMARY_BUFSIZE   40
#define SECONDARY_BUFSIZE 10

typedef struct {
    uint32_t timestamp;
    uint32_t delta;
    can_id_t id;
    uint8_t len;
    uint8_t data[8];
} cansniffer_elem_t;

void cansniffer_secondary_new_message(can_message_t *msg);
void update_primary_cansniffer_ui(void);
void cansniffer_primary_new_message(can_message_t *msg);
void update_secondary_cansniffer_ui(void);
void cansniffer_inverters_new_message(can_message_t *msg);
void update_inverters_cansniffer_ui(void);
int8_t min_heap_compare_indexes(void *a, void *b);

#endif  // CANSNIFFER_H

#endif  // CANSNIFFER_ENABLED == 1
