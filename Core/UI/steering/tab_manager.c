#include "tab_manager.h"

lv_obj_t *tab_racing_ptr;
lv_obj_t *tab_sensors_ptr;
lv_obj_t *tab_calibration_ptr;
lv_obj_t *tab_track_test_ptr;
lv_obj_t *tab_debug_ptr;
lv_obj_t *notif_screen;
lv_obj_t *tab_engineer_mode_ptr;
lv_obj_t *tab_cooling_ptr;
lv_obj_t *tab_cansniffer_ptr;

lv_timer_t *notification_timer;
lv_group_t *g;

racing_tab_t current_racing_tab;
engineer_tab_t current_engineer_tab;

bool steering_initialized = false;

void load_current_racing_tab();
void load_current_engineering_tab();

void tab_manager(void) {
  init_custom_styles();

  tab_racing_ptr = lv_obj_create(NULL);
  tab_sensors_ptr = lv_obj_create(NULL);
  tab_calibration_ptr = lv_obj_create(NULL);
  tab_track_test_ptr = lv_obj_create(NULL);
  tab_debug_ptr = lv_obj_create(NULL);
  notif_screen = lv_obj_create(NULL);
  tab_engineer_mode_ptr = lv_obj_create(NULL);
  tab_cooling_ptr = lv_obj_create(NULL);
  tab_cansniffer_ptr = lv_obj_create(NULL);

  lv_group_add_obj(g, tab_racing_ptr);
  lv_group_add_obj(g, tab_sensors_ptr);
  lv_group_add_obj(g, tab_calibration_ptr);
  lv_group_add_obj(g, tab_debug_ptr);
  lv_group_add_obj(g, tab_track_test_ptr);
  lv_group_add_obj(g, notif_screen);
  lv_group_add_obj(g, tab_engineer_mode_ptr);
  lv_group_add_obj(g, tab_cooling_ptr);
  lv_group_add_obj(g, tab_cansniffer_ptr);

  tab_racing_create(tab_racing_ptr);
  tab_sensors_create(tab_sensors_ptr);
  tab_calibration_create(tab_calibration_ptr);
  tab_debug_create(tab_debug_ptr);
  tab_track_test_create(tab_track_test_ptr);
  tab_notification_screen_create(notif_screen);
  tab_engineer_screen_create(tab_engineer_mode_ptr);
  tab_cooling_create(tab_cooling_ptr);
  tab_cansniffer_create(tab_cansniffer_ptr);
  lv_scr_load(tab_racing_ptr);
  current_racing_tab = TAB_RACING;
  current_engineer_tab = TAB_ENGINEER_MODE;

  steering_initialized = true;
}

/***
 * Engineer Mode
 */
bool engineer_mode = false;

void load_engineer_mode_screen(void) { load_current_engineering_tab(); }

void remove_engineer_mode_screen(void) { load_current_racing_tab(); }

void switch_mode(void) {
  if (engineer_mode) {
    // exit EM
    engineer_mode = false;
    remove_engineer_mode_screen();
  } else {
    // enter EM
    engineer_mode = true;
    load_engineer_mode_screen();
  }
}

/***
 * Tab Manager
 */
void load_current_racing_tab() {
  switch (current_racing_tab) {
  case TAB_RACING:
    lv_scr_load(tab_racing_ptr);
    break;
  case TAB_CALIBRATION:
    lv_scr_load(tab_calibration_ptr);
    break;
  case TAB_DEBUG:
    lv_scr_load(tab_debug_ptr);
    break;
  case TAB_TRACK_TEST:
    lv_scr_load(tab_track_test_ptr);
    break;
  case TAB_SENSORS:
    lv_scr_load(tab_sensors_ptr);
    break;
  default:
    break;
  }
}

void load_current_engineering_tab() {
  switch (current_engineer_tab) {
  case TAB_ENGINEER_MODE:
    lv_scr_load(tab_engineer_mode_ptr);
    break;
  case TAB_COOLING:
    lv_scr_load(tab_cooling_ptr);
    break;
  case TAB_CANSNIFFER:
    lv_scr_load(tab_cansniffer_ptr);
    break;
  default:
    break;
  }
}

void change_racing_tab(bool forward) {
  if (forward)
    current_racing_tab = (current_racing_tab + 1) % NUM_TABS;
  else
    current_racing_tab = (current_racing_tab + NUM_TABS - 1) % NUM_TABS;

  load_current_racing_tab();
}

void change_engineer_tab(bool forward) {
  if (forward)
    current_engineer_tab = (current_engineer_tab + 1) % NUM_ENGINEER_TABS;
  else
    current_engineer_tab =
        (current_engineer_tab + NUM_ENGINEER_TABS - 1) % NUM_ENGINEER_TABS;

  load_current_engineering_tab();
}

void steering_change_tab(bool forward) {
  if (!engineer_mode) {
    change_racing_tab(forward);
  } else {
    change_engineer_tab(forward);
  }
}

/***
 * Notification Screen
 */

bool on_animation = false;

void display_notification(const char *label_content, uint32_t timeout_ms) {
  if (on_animation) {
    lv_timer_set_repeat_count(notification_timer, 0);
  }
  on_animation = true;
  notification_timer =
      lv_timer_create(restore_previous_screen, timeout_ms, NULL);
  lv_timer_set_repeat_count(notification_timer, 1);
  lv_label_set_text(steering.notification_screen_label, label_content);
  lv_scr_load(notif_screen);
  lv_timer_reset(notification_timer);
}

void restore_previous_screen(lv_timer_t *timer) {
  if (engineer_mode) {
    load_current_engineering_tab();
  } else {
    load_current_racing_tab();
  }
  on_animation = false;
}
