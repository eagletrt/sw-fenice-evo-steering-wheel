#ifndef STEERING_H
#define STEERING_H

#include "engineer_mode/tab_cooling.h"
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

void set_tab_racing_lb_speed(const char *s);
void set_tab_racing_lb_pack_voltage(const char *s);
void set_tab_racing_lb_hv_current(const char *s);
void set_tab_racing_lb_average_temperature(const char *s);
void set_tab_racing_lb_battery_temperature(const char *s);
void set_tab_racing_lb_last_time(const char *s);
void set_tab_racing_lb_delta_time(const char *s);
void set_tab_racing_lb_best_time(const char *s);
void set_tab_racing_lb_lap_count(const char *s);
void set_tab_racing_lb_left_motor_temp(const char *s);
void set_tab_racing_lb_left_inverter_temp(const char *s);
void set_tab_racing_lb_slip(const char *s);
void set_tab_racing_lb_torque(const char *s);
void set_tab_racing_lb_power(const char *s);

void set_tab_sensors_lb_pack_voltage(const char *s);
void set_tab_sensors_lb_hv_delta(const char *s);
void set_tab_sensors_lb_hv_current(const char *s);
void set_tab_sensors_lb_average_temperature(const char *s);
void set_tab_sensors_lb_lv_current(const char *s);
void set_tab_sensors_lb_battery_temperature(const char *s);
void set_tab_sensors_lb_voltage(const char *s);
void set_tab_sensors_lb_tlm_status(const char *s);
void set_tab_sensors_lb_min_cell_voltage(const char *s);
void set_tab_sensors_lb_left_motor_temp(const char *s);
void set_tab_sensors_lb_left_inverter_temp(const char *s);
void set_tab_sensors_lb_right_motor_temp(const char *s);
void set_tab_sensors_lb_right_inverter_temp(const char *s);

void set_tab_track_test_lb_inverter_speed_x(const char *s);
void set_tab_track_test_lb_inverter_speed_y(const char *s);
void set_tab_track_test_steering_angle_bar(float v);
void set_tab_track_test_lb_speed(const char *s);

lv_obj_t *get_tab_racing_bottom_lb_speed();
lv_obj_t *get_tab_racing_custom_meter();
lv_obj_t *get_tab_racing_racing_lv_bar();
lv_meter_indicator_t *get_tab_racing_indicator_blue();
lv_obj_t *get_tab_racing_racing_hv_bar();

calibration_box_t *get_tab_calibration_curr_focus();
lv_obj_t *get_tab_calibration_slider();

lv_obj_t *get_tab_sensors_extra_value0();
lv_obj_t *get_tab_sensors_extra_value1();
lv_obj_t *get_tab_sensors_extra_value2();

#endif /* STEERING_H */
