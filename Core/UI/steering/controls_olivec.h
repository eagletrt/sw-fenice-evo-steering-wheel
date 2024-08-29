#ifndef CONTROLS_OLIVEC_H
#define CONTROLS_OLIVEC_H

#include "steering_config.h"

#if STEERING_WHEEL_MODE == STEERING_WHEEL_OLIVEC_MODE

void manettino_right_actions(int dsteps);
void manettino_center_actions(int dsteps);
void manettino_left_actions(int dsteps);

void buttons_pressed_actions(uint8_t button);
void buttons_released_actions(uint8_t button);
void buttons_long_pressed_actions(uint8_t button);

#endif  // STEERING_WHEEL_MODE == STEERING_WHEEL_OLIVEC_MODE

#endif  // CONTROLS_OLIVEC_H
