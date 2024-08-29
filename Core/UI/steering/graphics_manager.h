#ifndef GRAPHICS_MANAGER_H
#define GRAPHICS_MANAGER_H

#include "messages.h"
#include "steering.h"
#include "steering_config.h"
#include "tab_manager.h"

#if STEERING_WHEEL_MODE == STEERING_WHEEL_LVGL_MODE
#include "lvgl.h"
#endif

#ifdef STM32H723xx
#include "screen_driver.h"
#endif

void init_graphics_manager(void);
void refresh_graphics(void);
void update_graphics(void *unused);

#endif  // GRAPHICS_MANAGER_H
