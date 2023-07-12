#include "steering.h"

steering_t steering = {0};
extern bool steering_initialized;

#define SPRINTF_BUFFER_SIZE 64
char sprintf_buffer[SPRINTF_BUFFER_SIZE];

/*
 ____    ____   ___ __     ___  ____   ____
 || \\  ||     //   ||    // \\ || \\ ||   
 ||  )) ||==  ((    ||    ||=|| ||_// ||== 
 ||_//  ||___  \\__ ||__| || || || \\ ||___

*/

primary_car_status_converted_t car_status_last_state = {0};
primary_control_output_converted_t control_output_last_state = {0};

primary_ambient_temperature_converted_t ambient_temperature_last_state = {0};
primary_tlm_status_converted_t tlm_status_last_state = {0};
primary_speed_converted_t speed_last_state = {0};

primary_hv_voltage_converted_t hv_voltage_last_state = {0};
primary_hv_current_converted_t hv_current_last_state = {0};
primary_hv_temp_converted_t hv_temp_last_state = {0};
primary_hv_errors_converted_t hv_errors_last_state = {0};
primary_hv_feedbacks_status_converted_t hv_feedbacks_status_last_state = {0};
primary_hv_cells_voltage_converted_t hv_cells_voltage_last_state = {0};
primary_hv_cells_temp_converted_t hv_cells_temp_last_state = {0};

primary_das_errors_converted_t das_errors_last_state = {0};

primary_lv_currents_converted_t lv_currents_last_state = {0};
primary_lv_cells_voltage_converted_t lv_cells_voltage_last_state_1 = {0};
primary_lv_cells_voltage_converted_t lv_cells_voltage_last_state_2 = {0};
primary_lv_cells_temp_converted_t lv_cells_temp_last_state_1 = {0};
primary_lv_cells_temp_converted_t lv_cells_temp_last_state_2 = {0};
uint16_t lv_cells_temp_mean_last_state = 0;
primary_lv_total_voltage_converted_t lv_total_voltage_last_state = {0};
primary_lv_errors_converted_t lv_errors_last_state = {1};

secondary_steering_angle_converted_t steering_angle_converted = {0};

inverters_inv_r_rcv_converted_t inv_r_last_state = {0};
inverters_inv_l_rcv_converted_t inv_l_last_state = {0};


/*
 __ __ ____  ____    ___  ______  ____
 || || || \\ || \\  // \\ | || | ||
 || || ||_// ||  )) ||=||   ||   ||==
 \\_// ||    ||_//  || ||   ||   ||___

*/


void car_status_update(primary_car_status_converted_t *data) {
  if (data->car_status != car_status_last_state.car_status) {
    car_status_last_state.car_status = data->car_status;
    switch (data->car_status) {
    case primary_car_status_car_status_INIT:
    case primary_car_status_car_status_ENABLE_INV_UPDATES:
    case primary_car_status_car_status_CHECK_INV_SETTINGS:
    case primary_car_status_car_status_IDLE: {
      lv_label_set_text_fmt(steering.das.bottom_lb_speed, "-");
      STEER_UPDATE_LABEL(steering.das.lb_speed, "IDLE");
      break;
    }
    case primary_car_status_car_status_START_TS_PRECHARGE:
    case primary_car_status_car_status_WAIT_TS_PRECHARGE: {
      lv_label_set_text_fmt(steering.das.bottom_lb_speed, "-");
      STEER_UPDATE_LABEL(steering.das.lb_speed, "PRCHG");
      break;
    }
    case primary_car_status_car_status_WAIT_DRIVER: {
      lv_label_set_text_fmt(steering.das.bottom_lb_speed, "-");
      STEER_UPDATE_LABEL(steering.das.lb_speed, "SETUP");
      break;
    }
    case primary_car_status_car_status_DRIVE: {
      lv_label_set_text_fmt(steering.das.bottom_lb_speed, "-");
      STEER_UPDATE_LABEL(steering.das.lb_speed, "DRIVE");
      break;
    }
    case primary_car_status_car_status_DISABLE_INV_DRIVE:
    case primary_car_status_car_status_START_TS_DISCHARGE:
    case primary_car_status_car_status_WAIT_TS_DISCHARGE: {
      lv_label_set_text_fmt(steering.das.bottom_lb_speed, "-");
      STEER_UPDATE_LABEL(steering.das.lb_speed, "TSOFF");
      break;
    }
    case primary_car_status_car_status_FATAL_ERROR: {
      lv_label_set_text_fmt(steering.das.bottom_lb_speed, "-");
      STEER_UPDATE_LABEL(steering.das.lb_speed, "FATAL");
      break;
    }
    default:
      break;
    }
  }

  primary_car_status_inverter_l invl = data->inverter_l;

  if (invl != car_status_last_state.inverter_l) {
  }

  primary_car_status_inverter_r invr = data->inverter_r;

  if (invr != car_status_last_state.inverter_r) {
  }
}

