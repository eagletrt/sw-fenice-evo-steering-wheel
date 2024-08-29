#ifndef WATCHDOG_H
#define WATCHDOG_H

#include "can_messages.h"
#include "messages.h"
#include "steering.h"
#include "steering_config.h"

void init_watchdog(void);
void watchdog_task_fn(void *unused);

#endif
