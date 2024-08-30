#ifndef CAN_MESSAGES_H
#define CAN_MESSAGES_H

#include "cansniffer.h"
#include "messages.h"
#include "steering.h"

#include <stdbool.h>
#include <stdint.h>

extern size_t tlm_ntw_interfaces_current_size;
extern uint32_t tlm_ntw_interfaces[TLM_NTW_INTERFACE_MAX_N];
extern uint32_t tlm_ntw_ips[TLM_NTW_INTERFACE_MAX_N];

bool can_send(can_message_t *msg, bool to_primary_network);

void send_steering_wheel_version(void *);
void send_ecu_set_power_maps(void *);
void handle_ptt_message();
void send_hv_set_balancing_status_steering_wheel(bool set_balancing_on, uint8_t balancing_threshold);

void handle_primary(can_message_t *msg);
void handle_secondary(can_message_t *msg);

void init_periodic_can_messages_timers(void);

#endif  // CAN_MESSAGES_H
