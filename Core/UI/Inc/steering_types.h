#ifndef STEERING_TYPES_H
#define STEERING_TYPES_H

#include "steering_defs.h"

typedef enum {
    steering_wheel_cooling_status_sync,
    steering_wheel_cooling_status_set,
} steering_wheel_cooling_status_t;

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
    swoc_soc_hv_name,
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
} Olivec_Boundaries;

typedef struct {
    int x;
    int y;
} Olivec_Margins;

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

typedef struct {
    bool swoc_elem_was_updated;
    char swoc_elem_label[SWOC_STRING_LEN];
    uint32_t swoc_elem_lb_color;
    uint32_t swoc_elem_bg_color;
    float swoc_elem_font_size;
    const Olivec_Boundaries swoc_elem_boundaries;
    const Olivec_Margins swoc_elem_margins;
    uint8_t swoc_elem_align;
} UI_elem_t;

typedef struct {
    Olivec_Canvas oc;
    UI_elem_t components[swoc_elems_n];
} UI_t;

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

#endif  // STEERING_TYPES_H
