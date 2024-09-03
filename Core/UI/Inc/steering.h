#ifndef STEERING_H
#define STEERING_H

#include "inverters/inverters_network.h"
#include "inverters/inverters_watchdog.h"
#include "primary/primary_network.h"
#include "primary/primary_watchdog.h"
#include "secondary/secondary_network.h"
#include "secondary/secondary_watchdog.h"
#include "steering_config.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define STEERING_ANGLE_RANGE_LOW  (-150)
#define STEERING_ANGLE_RANGE_HIGH (150u)
#define APPS_RANGE_LOW            (0u)
#define APPS_RANGE_HIGH           (130u)
#define BRAKE_RANGE_LOW           (0u)
#define BRAKE_RANGE_HIGH          (180u)
#define N_PORK_CELLBOARD          (6u)

typedef enum { BSE, STEER, APPS, CALBOX_N } calibration_box_t;

extern uint8_t primary_messages_last_state[primary_MESSAGE_COUNT][primary_MAX_STRUCT_SIZE_CONVERSION];
extern uint8_t secondary_messages_last_state[secondary_MESSAGE_COUNT][secondary_MAX_STRUCT_SIZE_CONVERSION];
extern uint8_t inverters_messages_last_state[inverters_MESSAGE_COUNT][inverters_MAX_STRUCT_SIZE_CONVERSION];

extern bool is_pmsg_valid[primary_MESSAGE_COUNT];
extern bool is_smsg_valid[secondary_MESSAGE_COUNT];
extern bool is_imsg_valid[inverters_MESSAGE_COUNT];

extern bool is_pmsg_new[primary_MESSAGE_COUNT];
extern bool is_smsg_new[secondary_MESSAGE_COUNT];
extern bool is_imsg_new[inverters_MESSAGE_COUNT];

extern bool ptt_button_pressed;
extern ptt_status_t current_ptt_status;

#if WATCHDOG_ENABLED == 1
extern primary_watchdog m_primary_watchdog;
extern secondary_watchdog m_secondary_watchdog;
extern inverters_watchdog m_inverters_watchdog;
#endif

void set_ptt_button_pressed(bool);
void ptt_periodic_check(UI_t *screen);

/****
 * Update graphics from can messages
 */

void ecu_status_update(UI_t *screen, bool valid);
void ecu_power_maps_update(UI_t *screen, bool valid);
void angular_velocity_update(UI_t *screen, bool valid);
void vehicle_speed_update(UI_t *screen, bool valid);
void ecu_feedbacks_update(UI_t *screen, bool valid);
void ecu_errors_update(UI_t *screen, bool valid);

void hv_debug_signals_update(UI_t *screen, bool valid);
void hv_cells_voltage_stats_update(UI_t *screen, bool valid);
void hv_total_voltage_update(UI_t *screen, bool valid);
void hv_current_update(UI_t *screen, bool valid);
void hv_soc_estimation_update(UI_t *screen, bool valid);
void lv_soc_estimation_update(UI_t *screen, bool valid);
void hv_cells_temp_stats_update(UI_t *screen, bool valid);
void hv_errors_update(UI_t *screen, bool valid);
void hv_cell_balancing_status_update(UI_t *screen, bool valid);
void hv_feedbacks_status_update(UI_t *screen, bool valid);
void debug_hv_feedbacks_status_update(UI_t *screen, bool valid);
void das_errors_update(UI_t *screen, bool valid);

void hv_feedback_ts_voltage_update(UI_t *screen, bool valid);
void hv_feedback_misc_voltage_update(UI_t *screen, bool valid);
void hv_feedback_sd_voltage_update(UI_t *screen, bool valid);

void lv_feedback_sd_voltage_update(UI_t *screen, bool valid);
void lv_feedback_ts_voltage_update(UI_t *screen, bool valid);
void lv_feedback_enclosure_voltage_update(UI_t *screen, bool valid);
void lv_errors_update(UI_t *screen, bool valid);
void lv_currents_update(UI_t *screen, bool valid);
void lv_total_voltage_update(UI_t *screen, bool valid);
void lv_cells_voltage_update(UI_t *screen, bool valid);
void lv_cells_voltage_stats_update(UI_t *screen, bool valid);
void lv_cells_temp_update(UI_t *screen, bool valid);
void lv_cells_temp_stats_update(UI_t *screen, bool valid);
void lv_cooling_aggressiveness_update(UI_t *screen, bool valid);

void hv_status_update(UI_t *screen, bool valid);
void hv_fans_override_status_update(UI_t *screen, bool valid);
void tlm_status_update(UI_t *screen, bool valid);

void odometer_update(UI_t *screen, bool valid);
void steer_angle_update(UI_t *screen, bool valid);
void tlm_network_interface_update(UI_t *screen, bool valid);
void imu_acceleration_update(UI_t *screen, bool valid);
void tlm_lap_time_update(UI_t *screen, bool valid);
void tlm_laps_stats_update(UI_t *screen, bool valid);
void inv_l_rcv_update(UI_t *screen, bool valid);
void inv_r_rcv_update(UI_t *screen, bool valid);

void irts_fl_update(UI_t *screen, bool valid);
void irts_fr_update(UI_t *screen, bool valid);
void irts_rl_update(UI_t *screen, bool valid);
void irts_rr_update(UI_t *screen, bool valid);
void pedal_throttle_update(UI_t *screen, bool valid);
void pedal_brakes_pressure_update(UI_t *screen, bool valid);

void ecu_version_update(UI_t *screen, bool valid);
void lv_version_update(UI_t *screen, bool valid);
void hv_cellboard_version_update(UI_t *screen, bool valid);
void hv_mainboard_version_update(UI_t *screen, bool valid);
void tlm_version_update(UI_t *screen, bool valid);
void control_status_update(UI_t *screen, bool valid);
void lv_radiator_speed_update(UI_t *screen, bool valid);
void lv_pumps_speed_update(UI_t *screen, bool valid);

#endif  // STEERING_H
