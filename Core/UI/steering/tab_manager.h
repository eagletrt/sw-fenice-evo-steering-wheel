#ifndef TAB_MANAGER_H
#define TAB_MANAGER_H

#include "engineer_mode/tab_cansniffer.h"
#include "engineer_mode/tab_cooling.h"
#include "engineer_mode/tab_engineer_mode.h"
#include "lvgl.h"
#include "racing_tabs/tab_calibration.h"
#include "racing_tabs/tab_debug.h"
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

void display_notification(const char *, uint32_t);
void restore_previous_screen(lv_timer_t *timer);

#endif