void control_output_update(primary_control_output_converted_t *data) {
  if (data->estimated_velocity !=
      control_output_last_state.estimated_velocity) {
  }
}

void tlm_status_update(primary_tlm_status_converted_t *data) {}

void ambient_temperature_update(primary_ambient_temperature_converted_t *data) {
}

void speed_update(primary_speed_converted_t *data) {
  if (car_status_last_state.car_status != primary_car_status_car_status_DRIVE)
    return;

  if (data->inverter_l != speed_last_state.inverter_l) {
    speed_last_state.inverter_l = data->inverter_l;
  }
  if (data->inverter_r != speed_last_state.inverter_r) {
    speed_last_state.inverter_r = data->inverter_r;
  }
  if (data->encoder_l != speed_last_state.encoder_l ||
      data->encoder_r != speed_last_state.encoder_r) {
    speed_last_state.encoder_l = data->encoder_l;
    speed_last_state.encoder_r = data->encoder_r;
    float mean = ((data->inverter_l + data->inverter_r) / 2) * 3.6;

    sprintf(sprintf_buffer, "%d", (int)mean);
    STEER_UPDATE_LABEL(steering.das.lb_speed, sprintf_buffer);
    lv_label_set_text_fmt(steering.das.bottom_lb_speed, "kmh");
  }
}

void hv_voltage_update(primary_hv_voltage_converted_t *data) {
  if (data->bus_voltage != hv_voltage_last_state.bus_voltage) {
    data->bus_voltage = hv_voltage_last_state.bus_voltage;
  }
  if (data->max_cell_voltage != hv_voltage_last_state.max_cell_voltage) {
    data->max_cell_voltage = hv_voltage_last_state.max_cell_voltage;
  }
  if (data->min_cell_voltage != hv_voltage_last_state.min_cell_voltage) {
    data->min_cell_voltage = hv_voltage_last_state.min_cell_voltage;
    sprintf(sprintf_buffer, "%d", (int)data->min_cell_voltage);
    STEER_UPDATE_LABEL(steering.hv.lb_min_cell_voltage, sprintf_buffer)
  }
  if (data->pack_voltage != hv_voltage_last_state.pack_voltage) {
    data->pack_voltage = hv_voltage_last_state.pack_voltage;
    sprintf(sprintf_buffer, "%d", (int)data->pack_voltage);
    STEER_UPDATE_LABEL(steering.hv.lb_max_cell_voltage, sprintf_buffer)
  }
}

void hv_current_update(primary_hv_current_converted_t *data) {
  if (data->current != hv_current_last_state.current) {
    data->current = hv_current_last_state.current;
    sprintf(sprintf_buffer, "%d", (int)data->current);
    STEER_UPDATE_LABEL(steering.hv.lb_current, sprintf_buffer)
  }
  if (data->soc != hv_current_last_state.soc) {
    data->soc = hv_current_last_state.soc;
    // lv_bar_set_value(steering.hv_bar, (int32_t)data->soc, LV_ANIM_OFF);
  }
}

void hv_temp_update(primary_hv_temp_converted_t *data) {
  if (data->average_temp != hv_temp_last_state.average_temp) {
    hv_temp_last_state.average_temp = data->average_temp;
    sprintf(sprintf_buffer, "%d", (int)data->average_temp);
    STEER_UPDATE_LABEL(steering.hv.lb_average_temperature, sprintf_buffer);
  }
  if (data->max_temp != hv_temp_last_state.max_temp) {
    hv_temp_last_state.max_temp = data->max_temp;
  }
  if (data->min_temp != hv_temp_last_state.min_temp) {
    hv_temp_last_state.min_temp = data->min_temp;
  }
}

