#ifndef TAB_MANAGER_H
#define TAB_MANAGER_H

#include "steering.h"
#include "lvgl.h"
#include "racing_tabs/tab_calibration.h"
#include "racing_tabs/tab_debug.h"
#include "racing_tabs/tab_notification.h"
#include "racing_tabs/tab_racing.h"
#include "racing_tabs/tab_sensors.h"
#include "racing_tabs/tab_track_test.h"
#include "engineer_mode/tab_engineer_mode.h"
#include "utils.h"
#ifndef STM32H723xx
#include "lv-drivers/indev/keyboard.h"
#endif

extern lv_group_t *g;

void tab_manager(void);
void change_tab(bool);
void load_current_tab(void);
void restore_previous_screen(lv_timer_t *timer);
void display_notification(const char *, uint32_t);

void load_engineer_mode_screen(void);
void remove_engineer_mode_screen(void);

#endif