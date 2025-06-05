#ifndef STEERING_TYPES_H
#define STEERING_TYPES_H

#include "steering_defs.h"

typedef enum {
    steering_wheel_cooling_status_sync,
    steering_wheel_cooling_status_set,
} steering_wheel_cooling_status_t;

typedef enum {
    swoc_sd = 0,
    swoc_status,
    swoc_ptt,
    swoc_temp_mot,
    swoc_soc_hv,
    swoc_soc_lv,
    swoc_temp_inv,
    swoc_temp_hv,
    swoc_pt_cooling,
    swoc_regen,
    swoc_slip,
    swoc_torque,
    swoc_lap_time,
    swoc_elems_n
} swoc_elems_t;

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
