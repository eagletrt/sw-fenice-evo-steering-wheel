#if CANSNIFFER_ENABLED == 1

#ifndef PRIMARY_TAB_CAN_SNIFFER_H
#define PRIMARY_TAB_CAN_SNIFFER_H

#include "cansniffer.h"
#include "steering.h"
#include "steering_config.h"
#include "utils.h"

void primary_tab_cansniffer_create(lv_obj_t *parent);

#endif  // PRIMARY_TAB_CAN_SNIFFER_H

#endif  // CANSNIFFER_ENABLED == 1
