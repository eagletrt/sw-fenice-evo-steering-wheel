#ifndef STEERING_H
#define STEERING_H

#include "engineer_mode/tab_cooling.h"
#include "inverters/inverters_network.h"
#include "lvgl.h"
#include "primary/primary_network.h"
#include "secondary/secondary_network.h"
#include "shutdown.h"
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

void remove_trailing(char *buf);
void reload_racing_tab(racing_tab_t tab);

#define STEER_UPDATE_COLOR_LABEL(name, color)                                  \
  for (uint32_t itab = 0; itab < NUM_RACING_TABS; itab++) {                    \
    if (name[itab] != NULL)                                                    \
      lv_obj_set_style_text_color(name[itab], lv_color_hex(color),             \
                                  LV_PART_MAIN);                               \
  }

#define STEER_ERROR_UPDATE(device, error_name, aindex)                         \
  if (device##_last_state.error_name != data->error_name) {                    \
    device##_last_state.error_name = data->error_name;                         \
    if (data->error_name) {                                                    \
      lv_obj_set_style_border_color(device[aindex],                   \
                                    lv_color_hex(COLOR_RED_STATUS_HEX),        \
                                    LV_PART_MAIN);                             \
      lv_obj_set_style_bg_color(device[aindex],                       \
                                lv_color_hex(COLOR_RED_STATUS_HEX),            \
                                LV_PART_MAIN);                                 \
    } else {                                                                   \
      lv_obj_set_style_border_color(device[aindex],                   \
                                    lv_color_hex(COLOR_GREEN_STATUS_HEX),      \
                                    LV_PART_MAIN);                             \
      lv_obj_set_style_bg_color(device[aindex],                       \
                                lv_color_hex(COLOR_GREEN_STATUS_HEX),          \
                                LV_PART_MAIN);                                 \
    }                                                                          \
  }

#define STEER_ERROR_INVALIDATE(device, error_name, aindex)                     \
  lv_obj_set_style_bg_color(steering.device[aindex],                           \
                            lv_color_hex(COLOR_YELLOW_STATUS_HEX),             \
                            LV_PART_MAIN);

/*
 * UPDATE
 */

void car_status_update(primary_car_status_converted_t *);
void control_output_update(primary_control_output_converted_t *);
void tlm_status_update(primary_tlm_status_converted_t *);
void ambient_temperature_update(primary_ambient_temperature_converted_t *);
void speed_update(primary_speed_converted_t *);

void hv_voltage_update(primary_hv_voltage_converted_t *);
void hv_current_update(primary_hv_current_converted_t *);
void hv_temp_update(primary_hv_temp_converted_t *);
void hv_errors_update(primary_hv_errors_converted_t *);
void hv_cell_balancing_status_update(
    primary_hv_cell_balancing_status_converted_t *);
void hv_feedbacks_status_update(primary_hv_feedbacks_status_converted_t *);
void lv_feedbacks_update(primary_lv_feedbacks_converted_t *);
void das_errors_update(primary_das_errors_converted_t *);

void lv_currents_update(primary_lv_currents_converted_t *);
void lv_cells_voltage_update(primary_lv_cells_voltage_converted_t *);
void lv_cells_temp_update(primary_lv_cells_temp_converted_t *);
void lv_total_voltage_update(primary_lv_total_voltage_converted_t *);
void lv_errors_update(primary_lv_errors_converted_t *);
void cooling_status_update(primary_cooling_status_converted_t *);
void hv_fans_override_status_update(
    primary_hv_fans_override_status_converted_t *);
void tlm_status_update(primary_tlm_status_converted_t *);
void handle_lap_counter_message(secondary_lap_count_converted_t *);

void steering_angle_update(secondary_steering_angle_converted_t *);
void pedals_output_update(secondary_pedals_output_converted_t *);
void imu_acceleration_update(secondary_imu_acceleration_converted_t *);
void lap_count_update(secondary_lap_count_converted_t *);
void lc_status_update(secondary_lc_status_converted_t *);
void timestamp_update(secondary_timestamp_converted_t *);

void inv_l_rcv_update(inverters_inv_l_rcv_converted_t *);
void inv_r_rcv_update(inverters_inv_r_rcv_converted_t *);

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

uint32_t get_current_time_ms(void);

//refactoring
void set_lb_estimated_velocity(const char* string);
void set_lb_apps(const char* string);
void set_lb_bse(const char* string);

void set_tab_calibration_lb_steering_angle(const char* string);

void set_tab_racing_lb_speed(const char* string);
void set_tab_racing_lb_pack_voltage(const char* string);
void set_tab_racing_lb_hv_current(const char* string);
void set_tab_racing_lb_average_temperature(const char* string);
void set_tab_racing_lb_battery_temperature(const char* string);
void set_tab_racing_lb_last_time(const char* string);
void set_tab_racing_lb_delta_time(const char* string);
void set_tab_racing_lb_best_time(const char* string);
void set_tab_racing_lb_lap_count(const char* string);
void set_tab_racing_lb_left_motor_temp(const char* string);
void set_tab_racing_lb_left_inverter_temp(const char* string);
void set_tab_racing_lb_slip(const char* string);
void set_tab_racing_lb_torque(const char* string);
void set_tab_racing_lb_power(const char* string);

void set_tab_sensors_lb_pack_voltage(const char* string);
void set_tab_sensors_lb_hv_delta(const char* string);
void set_tab_sensors_lb_hv_current(const char* string);
void set_tab_sensors_lb_average_temperature(const char* string);
void set_tab_sensors_lb_lv_current(const char* string);
void set_tab_sensors_lb_battery_temperature(const char* string);
void set_tab_sensors_lb_voltage(const char* string);
void set_tab_sensors_lb_tlm_status(const char* string);
void set_tab_sensors_lb_min_cell_voltage(const char* string);
void set_tab_sensors_lb_left_motor_temp(const char* string);
void set_tab_sensors_lb_left_inverter_temp(const char* string);
void set_tab_sensors_lb_right_motor_temp(const char* string);
void set_tab_sensors_lb_right_inverter_temp(const char* string);

void set_tab_track_test_lb_inverter_speed_x(const char* string);
void set_tab_track_test_lb_inverter_speed_y(const char* string);
void set_tab_track_test_lb_steering_angle(const char* string);
void set_tab_track_test_lb_speed(const char* string);

lv_obj_t* get_tab_racing_bottom_lb_speed();
lv_obj_t* get_tab_racing_custom_meter();
lv_obj_t* get_tab_racing_racing_lv_bar();
lv_meter_indicator_t* get_tab_racing_indicator_blue();
lv_obj_t *get_tab_racing_racing_hv_bar();

calibration_box_t* get_tab_calibration_curr_focus();
lv_obj_t *get_tab_calibration_slider();

lv_obj_t *get_tab_sensors_extra_value0();
lv_obj_t *get_tab_sensors_extra_value1();
lv_obj_t *get_tab_sensors_extra_value2();


#endif /* STEERING_H */
