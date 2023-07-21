#include "steering.h"

steering_tabs_t steering = {0};
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

primary_das_errors_converted_t das_errors_last_state;

primary_lv_currents_converted_t lv_currents_last_state = {0};
primary_lv_cells_voltage_converted_t lv_cells_voltage_last_state_1 = {0};
primary_lv_cells_voltage_converted_t lv_cells_voltage_last_state_2 = {0};
primary_lv_cells_temp_converted_t lv_cells_temp_last_state_1 = {0};
primary_lv_cells_temp_converted_t lv_cells_temp_last_state_2 = {0};
primary_lv_cells_temp_converted_t lv_cells_temp_last_state_3 = {0};
primary_lv_cells_temp_converted_t lv_cells_temp_last_state_4 = {0};
float lv_cells_temp_mean_last_state = 0;
primary_lv_total_voltage_converted_t lv_total_voltage_last_state = {0};
primary_lv_errors_converted_t lv_errors_last_state = {0};

secondary_steering_angle_converted_t steering_angle_converted = {0};
secondary_pedals_output_converted_t pedals_output_last_state = {0};

inverters_inv_r_rcv_converted_t inv_r_last_state = {0};
inverters_inv_l_rcv_converted_t inv_l_last_state = {0};

inline void remove_trailing(char *buf) {
  char init = buf[0];
  bool is_cifra = init >= '0' && init <= '9';
  bool is_capital_letter = init >= 'A' && init <= 'Z';
  bool is_letter = init >= 'a' && init <= 'z';
  bool is_minus = init == '-';
  if (!is_cifra && !is_capital_letter && !is_letter && !is_minus)
    buf[0] = ' ';
}
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
    case primary_car_status_car_status_CHECK_INV_SETTINGS: {
      lv_label_set_text_fmt(steering.bottom_lb_speed, "-");
      STEER_UPDATE_LABEL(steering.lb_speed, "INIT");
      break;
    }
    case primary_car_status_car_status_IDLE: {
      lv_label_set_text_fmt(steering.bottom_lb_speed, "-");
      STEER_UPDATE_LABEL(steering.lb_speed, " IDLE ");
      break;
    }
    case primary_car_status_car_status_START_TS_PRECHARGE:
    case primary_car_status_car_status_WAIT_TS_PRECHARGE: {
      lv_label_set_text_fmt(steering.bottom_lb_speed, "-");
      STEER_UPDATE_LABEL(steering.lb_speed, "PRCHG");
      break;
    }
    case primary_car_status_car_status_WAIT_DRIVER: {
      lv_label_set_text_fmt(steering.bottom_lb_speed, "-");
      STEER_UPDATE_LABEL(steering.lb_speed, "SETUP");
      break;
    }
    case primary_car_status_car_status_ENABLE_INV_DRIVE: {
      lv_label_set_text_fmt(steering.bottom_lb_speed, "-");
      STEER_UPDATE_LABEL(steering.lb_speed, "ENINV");
      break;
    }
    case primary_car_status_car_status_DRIVE: {
      lv_label_set_text_fmt(steering.bottom_lb_speed, "-");
      STEER_UPDATE_LABEL(steering.lb_speed, "DRIVE");
      break;
    }
    case primary_car_status_car_status_DISABLE_INV_DRIVE:
    case primary_car_status_car_status_START_TS_DISCHARGE:
    case primary_car_status_car_status_WAIT_TS_DISCHARGE: {
      lv_label_set_text_fmt(steering.bottom_lb_speed, "-");
      STEER_UPDATE_LABEL(steering.lb_speed, "TSOFF");
      break;
    }
    case primary_car_status_car_status_FATAL_ERROR: {
      lv_label_set_text_fmt(steering.bottom_lb_speed, "-");
      STEER_UPDATE_LABEL(steering.lb_speed, "FATAL");
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
    control_output_last_state.estimated_velocity = data->estimated_velocity;
    sprintf(sprintf_buffer, "%.1f", data->estimated_velocity);
    STEER_UPDATE_LABEL(steering.lb_estimated_velocity, sprintf_buffer);
  }
}

