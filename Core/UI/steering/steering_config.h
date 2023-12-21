/**
 * @file steering_config.h
 * @author Giacomo Mazzucchi (giacomo.mazzucchi@protonmail.com)
 * @brief General configuration file for steering wheel
 * @version 0.1
 * @date 2023-12-12
 *
 * @copyright Copyright (c) 2023
 *
 */
#ifndef STEERING_CONFIG_H
#define STEERING_CONFIG_H

#include <stdbool.h>
#include <stdint.h>

/****
 * If this is enabled, when changing tabs clears memory of past messages.
 * Should not be a problem with watchdog enabled
 */
#define STRICT_RELOAD_ALL_ENABLED 0
/***
 * Activate candump in the engineering tabs
 * To date, it is very memory inefficient, so use only if strictly necessary
*/
#define CANSNIFFER_ENABLED 1
#define CAN_LOG_ENABLED 0
#define WATCHDOG_ENABLED 0
#define ENGINEERING_TAB_ENABLED 1
#define CAN_OVER_SERIAL_ENABLED 0

#define STEER_TAB_CALIBRATION_ENABLED 0
#define STEER_TAB_SENSORS_ENABLED 1
#define STEER_TAB_TRACK_TEST_ENABLED 1
#define STEER_TAB_DEBUG_ENABLED 0

#define SDRAM_BASE_ADDRESS 0xC0000000
#define FRAMEBUFFER1_ADDR SDRAM_BASE_ADDRESS
#define FRAMEBUFFER2_ADDR 0xC0200000

#if CANSNIFFER_ENABLED == 1
#define PRIMARY_CANSNIFFER_MEMORY_POOL_ADDRESS 0xC0600000
#define SECONDARY_CANSNIFFER_MEMORY_POOL_ADDRESS \
  ((0xC0600000) + ((CANSNIFFER_ELEM_T_SIZE) * CAN_POSSIBLE_IDS))
#endif

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 480

typedef enum
{
  NOT_SCREEN = -1,
  TAB_RACING,
#if STEER_TAB_CALIBRATION_ENABLED == 1
  TAB_CALIBRATION,
#endif
#if STEER_TAB_DEBUG_ENABLED == 1
  TAB_DEBUG,
#endif
  TAB_TRACK_TEST,
  TAB_SENSORS,
  TAB_HV,
  TAB_LV,
  // add here new tabs
  NUM_RACING_TABS
} racing_tab_t;

typedef enum
{
  TAB_ENGINEER_MODE,
  TAB_COOLING,
#if CANSNIFFER_ENABLED == 1
  TAB_SECONDARY_CANSNIFFER,
  TAB_PRIMARY_CANSNIFFER,
#endif
  TAB_TERMINAL,
  TAB_SHUTDOWN_CIRCUIT,
  TAB_BALANCING_STATUS,
  // add here new tabs
  NUM_ENGINEER_TABS
} engineer_tab_t;

extern racing_tab_t current_racing_tab;
#if ENGINEERING_TAB_ENABLED == 1
extern engineer_tab_t current_engineer_tab;
#endif
extern bool engineer_mode;
extern bool racing_mode;

typedef enum
{
  tab_rac_pack_voltage_idx,
  tab_rac_hv_curr_idx,
  tab_rac_best_time_idx,
  tab_rac_last_time_idx,
  tab_rac_dtime_idx,
  tab_rac_torque_idx,
  tab_rac_slip_idx,
  tab_rac_inv_idx,
  tab_rac_mot_idx,
  tab_rac_lv_temp_idx,
  tab_rac_hv_avg_temp_idx,
  tab_rac_pow_idx,
  tab_rac_status_idx,
  tab_rac_bottom_status_idx,
  tab_rac_lap_count_idx,
  tab_rac_labels_n
} tab_racing_labels_enum;

typedef enum
{
  tab_sensors_lb_fl_temp,
  tab_sensors_lb_fr_temp,
  tab_sensors_lb_rl_temp,
  tab_sensors_lb_rr_temp,
  tab_sensors_lb_fl_press,
  tab_sensors_lb_fr_press,
  tab_sensors_lb_rl_press,
  tab_sensors_lb_rr_press,
  tab_sensors_lb_right_inverter_temp,
  tab_sensors_lb_right_motor_temp,
  tab_sensors_lb_average_temperature,
  tab_sensors_lb_left_inverter_temp,
  tab_sensors_lb_left_motor_temp,
  tab_sensors_lb_pack_voltage,
  tab_sensors_lb_hv_current,
  tab_sensors_lb_min_cell_voltage,
  tab_sensors_lb_battery_temperature,
  tab_sensors_lb_voltage,
  tab_sensors_lb_lv_current,
  tab_sensors_lb_hv_delta,
  tab_sensors_extra_value0,
  tab_sensors_extra_value1,
  tab_sensors_extra_value2,
  tab_sensors_lb_tlm_status,
  tab_sensors_labels_n
} tab_sensors_labels_enum;

typedef enum
{
  tab_hv_lb_temp_max,
  tab_hv_lb_temp_min,
  tab_hv_lb_temp_avg,
  tab_hv_lb_voltage_max,
  tab_hv_lb_voltage_min,
  tab_hv_lb_voltage_delta,
  tab_hv_lb_pack_voltage,
  tab_hv_lb_pack_voltage_2,
  tab_hv_lb_bus_voltage,
  tab_hv_lb_current_state,
  tab_hv_lb_last_error,
  tab_hv_pork_speed_value,
  shutdown_status,
  tab_hv_labels_n
} tab_hv_labels_enum;

typedef enum
{
  tab_lv_lb_temp_max,
  tab_lv_lb_temp_min,
  tab_lv_lb_temp_avg,
  tab_lv_lb_voltage_max,
  tab_lv_lb_voltage_min,
  tab_lv_lb_voltage_delta,
  tab_lv_lb_pack_voltage,
  tab_lv_lb_pack_voltage_2,
  tab_lv_lb_bus_voltage,
  tab_lv_lb_current_state,
  tab_lv_lb_last_error,
  tab_lv_lb_pumps_actual,
  tab_lv_lb_pumps_local,
  tab_lv_lb_radiators_actual,
  tab_lv_lb_radiators_local,
  tab_lv_lb_state,
  tab_lv_labels_n
} tab_lv_labels_enum;

uint32_t get_current_time_ms(void);
void openblt_reset(void);

#endif // STEERING_CONFIG_H
