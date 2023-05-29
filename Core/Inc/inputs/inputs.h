#ifndef INPUTS_H
#define INPUTS_H

#include "mcp23017.h"

#define BUTTON_MAPPING                                                         \
  { 7, 5, 3, 1, 0, 6, 4, 2 }

void inputs_init();
void print_buttons();
void read_buttons();
void print_buttons();

#endif
