#ifndef CONTROLS_H
#define CONTROLS_H

#include "can_messages.h"
#include "steering.h"
#include "steering_config.h"

#include <math.h>
#include <stdint.h>

#define BUTTONS_N                    8
#define MANETTINI_N                  3
#define BUTTONS_LONG_PRESS_TIME      1000
#define CALIBRATION_TIMEOUT_RESPONSE 3000

#define MANETTINO_STEPS_N 8

#define PADDLE_TOP_RIGHT    (2)
#define PADDLE_TOP_LEFT     (1)
#define PADDLE_BOTTOM_RIGHT (4)
#define PADDLE_BOTTOM_LEFT  (5)
#define BUTTON_TOP_RIGHT    (3)
#define BUTTON_TOP_LEFT     (0)
#define BUTTON_BOTTOM_RIGHT (7)
#define BUTTON_BOTTOM_LEFT  (6)

#define TORQUE_VECTORING_MAX            (100)
#define TORQUE_VECTORING_MIN            (0)
#define SLIP_CONTROL_MAX                (100)
#define SLIP_CONTROL_MIN                (0)
#define SET_PUMP_SPEED_MAX              (1.0f)
#define SET_PUMP_SPEED_MIN              (-0.1f)
#define SET_RADIATORS_MAX               (1.0f)
#define SET_RADIATORS_MIN               (-0.1f)
#define PORK_HIGH_FANS_SPEED            (100)
#define PORK_LOW_FANS_SPEED             (-10)
#define BALANCING_THRESHOLD_HIGH        (50)
#define BALANCING_THRESHOLD_LOW         (5)
#define POWER_MAP_MAX                   (100)
#define POWER_MAP_MIN                   (0)
#define LEFT_MANETTINO_SLIP_OPTION      0
#define LEFT_MANETTINO_POWER_MAP_OPTION 1
#define LEFT_MANETTINO_TORQUE_OPTION    2


typedef enum { RADS_AUTO, RADS_OFF, RADS_025, RADS_050, RADS_075, RADS_100, RADS_LENGTH } radiator_speed_t;

typedef enum { PUMPS_AUTO, PUMPS_OFF, PUMPS_025, PUMPS_050, PUMPS_075, PUMPS_100, P_LENGTH } pumps_speed_t;

void manettino_right_actions(int dsteps);
void manettino_center_actions(int dsteps);
void manettino_left_actions(int dsteps);

void buttons_pressed_actions(uint8_t button);
void buttons_released_actions(uint8_t button);
void buttons_long_pressed_actions(uint8_t button);

void prepare_set_car_status(void);
bool send_set_car_status_directly(void);

#endif  // CONTROLS_H

