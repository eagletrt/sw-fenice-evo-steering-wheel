#ifndef STEERING_H
#define STEERING_H

#include "inverters/inverters_network.h"
#include "lvgl.h"
#include "primary/primary_network.h"
#include "secondary/secondary_network.h"
#include "steering_config.h"
#include "utils.h"

#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define INV_MAX_SPEED (6500.f)  // MOT_RPM_LIMIT_REAL

#define STEERING_ANGLE_RANGE_LOW  (-150)
#define STEERING_ANGLE_RANGE_HIGH (150u)
#define APPS_RANGE_LOW            (0u)
#define APPS_RANGE_HIGH           (130u)
#define BRAKE_RANGE_LOW           (0u)
#define BRAKE_RANGE_HIGH          (180u)
#define N_PORK_CELLBOARD          (6u)

typedef enum { BSE, STEER, APPS, CALBOX_N } calibration_box_t;

#define GET_LAST_STATE(ntw, msg, NTW, MSG) \
    ntw##_##msg##_converted_t *ntw##_##msg##_last_state = (ntw##_##msg##_converted_t *)&ntw##_messages_last_state[NTW##_##MSG##_INDEX][0]

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

#define STEER_ERROR_INVALIDATE(device, error_name, aindex) \
    lv_obj_set_style_bg_color(steering.device[aindex], lv_color_hex(COLOR_YELLOW_STATUS_HEX), LV_PART_MAIN);

// #define CHECK_CURRENT_TAB(condition, mod, curr) if ((condition) || (current_##mod##_tab != curr)) return

#define CHECK_CURRENT_TAB(condition, mod, curr) (void) 0;

/*
 * UPDATE
 */

void car_status_update();
void tlm_status_update();
void angular_velocity_update();
void ecu_feedbacks_update();

void hv_debug_signals_update();
void hv_cells_voltage_stats_update();
void hv_total_voltage_update();
void hv_current_update();
void hv_soc_estimation_update();
void hv_cells_temp_stats_update();
void hv_errors_update();
void hv_cell_balancing_status_update();
void hv_feedbacks_status_update();
void debug_hv_feedbacks_status_update();
void das_errors_update();

void hv_feedback_ts_voltage_update(void);
void hv_feedback_misc_voltage_update(void);
void hv_feedback_sd_voltage_update(void);

void lv_feedback_sd_voltage_update();
void lv_feedback_ts_voltage_update();
void lv_feedback_enclosure_voltage_update();
void lv_errors_update();
void lv_currents_update();
void lv_total_voltage_update();
void lv_cells_voltage_update();
void lv_cells_voltage_stats_update();
void lv_cells_temp_update();
void lv_cells_temp_stats_update();

void hv_status_update();
void hv_fans_override_status_update();
void tlm_status_update();

void odometer_update();
void steer_angle_update();
void tlm_network_interface_update(void);
void imu_acceleration_update();
void tlm_lap_time_update(void);
void tlm_laps_stats_update(void);
void inv_l_rcv_update(void);
void inv_r_rcv_update(void);

void irts_fl_update(void);
void irts_fr_update(void);
void irts_rl_update(void);
void irts_rr_update(void);
void pedal_throttle_update(void);
void pedal_brakes_pressure_update(void);
void set_tab_sensors_value_brake_f(float value_in_bar);
void set_tab_sensors_value_brake_r (float value_in_bar);
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
void set_ptt_button_pressed(bool);

/***
 * Tab racing
 */
void set_tab_racing_hv_current_bar(float v);
void set_tab_racing_hv_pack_voltage_bar(float v);
void set_tab_racing_label_text(const char *s, tab_racing_labels_enum idx);

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
void tab_track_test_lb_steering_angle_invalidate();
void set_tab_track_test_lb_speed(const char *s);
void tab_track_test_lb_speed_invalidate();

void bal_status_label_invalidate();

void pumps_speed_bar_invalidate();
void radiators_speed_bar_invalidate();
void pork_speed_bar_invalidate();
void pumps_speed_value_label_invalidate();
void radiators_speed_value_label_invalidate();
void pork_speed_value_label_invalidate();
void precharge_bar_update(int32_t);
void precharge_bar_insert(bool precharge);

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
void tab_hv_pork_speed_bar_invalidate();

void update_shutdown_circuit_component(shutdown_circuit_indexes_t idx, bool is_close);

/***
 * Tab lv
 */
void set_tab_lv_label_text(const char *s, tab_lv_labels_enum idx);
void lv_set_pumps_speed_bar(int32_t);
void lv_set_radiators_speed_bar(int32_t val, bool auto_mode);

void lv_radiator_speed_update(void);
void lv_pumps_speed_update(void);

extern uint8_t primary_messages_last_state[primary_MESSAGE_COUNT][primary_MAX_STRUCT_SIZE_CONVERSION];
extern uint8_t secondary_messages_last_state[secondary_MESSAGE_COUNT][secondary_MAX_STRUCT_SIZE_CONVERSION];
extern uint8_t inverters_messages_last_state[inverters_MESSAGE_COUNT][inverters_MAX_STRUCT_SIZE_CONVERSION];

#endif /* STEERING_H */
