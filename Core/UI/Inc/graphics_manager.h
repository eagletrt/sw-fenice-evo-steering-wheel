#ifndef GRAPHICS_MANAGER_H
#define GRAPHICS_MANAGER_H

#include "can_messages.h"
#include "steering.h"
#include "steering_config.h"

void sw_init_screen(UI_t *sw_screen);
void sw_set_canvas(UI_t *scr, uint32_t *pixels, size_t width, size_t height, size_t stride);

void sw_update_graphics_from_can_messages(UI_t *scr);
void sw_update_screen(float dt, UI_t *scr);
void sw_screen_white(UI_t *scr);

#endif  // GRAPHICS_MANAGER_H
