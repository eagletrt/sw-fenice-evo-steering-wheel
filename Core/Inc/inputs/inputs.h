#ifndef INPUTS_H
#define INPUTS_H

#include "fdcan.h"
#include "lvgl.h"
#include "mcp23017.h"
#include "ptt.h"
#include "steering.h"

#define BUTTONS_N 8
#define MANETTINI_N 3
#define BUTTONS_LONG_PRESS_TIME 1500
#define CALIBRATION_TIMEOUT_RESPONSE 3000

#define MANETTINO_STEPS_N 8
#define BUTTON_MAPPING                                                         \
  { 7, 5, 3, 1, 0, 6, 4, 2 }

#define MANETTINO_DEBOUNCE 50

#define PADDLE_TOP_RIGHT 2
#define PADDLE_TOP_LEFT 1
#define PADDLE_BOTTOM_RIGHT 4
#define PADDLE_BOTTOM_LEFT 5
#define BUTTON_TOP_RIGHT 3
#define BUTTON_TOP_LEFT 0
#define BUTTON_BOTTOM_RIGHT 7
#define BUTTON_BOTTOM_LEFT 6

#define MANETTINO_DEBOUNCE_VALUE 255 // value to be ignored
#define MANETTINO_RIGHT_VALS                                                   \
  { 253, 251, 239, 127, 191, 223, 247, 254 }
#define MANETTINO_CENTER_VALS                                                  \
  { 239, 247, 251, 254, 223, 191, 127, 253 }
#define MANETTINO_LEFT_VALS                                                    \
  { 223, 239, 254, 253, 251, 247, 191, 127 }

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
  { -0.1f, 0.0f, 0.1f, 0.2f, 0.3f, 0.4f, 0.8f, 1.0f }
#define TORQUE_MAP_MAPPING                                                     \
  { 0.0f, 0.066f, 0.133f, 0.266f, 0.466f, 0.666f, 0.8f, 1.0f }
#define SLIP_MAP_MAPPING                                                       \
  { 0.0f, 0.066f, 0.133f, 0.266f, 0.466f, 0.666f, 0.8f, 1.0f }

void inputs_init(void);
void read_inputs(lv_timer_t *);
void send_set_car_status(void);
void turn_telemetry_on_off(void);
void pedal_calibration_ack(primary_pedal_calibration_ack_converted_t *data);
void calibration_request_timeout_check(uint32_t current_time);

#endif