void hv_errors_update(primary_hv_errors_converted_t *data) {
  STEER_ERROR_UPDATE(hv_errors, errors_cell_low_voltage, 0)
  STEER_ERROR_UPDATE(hv_errors, errors_cell_under_voltage, 1)
  STEER_ERROR_UPDATE(hv_errors, errors_cell_over_voltage, 2)
  STEER_ERROR_UPDATE(hv_errors, errors_cell_high_temperature, 3)
  STEER_ERROR_UPDATE(hv_errors, errors_cell_over_temperature, 4)
  STEER_ERROR_UPDATE(hv_errors, errors_over_current, 5)
  STEER_ERROR_UPDATE(hv_errors, errors_can, 6)
  STEER_ERROR_UPDATE(hv_errors, errors_int_voltage_mismatch, 7)
  STEER_ERROR_UPDATE(hv_errors, errors_cellboard_comm, 8)
  STEER_ERROR_UPDATE(hv_errors, errors_cellboard_internal, 9)
  STEER_ERROR_UPDATE(hv_errors, errors_connector_disconnected, 10)
  STEER_ERROR_UPDATE(hv_errors, errors_fans_disconnected, 11)
  STEER_ERROR_UPDATE(hv_errors, errors_feedback, 12)
  STEER_ERROR_UPDATE(hv_errors, errors_feedback_circuitry, 13)
  STEER_ERROR_UPDATE(hv_errors, errors_eeprom_comm, 14)
  STEER_ERROR_UPDATE(hv_errors, errors_eeprom_write, 15)
}

void hv_total_voltage_update(primary_hv_voltage_converted_t *data) {
  float old_max_v = hv_voltage_last_state.max_cell_voltage;
  float old_min_v = hv_voltage_last_state.min_cell_voltage;
  float old_bus_v = hv_voltage_last_state.bus_voltage;
  float old_pack_v = hv_voltage_last_state.pack_voltage;
  if (old_max_v != data->max_cell_voltage) {
    hv_voltage_last_state.max_cell_voltage = data->max_cell_voltage;
    sprintf(sprintf_buffer, "%d", (int)data->max_cell_voltage);
    STEER_UPDATE_LABEL(steering.hv.lb_max_cell_voltage, sprintf_buffer);
  }
  if (old_min_v != data->min_cell_voltage) {
    hv_voltage_last_state.min_cell_voltage = data->min_cell_voltage;
    sprintf(sprintf_buffer, "%d", (int)data->min_cell_voltage);
    STEER_UPDATE_LABEL(steering.hv.lb_min_cell_voltage, sprintf_buffer);
  }
  if (old_bus_v != data->bus_voltage) {
    hv_voltage_last_state.bus_voltage = data->bus_voltage;
  }
  if (old_pack_v != data->pack_voltage) {
    hv_voltage_last_state.pack_voltage = data->pack_voltage;
    sprintf(sprintf_buffer, "%d", (int)data->pack_voltage);
    STEER_UPDATE_LABEL(steering.hv.lb_pack_voltage, sprintf_buffer);
  }
}

