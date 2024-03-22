#ifndef SHUTDOWN_H
#define SHUTDOWN_H

#include <stdbool.h>
#include "steering_config.h"

#define SHUTDOWN_CIRCUIT_ROWS 8

typedef enum { SC_UNKNOWN, SC_LOW, SC_HIGH } shutdown_component_state_t;

typedef enum {
  sd_start_index,
  feedbacks_status_feedback_sd_in_index,
  feedbacks_status_feedback_sd_out_index,
  feedbacks_interlock_fb_index,
  ecu_feedbacks_sd_in_index,
  ecu_feedbacks_sd_cock_fb_index,
  ecu_feedbacks_sd_interial_fb_index,
  ecu_feedbacks_sd_bots_fb_index,
  feedbacks_invc_lid_fb_index,
  feedbacks_hvd_fb_index,
  feedbacks_bspd_fb_index,
  feedbacks_invc_interlock_fb_index,
  sd_end_index,
  feedbacks_status_feedback_sd_end_index,
  feedbacks_status_feedback_precharge_status_index,
  feedbacks_status_feedback_airp_gate_index,
  feedbacks_status_feedback_airn_gate_index,
  //
  SHUTDOWN_COMPONENT_SIZE
} shutdown_circuit_indexes_t;

/*
* true: close
* false: open
*/
extern char shutdown_labels[SHUTDOWN_COMPONENT_SIZE][20];

extern bool shutdown_status_array[SHUTDOWN_COMPONENT_SIZE];

#endif // SHUTDOWN_H
