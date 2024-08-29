#ifndef ENDURANCE_H
#define ENDURANCE_H

#include "steering_config.h"

#if STEERING_WHEEL_MODE == STEERING_WHEEL_LVGL_MODE

#ifdef ENDURANCE_MODE_ENABLED

#include "steering.h"
#include "utils.h"

void endurance_screen_create(lv_obj_t *parent);

#endif  // ENDURANCE_MODE_ENABLED

#endif  // STEERING_WHEEL_MODE == STEERING_WHEEL_LVGL_MODE

#endif  // ENDURANCE_H