void hv_feedbacks_status_update(primary_hv_feedbacks_status_converted_t *data) {
  STEER_ERROR_UPDATE(hv_feedbacks_status, 
        feedbacks_status_feedback_implausibility_detected, 0)
  STEER_ERROR_UPDATE(hv_feedbacks_status, 
        feedbacks_status_feedback_imd_cockpit, 1)
  STEER_ERROR_UPDATE(hv_feedbacks_status, 
        feedbacks_status_feedback_tsal_green_fault_latched, 2)
  STEER_ERROR_UPDATE(hv_feedbacks_status, 
        feedbacks_status_feedback_bms_cockpit, 3)
  STEER_ERROR_UPDATE(hv_feedbacks_status, 
        feedbacks_status_feedback_ext_latched, 4)
  STEER_ERROR_UPDATE(hv_feedbacks_status, 
        feedbacks_status_feedback_tsal_green, 5)
  STEER_ERROR_UPDATE(hv_feedbacks_status, 
        feedbacks_status_feedback_ts_over_60v_status, 6)
  STEER_ERROR_UPDATE(hv_feedbacks_status, 
        feedbacks_status_feedback_airn_status, 7)
  STEER_ERROR_UPDATE(hv_feedbacks_status, 
        feedbacks_status_feedback_airp_status, 8)
  STEER_ERROR_UPDATE(hv_feedbacks_status, 
        feedbacks_status_feedback_airp_gate, 9)
  STEER_ERROR_UPDATE(hv_feedbacks_status, 
        feedbacks_status_feedback_airn_gate, 10)
  STEER_ERROR_UPDATE(hv_feedbacks_status, 
        feedbacks_status_feedback_precharge_status, 11)
  STEER_ERROR_UPDATE(hv_feedbacks_status, 
        feedbacks_status_feedback_tsp_over_60v_status, 12)
  STEER_ERROR_UPDATE(hv_feedbacks_status, 
        feedbacks_status_feedback_imd_fault, 13)
  STEER_ERROR_UPDATE(hv_feedbacks_status, 
        feedbacks_status_feedback_check_mux, 14)
  STEER_ERROR_UPDATE(hv_feedbacks_status, 
        feedbacks_status_feedback_sd_end, 15)
  STEER_ERROR_UPDATE(hv_feedbacks_status, 
        feedbacks_status_feedback_sd_out, 16)
  STEER_ERROR_UPDATE(hv_feedbacks_status, 
        feedbacks_status_feedback_sd_in, 17)
  STEER_ERROR_UPDATE(hv_feedbacks_status, 
        feedbacks_status_feedback_sd_bms, 18)
  STEER_ERROR_UPDATE(hv_feedbacks_status, 
        feedbacks_status_feedback_sd_imd, 19)
}

void hv_cells_voltage_update(primary_hv_cells_voltage_converted_t *data) {}

void hv_cells_temp_update(primary_hv_cells_temp_converted_t *data) {}

void das_errors_update(primary_das_errors_converted_t *data) {
  STEER_ERROR_UPDATE(das_errors, das_error_pedal_adc, 0)
  STEER_ERROR_UPDATE(das_errors, das_error_pedal_implausibility, 1)
  STEER_ERROR_UPDATE(das_errors, das_error_imu_tout, 2)
  STEER_ERROR_UPDATE(das_errors, das_error_irts_tout, 3)
  STEER_ERROR_UPDATE(das_errors, das_error_ts_tout, 4)
  STEER_ERROR_UPDATE(das_errors, das_error_invl_tout, 5)
  STEER_ERROR_UPDATE(das_errors, das_error_invr_tout, 6)
  STEER_ERROR_UPDATE(das_errors, das_error_steer_tout, 7)
  STEER_ERROR_UPDATE(das_errors, das_error_fsm, 8)
}

void lv_currents_update(primary_lv_currents_converted_t *data) {
  float old_current_lv_battery = lv_currents_last_state.current_lv_battery;
  if (old_current_lv_battery != data->current_lv_battery) {
    lv_currents_last_state.current_lv_battery = data->current_lv_battery;
    sprintf(sprintf_buffer, "%d", (int)data->current_lv_battery);
    STEER_UPDATE_LABEL(steering.lv.lb_current, sprintf_buffer);
  }  
}

void lv_control_update(primary_control_output_converted_t *data) {}

