#ifndef INPUTS_H
#define INPUTS_H

#include "fdcan.h"
#include "mcp23017.h"
#include "ptt.h"
#include "lvgl.h"

#define BUTTON_MAPPING                                                         \
  { 7, 5, 3, 1, 0, 6, 4, 2 }

#define MANETTINO_DEBOUNCE 50

#define PADDLE_TOP_RIGHT 2
#define PADDLE_TOP_LEFT 1
#define PADDLE_BOTTOM_RIGHT 4
#define PADDLE_BOTTOM_LEFT 5
#define BUTTON_TOP_RIGHT 3
#define BUTTON_TOP_LEFT 0
#define BUTTON_BOTTOM_RIGHT 7
#define BUTTON_BOTTOM_LEFT 6

#define MANETTINO_DEBOUNCE_VALUE 255 // value to be ignored
#define MANETTINO_RIGHT_VALS                                                   \
  { 253, 251, 239, 127, 191, 223, 247, 254 }
#define MANETTINO_CENTER_VALS                                                  \
  { 239, 247, 251, 254, 223, 191, 127, 253 }
#define MANETTINO_LEFT_VALS                                                    \
  { 223, 239, 254, 253, 251, 247, 191, 127 }

void inputs_init(void);
void read_inputs(lv_timer_t* main_timer);
void send_tson(void);
void turnon_telemetry(void);

#endif