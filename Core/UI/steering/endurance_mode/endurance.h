#ifndef ENDURANCE_H
#define ENDURANCE_H

#include "steering_config.h"

#ifdef ENDURANCE_MODE_ENABLED

#include "lvgl.h"

extern lv_group_t *steering_wheel_tab_group;

void endurance_screen_create(void);

#endif  // ENDURANCE_MODE_ENABLED

#endif  // ENDURANCE_H
