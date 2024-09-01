#ifndef CAN_MESSAGES_H
#define CAN_MESSAGES_H

#include "steering.h"

#include "inverters/inverters_network.h"
#include "inverters/inverters_watchdog.h"
#include "primary/primary_network.h"
#include "primary/primary_watchdog.h"
#include "secondary/secondary_network.h"
#include "secondary/secondary_watchdog.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

bool can_send(can_message_t *msg, bool to_primary_network);

void send_steering_wheel_version(void);
void send_ecu_set_power_maps(void);
void send_hv_set_balancing_status_steering_wheel(void);
void send_hv_fans_speed(void);
void send_pumps_speed(void);
void send_radiator_speed(void);

void handle_primary(can_message_t *msg);
void handle_secondary(can_message_t *msg);

#endif  // CAN_MESSAGES_H

