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

#define STEER_UPDATE_LABEL(name, value)                                        \
  for (uint32_t itab = 0; itab < NUM_RACING_TABS; itab++) {                    \
    if (name[itab] != NULL) {                                                  \
      remove_trailing(value);                                                  \
      lv_label_set_text_fmt(name[itab], "%s", value);                          \
    }                                                                          \
  }

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
      lv_obj_set_style_border_color(steering.device[aindex],                   \
                                    lv_color_hex(COLOR_RED_STATUS_HEX),        \
                                    LV_PART_MAIN);                             \
      lv_obj_set_style_bg_color(steering.device[aindex],                       \
                                lv_color_hex(COLOR_RED_STATUS_HEX),            \
                                LV_PART_MAIN);                                 \
    } else {                                                                   \
      lv_obj_set_style_border_color(steering.device[aindex],                   \
                                    lv_color_hex(COLOR_GREEN_STATUS_HEX),      \
                                    LV_PART_MAIN);                             \
      lv_obj_set_style_bg_color(steering.device[aindex],                       \
                                lv_color_hex(COLOR_GREEN_STATUS_HEX),          \
                                LV_PART_MAIN);                                 \
    }                                                                          \
  }

#define STEER_ERROR_INVALIDATE(device, error_name, aindex)                     \
  lv_obj_set_style_bg_color(steering.device[aindex],                           \
                            lv_color_hex(COLOR_YELLOW_STATUS_HEX),             \
                            LV_PART_MAIN);

typedef struct {
  lv_obj_t *lb_speed[NUM_RACING_TABS];
  lv_obj_t *bottom_lb_speed;

  lv_obj_t *lb_max_cell_voltage[NUM_RACING_TABS];
  lv_obj_t *lb_min_cell_voltage[NUM_RACING_TABS];
  lv_obj_t *lb_hv_current[NUM_RACING_TABS];
  lv_obj_t *lb_average_temperature[NUM_RACING_TABS];
  lv_obj_t *lb_hv_percent[NUM_RACING_TABS];
  lv_obj_t *lb_pack_voltage[NUM_RACING_TABS];
  lv_obj_t *lb_hv_delta[NUM_RACING_TABS];

  lv_obj_t *lb_right_motor_temp[NUM_RACING_TABS];
  lv_obj_t *lb_right_inverter_temp[NUM_RACING_TABS];
  lv_obj_t *lb_left_motor_temp[NUM_RACING_TABS];
  lv_obj_t *lb_left_inverter_temp[NUM_RACING_TABS];

  lv_obj_t *lb_lv_current[NUM_RACING_TABS];
  lv_obj_t *lb_voltage[NUM_RACING_TABS];
  lv_obj_t *lb_battery_temperature[NUM_RACING_TABS];
  lv_obj_t *lb_lv_percent[NUM_RACING_TABS];
  lv_obj_t *lb_lv_delta[NUM_RACING_TABS];

  lv_obj_t *lb_best_time[NUM_RACING_TABS];
  lv_obj_t *lb_last_time[NUM_RACING_TABS];
  lv_obj_t *lb_delta_time[NUM_RACING_TABS];
  lv_obj_t *lb_estimated_velocity[NUM_RACING_TABS];
  lv_obj_t *lb_steering_angle[NUM_RACING_TABS];
  lv_obj_t *lb_apps[NUM_RACING_TABS];
  lv_obj_t *lb_bse[NUM_RACING_TABS];

  lv_obj_t *lb_tlm_status[NUM_RACING_TABS];
  lv_obj_t *lb_lap_count[NUM_RACING_TABS];

  lv_obj_t *lb_power[NUM_RACING_TABS];
  lv_obj_t *lb_torque[NUM_RACING_TABS];
  lv_obj_t *lb_slip[NUM_RACING_TABS];
  lv_obj_t *lb_inverter_speed_x[NUM_RACING_TABS];
  lv_obj_t *lb_inverter_speed_y[NUM_RACING_TABS];

  lv_obj_t *lb_fl_temp[NUM_RACING_TABS];
  lv_obj_t *lb_fr_temp[NUM_RACING_TABS];
  lv_obj_t *lb_rl_temp[NUM_RACING_TABS];
  lv_obj_t *lb_rr_temp[NUM_RACING_TABS];

  lv_obj_t *lb_fl_press[NUM_RACING_TABS];
  lv_obj_t *lb_fr_press[NUM_RACING_TABS];
  lv_obj_t *lb_rl_press[NUM_RACING_TABS];
  lv_obj_t *lb_rr_press[NUM_RACING_TABS];

  lv_obj_t *lb_radiators_speed[NUM_RACING_TABS];
  lv_obj_t *lb_pumps_speed[NUM_RACING_TABS];

  // TODO replace the size with the canlib real value
  lv_obj_t *hv_feedbacks_status[20];
  lv_obj_t *hv_errors[16];
  lv_obj_t *das_errors[9];
  lv_obj_t *lv_errors[17];

  lv_obj_t *tab_sensors_extra_value0;
  lv_obj_t *tab_sensors_extra_value1;
  lv_obj_t *tab_sensors_extra_value2;

  lv_obj_t *notification_screen_label;

  /* Tab Racing */
  lv_obj_t *custom_meter;

  lv_obj_t *hv_bar;
  lv_obj_t *lv_bar;

  lv_meter_indicator_t *indicator_blue;
  lv_meter_indicator_t *indicator_white;
  lv_obj_t *racing_hv_bar;
  lv_obj_t *racing_lv_bar;

  /* Tab Sensors */

  /* Calibration Tab */
  calibration_box_t curr_focus;
  lv_obj_t *slider;

  uint32_t timestamp;

} steering_tabs_t;

extern steering_tabs_t steering;

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

#endif /* STEERING_H */
