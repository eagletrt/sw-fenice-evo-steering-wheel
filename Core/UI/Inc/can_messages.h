#ifndef CAN_MESSAGES_H
#define CAN_MESSAGES_H

#include "inverters/inverters_network.h"
#include "inverters/inverters_watchdog.h"
#include "primary/primary_network.h"
#include "primary/primary_watchdog.h"
#include "secondary/secondary_network.h"
#include "secondary/secondary_watchdog.h"
#include "steering.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

bool can_send(can_message_t *msg, bool to_primary_network);

void send_set_car_status(primary_ecu_set_status_status val);

void handle_primary(can_message_t *msg);
void handle_secondary(can_message_t *msg);

#endif  // CAN_MESSAGES_H
