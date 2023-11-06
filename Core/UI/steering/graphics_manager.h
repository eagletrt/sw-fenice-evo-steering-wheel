#ifndef GRAPHICS_MANAGER_H
#define GRAPHICS_MANAGER_H

#include "lvgl.h"
#include "messages.h"
#include "screen_driver.h"
#include "steering_config.h"
#include "tab_manager.h"

void init_graphics_manager(void);
void refresh_graphics(void);
void update_graphics(void);

#endif // GRAPHICS_MANAGER_H