void speed_update(primary_speed_converted_t *data) {
  if (car_status_last_state.car_status != primary_car_status_car_status_DRIVE)
    return;

  if (data->inverter_l != speed_last_state.inverter_l ||
      data->inverter_r != speed_last_state.inverter_r) {
    speed_last_state.inverter_l = data->inverter_l;
    speed_last_state.inverter_r = data->inverter_r;
#if 0
        const float wheel_radius = 0.2368760861f;
        float velocity = wheel_radius * ((data->inverter_l + data->inverter_r) / 2.0f);
        sprintf(sprintf_buffer, "%.1f", velocity);
        STEER_UPDATE_LABEL(steering.lb_speed, sprintf_buffer);
        lv_label_set_text_fmt(steering.bottom_lb_speed, "kmh");
#endif
  }
  if (data->encoder_l != speed_last_state.encoder_l ||
      data->encoder_r != speed_last_state.encoder_r) {
    speed_last_state.encoder_l = data->encoder_l;
    speed_last_state.encoder_r = data->encoder_r;
#if 0 // this is very broken
        const float wheel_radius = 0.2368760861f;
        float velocity = wheel_radius * ((data->encoder_l + data->encoder_r) / 2.0f);
        sprintf(sprintf_buffer, "%.1f", velocity);
        STEER_UPDATE_LABEL(steering.lb_speed, sprintf_buffer);
        lv_label_set_text_fmt(steering.bottom_lb_speed, "kmh");
#endif
  }
}

void hv_voltage_update(primary_hv_voltage_converted_t *data) {
  if (data->min_cell_voltage != hv_voltage_last_state.min_cell_voltage) {
    hv_voltage_last_state.min_cell_voltage = data->min_cell_voltage;
    sprintf(sprintf_buffer, "%.1f", data->min_cell_voltage);
    STEER_UPDATE_LABEL(steering.lb_min_cell_voltage, sprintf_buffer)
  }
  if (data->pack_voltage != hv_voltage_last_state.pack_voltage) {
    hv_voltage_last_state.pack_voltage = data->pack_voltage;
    sprintf(sprintf_buffer, "%.1f", data->pack_voltage);
    STEER_UPDATE_LABEL(steering.lb_pack_voltage, sprintf_buffer);
  }
}

void hv_current_update(primary_hv_current_converted_t *data) {
  if (data->current != hv_current_last_state.current) {
    hv_current_last_state.current = data->current + 10;
    sprintf(sprintf_buffer, "%.1f", hv_current_last_state.current);
    STEER_UPDATE_LABEL(steering.lb_hv_current, sprintf_buffer)
    lv_bar_set_value(steering.racing_hv_bar, hv_current_last_state.current,
                     LV_ANIM_OFF);
  }
}

void hv_temp_update(primary_hv_temp_converted_t *data) {
  if (data->average_temp != hv_temp_last_state.average_temp) {
    hv_temp_last_state.average_temp = data->average_temp;
    sprintf(sprintf_buffer, "%0.f", data->average_temp);
    STEER_UPDATE_LABEL(steering.lb_average_temperature, sprintf_buffer);
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

void hv_feedbacks_status_update(primary_hv_feedbacks_status_converted_t *data) {
  STEER_ERROR_UPDATE(hv_feedbacks_status,
                     feedbacks_status_feedback_implausibility_detected, 0)
  STEER_ERROR_UPDATE(hv_feedbacks_status, feedbacks_status_feedback_imd_cockpit,
                     1)
  STEER_ERROR_UPDATE(hv_feedbacks_status,
                     feedbacks_status_feedback_tsal_green_fault_latched, 2)
  STEER_ERROR_UPDATE(hv_feedbacks_status, feedbacks_status_feedback_bms_cockpit,
                     3)
  STEER_ERROR_UPDATE(hv_feedbacks_status, feedbacks_status_feedback_ext_latched,
                     4)
  STEER_ERROR_UPDATE(hv_feedbacks_status, feedbacks_status_feedback_tsal_green,
                     5)
  STEER_ERROR_UPDATE(hv_feedbacks_status,
                     feedbacks_status_feedback_ts_over_60v_status, 6)
  STEER_ERROR_UPDATE(hv_feedbacks_status, feedbacks_status_feedback_airn_status,
                     7)
  STEER_ERROR_UPDATE(hv_feedbacks_status, feedbacks_status_feedback_airp_status,
                     8)
  STEER_ERROR_UPDATE(hv_feedbacks_status, feedbacks_status_feedback_airp_gate,
                     9)
  STEER_ERROR_UPDATE(hv_feedbacks_status, feedbacks_status_feedback_airn_gate,
                     10)
  STEER_ERROR_UPDATE(hv_feedbacks_status,
                     feedbacks_status_feedback_precharge_status, 11)
  STEER_ERROR_UPDATE(hv_feedbacks_status,
                     feedbacks_status_feedback_tsp_over_60v_status, 12)
  STEER_ERROR_UPDATE(hv_feedbacks_status, feedbacks_status_feedback_imd_fault,
                     13)
  STEER_ERROR_UPDATE(hv_feedbacks_status, feedbacks_status_feedback_check_mux,
                     14)
  STEER_ERROR_UPDATE(hv_feedbacks_status, feedbacks_status_feedback_sd_end, 15)
  STEER_ERROR_UPDATE(hv_feedbacks_status, feedbacks_status_feedback_sd_out, 16)
  STEER_ERROR_UPDATE(hv_feedbacks_status, feedbacks_status_feedback_sd_in, 17)
  STEER_ERROR_UPDATE(hv_feedbacks_status, feedbacks_status_feedback_sd_bms, 18)
  STEER_ERROR_UPDATE(hv_feedbacks_status, feedbacks_status_feedback_sd_imd, 19)
}

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
    sprintf(sprintf_buffer, "%.1f", data->current_lv_battery);
    STEER_UPDATE_LABEL(steering.lb_lv_current, sprintf_buffer);
    lv_bar_set_value(steering.racing_lv_bar, data->current_lv_battery,
                     LV_ANIM_OFF);
  }
}

