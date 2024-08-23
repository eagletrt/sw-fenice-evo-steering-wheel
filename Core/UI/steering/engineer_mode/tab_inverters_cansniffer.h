#ifndef INVERTERS_TAB_CAN_SNIFFER_H
#define INVERTERS_TAB_CAN_SNIFFER_H

#include "steering_config.h"

#ifdef CANSNIFFER_ENABLED

#include "cansniffer.h"
#include "steering.h"
#include "utils.h"

void inverters_tab_cansniffer_create(lv_obj_t *parent);

#endif  // CANSNIFFER_ENABLED

#endif  // INVERTERS_TAB_CAN_SNIFFER_H
