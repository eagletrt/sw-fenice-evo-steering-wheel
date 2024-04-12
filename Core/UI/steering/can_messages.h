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

extern uint8_t primary_messages_last_state[primary_MESSAGE_COUNT][primary_MAX_STRUCT_SIZE_CONVERSION];
extern uint8_t secondary_messages_last_state[secondary_MESSAGE_COUNT][secondary_MAX_STRUCT_SIZE_CONVERSION];
extern uint8_t inverters_messages_last_state[inverters_MESSAGE_COUNT][inverters_MAX_STRUCT_SIZE_CONVERSION];

extern primary_lv_cells_voltage_converted_t lv_voltages_stock_1;
extern primary_lv_cells_voltage_converted_t lv_voltages_stock_2;
extern primary_lv_cells_temp_converted_t lv_temps_stock_1;
extern primary_lv_cells_temp_converted_t lv_temps_stock_2;

bool can_send(can_message_t *msg, bool to_primary_network);

void send_steer_version(lv_timer_t *);
void send_steer_status(lv_timer_t *);
void pedal_calibration_ack();
void handle_ptt_message();
void send_bal(bool on);

void handle_primary(can_message_t *msg);
void handle_secondary(can_message_t *msg);

void init_periodic_can_messages_timers(void);

#endif  // CAN_MESSAGES_H
