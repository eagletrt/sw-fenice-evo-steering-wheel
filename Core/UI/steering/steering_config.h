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

/***
 * Color resolution in bytes, if you change this please make sure to also change the setting in STM32 CubeMX
 */
#define COLOR_RESOLUTION (2U)

/***
 * If defined, SET_ECU_POWER_MAPS message is sent periodically according to canlib interval, disable it only for debugging purposes on ECU
 */
#define PERIODIC_SET_ECU_POWER_MAPS

/***
 * Activate the watchdog on CAN bus values: if a value is not update, the watchdog triggers an action
 */
// #define WATCHDOG_ENABLED

#ifdef WATCHDOG_ENABLED
/***
 * If WATCHDOG_ENABLED not-updated values are encoraged to use NOT_AVAILABLE_STRING_LABEL
 */
#define NOT_AVAILABLE_STRING_LABEL "NA"
#endif  // WATCHDOG_ENABLED

/***
 * Until now MCP23017 is not working with interrupts enables
 */
// #define MCP23017_IT_ENABLED

/***
 * Activate endurance mode
 */
#define ENDURANCE_MODE_ENABLED

#ifndef ENDURANCE_MODE_ENABLED
/***
 * Activate the precharge bar when HV accumulator is in precharge or in discharge
 * TODO: make the bar nicer
 */
// #define PRECHARGE_BAR_ENABLED

/***
 * Activate candump in the engineering tabs
 */
#define CANSNIFFER_ENABLED

/***
 * Not very useful, now the tab hv gives feedback on shutdown circuit, which is enough
 * TODO: remove it
 */
// #define STEER_TAB_DEBUG_ENABLED

#endif  // ENDURANCE_MODE_ENABLED

/***
 * Legacy flags
 */
#define CAN_OVER_SERIAL_ENABLED (0U)

/***
 * LOW LEVEL GRAPHICS SETTING, if you change this be sure to change also STM32 cubeMX settings accordingly
 */
#define SDRAM_BASE_ADDRESS (0xC0000000)
#define FRAMEBUFFER1_ADDR  SDRAM_BASE_ADDRESS
#define FRAMEBUFFER2_ADDR  (0xC0200000)
#define SCREEN_WIDTH       (800u)
#define SCREEN_HEIGHT      (480u)

/***
 * General defines
*/
#define HV_BUS_VOLTAGE_LIMIT    (450U)
#define TLM_NTW_INTERFACE_MAX_N (6U)

/**
 * 0 = INT1 -> buttons <br>
 * 1 = INT2 -> left manettino<br>
 * 2 = INT3 -> center manettino<br>
 * 3 = INT4 -> right manettino<br>
 * 4 = ExtraButton
 */
#define BUTTONS_INTERRUPT_INDEX          0
#define LEFT_MANETTINO_INTERRUPT_INDEX   1
#define CENTER_MANETTINO_INTERRUPT_INDEX 2
#define RIGHT_MANETTINO_INTERRUPT_INDEX  3
#define NUM_INTERRUPT_PINS               4

// #ifndef ENDURANCE_MODE_ENABLED // TODO: in the future
typedef enum {
    NOT_SCREEN = -1,
    STEERING_WHEEL_TAB_RACING,
    STEERING_WHEEL_TAB_TRACK_TEST,
    STEERING_WHEEL_TAB_SENSORS,
    STEERING_WHEEL_TAB_HV,
    STEERING_WHEEL_TAB_LV,
    // add here new tabs
    NUM_RACING_TABS
} racing_tab_t;

typedef enum {
    STEERING_WHEEL_TAB_TERMINAL,
#ifdef CANSNIFFER_ENABLED
    STEERING_WHEEL_TAB_PRIMARY_CANSNIFFER,
    STEERING_WHEEL_TAB_INVERTERS_CANSNIFFER,
    STEERING_WHEEL_TAB_SECONDARY_CANSNIFFER,
#endif
#ifdef STEER_TAB_DEBUG_ENABLED
    TAB_DEBUG,
#endif
    // add here new tabs
    NUM_ENGINEER_TABS
} engineer_tab_t;

extern racing_tab_t current_racing_tab;
extern engineer_tab_t current_engineer_tab;
extern bool engineer_mode;

