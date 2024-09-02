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
#include <stdlib.h>

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
#define COLOR_RESOLUTION (4U)

/***
 * Activate the watchdog on CAN bus values: if a value is not update, the watchdog triggers an action
 */
#define WATCHDOG_ENABLED (0U)

/***
 * If WATCHDOG_ENABLED not-updated values are encouraged to use NOT_AVAILABLE_STRING_LABEL
 */
#define NOT_AVAILABLE_STRING_LABEL "NA"

/***
 * Until now MCP23017 is not working with interrupts enables
 */
#define MCP23017_IT_ENABLED (0U)

#define CAN_LOG_ENABLED         (0U)
#define CAN_OVER_SERIAL_ENABLED (0U)

/***
 * LOW LEVEL GRAPHICS SETTING, if you change this be sure to change also STM32 cubeMX settings accordingly
 */
#define SDRAM_BASE_ADDRESS (0xC0000000)
#define FRAMEBUFFER1_ADDR  SDRAM_BASE_ADDRESS
#define FRAMEBUFFER2_ADDR  (0xC0200000)
#define SCREEN_WIDTH       (800u)
#define SCREEN_HEIGHT      (480u)

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

#define BUTTONS_LONG_PRESS_TIME      500

#define clamp(value, min, max) ((value) < (min) ? (min) : ((value) > (max) ? (max) : (value)))

#define GET_LAST_STATE(ntw, msg, NTW, MSG) \
    ntw##_##msg##_converted_t *ntw##_##msg##_last_state = (ntw##_##msg##_converted_t *)&ntw##_messages_last_state[NTW##_##MSG##_INDEX][0]

#define STEER_CAN_UNPACK(ntw, NTW, msg_name, MSG_NAME, arr, update_cond)                                                                        \
    ntw##_##msg_name##_t raw;                                                                                                                   \
    ntw##_##msg_name##_converted_t converted;                                                                                                   \
    ntw##_##msg_name##_unpack(&raw, msg->data, NTW##_##MSG_NAME##_BYTE_SIZE);                                                                   \
    ntw##_##msg_name##_raw_to_conversion_struct(&converted, &raw);                                                                              \
    ntw##_##msg_name##_converted_t *last_state = (ntw##_##msg_name##_converted_t *)&ntw##_messages_last_state[ntw##_index_from_id(msg->id)][0]; \
    if (update_cond) {                                                                                                                          \
        memcpy(last_state, &converted, sizeof(ntw##_##msg_name##_converted_t));                                                                 \
        arr##_new[ntw##_index_from_id(NTW##_##MSG_NAME##_FRAME_ID)]   = true;                                                                   \
        arr##_valid[ntw##_index_from_id(NTW##_##MSG_NAME##_FRAME_ID)] = true;                                                                   \
    }

#define STEER_CAN_PACK(ntw, NTW, msg_name, MSG_NAME)               \
    can_message_t msg        = {0};                                \
    msg.id                   = NTW##_##MSG_NAME##_FRAME_ID;        \
    msg.size                 = NTW##_##MSG_NAME##_BYTE_SIZE;       \
    ntw##_##msg_name##_t raw = {0};                                \
    ntw##_##msg_name##_conversion_to_raw_struct(&raw, &converted); \
    ntw##_##msg_name##_pack(msg.data, &raw, NTW##_##MSG_NAME##_BYTE_SIZE);

#define PERIODIC_SEND(ntw, NTW, msg_name, MSG_NAME)                                           \
    static uint32_t last_sent_##ntw##_##msg_name = 0;                                         \
    void send_##ntw##_##msg_name();                                                           \
    if ((get_current_time_ms() - last_sent_##ntw##_##msg_name) > NTW##_INTERVAL_##MSG_NAME) { \
        last_sent_##ntw##_##msg_name             = get_current_time_ms();                     \
        ntw##_##msg_name##_converted_t converted = *ntw##_##msg_name##_last_state;            \
        STEER_CAN_PACK(ntw, NTW, msg_name, MSG_NAME)                                          \
        can_send(&msg, true);                                                                 \
    }

typedef uint16_t can_id_t;

typedef struct {
    can_id_t id;
    uint8_t size;
    uint8_t data[8];
} can_message_t;

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
 * Cooling timeout
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

typedef struct {
    int x;
    int y;
    int w;
    int h;
} olivec_boundaries_t;

typedef struct {
    uint32_t *pixels;
    size_t width;
    size_t height;
    size_t stride;
} Olivec_Canvas;

typedef struct {
    size_t width, height;
    const uint8_t *glyphs;
} Olivec_Font;

#define SWOC_STRING_LEN (32U)

typedef struct {
    Olivec_Canvas oc;
    bool swoc_elem_was_updated[swoc_elems_n];
    char swoc_elem_label[swoc_elems_n][SWOC_STRING_LEN];
    uint32_t swoc_elem_lb_color[swoc_elems_n];
    uint32_t swoc_elem_bg_color[swoc_elems_n];
    const Olivec_Font *swoc_elem_font[swoc_elems_n];
    const size_t swoc_elem_font_size[swoc_elems_n];
    const olivec_boundaries_t swoc_elem_boundaries[swoc_elems_n];
} steering_wheel_endurance_screen_t;

#endif  // STEERING_CONFIG_H
