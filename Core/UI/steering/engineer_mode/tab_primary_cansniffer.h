#ifndef PRIMARY_TAB_CAN_SNIFFER_H
#define PRIMARY_TAB_CAN_SNIFFER_H

#include "steering_config.h"

#if STEERING_WHEEL_MODE == STEERING_WHEEL_LVGL_MODE

#if CANSNIFFER_ENABLED == 1

#include "cansniffer.h"
#include "steering.h"
#include "utils.h"

void primary_tab_cansniffer_create(lv_obj_t *parent);

#endif  // CANSNIFFER_ENABLED == 1

#endif  // STEERING_WHEEL_MODE == STEERING_WHEEL_LVGL_MODE

#endif  // PRIMARY_TAB_CAN_SNIFFER_H