typedef enum {
    shutdown_circuit_no_element_index = -1,
    shutdown_circuit_sd_start_index,
    shutdown_circuit_feedbacks_status_feedback_sd_in_index,
    shutdown_circuit_feedbacks_status_feedback_sd_out_index,
    shutdown_circuit_feedbacks_interlock_fb_index,
    shutdown_circuit_ecu_feedbacks_sd_in_index,
    shutdown_circuit_ecu_feedbacks_sd_cock_fb_index,
    shutdown_circuit_ecu_feedbacks_sd_inertial_fb_index,
    shutdown_circuit_ecu_feedbacks_sd_bots_fb_index,
    shutdown_circuit_feedbacks_invc_lid_fb_index,
    shutdown_circuit_feedbacks_hvd_fb_index,
    shutdown_circuit_feedbacks_bspd_fb_index,
    shutdown_circuit_feedbacks_invc_interlock_fb_index,
    shutdown_circuit_sd_end_index,
    shutdown_circuit_feedbacks_status_feedback_sd_end_index,
    // shutdown_circuit_feedbacks_status_feedback_precharge_status_index,
    // shutdown_circuit_feedbacks_status_feedback_airp_gate_index,
    // shutdown_circuit_feedbacks_status_feedback_airn_gate_index,
    //
    SHUTDOWN_COMPONENT_SIZE
} shutdown_circuit_indexes_t;

typedef enum { SC_UNKNOWN, SC_OPEN, SC_CLOSE } shutdown_circuit_component_state_t;

typedef enum {
    tab_rac_hv_soc_idx,
    tab_rac_lv_soc_idx,
    tab_rac_curr_time_idx,
    tab_rac_last_time_idx,
    tab_rac_best_time_idx,
    tab_rac_torque_idx,
    tab_rac_slip_idx,
    tab_rac_inv_idx,
    tab_rac_mot_idx,
    tab_rac_lv_temp_idx,
    tab_rac_hv_avg_temp_idx,
    tab_rac_pow_idx,
    tab_rac_status_idx,
    tab_rac_bottom_status_idx,
    tab_rac_ptt_status_idx,
    tab_rac_odometer_idx,
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
    tab_sensors_lb_left_inverter_temp,
    tab_sensors_lb_left_motor_temp,
    tab_sensors_lb_tlm_ntw_interface_0,
    tab_sensors_lb_tlm_ntw_interface_1,
    tab_sensors_lb_tlm_ntw_interface_2,
    tab_sensors_lb_tlm_ntw_interface_3,
    tab_sensors_lb_tlm_ntw_interface_4,
    tab_sensors_lb_tlm_ntw_interface_5,
    tab_sensors_labels_n
} tab_sensors_labels_enum;

typedef enum {
    tab_hv_lb_temp_max,
    tab_hv_lb_temp_min,
    tab_hv_threshold_setting,
    tab_hv_lb_voltage_max,
    tab_hv_lb_voltage_min,
    tab_hv_lb_voltage_delta,
    tab_hv_lb_pack_voltage,
    tab_hv_lb_pack_voltage_2,
    tab_hv_lb_bus_voltage,
    tab_hv_lb_current_state,
    tab_hv_lb_last_error,
    tab_hv_pork_speed_value,
    shutdown_status_lb,
    tab_hv_labels_n
} tab_hv_labels_enum;

typedef enum {
    debug_signal_error_cell_low_voltage,
    debug_signal_error_cell_under_voltage,
    debug_signal_error_cell_over_voltage,
    debug_signal_error_cell_high_temperature,
    debug_signal_error_cell_over_temperature,
    debug_signal_error_over_current,
    debug_signal_error_can,
    debug_signal_error_int_voltage_mismatch,
    debug_signal_error_cellboard_comm,
    debug_signal_error_cellboard_internal,
    debug_signal_error_connector_disconnected,
    debug_signal_error_fans_disconnected,
    debug_signal_error_feedback,
    debug_signal_error_feedback_circuitry,
    debug_signal_error_eeprom_comm,
    debug_signal_error_eeprom_write,
    DEBUG_SIGNAL_ERROR_SIZE
} debug_signal_error_t;

typedef enum {
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
    tab_lv_lb_pumps_name,
    tab_lv_lb_pumps_value,
    tab_lv_lb_radiators_name,
    tab_lv_lb_radiators_value,
    tab_lv_lb_state,
    tab_lv_labels_n
} tab_lv_labels_enum;

/***
 * Cooling
 */
#define COOLING_STATE_SYNC_TIMEOUT (1000U)  // in ms

typedef enum {
    STEERING_WHEEL_COOLING_STATUS_SYNC,
    STEERING_WHEEL_COOLING_STATUS_SET,
} steering_wheel_cooling_status_t;

/***
 * Hardware dependent functions
 */
uint32_t get_current_time_ms(void);
void openblt_reset(void);
void system_reset(void);

#endif  // STEERING_CONFIG_H
