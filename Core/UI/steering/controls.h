#ifndef CONTROLS_H
#define CONTROLS_H

#include <stdint.h>
#include "can_messages.h"
#include "tab_manager.h"
#include "lvgl.h"
#include "steering.h"
#include "primary/primary_network.h"
#include "secondary/secondary_network.h"
#include "inverters/inverters_network.h"

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

#define POWER_MAP_MAPPING                                                      \
  { -0.1f, 0.0f, 0.2f, 0.4f, 0.6f, 0.7f, 0.8f, 1.0f }
#define TORQUE_MAP_MAPPING                                                     \
  { 0.0f, 0.066f, 0.133f, 0.266f, 0.466f, 0.666f, 0.8f, 1.0f }
#define SLIP_MAP_MAPPING                                                       \
  { 0.0f, 0.066f, 0.133f, 0.266f, 0.466f, 0.666f, 0.8f, 1.0f }
#define PUMPS_MAPPING                                                          \
  { -1.0f, 0.0f, 0.10f, 0.20f, 0.40f, 0.60f, 0.80f, 1.0f }
#define RADIATORS_MAPPING                                                      \
  { -1.0f, 0.0f, 0.10f, 0.20f, 0.40f, 0.60f, 0.80f, 1.0f }

void switch_mode(void);
void steering_change_tab(bool forward);

void prepare_set_car_status(void);
bool send_set_car_status_directly(void);

void manettino_send_slip_control(float);
void manettino_send_torque_vectoring(float);
void manettino_send_power_map(float);
void manettino_send_set_pumps_speed(float);
void manettino_send_set_radiators(float);

void turn_telemetry_on_off(void);
void calibration_tool_set_min_max(bool);
void calibration_request_timeout_check(uint32_t);

#endif
