#ifndef STEERING_LVGL_H
#define STEERING_LVGL_H

#include "steering.h"

#if STEERING_WHEEL_MODE == STEERING_WHEEL_LVGL_MODE

#include "lvgl.h"
#include "utils.h"

#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define INV_MAX_SPEED (6500.f)  // MOT_RPM_LIMIT_REAL

#define STEER_UPDATE_COLOR_LABEL(name, color)                                           \
    for (uint32_t itab = 0; itab < NUM_RACING_TABS; itab++) {                           \
        if (name[itab] != NULL)                                                         \
            lv_obj_set_style_text_color(name[itab], lv_color_hex(color), LV_PART_MAIN); \
    }

#if STEER_TAB_DEBUG_ENABLED == 1
#define STEER_ERROR_UPDATE(device, error_name, aindex)                                                         \
    if (device##_last_state.error_name != data->error_name) {                                                  \
        device##_last_state.error_name = data->error_name;                                                     \
        if (data->error_name) {                                                                                \
            lv_obj_set_style_border_color(device[aindex], lv_color_hex(COLOR_RED_STATUS_HEX), LV_PART_MAIN);   \
            lv_obj_set_style_bg_color(device[aindex], lv_color_hex(COLOR_RED_STATUS_HEX), LV_PART_MAIN);       \
        } else {                                                                                               \
            lv_obj_set_style_border_color(device[aindex], lv_color_hex(COLOR_GREEN_STATUS_HEX), LV_PART_MAIN); \
            lv_obj_set_style_bg_color(device[aindex], lv_color_hex(COLOR_GREEN_STATUS_HEX), LV_PART_MAIN);     \
        }                                                                                                      \
    }
#endif

// #define STEER_ERROR_INVALIDATE(device, error_name, aindex) lv_obj_set_style_bg_color(steering.device[aindex], lv_color_hex(COLOR_YELLOW_STATUS_HEX), LV_PART_MAIN);

// #define CHECK_CURRENT_TAB(condition, mod, curr) if ((condition) || (current_##mod##_tab != curr)) return

#define CHECK_CURRENT_TAB(condition, mod, curr) (void)0;

/*
 * UPDATE
 */
void set_tab_sensors_value_brake_f(float value_in_bar);
void set_tab_sensors_value_brake_r(float value_in_bar);
void set_tab_sensors_value_apps(int32_t value_0_to_100);

void keep_lap_counter_value(uint32_t);
void remove_keep_lap_counter(lv_timer_t *timer);

void display_notification(const char *label_content, uint32_t timeout_ms, uint32_t background_color_hex, uint32_t label_color_hex);
void enter_fatal_error_mode(const char *);
void restore_previous_screen(lv_timer_t *timer);
bool is_on_help_animation(void);
void set_on_help_animation(void);
void update_sensors_extra_value(const char *, uint8_t);
void all_leds_green(void);
void all_leds_red(void);
void tab_terminal_new_message(const char *message);
void tab_terminal_clear(void);

/***
 * Tab racing
 */
void set_tab_racing_lv_soc_bar(int32_t v);
void set_tab_racing_hv_soc_bar(int32_t v);
void set_tab_racing_label_text(const char *s, tab_racing_labels_enum idx);
void set_tab_racing_ptt_label_color(bool active);
void set_tab_racing_speedometer_indicator(int32_t value);

/***
 * Tab sensors
 */
void set_tab_sensors_label_text(const char *s, tab_sensors_labels_enum idx);

/***
 * Tab Track Test
 */
void set_tab_track_test_steering_angle_bar(float v);
void set_tab_track_test_lb_speed(const char *s);
void set_tab_track_test_dmt_steering_angle_target(float f);
void set_tab_track_test_lb_steering_angle(const char *s);

//TODO should we delete these fn?
void __precharge_bar_update(int32_t);         // Deprecated
void __precharge_bar_insert(bool precharge);  // Deprecated

void precharge_bar_popup_show();
void precharge_bar_popup_hide();
void precharge_bar_set_pack_voltage(float pack_voltage);
void precharge_bar_set_bus_voltage(float bus_voltage);

calibration_box_t *get_tab_calibration_curr_focus();
lv_obj_t *get_tab_calibration_slider();

void set_label_color_lv_errors(bool label, int i);
void set_label_color_hv_errors(int label, int i);
void set_label_color_das_errors(bool label, int i);
void set_label_color_hv_feedbacks(int label, int i);

/***
 * Tab hv
 */
void tab_hv_set_error_status(debug_signal_error_t error, bool status);
void tab_hv_update_error_label();
void set_tab_hv_label_text(const char *s, tab_hv_labels_enum idx);
void set_balancing_column(bool balancing, uint8_t idx);
void tab_hv_set_pork_speed_bar(int32_t, bool);
bool is_shutdown_closed(void);

void update_shutdown_circuit_component(shutdown_circuit_indexes_t idx, bool is_close);

/***
 * Tab lv
 */
void set_tab_lv_label_text(const char *s, tab_lv_labels_enum idx);
void lv_set_pumps_speed_bar(int32_t, bool auto_mode);
void lv_set_radiators_speed_bar(int32_t val, bool auto_mode);

/***
 * Endurance screen
 */
void endurance_screen_set_label(const char *text, endurance_screen_labels_idx_t label_idx);
void endurance_screen_set_color(uint32_t fg_color_hex, uint32_t bg_color_hex, endurance_screen_labels_idx_t label_idx);

#endif  // STEERING_WHEEL_MODE == STEERING_WHEEL_LVGL_MODE

#endif  // STEERING_H