void lv_cells_voltage_update(primary_lv_cells_voltage_converted_t *data) {
#if 0
  float old_cells_voltage_0;
  float old_cells_voltage_1;
  float old_cells_voltage_2;
  if (data->start_index == 0) {
    old_cells_voltage_0 = lv_cells_voltage_last_state_1.voltage_0;
    old_cells_voltage_1 = lv_cells_voltage_last_state_1.voltage_1;
    old_cells_voltage_2 = lv_cells_voltage_last_state_1.voltage_2;
    lv_cells_voltage_last_state_1.voltage_0 = data->voltage_0;
    lv_cells_voltage_last_state_1.voltage_1 = data->voltage_1;
    lv_cells_voltage_last_state_1.voltage_2 = data->voltage_2;
  } else {
    old_cells_voltage_0 = lv_cells_voltage_last_state_2.voltage_0;
    old_cells_voltage_1 = lv_cells_voltage_last_state_2.voltage_1;
    old_cells_voltage_2 = lv_cells_voltage_last_state_2.voltage_2;
    lv_cells_voltage_last_state_2.voltage_0 = data->voltage_0;
    lv_cells_voltage_last_state_2.voltage_1 = data->voltage_1;
    lv_cells_voltage_last_state_2.voltage_2 = data->voltage_2;
  }

  if (old_cells_voltage_0 == data->voltage_0 &&
      old_cells_voltage_1 == data->voltage_1 &&
      old_cells_voltage_2 == data->voltage_2)
    return;
  
  float mean = (lv_cells_voltage_last_state_1.voltage_0 +
                lv_cells_voltage_last_state_1.voltage_1 +
                lv_cells_voltage_last_state_1.voltage_2 +
                lv_cells_voltage_last_state_2.voltage_0 +
                lv_cells_voltage_last_state_2.voltage_1 +
                lv_cells_voltage_last_state_2.voltage_2) /
               6.0f;
  snprintf(sprintf_buffer, sizeof(sprintf_buffer), "%d", (int)mean);
  STEER_UPDATE_LABEL(steering.lv.lb_voltage, sprintf_buffer);
#endif
}

void lv_cells_temp_update(primary_lv_cells_temp_converted_t *data) {
  uint8_t start_index = data->start_index;
  float old_lv_temp_0;
  float old_lv_temp_1;
  float old_lv_temp_2;
  float old_lv_temp_3;
  float old_lv_temp_4;
  float old_lv_temp_5;
  if (start_index == 1) {
    old_lv_temp_0 = lv_cells_temp_last_state_1.temp_0;
    old_lv_temp_1 = lv_cells_temp_last_state_1.temp_1;
    old_lv_temp_2 = lv_cells_temp_last_state_1.temp_2;
    old_lv_temp_3 = data->temp_0;
    old_lv_temp_4 = data->temp_1;
    old_lv_temp_5 = data->temp_2;
  } else {
    old_lv_temp_0 = data->temp_0;
    old_lv_temp_1 = data->temp_1;
    old_lv_temp_2 = data->temp_2;
    old_lv_temp_3 = lv_cells_temp_last_state_2.temp_0;
    old_lv_temp_4 = lv_cells_temp_last_state_2.temp_1;
    old_lv_temp_5 = lv_cells_temp_last_state_2.temp_2;
  }

  uint16_t mean_temp = (old_lv_temp_0 + old_lv_temp_1 + old_lv_temp_2 +
                        old_lv_temp_3 + old_lv_temp_4 + old_lv_temp_5) /
                       6;
  if (mean_temp != lv_cells_temp_mean_last_state) {
    lv_cells_temp_mean_last_state = mean_temp;
    sprintf(sprintf_buffer, "%d", (int)mean_temp);
    STEER_UPDATE_LABEL(steering.lv.lb_battery_temperature, sprintf_buffer);
  }
}

void lv_total_voltage_update(primary_lv_total_voltage_converted_t *data) {
  float old_total_v = lv_total_voltage_last_state.total_voltage;
  if (old_total_v != data->total_voltage) {
    lv_total_voltage_last_state.total_voltage = data->total_voltage;
    sprintf(sprintf_buffer, "%d", (int)data->total_voltage);
    // lv_bar_set_value(steering.lv_bar, (int32_t)data->total_voltage, LV_ANIM_OFF);
    STEER_UPDATE_LABEL(steering.lv.lb_voltage, sprintf_buffer);
  }
}

