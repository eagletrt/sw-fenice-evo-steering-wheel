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
 * If you change one to the other, remember to change LTDC and DMA2D config in cubeMX
 */
#define STEERING_WHEEL_LVGL_MODE   (0U)
#define STEERING_WHEEL_OLIVEC_MODE (1U)
#define STEERING_WHEEL_MODE        STEERING_WHEEL_OLIVEC_MODE

#define PERIODIC_SET_ECU_POWER_MAPS

/***
 * Activate the precharge bar when HV accumulator is in precharge or in discharge
 * TODO: make the bar nicer
 */
#define PRECHARGE_BAR_ENABLED (0U)

/***
 * Color resolution in bytes, if you change this please make sure to also change the setting in STM32 CubeMX
 */
#if STEERING_WHEEL_MODE == STEERING_WHEEL_LVGL_MODE
#define COLOR_RESOLUTION (2U)
#elif STEERING_WHEEL_MODE == STEERING_WHEEL_OLIVEC_MODE
#define COLOR_RESOLUTION (4U)
#endif

/***
 * Activate candump in the engineering tabs
 */
#define CANSNIFFER_ENABLED ((STEERING_WHEEL_MODE == STEERING_WHEEL_LVGL_MODE) && 1U)

/***
 * Activate the watchdog on CAN bus values: if a value is not update, the watchdog triggers an action
 */
#define WATCHDOG_ENABLED (0U)

/***
 * If WATCHDOG_ENABLED not-updated values are encoraged to use NOT_AVAILABLE_STRING_LABEL
 */
#define NOT_AVAILABLE_STRING_LABEL "NA"

/***
 * Until now MCP23017 is not working with interrupts enables
 */
#define MCP23017_IT_ENABLED (0U)

/***
 * Legacy flags
 */
#define CAN_LOG_ENABLED         (0U)
#define CAN_OVER_SERIAL_ENABLED (0U)

/***
 * Not very useful, now the tab hv gives feedback on shutdown circuit, which is enough
 * TODO: remove it
 */
#define STEER_TAB_DEBUG_ENABLED (0U)

/***
 * Enable endurance mode instead of classic racing tab
 */
#define ENDURANCE_MODE_ENABLED

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

#define clamp(x, a, b) (((x) < (a)) ? (a) : (((x) > (b)) ? (b) : (x)))
#define GET_LAST_STATE(ntw, msg, NTW, MSG) \
    ntw##_##msg##_converted_t *ntw##_##msg##_last_state = (ntw##_##msg##_converted_t *)&ntw##_messages_last_state[NTW##_##MSG##_INDEX][0]

typedef enum {
    NOT_SCREEN = -1,
#ifdef ENDURANCE_MODE_ENABLED
    STEERING_WHEEL_ENDURANCE_SCREEN,
#endif
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
#if CANSNIFFER_ENABLED == 1
    STEERING_WHEEL_TAB_PRIMARY_CANSNIFFER,
    STEERING_WHEEL_TAB_INVERTERS_CANSNIFFER,
    STEERING_WHEEL_TAB_SECONDARY_CANSNIFFER,
#endif
#if STEER_TAB_DEBUG_ENABLED == 1
    TAB_DEBUG,
#endif
    // add here new tabs
    NUM_ENGINEER_TABS
} engineer_tab_t;

extern racing_tab_t current_racing_tab;
extern engineer_tab_t current_engineer_tab;
extern bool engineer_mode;

typedef enum {
    ptt_status_OFF     = 0,
    ptt_status_SET_ON  = 1,
    ptt_status_ON      = 2,
    ptt_status_SET_OFF = 3,
} ptt_status_t;

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

#ifdef ENDURANCE_MODE_ENABLED
typedef enum {
    shutdown_status_idx,
    car_status_idx,
    ptt_status_idx,
    motor_temp_idx,
    hv_soc_idx,
    lv_soc_idx,
    inverter_temp_idx,
    hv_temp_idx,
    pt_cooling_idx,
    slip_map_idx,
    regen_on_off_idx,
    torque_map_idx,
    hv_cooling_idx,
    endurance_screen_n_labels
} endurance_screen_labels_idx_t;
#endif

#define MOTOR_TEMP_ALERT_THRESHOLD    (85.0f)
#define MOTOR_TEMP_CRITICAL_THRESHOLD (100.0f)

#define INVERTER_TEMP_ALERT_THRESHOLD    (65.0f)
#define INVERTER_TEMP_CRITICAL_THRESHOLD (73.0f)

#define SOC_ALERT_THRESHOLD    (0.4f)
#define SOC_CRITICAL_THRESHOLD (0.2f)

#define HV_TEMP_ALERT_THRESHOLD    (46.0f)
#define HV_TEMP_CRITICAL_THRESHOLD (54.0f)

#define OLIVEC_COLOR_RED        (0xFFFF0000)
#define OLIVEC_COLOR_BLUE       (0xFF0000FF)
#define OLIVEC_COLOR_LIGHT_BLUE (0xFFADD8E6)
#define OLIVEC_COLOR_YELLOW     (0xFFFFFF00)
#define OLIVEC_COLOR_GREEN      (0xFF00FF00)
#define OLIVEC_COLOR_BLACK      (0xFF000000)
#define OLIVEC_COLOR_WHITE      (0xFFFFFFFF)

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

typedef enum {
    swoc_sd = 0,
    swoc_lap_time,
    swoc_ptt,
    swoc_temp_mot,
    swoc_temp_mot_name,
    swoc_soc_hv,
    swoc_soc_lv,
    swoc_soc_lv_name,
    swoc_temp_inv,
    swoc_temp_inv_name,
    swoc_temp_hv,
    swoc_temp_hv_name,
    swoc_pt_cooling,
    swoc_pt_cooling_name,
    swoc_regen,
    swoc_slip,
    swoc_torque,
    swoc_hv_cooling,
    swoc_hv_cooling_name,
    swoc_elems_n
} swoc_elems_t;

#define SWOC_STRING_LEN (32U)
extern bool swoc_elem_was_updated[swoc_elems_n];
extern char swoc_elem_label[swoc_elems_n][SWOC_STRING_LEN];
extern uint32_t swoc_elem_lb_color[swoc_elems_n];
extern uint32_t swoc_elem_bg_color[swoc_elems_n];

#endif  // STEERING_CONFIG_H
