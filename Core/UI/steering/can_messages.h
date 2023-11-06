#ifndef CAN_MESSAGES_H
#define CAN_MESSAGES_H

#include "cansniffer.h"
#include "inverters/inverters_network.h"
#include "inverters/inverters_watchdog.h"
#include "lvgl.h"
#include "messages.h"
#include "primary/primary_network.h"
#include "primary/primary_watchdog.h"
#include "secondary/secondary_network.h"
#include "secondary/secondary_watchdog.h"
#include "steering.h"
#include <stdbool.h>
#include <stdint.h>

bool can_send(can_message_t *msg, bool to_primary_network);
void openblt_reset(void);

void send_steer_version(lv_timer_t *);
void send_steer_status(lv_timer_t *);
void pedal_calibration_ack(primary_pedal_calibration_ack_converted_t *data);
void handle_ptt_message(primary_ptt_status_status val);
void send_bal(bool on);

void handle_primary(can_message_t *msg);
void handle_secondary(can_message_t *msg);

void init_periodic_can_messages_timers(void);

#endif // CAN_MESSAGES_H
