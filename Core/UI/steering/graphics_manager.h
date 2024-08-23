#ifndef GRAPHICS_MANAGER_H
#define GRAPHICS_MANAGER_H

#include "lvgl.h"
#include "messages.h"
#include "steering_config.h"

#ifdef STM32H723xx
#include "screen_driver.h"
#endif  // STM32H723xx

#ifdef ENDURANCE_MODE_ENABLED
#include "endurance_mode/endurance.h"
#else
#include "tab_manager.h"
#endif  // ENDURANCE_MODE_ENABLED

void init_graphics_manager(void);
void refresh_graphics(void);
void update_graphics(lv_timer_t *t);

#endif  // GRAPHICS_MANAGER_H