void lv_errors_update(primary_lv_errors_converted_t *data) {
  STEER_ERROR_UPDATE(lv_errors, errors_cell_undervoltage, 0)
  STEER_ERROR_UPDATE(lv_errors, errors_cell_overvoltage, 1)
  STEER_ERROR_UPDATE(lv_errors, errors_battery_open_wire, 2)
  STEER_ERROR_UPDATE(lv_errors, errors_can, 3)
  STEER_ERROR_UPDATE(lv_errors, errors_spi, 4)
  STEER_ERROR_UPDATE(lv_errors, errors_over_current, 5)
  STEER_ERROR_UPDATE(lv_errors, errors_cell_under_temperature, 6)
  STEER_ERROR_UPDATE(lv_errors, errors_cell_over_temperature, 7)
  STEER_ERROR_UPDATE(lv_errors, errors_relay, 8)
  STEER_ERROR_UPDATE(lv_errors, errors_bms_monitor, 9)
  STEER_ERROR_UPDATE(lv_errors, errors_voltages_not_ready, 10)
  STEER_ERROR_UPDATE(lv_errors, errors_mcp23017, 11)
  STEER_ERROR_UPDATE(lv_errors, errors_radiator, 12)
  STEER_ERROR_UPDATE(lv_errors, errors_fan, 13)
  STEER_ERROR_UPDATE(lv_errors, errors_pump, 14)
  STEER_ERROR_UPDATE(lv_errors, errors_adc_init, 15)
  STEER_ERROR_UPDATE(lv_errors, errors_mux, 16)
}

void steering_angle_update(secondary_steering_angle_converted_t *data) {
  if (data->angle != steering_angle_converted.angle) {
    steering_angle_converted.angle = data->angle;
    sprintf(sprintf_buffer, "%d", (int)data->angle);
    STEER_UPDATE_LABEL(steering.steering.lb_steering_angle, sprintf_buffer);
  }
}

void inv_l_rcv_update(inverters_inv_l_rcv_converted_t *data) {
  if (data->t_motor != inv_l_last_state.t_motor) {
    int new_value = (int)((float)data->t_motor - 9393.9f) / 55.1f;
    inv_l_last_state.t_motor = new_value;
    sprintf(sprintf_buffer, "%d", new_value);
    STEER_UPDATE_LABEL(steering.inverters.lb_left_motor_temp, sprintf_buffer);
  }
  if (data->t_igbt != inv_l_last_state.t_igbt) {
    int new_value = (int) ( (float) data->t_igbt * 0.005f - 38.0f);
    inv_l_last_state.t_igbt = new_value;
    sprintf(sprintf_buffer, "%d", new_value);
    STEER_UPDATE_LABEL(steering.inverters.lb_left_inverter_temp, sprintf_buffer);
  } 
}

void inv_r_rcv_update(inverters_inv_r_rcv_converted_t *data) {
  if (data->t_motor != inv_r_last_state.t_motor) {
    int new_value = (int)((float)data->t_motor - 9393.9f) / 55.1f;
    inv_r_last_state.t_motor = new_value;
    sprintf(sprintf_buffer, "%d", new_value);
    STEER_UPDATE_LABEL(steering.inverters.lb_right_motor_temp, sprintf_buffer);
  }
  if (data->t_igbt != inv_r_last_state.t_igbt) {
    int new_value = (int) ( (float) data->t_igbt * 0.005f - 38.0f);
    inv_r_last_state.t_igbt = new_value;
    sprintf(sprintf_buffer, "%d", new_value);
    STEER_UPDATE_LABEL(steering.inverters.lb_right_inverter_temp, sprintf_buffer);
  } 
}


/*
 __ __  __ __ __  ___  __    __ ____    ___  ______  ____
 || ||\ || || || // \\ ||    || || \\  // \\ | || | ||
 || ||\\|| \\ // ||=|| ||    || ||  )) ||=||   ||   ||==
 || || \||  \V/  || || ||__| || ||_//  || ||   ||   ||___

*/


void car_status_invalidate() {
  lv_label_set_text_fmt(steering.das.bottom_lb_speed, "-");
  STEER_UPDATE_LABEL(steering.das.lb_speed, "N/A");
}

void control_output_invalidate() {
}

void tlm_status_invalidate() {

}

void ambient_temperature_invalidate() {

}

void speed_invalidate() {
  if (car_status_last_state.car_status != primary_car_status_car_status_DRIVE) {
    lv_label_set_text_fmt(steering.das.bottom_lb_speed, "-");
    STEER_UPDATE_LABEL(steering.das.lb_speed, "N/A");
  }
}

void hv_voltage_invalidate() {
  STEER_UPDATE_LABEL(steering.hv.lb_min_cell_voltage, "N/A")
  STEER_UPDATE_LABEL(steering.hv.lb_max_cell_voltage, "N/A")
}

