#ifndef PRECHARGE_BAR_H
#define PRECHARGE_BAR_H

#include "steering_config.h"

#if STEERING_WHEEL_MODE == STEERING_WHEEL_LVGL_MODE

#include "steering.h"
#include "utils.h"

void precharge_bar_screen_create(lv_obj_t *screen);

#endif  // STEERING_WHEEL_MODE == STEERING_WHEEL_LVGL_MODE

#endif  // PRECHARGE_BAR_H
