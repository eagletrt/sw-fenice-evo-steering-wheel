#ifndef GRAPHICS_MANAGER_H
#define GRAPHICS_MANAGER_H

#include "lvgl.h"
#include "messages.h"
#ifdef STM32H723xx
#include "screen_driver.h"
#endif
#include "steering_config.h"
#include "tab_manager.h"


void init_graphics_manager(void);
void refresh_graphics(void);
void update_graphics(lv_timer_t* t);

#endif // GRAPHICS_MANAGER_H