void lv_cells_temp_update(primary_lv_cells_temp_converted_t *data) {
  uint8_t start_index = data->start_index;
  uint8_t offset = 20;
  float current_lv_temp[12];
  current_lv_temp[0] = 27.0f;
  current_lv_temp[1] = 27.0f;
  current_lv_temp[2] = 27.0f;
  current_lv_temp[3] = 27.0f;
  current_lv_temp[4] = 27.0f;
  current_lv_temp[5] = 27.0f;
  current_lv_temp[6] = 27.0f;
  current_lv_temp[7] = 27.0f;
  current_lv_temp[8] = 27.0f;
  current_lv_temp[9] = 27.0f;
  current_lv_temp[10] = 27.0f;
  current_lv_temp[11] = 27.0f;
  if (start_index == 0) {
    current_lv_temp[0] = data->temp_0 + offset;
    current_lv_temp[1] = data->temp_1 + offset;
    current_lv_temp[2] = data->temp_2 + offset;
    current_lv_temp[3] = lv_cells_temp_last_state_2.temp_0 + offset;
    current_lv_temp[4] = lv_cells_temp_last_state_2.temp_1 + offset;
    current_lv_temp[5] = lv_cells_temp_last_state_2.temp_2 + offset;
    current_lv_temp[6] = lv_cells_temp_last_state_3.temp_0 + offset;
    current_lv_temp[7] = lv_cells_temp_last_state_3.temp_1 + offset;
    current_lv_temp[8] = lv_cells_temp_last_state_3.temp_2 + offset;
    current_lv_temp[9] = lv_cells_temp_last_state_4.temp_0 + offset;
    current_lv_temp[10] = lv_cells_temp_last_state_4.temp_1 + offset;
    current_lv_temp[11] = lv_cells_temp_last_state_4.temp_2 + offset;
  } else if (start_index == 3) {
    current_lv_temp[0] = lv_cells_temp_last_state_1.temp_0 + offset;
    current_lv_temp[1] = lv_cells_temp_last_state_1.temp_1 + offset;
    current_lv_temp[2] = lv_cells_temp_last_state_1.temp_2 + offset;
    current_lv_temp[3] = data->temp_0 + offset;
    current_lv_temp[4] = data->temp_1 + offset;
    current_lv_temp[5] = data->temp_2 + offset;
    current_lv_temp[6] = lv_cells_temp_last_state_3.temp_0 + offset;
    current_lv_temp[7] = lv_cells_temp_last_state_3.temp_1 + offset;
    current_lv_temp[8] = lv_cells_temp_last_state_3.temp_2 + offset;
    current_lv_temp[9] = lv_cells_temp_last_state_4.temp_0 + offset;
    current_lv_temp[10] = lv_cells_temp_last_state_4.temp_1 + offset;
    current_lv_temp[11] = lv_cells_temp_last_state_4.temp_2 + offset;
  } else if (start_index == 6) {
    current_lv_temp[0] = lv_cells_temp_last_state_1.temp_0 + offset;
    current_lv_temp[1] = lv_cells_temp_last_state_1.temp_1 + offset;
    current_lv_temp[2] = lv_cells_temp_last_state_1.temp_2 + offset;
    current_lv_temp[3] = lv_cells_temp_last_state_2.temp_0 + offset;
    current_lv_temp[4] = lv_cells_temp_last_state_2.temp_1 + offset;
    current_lv_temp[5] = lv_cells_temp_last_state_2.temp_2 + offset;
    current_lv_temp[6] = data->temp_0 + offset;
    current_lv_temp[7] = data->temp_1 + offset;
    current_lv_temp[8] = data->temp_2 + offset;
    current_lv_temp[9] = lv_cells_temp_last_state_4.temp_0 + offset;
    current_lv_temp[10] = lv_cells_temp_last_state_4.temp_1 + offset;
    current_lv_temp[11] = lv_cells_temp_last_state_4.temp_2 + offset;
  } else if (start_index == 9) {
    current_lv_temp[0] = lv_cells_temp_last_state_1.temp_0 + offset;
    current_lv_temp[1] = lv_cells_temp_last_state_1.temp_1 + offset;
    current_lv_temp[2] = lv_cells_temp_last_state_1.temp_2 + offset;
    current_lv_temp[3] = lv_cells_temp_last_state_2.temp_0 + offset;
    current_lv_temp[4] = lv_cells_temp_last_state_2.temp_1 + offset;
    current_lv_temp[5] = lv_cells_temp_last_state_2.temp_2 + offset;
    current_lv_temp[6] = lv_cells_temp_last_state_3.temp_0 + offset;
    current_lv_temp[7] = lv_cells_temp_last_state_3.temp_1 + offset;
    current_lv_temp[8] = lv_cells_temp_last_state_3.temp_2 + offset;
    current_lv_temp[9] = data->temp_0 + offset;
    current_lv_temp[10] = data->temp_1 + offset;
    current_lv_temp[11] = data->temp_2 + offset;
  }

  size_t temp_size = 0;
  float sum = 0;
  for (uint8_t temp_index = 0; temp_index < 12; temp_index++) {
    if (current_lv_temp[temp_index] != -1) {
      sum += current_lv_temp[temp_index];
      temp_size++;
    }
  }

  float mean_temp = (float)(sum / temp_size);
  if (mean_temp != lv_cells_temp_mean_last_state) {
    lv_cells_temp_mean_last_state = mean_temp;
    sprintf(sprintf_buffer, "%.0f", mean_temp);
    STEER_UPDATE_LABEL(steering.lb_battery_temperature, sprintf_buffer);
  }
}

