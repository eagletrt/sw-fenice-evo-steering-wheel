#include "tab_manager.h"

lv_obj_t *tab_racing_ptr;
lv_obj_t *tab_sensors_ptr;
lv_obj_t *tab_hv_ptr;
lv_obj_t *tab_lv_ptr;

#if STEER_TAB_CALIBRATION_ENABLED == 1
lv_obj_t *tab_calibration_ptr;
#endif

lv_obj_t *tab_track_test_ptr;

#if STEER_TAB_DEBUG_ENABLED == 1
lv_obj_t *tab_debug_ptr;
#endif

lv_obj_t *notif_screen;
lv_obj_t *tab_engineer_mode_ptr;
lv_obj_t *tab_cooling_ptr;
lv_obj_t *tab_fatal_error_ptr;
lv_obj_t *tab_shutdown_circuit_ptr;

#if CANSNIFFER_ENABLED == 1
lv_obj_t *tab_primary_cansniffer_ptr;
lv_obj_t *tab_secondary_cansniffer_ptr;
#endif

lv_obj_t *tab_terminal_ptr;
lv_obj_t *tab_balancing_status_ptr;

lv_timer_t *notification_timer;
lv_group_t *g;

racing_tab_t current_racing_tab;
#if ENGINEERING_TAB_ENABLED == 1
engineer_tab_t current_engineer_tab;
#endif

bool steering_initialized = false;

void load_current_racing_tab();
#if ENGINEERING_TAB_ENABLED == 1
void load_current_engineering_tab();
#endif

void tab_manager(void) {
  init_custom_styles();

  tab_racing_ptr = lv_obj_create(NULL);
  tab_sensors_ptr = lv_obj_create(NULL);
  tab_hv_ptr = lv_obj_create(NULL);
  tab_lv_ptr = lv_obj_create(NULL);
#if STEER_TAB_CALIBRATION_ENABLED == 1
  tab_calibration_ptr = lv_obj_create(NULL);
#endif
  tab_track_test_ptr = lv_obj_create(NULL);
#if STEER_TAB_DEBUG_ENABLED == 1
  tab_debug_ptr = lv_obj_create(NULL);
#endif
  notif_screen = lv_obj_create(NULL);

#if ENGINEERING_TAB_ENABLED == 1
  tab_engineer_mode_ptr = lv_obj_create(NULL);
  tab_cooling_ptr = lv_obj_create(NULL);
#if CANSNIFFER_ENABLED == 1
  tab_primary_cansniffer_ptr = lv_obj_create(NULL);
  tab_secondary_cansniffer_ptr = lv_obj_create(NULL);
#endif
  tab_terminal_ptr = lv_obj_create(NULL);
  tab_fatal_error_ptr = lv_obj_create(NULL);
  tab_shutdown_circuit_ptr = lv_obj_create(NULL);
  tab_balancing_status_ptr = lv_obj_create(NULL);
#endif

  lv_group_add_obj(g, tab_racing_ptr);
  lv_group_add_obj(g, tab_sensors_ptr);
  lv_group_add_obj(g, tab_hv_ptr);
  lv_group_add_obj(g, tab_lv_ptr);

#if STEER_TAB_CALIBRATION_ENABLED == 1
  lv_group_add_obj(g, tab_calibration_ptr);
#endif

#if STEER_TAB_DEBUG_ENABLED == 1
  lv_group_add_obj(g, tab_debug_ptr);
#endif

  lv_group_add_obj(g, tab_track_test_ptr);
  lv_group_add_obj(g, notif_screen);

#if ENGINEERING_TAB_ENABLED == 1
  lv_group_add_obj(g, tab_engineer_mode_ptr);
  lv_group_add_obj(g, tab_cooling_ptr);
#if CANSNIFFER_ENABLED == 1
  lv_group_add_obj(g, tab_primary_cansniffer_ptr);
  lv_group_add_obj(g, tab_secondary_cansniffer_ptr);
#endif
  lv_group_add_obj(g, tab_terminal_ptr);
  lv_group_add_obj(g, tab_fatal_error_ptr);
  lv_group_add_obj(g, tab_shutdown_circuit_ptr);
  lv_group_add_obj(g, tab_balancing_status_ptr);
#endif

  tab_racing_create(tab_racing_ptr);
  tab_sensors_create(tab_sensors_ptr);
  tab_hv_create(tab_hv_ptr);
  tab_lv_create(tab_lv_ptr);

#if STEER_TAB_CALIBRATION_ENABLED == 1
  tab_calibration_create(tab_calibration_ptr);
#endif

#if STEER_TAB_DEBUG_ENABLED == 1
  tab_debug_create(tab_debug_ptr);
#endif

  tab_track_test_create(tab_track_test_ptr);
  tab_notification_screen_create(notif_screen);

#if ENGINEERING_TAB_ENABLED == 1
  tab_engineer_screen_create(tab_engineer_mode_ptr);
  tab_cooling_create(tab_cooling_ptr);
#if CANSNIFFER_ENABLED == 1
  primary_tab_cansniffer_create(tab_primary_cansniffer_ptr);
  secondary_tab_cansniffer_create(tab_secondary_cansniffer_ptr);
#endif
  tab_terminal_create(tab_terminal_ptr);
  tab_fatal_error_create(tab_fatal_error_ptr);
  tab_shutdown_circuit_create(tab_shutdown_circuit_ptr);
  tab_balancing_status_screen_create(tab_balancing_status_ptr);
#endif

  lv_scr_load(tab_racing_ptr);
  current_racing_tab = TAB_RACING;

#if ENGINEERING_TAB_ENABLED == 1
  current_engineer_tab = TAB_ENGINEER_MODE;
#endif

  steering_initialized = true;
}

