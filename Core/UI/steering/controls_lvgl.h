#ifndef CONTROLS_LVGL_H
#define CONTROLS_LVGL_H

#include "steering_config.h"

#if STEERING_WHEEL_MODE == STEERING_WHEEL_LVGL_MODE

#include "can_messages.h"
#include "lvgl.h"
#include "steering.h"
#include "tab_manager.h"

void set_dmt_steering_angle_target(void);
void manettino_send_slip_control(float);
void manettino_send_torque_vectoring(float);
void manettino_send_power_map(float);
void send_pork_fans_status(float val);

void turn_telemetry_on_off(void);

void manettino_right_actions(int dsteps);
void manettino_center_actions(int dsteps);
void manettino_left_actions(int dsteps);

void buttons_pressed_actions(uint8_t button);
void buttons_released_actions(uint8_t button);
void buttons_long_pressed_actions(uint8_t button);

// float fmin(float x, float y);
// float fmax(float x, float y);

#if CANSNIFFER_ENABLED == 1
void change_cansniffer_index(bool);
#endif

#endif  // STEERING_WHEEL_MODE == STEERING_WHEEL_LVGL_MODE

#endif