void lv_total_voltage_update(primary_lv_total_voltage_converted_t *data) {
  float old_total_v = lv_total_voltage_last_state.total_voltage;
  if (old_total_v != data->total_voltage) {
    lv_total_voltage_last_state.total_voltage = data->total_voltage;
    sprintf(sprintf_buffer, "%.1f", data->total_voltage);
    STEER_UPDATE_LABEL(steering.lb_voltage, sprintf_buffer);
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
    sprintf(sprintf_buffer, "%.1f", data->angle);
    STEER_UPDATE_LABEL(steering.lb_steering_angle, sprintf_buffer);
    if (steering.curr_focus == STEER) {
      lv_slider_set_mode(steering.slider, LV_BAR_MODE_SYMMETRICAL);
      lv_slider_set_range(
          steering.slider, STEERING_ANGLE_RANGE_LOW,
          STEERING_ANGLE_RANGE_HIGH); // se range 45 e max value 180 ->
                                      // set_value ( 0.25 * gradi_inclinazione )
      lv_slider_set_value(steering.slider, steering_angle_converted.angle,
                          LV_ANIM_OFF);
    }
  }
}

void pedals_output_update(secondary_pedals_output_converted_t *data) {
  if (data->apps != pedals_output_last_state.apps) {
    pedals_output_last_state.apps = data->apps;
    sprintf(sprintf_buffer, "%d", (int)data->apps);
    STEER_UPDATE_LABEL(steering.lb_apps, sprintf_buffer);
    if (steering.curr_focus == APPS) {
      lv_slider_set_mode(steering.slider, LV_BAR_MODE_RANGE);
      lv_slider_set_range(steering.slider, APPS_RANGE_LOW, APPS_RANGE_HIGH);
      lv_slider_set_value(steering.slider, pedals_output_last_state.apps,
                          LV_ANIM_OFF);
    }
  }
  if (data->bse_front != pedals_output_last_state.bse_front) {
    pedals_output_last_state.bse_front = data->bse_front;
    sprintf(sprintf_buffer, "%.1f", data->bse_front);
    STEER_UPDATE_LABEL(steering.lb_bse, sprintf_buffer);
    if (steering.curr_focus == BSE) {
      lv_slider_set_mode(steering.slider, LV_BAR_MODE_RANGE);
      lv_slider_set_range(steering.slider, BRAKE_RANGE_LOW, BRAKE_RANGE_HIGH);
      lv_slider_set_value(steering.slider, data->bse_front, LV_ANIM_OFF);
    }
  }
}

