#ifndef PRIMARY_TAB_CAN_SNIFFER_H
#define PRIMARY_TAB_CAN_SNIFFER_H

#include "steering_config.h"

#ifdef CANSNIFFER_ENABLED

#include "cansniffer.h"
#include "steering.h"
#include "utils.h"

void primary_tab_cansniffer_create(lv_obj_t *parent);

#endif  // CANSNIFFER_ENABLED

#endif  // PRIMARY_TAB_CAN_SNIFFER_H
