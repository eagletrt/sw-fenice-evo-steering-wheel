#ifndef STEERING_H
#define STEERING_H

#include "engineer_mode/tab_cooling.h"
#include "inverters/inverters_network.h"
#include "lvgl.h"
#include "primary/primary_network.h"
#include "secondary/secondary_network.h"
#include "shutdown.h"
#include "steering_config.h"
#include "utils.h"
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define INV_MAX_SPEED 6500.f // MOT_RPM_LIMIT_REAL

#define STEERING_ANGLE_RANGE_LOW -150
#define STEERING_ANGLE_RANGE_HIGH 150
#define APPS_RANGE_LOW 0
#define APPS_RANGE_HIGH 130
#define BRAKE_RANGE_LOW 0
#define BRAKE_RANGE_HIGH 180

typedef enum { BSE, STEER, APPS, CALBOX_N } calibration_box_t;

#define GET_LAST_STATE(ntw, msg, NTW, MSG)                                     \
  ntw##_##msg##_converted_t *ntw##_##msg##_last_state =                        \
      (ntw##_##msg##_converted_t                                               \
           *)&ntw##_messages_last_state[NTW##_##MSG##_INDEX][0]

#define STEER_UPDATE_COLOR_LABEL(name, color)                                  \
  for (uint32_t itab = 0; itab < NUM_RACING_TABS; itab++) {                    \
    if (name[itab] != NULL)                                                    \
      lv_obj_set_style_text_color(name[itab], lv_color_hex(color),             \
                                  LV_PART_MAIN);                               \
  }

#if STEER_TAB_DEBUG_ENABLED == 1
#define STEER_ERROR_UPDATE(device, error_name, aindex)                         \
  if (device##_last_state.error_name != data->error_name) {                    \
    device##_last_state.error_name = data->error_name;                         \
    if (data->error_name) {                                                    \
      lv_obj_set_style_border_color(                                           \
          device[aindex], lv_color_hex(COLOR_RED_STATUS_HEX), LV_PART_MAIN);   \
      lv_obj_set_style_bg_color(                                               \
          device[aindex], lv_color_hex(COLOR_RED_STATUS_HEX), LV_PART_MAIN);   \
    } else {                                                                   \
      lv_obj_set_style_border_color(                                           \
          device[aindex], lv_color_hex(COLOR_GREEN_STATUS_HEX), LV_PART_MAIN); \
      lv_obj_set_style_bg_color(                                               \
          device[aindex], lv_color_hex(COLOR_GREEN_STATUS_HEX), LV_PART_MAIN); \
    }                                                                          \
  }
#endif

#define STEER_ERROR_INVALIDATE(device, error_name, aindex)                     \
  lv_obj_set_style_bg_color(steering.device[aindex],                           \
                            lv_color_hex(COLOR_YELLOW_STATUS_HEX),             \
                            LV_PART_MAIN);

#define CHECK_CURRENT_TAB(mod, curr)                                           \
  if ((!mod##_mode) || (current_##mod##_tab != curr))                          \
  return

/*
 * UPDATE
 */

void car_status_update();
void tlm_status_update();
void speed_update();

void hv_voltage_update();
void hv_current_update();
void hv_temp_update();
void hv_errors_update();
void hv_cell_balancing_status_update();
void hv_feedbacks_status_update();
void lv_feedbacks_update();
void das_errors_update();

void lv_cells_temp_update();
void lv_currents_update();
void lv_cells_voltage_update();
void cooling_status_update();
void lv_total_voltage_update();
void lv_errors_update();
void hv_fans_override_status_update();
void tlm_status_update();

void steering_angle_update();
void pedals_output_update();
void imu_acceleration_update();
void lap_count_update();
void lc_status_update();
void inv_l_rcv_update(void);
void inv_r_rcv_update(void);

void irts_fl_update(void);
void irts_fr_update(void);
void irts_rl_update(void);
void irts_rr_update(void);

void keep_lap_counter_value(uint32_t);
void remove_keep_lap_counter(lv_timer_t *timer);

void display_notification(const char *, uint32_t);
void enter_fatal_error_mode(const char *);
void restore_previous_screen(lv_timer_t *timer);
void update_sensors_extra_value(const char *, uint8_t);
void all_leds_green(void);
void all_leds_red(void);
void tab_terminal_new_message(const char *message);
void tab_terminal_clear(void);
void update_shutdown_circuit(shutdown_component_state_t *);
void set_ptt_button_pressed(bool);

void set_lb_estimated_velocity(const char *s);
void set_lb_apps(const char *s);
void set_lb_bse(const char *s);

void set_tab_calibration_lb_steering_angle(const char *s);
void tab_calibration_lb_steering_angle_invalidate();

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

void set_tab_track_test_lb_inverter_speed_x(const char *s);
void tab_track_test_lb_inverter_speed_x_invalidate();
void set_tab_track_test_lb_inverter_speed_y(const char *s);
void set_tab_track_test_steering_angle_bar(float v);
void set_tab_track_test_lb_speed(const char *s);
void set_tab_track_test_dmt_steering_angle_target(float f);
void tab_track_test_lb_inverter_speed_y_invalidate();
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

calibration_box_t *get_tab_calibration_curr_focus();
lv_obj_t *get_tab_calibration_slider();

void set_label_color_lv_errors(bool label, int i);
void set_label_color_hv_errors(int label, int i);
void set_label_color_das_errors(bool label, int i);
void set_label_color_hv_feedbacks(int label, int i);
#endif /* STEERING_H */
