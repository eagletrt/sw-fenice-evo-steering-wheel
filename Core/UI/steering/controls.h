#ifndef CONTROLS_H
#define CONTROLS_H

#include "can_messages.h"
#include "lvgl.h"
#include "primary/primary_network.h"
#include "secondary/secondary_network.h"
#include "steering.h"
#include "steering_config.h"
#include "tab_manager.h"
#include <math.h>
#include <stdint.h>

#define BUTTONS_N 8
#define MANETTINI_N 3
#define BUTTONS_LONG_PRESS_TIME 1500
#define CALIBRATION_TIMEOUT_RESPONSE 3000

#define MANETTINO_STEPS_N 8

#define PADDLE_TOP_RIGHT 2
#define PADDLE_TOP_LEFT 1
#define PADDLE_BOTTOM_RIGHT 4
#define PADDLE_BOTTOM_LEFT 5
#define BUTTON_TOP_RIGHT 3
#define BUTTON_TOP_LEFT 0
#define BUTTON_BOTTOM_RIGHT 7
#define BUTTON_BOTTOM_LEFT 6

typedef enum {
  RADS_AUTO,
  RADS_OFF,
  RADS_025,
  RADS_050,
  RADS_075,
  RADS_100,
  RADS_LENGTH
} radiator_speed_t;

typedef enum {
  PUMPS_AUTO,
  PUMPS_OFF,
  PUMPS_025,
  PUMPS_050,
  PUMPS_075,
  PUMPS_100,
  P_LENGTH
} pumps_speed_t;

void prepare_set_car_status(void);
bool send_set_car_status_directly(void);

void set_dmt_steering_angle_target(void);
void manettino_send_slip_control(float);
void manettino_send_torque_vectoring(float);
void manettino_send_power_map(float);
void manettino_send_set_pumps_speed(float);
void manettino_send_set_radiators(float);
void send_pork_fans_status(float val);

void turn_telemetry_on_off(void);

void manettino_right_actions(int dsteps);
void manettino_center_actions(int dsteps);
void manettino_left_actions(int dsteps);

#if CANSNIFFER_ENABLED == 1
void switch_cansniffer(void);
void change_cansniffer_index(bool);
#endif

#endif
