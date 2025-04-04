#ifndef CAN_MESSAGES_H
#define CAN_MESSAGES_H

#include "inverters_network.h"
#include "inverters_watchdog.h"
#include "primary_network.h"
#include "primary_watchdog.h"
#include "secondary_network.h"
#include "secondary_watchdog.h"
#include "steering.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

bool can_send(can_message_t *msg, bool to_primary_network);

void send_ecu_set_status(primary_ecu_set_status_status val);

void handle_primary(can_message_t *msg);
void handle_secondary(can_message_t *msg);

#endif  // CAN_MESSAGES_H
