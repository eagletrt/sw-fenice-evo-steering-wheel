#ifndef INPUTS_H
#define INPUTS_H

#include "can_messages.h"
#include "controls.h"
#include "lvgl.h"
#include "mcp23017.h"
#include "ptt.h"
#include "steering.h"
#include "tab_manager.h"
#include <math.h>

#define BUTTON_MAPPING                                                         \
  { 7, 5, 3, 1, 0, 6, 4, 2 }

#define MANETTINO_DEBOUNCE 50

#define MANETTINO_LEFT_INDEX 0
#define MANETTINO_CENTER_INDEX 1
#define MANETTINO_RIGHT_INDEX 2

#define MANETTINO_DEBOUNCE_VALUE 255 // value to be ignored
#define MANETTINO_INVALID_VALUE 0xFF

#define MANETTINO_RIGHT_VALS                                                   \
  { 253, 251, 239, 127, 191, 223, 247, 254 }
#define MANETTINO_CENTER_VALS                                                  \
  { 239, 247, 251, 254, 223, 191, 127, 253 }
#define MANETTINO_LEFT_VALS                                                    \
  { 127, 191, 247, 251, 253, 254, 239, 223 }

void inputs_init(void);
void read_inputs(lv_timer_t *);

void buttons_pressed_actions(uint8_t button);
void buttons_released_actions(uint8_t button);
void buttons_long_pressed_actions(uint8_t button);

#endif
