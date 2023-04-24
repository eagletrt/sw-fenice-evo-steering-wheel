#ifndef STEERING_H
#define STEERING_H

#include "lvgl.h"
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define LV_PROPERTY(type, name)                                                \
  type name;                                                                   \
  lv_obj_t *lb_##name;

#define LV_UPDATE_PROPERTY(name, value)                                        \
  steering.name = value;                                                       \
  lv_label_set_text_fmt(steering.lb_##name, "%d", steering.name);

typedef struct {

  /*----primary messages-----*/
  LV_PROPERTY(uint8_t, AMBIENT_TEMPERATURE)

  // struct {
  LV_PROPERTY(uint8_t, HV_VOLTAGE)
  LV_PROPERTY(uint8_t, HV_CURRENT)
  LV_PROPERTY(uint8_t, HV_TEMP)
  LV_PROPERTY(uint8_t, HV_PERCENT)
  // } hv;

  /*--HV system--*/

  /*--LV system--*/
  LV_PROPERTY(uint8_t, LV_VOLTAGE)
  LV_PROPERTY(uint8_t, LV_CURRENT)
  LV_PROPERTY(uint8_t, LV_TEMP)
  LV_PROPERTY(uint8_t, LV_PERCENT)
  LV_PROPERTY(uint8_t, CAR_STATUS)

  /*----secondary messages-----*/
  LV_PROPERTY(uint8_t, GPS_SPEED)
  LV_PROPERTY(uint8_t, LAP_COUNT)

  /*secondary_message_CONTROL_OUTPUT;*/
  LV_PROPERTY(uint8_t, ESTIMATED_VELOCITY)
  LV_PROPERTY(uint8_t, TORQUE)

  /*temps tyres*/
  LV_PROPERTY(uint8_t, FL_TEMP)
  LV_PROPERTY(uint8_t, FR_TEMP)
  LV_PROPERTY(uint8_t, RL_TEMP)
  LV_PROPERTY(uint8_t, RR_TEMP)

  /*pressure tyres*/
  LV_PROPERTY(uint8_t, FL_PRESS)
  LV_PROPERTY(uint8_t, FR_PRESS)
  LV_PROPERTY(uint8_t, RL_PRESS)
  LV_PROPERTY(uint8_t, RR_PRESS)

  /*----the following are guessed (don't know which messages are in can)----*/
  LV_PROPERTY(uint8_t, MOTOR_TEMP)
  LV_PROPERTY(uint8_t, INVERTER_TEMP)

  /*values for lap*/
  LV_PROPERTY(uint8_t, BEST_TIME)
  LV_PROPERTY(uint8_t, LAST_TIME)
  LV_PROPERTY(uint8_t, DELTA_TIME)
  LV_PROPERTY(uint8_t, SLIP)
  LV_PROPERTY(uint8_t, POWER)
  LV_PROPERTY(uint8_t, KM)

  /*parts to elements that can't be accessed otherwise*/
  lv_obj_t *custom_meter;

} steering_t;

extern steering_t steering;
void test_value_update_incremental(void);

#endif