/***
 * Engineer Mode
 */

/**
 * @brief Reload all the messages by setting the last state to 0
 */
void reload_all(void) {
#if STRICT_RELOAD_ALL_ENABLED == 1
  memset((void *)primary_messages_last_state, 0,
         primary_MESSAGE_COUNT * primary_MAX_STRUCT_SIZE_CONVERSION);
  memset((void *)secondary_messages_last_state, 0,
         secondary_MESSAGE_COUNT * secondary_MAX_STRUCT_SIZE_CONVERSION);
  memset((void *)inverters_messages_last_state, 0,
         inverters_MESSAGE_COUNT * inverters_MAX_STRUCT_SIZE_CONVERSION);
#endif
}

bool engineer_mode = false;

#if ENGINEERING_TAB_ENABLED == 1
void load_engineer_mode_screen(void) { load_current_engineering_tab(); }
#endif

void remove_engineer_mode_screen(void) { load_current_racing_tab(); }

void switch_mode(void) {
  if (engineer_mode) {
    engineer_mode = false;
    remove_engineer_mode_screen();
  } else {
#if ENGINEERING_TAB_ENABLED == 1
    engineer_mode = true;
    load_engineer_mode_screen();
#endif
  }
  reload_all();
}

/***
 * Tab Manager
 */
/**
 * @brief Load the current racing tab
 */
void load_current_racing_tab() {
  switch (current_racing_tab) {
  case TAB_RACING:
    lv_scr_load(tab_racing_ptr);
    break;
#if STEER_TAB_CALIBRATION_ENABLED == 1
  case TAB_CALIBRATION:
    lv_scr_load(tab_calibration_ptr);
    break;
#endif
#if STEER_TAB_DEBUG_ENABLED == 1
  case TAB_DEBUG:
    lv_scr_load(tab_debug_ptr);
    break;
#endif
  case TAB_TRACK_TEST:
    lv_scr_load(tab_track_test_ptr);
    break;
  case TAB_SENSORS:
    lv_scr_load(tab_sensors_ptr);
    break;
  case TAB_HV:
    lv_scr_load(tab_hv_ptr);
    break;
  case TAB_LV:
    lv_scr_load(tab_lv_ptr);
    break;
  default:
    break;
  }
}

#if ENGINEERING_TAB_ENABLED == 1
void load_current_engineering_tab() {
  switch (current_engineer_tab) {
  case TAB_ENGINEER_MODE:
    lv_scr_load(tab_engineer_mode_ptr);
    break;
  case TAB_COOLING:
    lv_scr_load(tab_cooling_ptr);
    break;
#if CANSNIFFER_ENABLED == 1
  case TAB_SECONDARY_CANSNIFFER:
    lv_scr_load(tab_secondary_cansniffer_ptr);
    break;
  case TAB_PRIMARY_CANSNIFFER:
    lv_scr_load(tab_primary_cansniffer_ptr);
    break;
#endif
  case TAB_TERMINAL:
    lv_scr_load(tab_terminal_ptr);
    break;
  case TAB_SHUTDOWN_CIRCUIT:
    lv_scr_load(tab_shutdown_circuit_ptr);
    break;
  case TAB_BALANCING_STATUS:
    lv_scr_load(tab_balancing_status_ptr);
    break;
  default:
    break;
  }
}
#endif

void change_racing_tab(bool forward) {
  if (forward)
    current_racing_tab = (current_racing_tab + 1) % NUM_RACING_TABS;
  else
    current_racing_tab =
        (current_racing_tab + NUM_RACING_TABS - 1) % NUM_RACING_TABS;

  load_current_racing_tab();
  reload_all();
}

#if ENGINEERING_TAB_ENABLED == 1
void change_engineer_tab(bool forward) {
  if (forward)
    current_engineer_tab = (current_engineer_tab + 1) % NUM_ENGINEER_TABS;
  else
    current_engineer_tab =
        (current_engineer_tab + NUM_ENGINEER_TABS - 1) % NUM_ENGINEER_TABS;

  load_current_engineering_tab();
  reload_all();
}
#endif

void steering_change_tab(bool forward) {
  if (!engineer_mode) {
    change_racing_tab(forward);
  } else {
#if ENGINEERING_TAB_ENABLED == 1
    change_engineer_tab(forward);
#endif
  }
}

/***
 * Tab Fatal Error
 */

void enter_fatal_error_mode(const char *message) {
  set_tab_fatal_error_text(message);
  lv_scr_load(tab_fatal_error_ptr);
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
  set_notification_screen_label(label_content);
  lv_scr_load(notif_screen);
  lv_timer_reset(notification_timer);
}

void restore_previous_screen(lv_timer_t *timer) {
  if (engineer_mode) {
#if ENGINEERING_TAB_ENABLED == 1
    load_current_engineering_tab();
#endif
  } else {
    load_current_racing_tab();
  }
  on_animation = false;
}

/***
 * Keep Lap Counter on screen for a couple of seconds
 * to allow the driver to see the lap time
 */

bool on_lap_keep = false;
lv_timer_t *lap_counter_timer;

void keep_lap_counter_value(uint32_t timeout_ms) {
  if (on_lap_keep) {
    lv_timer_set_repeat_count(lap_counter_timer, 0);
  }
  on_lap_keep = true;
  lap_counter_timer =
      lv_timer_create(remove_keep_lap_counter, timeout_ms, NULL);
  lv_timer_set_repeat_count(lap_counter_timer, 1);
  lv_scr_load(tab_racing_ptr);
  lv_timer_reset(lap_counter_timer);
}

void remove_keep_lap_counter(lv_timer_t *timer) {
  on_lap_keep = false;
  lv_timer_set_repeat_count(lap_counter_timer, 0);
}