void hv_current_invalidate() {
  STEER_UPDATE_LABEL(steering.hv.lb_current, "N/A")
}

void hv_temp_invalidate() {
  STEER_UPDATE_LABEL(steering.hv.lb_average_temperature, "N/A");
}

void hv_errors_invalidate() {
  STEER_ERROR_INVALIDATE(hv_errors, errors_cell_low_voltage, 0)
  STEER_ERROR_INVALIDATE(hv_errors, errors_cell_under_voltage, 1)
  STEER_ERROR_INVALIDATE(hv_errors, errors_cell_over_voltage, 2)
  STEER_ERROR_INVALIDATE(hv_errors, errors_cell_high_temperature, 3)
  STEER_ERROR_INVALIDATE(hv_errors, errors_cell_over_temperature, 4)
  STEER_ERROR_INVALIDATE(hv_errors, errors_over_current, 5)
  STEER_ERROR_INVALIDATE(hv_errors, errors_can, 6)
  STEER_ERROR_INVALIDATE(hv_errors, errors_int_voltage_mismatch, 7)
  STEER_ERROR_INVALIDATE(hv_errors, errors_cellboard_comm, 8)
  STEER_ERROR_INVALIDATE(hv_errors, errors_cellboard_internal, 9)
  STEER_ERROR_INVALIDATE(hv_errors, errors_connector_disconnected, 10)
  STEER_ERROR_INVALIDATE(hv_errors, errors_fans_disconnected, 11)
  STEER_ERROR_INVALIDATE(hv_errors, errors_feedback, 12)
  STEER_ERROR_INVALIDATE(hv_errors, errors_feedback_circuitry, 13)
  STEER_ERROR_INVALIDATE(hv_errors, errors_eeprom_comm, 14)
  STEER_ERROR_INVALIDATE(hv_errors, errors_eeprom_write, 15)
}

void hv_feedbacks_status_invalidate() {
  STEER_ERROR_INVALIDATE(hv_feedbacks_status, 
        feedbacks_status_feedback_implausibility_detected, 0)
  STEER_ERROR_INVALIDATE(hv_feedbacks_status, 
        feedbacks_status_feedback_imd_cockpit, 1)
  STEER_ERROR_INVALIDATE(hv_feedbacks_status, 
        feedbacks_status_feedback_tsal_green_fault_latched, 2)
  STEER_ERROR_INVALIDATE(hv_feedbacks_status, 
        feedbacks_status_feedback_bms_cockpit, 3)
  STEER_ERROR_INVALIDATE(hv_feedbacks_status, 
        feedbacks_status_feedback_ext_latched, 4)
  STEER_ERROR_INVALIDATE(hv_feedbacks_status, 
        feedbacks_status_feedback_tsal_green, 5)
  STEER_ERROR_INVALIDATE(hv_feedbacks_status, 
        feedbacks_status_feedback_ts_over_60v_status, 6)
  STEER_ERROR_INVALIDATE(hv_feedbacks_status, 
        feedbacks_status_feedback_airn_status, 7)
  STEER_ERROR_INVALIDATE(hv_feedbacks_status, 
        feedbacks_status_feedback_airp_status, 8)
  STEER_ERROR_INVALIDATE(hv_feedbacks_status, 
        feedbacks_status_feedback_airp_gate, 9)
  STEER_ERROR_INVALIDATE(hv_feedbacks_status, 
        feedbacks_status_feedback_airn_gate, 10)
  STEER_ERROR_INVALIDATE(hv_feedbacks_status, 
        feedbacks_status_feedback_precharge_status, 11)
  STEER_ERROR_INVALIDATE(hv_feedbacks_status, 
        feedbacks_status_feedback_tsp_over_60v_status, 12)
  STEER_ERROR_INVALIDATE(hv_feedbacks_status, 
        feedbacks_status_feedback_imd_fault, 13)
  STEER_ERROR_INVALIDATE(hv_feedbacks_status, 
        feedbacks_status_feedback_check_mux, 14)
  STEER_ERROR_INVALIDATE(hv_feedbacks_status, 
        feedbacks_status_feedback_sd_end, 15)
  STEER_ERROR_INVALIDATE(hv_feedbacks_status, 
        feedbacks_status_feedback_sd_out, 16)
  STEER_ERROR_INVALIDATE(hv_feedbacks_status, 
        feedbacks_status_feedback_sd_in, 17)
  STEER_ERROR_INVALIDATE(hv_feedbacks_status, 
        feedbacks_status_feedback_sd_bms, 18)
  STEER_ERROR_INVALIDATE(hv_feedbacks_status, 
        feedbacks_status_feedback_sd_imd, 19)
}

