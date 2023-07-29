#ifndef TAB_CAN_SNIFFER_H
#define TAB_CAN_SNIFFER_H

#include "cansniffer.h"
#include "steering.h"
#include "utils.h"

#define TAB_CANSNIFFER_N_MESSAGES_SHOWN 12

void tab_cansniffer_create(lv_obj_t *parent);
void cansniffer_primary_new_message(can_message_t *msg);
void switch_primary_cansniffer();
void update_primary_cansniffer_ui(lv_timer_t *tim);

#endif // TAB_CAN_SNIFFER_H
