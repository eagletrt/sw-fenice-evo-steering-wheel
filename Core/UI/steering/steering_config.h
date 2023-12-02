#ifndef STEERING_CONFIG_H
#define STEERING_CONFIG_H

#include <stdbool.h>
#include <stdint.h>

#define STRICT_RELOAD_ALL_ENABLED 0
#define CANSNIFFER_ENABLED 1
#define SCREEN_ENABLED 1
#define CAN_LOG_ENABLED 0

#define STEER_TAB_CALIBRATION_ENABLED 0
#define STEER_TAB_SENSORS_ENABLED 1
#define STEER_TAB_TRACK_TEST_ENABLED 1
#define STEER_TAB_DEBUG_ENABLED 1

#define SDRAM_BASE_ADDRESS 0xC0000000
#define FRAMEBUFFER1_ADDR SDRAM_BASE_ADDRESS
#define FRAMEBUFFER2_ADDR 0xC0200000

#if CANSNIFFER_ENABLED == 1
#define PRIMARY_CANSNIFFER_MEMORY_POOL_ADDRESS 0xC0600000
#define SECONDARY_CANSNIFFER_MEMORY_POOL_ADDRESS                               \
  ((0xC0600000) + ((CANSNIFFER_ELEM_T_SIZE) * CAN_POSSIBLE_IDS))
#endif

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 480

typedef enum {
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
  // add here new tabs
  NUM_RACING_TABS
} racing_tab_t;

typedef enum {
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
extern engineer_tab_t current_engineer_tab;
extern bool engineer_mode;
extern bool racing_mode;

typedef enum {
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

typedef enum {
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

uint32_t get_current_time_ms(void);
void openblt_reset(void);

#endif // STEERING_CONFIG_H
