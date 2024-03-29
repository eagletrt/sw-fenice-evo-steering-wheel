#include "steering_config.h"

#if CANSNIFFER_ENABLED == 1

#include "cansniffer.h"

static_assert(
    sizeof(cansniffer_elem_t) == CANSNIFFER_ELEM_T_SIZE,
    "Please set CANSNIFFER_ELEM_T_SIZE accordingly to cansniffer_elem_t size");

extern cansniffer_elem_t *primary_cansniffer_buffer;
extern cansniffer_elem_t *secondary_cansniffer_buffer;

bool cansniffer_initialized = false;

int primary_cansniffer_ids[CAN_POSSIBLE_IDS];
size_t primary_cansniffer_ids_size = 0;

int secondary_cansniffer_ids[CAN_POSSIBLE_IDS];
size_t secondary_cansniffer_ids_size = 0;

void cansniffer_buffer_init(void) {
  memset(primary_cansniffer_buffer, 0,
         CAN_POSSIBLE_IDS * CANSNIFFER_ELEM_T_SIZE);
  memset(secondary_cansniffer_buffer, 0,
         CAN_POSSIBLE_IDS * CANSNIFFER_ELEM_T_SIZE);
  cansniffer_initialized = true;
}

#endif
