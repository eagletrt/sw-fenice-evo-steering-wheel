#ifndef INPUTS_H
#define INPUTS_H

#include "fdcan.h"
#include "mcp23017.h"

#define BUTTON_MAPPING                                                         \
  { 7, 5, 3, 1, 0, 6, 4, 2 }

#define MANETTINO_DEBOUNCE 50

void inputs_init(void);
void read_inputs(void);
void send_tson(void);

#endif
