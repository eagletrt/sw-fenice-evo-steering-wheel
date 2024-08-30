#ifndef CONTROLS_OLIVEC_H
#define CONTROLS_OLIVEC_H

#include "steering_config.h"

#if STEERING_WHEEL_MODE == STEERING_WHEEL_OLIVEC_MODE

extern primary_ecu_set_power_maps_converted_t ecu_set_power_maps_last_state;
extern primary_lv_radiator_speed_converted_t steering_wheel_state_radiator_speed;
extern primary_lv_pumps_speed_converted_t steering_wheel_state_pumps_speed;
extern primary_hv_set_fans_status_converted_t steering_wheel_set_fans_status;

void manettino_right_actions(int dsteps);
void manettino_center_actions(int dsteps);
void manettino_left_actions(int dsteps);

void buttons_pressed_actions(uint8_t button);
void buttons_released_actions(uint8_t button);
void buttons_long_pressed_actions(uint8_t button);

#endif  // STEERING_WHEEL_MODE == STEERING_WHEEL_OLIVEC_MODE

#endif  // CONTROLS_OLIVEC_H
