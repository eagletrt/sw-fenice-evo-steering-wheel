#ifndef GRAPHICS_MANAGER_H
#define GRAPHICS_MANAGER_H

#include "can_messages.h"
#include "steering.h"
#include "steering_config.h"

#define OLIVEC_IMPLEMENTATION
#include "olive.c"

void update_graphics(UI_t *scr);
void init_screen(UI_t *endurance_screen);
void olivec_update_graphics(float dt, UI_t *scr);
void white_screen(UI_t *scr);

#endif  // GRAPHICS_MANAGER_H
