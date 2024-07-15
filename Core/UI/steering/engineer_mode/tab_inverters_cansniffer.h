#ifndef INVERTERS_TAB_CAN_SNIFFER_H
#define INVERTERS_TAB_CAN_SNIFFER_H

#include "steering_config.h"

#if CANSNIFFER_ENABLED == 1

#include "cansniffer.h"
#include "steering.h"
#include "utils.h"

void inverters_tab_cansniffer_create(lv_obj_t *parent);

#endif  // CANSNIFFER_ENABLED == 1

#endif  // INVERTERS_TAB_CAN_SNIFFER_H