void inv_l_rcv_update(inverters_inv_l_rcv_converted_t *data) {

  if (data->t_motor != inv_l_last_state.t_motor &&
      data->rcv_mux == INVERTERS_INV_L_RCV_RCV_MUX_ID_49_T_MOTOR_CHOICE) {
    float new_value = (data->t_motor - 9393.9f) / 55.1f;
    inv_l_last_state.t_motor = new_value;
    sprintf(sprintf_buffer, "%.0f", new_value);
    STEER_UPDATE_LABEL(steering.lb_left_motor_temp, sprintf_buffer);
  }
  if (data->t_igbt != inv_l_last_state.t_igbt &&
      data->rcv_mux == INVERTERS_INV_L_RCV_RCV_MUX_ID_4A_T_IGBT_CHOICE) {
    float new_value = (data->t_igbt * 0.005f - 38.0f);
    inv_l_last_state.t_igbt = new_value;
    sprintf(sprintf_buffer, "%.0f", new_value);
    STEER_UPDATE_LABEL(steering.lb_left_inverter_temp, sprintf_buffer);
  }
#if 0
    if (data->n_actual_filt != inv_l_last_state.n_actual_filt && data->rcv_mux == INVERTERS_INV_L_RCV_RCV_MUX_ID_A8_N_ACTUAL_FILT_CHOICE) {
        float new_value = data->n_actual_filt * 0.1f;
        inv_l_last_state.n_actual_filt = new_value;
        sprintf(sprintf_buffer, "%.0f", new_value);
        STEER_UPDATE_LABEL(steering.lb_speed, sprintf_buffer);
    }
#endif
}

void inv_r_rcv_update(inverters_inv_r_rcv_converted_t *data) {
  if (data->t_motor != inv_r_last_state.t_motor &&
      data->rcv_mux == INVERTERS_INV_R_RCV_RCV_MUX_ID_49_T_MOTOR_CHOICE) {
    float new_value = (data->t_motor - 9393.9f) / 55.1f;
    inv_r_last_state.t_motor = new_value;
    sprintf(sprintf_buffer, "%.0f", new_value);
    STEER_UPDATE_LABEL(steering.lb_right_motor_temp, sprintf_buffer);
  }
  if (data->t_igbt != inv_r_last_state.t_igbt &&
      data->rcv_mux == INVERTERS_INV_R_RCV_RCV_MUX_ID_4A_T_IGBT_CHOICE) {
    float new_value = (data->t_igbt * 0.005f - 38.0f);
    inv_r_last_state.t_igbt = new_value;
    sprintf(sprintf_buffer, "%.0f", new_value);
    STEER_UPDATE_LABEL(steering.lb_right_inverter_temp, sprintf_buffer);
  }
#if 0
    // net_signals["INV_L_RCV_ELAB"]["n_actual_filt"].push((net_signals["INV_L_RCV"]["n_actual_filt"][i] * INV_MAX_SPEED) / 32767.f);
    if (data->n_actual_filt != inv_r_last_state.n_actual_filt && data->rcv_mux == INVERTERS_INV_R_RCV_RCV_MUX_ID_A8_N_ACTUAL_FILT_CHOICE) {
        float new_value = (data->n_actual_filt * INV_MAX_SPEED) / 32767.f;
        inv_r_last_state.n_actual_filt = new_value;
        sprintf(sprintf_buffer, "%.0f", new_value);
        STEER_UPDATE_LABEL(steering.lb_speed, sprintf_buffer);
    }
#endif
}
