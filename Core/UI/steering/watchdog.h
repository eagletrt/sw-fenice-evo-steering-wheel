#ifndef WATCHDOG_H
#define WATCHDOG_H

#include "can_messages.h"
#include "lvgl.h"
#include "messages.h"
#include "primary/primary_network.h"
#include "primary/primary_watchdog.h"
#include "secondary/secondary_network.h"
#include "secondary/secondary_watchdog.h"
#include "steering.h"
#include "steering_config.h"

void init_watchdog(void);
void watchdog_task_fn(lv_timer_t *main_timer);

#endif