void hv_cells_voltage_invalidate() {

}

void hv_cells_temp_invalidate() {

}

void das_errors_invalidate() {
  STEER_ERROR_INVALIDATE(das_errors, das_error_pedal_adc, 0)
  STEER_ERROR_INVALIDATE(das_errors, das_error_pedal_implausibility, 1)
  STEER_ERROR_INVALIDATE(das_errors, das_error_imu_tout, 2)
  STEER_ERROR_INVALIDATE(das_errors, das_error_irts_tout, 3)
  STEER_ERROR_INVALIDATE(das_errors, das_error_ts_tout, 4)
  STEER_ERROR_INVALIDATE(das_errors, das_error_invl_tout, 5)
  STEER_ERROR_INVALIDATE(das_errors, das_error_invr_tout, 6)
  STEER_ERROR_INVALIDATE(das_errors, das_error_steer_tout, 7)
  STEER_ERROR_INVALIDATE(das_errors, das_error_fsm, 8)
}

void lv_currents_invalidate() {
  STEER_UPDATE_LABEL(steering.lv.lb_current, "N/A");
}

void lv_cells_voltage_invalidate() {

}

void lv_cells_temp_invalidate() {
  STEER_UPDATE_LABEL(steering.lv.lb_battery_temperature, "N/A");
}

void lv_total_voltage_invalidate() {
  STEER_UPDATE_LABEL(steering.lv.lb_voltage, "N/A");
}

void lv_errors_invalidate() {
  STEER_ERROR_INVALIDATE(lv_errors, errors_cell_undervoltage, 0)
  STEER_ERROR_INVALIDATE(lv_errors, errors_cell_overvoltage, 1)
  STEER_ERROR_INVALIDATE(lv_errors, errors_battery_open_wire, 2)
  STEER_ERROR_INVALIDATE(lv_errors, errors_can, 3)
  STEER_ERROR_INVALIDATE(lv_errors, errors_spi, 4)
  STEER_ERROR_INVALIDATE(lv_errors, errors_over_current, 5)
  STEER_ERROR_INVALIDATE(lv_errors, errors_cell_under_temperature, 6)
  STEER_ERROR_INVALIDATE(lv_errors, errors_cell_over_temperature, 7)
  STEER_ERROR_INVALIDATE(lv_errors, errors_relay, 8)
  STEER_ERROR_INVALIDATE(lv_errors, errors_bms_monitor, 9)
  STEER_ERROR_INVALIDATE(lv_errors, errors_voltages_not_ready, 10)
  STEER_ERROR_INVALIDATE(lv_errors, errors_mcp23017, 11)
  STEER_ERROR_INVALIDATE(lv_errors, errors_radiator, 12)
  STEER_ERROR_INVALIDATE(lv_errors, errors_fan, 13)
  STEER_ERROR_INVALIDATE(lv_errors, errors_pump, 14)
  STEER_ERROR_INVALIDATE(lv_errors, errors_adc_init, 15)
  STEER_ERROR_INVALIDATE(lv_errors, errors_mux, 16)
}

void steering_angle_invalidate() {
  STEER_UPDATE_LABEL(steering.steering.lb_steering_angle, "N/A");
}

void inv_l_rcv_invalidate() {
  STEER_UPDATE_LABEL(steering.inverters.lb_left_motor_temp, "N/A");
  STEER_UPDATE_LABEL(steering.inverters.lb_left_inverter_temp, "N/A");
}

void inv_r_rcv_invalidate() {
  STEER_UPDATE_LABEL(steering.inverters.lb_right_motor_temp, "N/A");
  STEER_UPDATE_LABEL(steering.inverters.lb_right_inverter_temp, "N/A");
}
