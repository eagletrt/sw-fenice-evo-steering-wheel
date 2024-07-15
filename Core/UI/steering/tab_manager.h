#include "steering_config.h"

#ifndef TAB_MANAGER_H
#define TAB_MANAGER_H

#include "can_messages.h"
#include "engineer_mode/tab_debug.h"
#include "engineer_mode/tab_fatal_error.h"
#include "engineer_mode/tab_inverters_cansniffer.h"
#include "engineer_mode/tab_primary_cansniffer.h"
#include "engineer_mode/tab_secondary_cansniffer.h"
#include "engineer_mode/tab_terminal.h"
#include "lvgl.h"
#include "racing_tabs/tab_hv.h"
#include "racing_tabs/tab_lv.h"
#include "racing_tabs/tab_notification.h"
#include "racing_tabs/tab_racing.h"
#include "racing_tabs/tab_sensors.h"
#include "racing_tabs/tab_track_test.h"
#include "steering.h"
#include "utils.h"
#ifndef STM32H723xx
#include "lv-drivers/indev/keyboard.h"
#endif

extern lv_group_t *g;

void tab_manager(void);

void switch_mode(void);

void steering_change_tab(bool forward);

#endif
