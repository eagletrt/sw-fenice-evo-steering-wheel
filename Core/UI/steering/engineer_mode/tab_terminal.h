#ifndef TAB_TERMINAL_H
#define TAB_TERMINAL_H

#include "steering_config.h"

#if STEERING_WHEEL_MODE == STEERING_WHEEL_LVGL_MODE

#include "steering.h"
#include "utils.h"

#define TERMINAL_ROWS_N 12

void tab_terminal_create(lv_obj_t *parent);

#endif  // STEERING_WHEEL_MODE == STEERING_WHEEL_LVGL_MODE

#endif  // TAB_TERMINAL_H
