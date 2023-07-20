#ifndef CANSNIFFER_H
#define CANSNIFFER_H

#include "messages.h"
#include <stdbool.h>
#include <stdint.h>

#define PRIMARY_BUFSIZE 40
#define SECONDARY_BUFSIZE 10

typedef struct {
  uint32_t timestamp;
  can_id_t id;
  uint8_t len;
  uint8_t data[8];
} cansniffer_elem_t;

#endif // CANSNIFFER_H
