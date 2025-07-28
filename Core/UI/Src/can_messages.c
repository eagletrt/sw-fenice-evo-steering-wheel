#include "can_messages.h"

void send_ecu_set_status(primary_ecu_set_status_status val) {
    primary_ecu_set_status_converted_t converted = { 0 };
    converted.status = val;
    STEER_CAN_PACK(primary, PRIMARY, ecu_set_status, ECU_SET_STATUS);
    can_send(&msg, true);
    can_send(&msg, true);
}

void handle_primary(can_message_t *msg) {
#if WATCHDOG_ENABLED == 1
    if (inverters_id_is_message(msg->id)) {
        inverters_watchdog_reset(&m_inverters_watchdog, msg->id, get_current_time_ms());
    } else {
        primary_watchdog_reset(&m_primary_watchdog, msg->id, get_current_time_ms());
    }
#endif

    can_id_t id = msg->id;
    switch (id) {
        case PRIMARY_STEERING_WHEEL_JMP_TO_BLT_FRAME_ID:
            openblt_reset();
            break;
        case PRIMARY_ECU_STATUS_FRAME_ID: {
            STEER_CAN_UNPACK(primary, PRIMARY, ecu_status, ECU_STATUS, is_pmsg, true);
            break;
        }
        case PRIMARY_ECU_POWER_MAPS_FRAME_ID: {
            STEER_CAN_UNPACK(primary, PRIMARY, ecu_power_maps, ECU_POWER_MAPS, is_pmsg, true);
            break;
        }
        case PRIMARY_ECU_PTT_STATUS_FRAME_ID: {
            STEER_CAN_UNPACK(primary, PRIMARY, ecu_ptt_status, ECU_PTT_STATUS, is_pmsg, true);
            break;
        }
        case PRIMARY_ECU_FEEDBACKS_FRAME_ID: {
            STEER_CAN_UNPACK(primary, PRIMARY, ecu_feedbacks, ECU_FEEDBACKS, is_pmsg, true);
            break;
        }
        case PRIMARY_ECU_ERRORS_FRAME_ID: {
            STEER_CAN_UNPACK(primary, PRIMARY, ecu_errors, ECU_ERRORS, is_pmsg, true);
            break;
        }
        case PRIMARY_HV_DEBUG_SIGNALS_FRAME_ID: {
            // STEER_CAN_UNPACK(primary, PRIMARY, hv_debug_signals, HV_DEBUG_SIGNALS, is_pmsg, true);
            break;
        }
        case PRIMARY_HV_CELLS_VOLTAGE_STATS_FRAME_ID: {
            // STEER_CAN_UNPACK(primary, PRIMARY, hv_cells_voltage_stats, HV_CELLS_VOLTAGE_STATS, is_pmsg, true);
            break;
        }
        case PRIMARY_HV_STATUS_FRAME_ID: {
            STEER_CAN_UNPACK(primary, PRIMARY, hv_status, HV_STATUS, is_pmsg, true);
            break;
        }
        case PRIMARY_HV_TS_VOLTAGE_FRAME_ID: {
            STEER_CAN_UNPACK(primary, PRIMARY, hv_ts_voltage, HV_TS_VOLTAGE, is_pmsg, true);
            break;
        }
        case PRIMARY_HV_CURRENT_FRAME_ID: {
            STEER_CAN_UNPACK(primary, PRIMARY, hv_current, HV_CURRENT, is_pmsg, true);
            break;
        }
        case PRIMARY_HV_CELLS_TEMP_STATS_FRAME_ID: {
            STEER_CAN_UNPACK(primary, PRIMARY, hv_cells_temp_stats, HV_CELLS_TEMP_STATS, is_pmsg, true);
            break;
        }
        case PRIMARY_HV_ERRORS_FRAME_ID: {
            STEER_CAN_UNPACK(primary, PRIMARY, hv_errors, HV_ERRORS, is_pmsg, true);
            break;
        }
        case PRIMARY_HV_BALANCING_STATUS_FRAME_ID: {
            STEER_CAN_UNPACK(primary, PRIMARY, hv_balancing_status, HV_BALANCING_STATUS, is_pmsg, true);
            break;
        }
        case PRIMARY_HV_FANS_STATUS_FRAME_ID: {
            // STEER_CAN_UNPACK(primary, PRIMARY, hv_fans_status, HV_FANS_STATUS, is_pmsg, true);
            break;
        }
        case PRIMARY_HV_FEEDBACK_STATUS_FRAME_ID: {
            STEER_CAN_UNPACK(primary, PRIMARY, hv_feedback_status, HV_FEEDBACK_STATUS, is_pmsg, true);
            break;
        }
        case PRIMARY_TLM_STATUS_FRAME_ID: {
            STEER_CAN_UNPACK(primary, PRIMARY, tlm_status, TLM_STATUS, is_pmsg, true);
            break;
        }
        case PRIMARY_HV_FEEDBACK_TS_VOLTAGE_FRAME_ID: {
            // STEER_CAN_UNPACK(primary, PRIMARY, hv_feedback_ts_voltage, HV_FEEDBACK_TS_VOLTAGE, is_pmsg, true);
            break;
        }
        case PRIMARY_HV_FEEDBACK_MISC_VOLTAGE_FRAME_ID: {
            // STEER_CAN_UNPACK(primary, PRIMARY, hv_feedback_misc_voltage, HV_FEEDBACK_MISC_VOLTAGE, is_pmsg, true);
            break;
        }
        case PRIMARY_HV_FEEDBACK_ANALOG_SD_FRAME_ID: {
            STEER_CAN_UNPACK(primary, PRIMARY, hv_feedback_analog_sd, HV_FEEDBACK_ANALOG_SD, is_pmsg, true);
            break;
        }
        case PRIMARY_LV_FEEDBACK_SD_VOLTAGE_FRAME_ID: {
            STEER_CAN_UNPACK(primary, PRIMARY, lv_feedback_sd_voltage, LV_FEEDBACK_SD_VOLTAGE, is_pmsg, true);
            break;
        }
        case PRIMARY_LV_FEEDBACK_TS_VOLTAGE_FRAME_ID: {
            STEER_CAN_UNPACK(primary, PRIMARY, lv_feedback_ts_voltage, LV_FEEDBACK_TS_VOLTAGE, is_pmsg, true);
            break;
        }
        case PRIMARY_LV_COOLING_AGGRESSIVENESS_FRAME_ID: {
            STEER_CAN_UNPACK(primary, PRIMARY, lv_cooling_aggressiveness, LV_COOLING_AGGRESSIVENESS, is_pmsg, true);
            break;
        }
        case PRIMARY_ECU_CONTROL_STATUS_FRAME_ID: {
            STEER_CAN_UNPACK(primary, PRIMARY, ecu_control_status, ECU_CONTROL_STATUS, is_pmsg, true);
            break;
        }
        case PRIMARY_LV_FEEDBACK_ENCLOSURE_VOLTAGE_FRAME_ID: {
            STEER_CAN_UNPACK(primary, PRIMARY, lv_feedback_enclosure_voltage, LV_FEEDBACK_ENCLOSURE_VOLTAGE, is_pmsg, true);
            break;
        }
        case PRIMARY_LV_ERRORS_FRAME_ID: {
            STEER_CAN_UNPACK(primary, PRIMARY, lv_errors, LV_ERRORS, is_pmsg, true);
            break;
        }
        case PRIMARY_LV_RADIATOR_SPEED_FRAME_ID: {
            STEER_CAN_UNPACK(primary, PRIMARY, lv_radiator_speed, LV_RADIATOR_SPEED, is_pmsg, true);
            break;
        }
        case PRIMARY_LV_PUMPS_SPEED_FRAME_ID: {
            STEER_CAN_UNPACK(primary, PRIMARY, lv_pumps_speed, LV_PUMPS_SPEED, is_pmsg, true);
            break;
        }
        case PRIMARY_LV_CURRENT_BATTERY_FRAME_ID: {
            STEER_CAN_UNPACK(primary, PRIMARY, lv_current_battery, LV_CURRENT_BATTERY, is_pmsg, true);
            break;
        }
        case PRIMARY_LV_TOTAL_VOLTAGE_FRAME_ID: {
            STEER_CAN_UNPACK(primary, PRIMARY, lv_total_voltage, LV_TOTAL_VOLTAGE, is_pmsg, true);
            break;
        }
        case PRIMARY_LV_CELLS_VOLTAGE_STATS_FRAME_ID: {
            STEER_CAN_UNPACK(primary, PRIMARY, lv_cells_voltage_stats, LV_CELLS_VOLTAGE_STATS, is_pmsg, true);
            break;
        }
        case PRIMARY_LV_CELLS_TEMP_STATS_FRAME_ID: {
            STEER_CAN_UNPACK(primary, PRIMARY, lv_cells_temp_stats, LV_CELLS_TEMP_STATS, is_pmsg, true);
            break;
        }
        case PRIMARY_CONTROL_OUTPUT_FRAME_ID: {
            STEER_CAN_UNPACK(primary, PRIMARY, control_output, CONTROL_OUTPUT, is_pmsg, true);
            break;
        }
        case PRIMARY_CONTROL_STATUS_FRAME_ID: {
            STEER_CAN_UNPACK(primary, PRIMARY, control_status, CONTROL_STATUS, is_pmsg, true);
            break;
        }
        case PRIMARY_ECU_VERSION_FRAME_ID: {
            STEER_CAN_UNPACK(primary, PRIMARY, ecu_version, ECU_VERSION, is_pmsg, true);
            break;
        }
        case PRIMARY_LV_VERSION_FRAME_ID: {
            STEER_CAN_UNPACK(primary, PRIMARY, lv_version, LV_VERSION, is_pmsg, true);
            break;
        }
        case PRIMARY_HV_CELLBOARD_VERSION_FRAME_ID: {
            STEER_CAN_UNPACK(primary, PRIMARY, hv_cellboard_version, HV_CELLBOARD_VERSION, is_pmsg, true);
            break;
        }
        case PRIMARY_HV_MAINBOARD_VERSION_FRAME_ID: {
            STEER_CAN_UNPACK(primary, PRIMARY, hv_mainboard_version, HV_MAINBOARD_VERSION, is_pmsg, true);
            break;
        }
        case PRIMARY_TLM_VERSION_FRAME_ID: {
            STEER_CAN_UNPACK(primary, PRIMARY, tlm_version, TLM_VERSION, is_pmsg, true);
            break;
        }

        // INVERTERS
        case INVERTERS_INV_L_RCV_FRAME_ID: {
            inverters_inv_l_rcv_t raw;
            inverters_inv_l_rcv_converted_t converted;
            inverters_inv_l_rcv_unpack(&raw, msg->data, INVERTERS_INV_L_RCV_BYTE_SIZE);
            inverters_inv_l_rcv_raw_to_conversion_struct(&converted, &raw);
            inverters_inv_l_rcv_converted_t *last_state =
                (inverters_inv_l_rcv_converted_t *)&inverters_messages_last_state[inverters_index_from_id(msg->id)][0];
            if (converted.rcv_mux == INVERTERS_INV_L_RCV_RCV_MUX_ID_49_T_MOTOR_CHOICE) {
                last_state->t_motor = converted.t_motor;
                is_imsg_new[inverters_index_from_id(INVERTERS_INV_L_RCV_FRAME_ID)] = true;
                is_imsg_valid[inverters_index_from_id(INVERTERS_INV_L_RCV_FRAME_ID)] = true;
            } else if (converted.rcv_mux == INVERTERS_INV_L_RCV_RCV_MUX_ID_4A_T_IGBT_CHOICE) {
                last_state->t_igbt = converted.t_igbt;
                is_imsg_new[inverters_index_from_id(INVERTERS_INV_L_RCV_FRAME_ID)] = true;
                is_imsg_valid[inverters_index_from_id(INVERTERS_INV_L_RCV_FRAME_ID)] = true;
            }
            break;
        }
        case INVERTERS_INV_R_RCV_FRAME_ID: {
            inverters_inv_r_rcv_t raw;
            inverters_inv_r_rcv_converted_t converted;
            inverters_inv_r_rcv_unpack(&raw, msg->data, INVERTERS_INV_R_RCV_BYTE_SIZE);
            inverters_inv_r_rcv_raw_to_conversion_struct(&converted, &raw);
            inverters_inv_r_rcv_converted_t *last_state =
                (inverters_inv_r_rcv_converted_t *)&inverters_messages_last_state[inverters_index_from_id(msg->id)][0];
            if (converted.rcv_mux == INVERTERS_INV_R_RCV_RCV_MUX_ID_49_T_MOTOR_CHOICE) {
                last_state->t_motor = converted.t_motor;
                is_imsg_new[inverters_index_from_id(INVERTERS_INV_R_RCV_FRAME_ID)] = true;
                is_imsg_valid[inverters_index_from_id(INVERTERS_INV_R_RCV_FRAME_ID)] = true;
            } else if (converted.rcv_mux == INVERTERS_INV_R_RCV_RCV_MUX_ID_4A_T_IGBT_CHOICE) {
                last_state->t_igbt = converted.t_igbt;
                is_imsg_new[inverters_index_from_id(INVERTERS_INV_R_RCV_FRAME_ID)] = true;
                is_imsg_valid[inverters_index_from_id(INVERTERS_INV_R_RCV_FRAME_ID)] = true;
            }
            break;
        }
        default:
            break;
    }
}

void handle_secondary(can_message_t *msg) {
#if WATCHDOG_ENABLED == 1
    secondary_watchdog_reset(&m_secondary_watchdog, msg->id, get_current_time_ms());
#endif

    can_id_t id = msg->id;
    switch (id) {
        case SECONDARY_PEDAL_THROTTLE_FRAME_ID: {
            STEER_CAN_UNPACK(secondary, SECONDARY, pedal_throttle, PEDAL_THROTTLE, is_smsg, true);
            break;
        }
        case SECONDARY_PEDAL_BRAKES_PRESSURE_FRAME_ID: {
            STEER_CAN_UNPACK(secondary, SECONDARY, pedal_brakes_pressure, PEDAL_BRAKES_PRESSURE, is_smsg, true);
            break;
        }
        case SECONDARY_VEHICLE_SPEED_FRAME_ID: {
            STEER_CAN_UNPACK(secondary, SECONDARY, vehicle_speed, VEHICLE_SPEED, is_smsg, true);
            break;
        }
        case SECONDARY_FRONT_ANGULAR_VELOCITY_FRAME_ID: {
            STEER_CAN_UNPACK(secondary, SECONDARY, front_angular_velocity, FRONT_ANGULAR_VELOCITY, is_smsg, true);
            break;
        }
        case SECONDARY_STEER_ANGLE_FRAME_ID: {
            STEER_CAN_UNPACK(secondary, SECONDARY, steer_angle, STEER_ANGLE, is_smsg, true);
            break;
        }
        case SECONDARY_TLM_LAP_TIME_FRAME_ID: {
            STEER_CAN_UNPACK(secondary, SECONDARY, tlm_lap_time, TLM_LAP_TIME, is_smsg, true);
            break;
        }
        case SECONDARY_TLM_LAPS_STATS_FRAME_ID: {
            STEER_CAN_UNPACK(secondary, SECONDARY, tlm_laps_stats, TLM_LAPS_STATS, is_smsg, true);
            break;
        }
        case SECONDARY_LV_SOC_ESTIMATION_STATE_FRAME_ID: {
            STEER_CAN_UNPACK(secondary, SECONDARY, lv_soc_estimation_state, LV_SOC_ESTIMATION_STATE, is_smsg, true);
            break;
        }
        case SECONDARY_HV_SOC_ESTIMATION_STATE_FRAME_ID: {
            STEER_CAN_UNPACK(secondary, SECONDARY, hv_soc_estimation_state, HV_SOC_ESTIMATION_STATE, is_smsg, true);
            break;
        }
        case SECONDARY_IMU_ACCELERATION_FRAME_ID: {
            STEER_CAN_UNPACK(secondary, SECONDARY, imu_acceleration, IMU_ACCELERATION, is_smsg, true);
            break;
        }
        case SECONDARY_IMU_ANGULAR_RATE_FRAME_ID: {
            STEER_CAN_UNPACK(secondary, SECONDARY, imu_angular_rate, IMU_ANGULAR_RATE, is_smsg, true);
            break;
        }
        case SECONDARY_TLM_UNIX_TIMESTAMP_FRAME_ID: {
            STEER_CAN_UNPACK(secondary, SECONDARY, tlm_unix_timestamp, TLM_UNIX_TIMESTAMP, is_smsg, true);
            break;
        }
        case SECONDARY_IRTS_FL_0_FRAME_ID: {
            STEER_CAN_UNPACK(secondary, SECONDARY, irts_fl_0, IRTS_FL_0, is_smsg, true);
            break;
        }
        case SECONDARY_IRTS_FL_1_FRAME_ID: {
            STEER_CAN_UNPACK(secondary, SECONDARY, irts_fl_1, IRTS_FL_1, is_smsg, true);
            break;
        }
        case SECONDARY_IRTS_FL_2_FRAME_ID: {
            STEER_CAN_UNPACK(secondary, SECONDARY, irts_fl_2, IRTS_FL_2, is_smsg, true);
            break;
        }
        case SECONDARY_IRTS_FL_3_FRAME_ID: {
            STEER_CAN_UNPACK(secondary, SECONDARY, irts_fl_3, IRTS_FL_3, is_smsg, true);
            break;
        }
        case SECONDARY_IRTS_FR_0_FRAME_ID: {
            STEER_CAN_UNPACK(secondary, SECONDARY, irts_fr_0, IRTS_FR_0, is_smsg, true);
            break;
        }
        case SECONDARY_IRTS_FR_1_FRAME_ID: {
            STEER_CAN_UNPACK(secondary, SECONDARY, irts_fr_1, IRTS_FR_1, is_smsg, true);
            break;
        }
        case SECONDARY_IRTS_FR_2_FRAME_ID: {
            STEER_CAN_UNPACK(secondary, SECONDARY, irts_fr_2, IRTS_FR_2, is_smsg, true);
            break;
        }
        case SECONDARY_IRTS_FR_3_FRAME_ID: {
            STEER_CAN_UNPACK(secondary, SECONDARY, irts_fr_3, IRTS_FR_3, is_smsg, true);
            break;
        }
        case SECONDARY_IRTS_RL_0_FRAME_ID: {
            STEER_CAN_UNPACK(secondary, SECONDARY, irts_rl_0, IRTS_RL_0, is_smsg, true);
            break;
        }
        case SECONDARY_IRTS_RL_1_FRAME_ID: {
            STEER_CAN_UNPACK(secondary, SECONDARY, irts_rl_1, IRTS_RL_1, is_smsg, true);
            break;
        }
        case SECONDARY_IRTS_RL_2_FRAME_ID: {
            STEER_CAN_UNPACK(secondary, SECONDARY, irts_rl_2, IRTS_RL_2, is_smsg, true);
            break;
        }
        case SECONDARY_IRTS_RL_3_FRAME_ID: {
            STEER_CAN_UNPACK(secondary, SECONDARY, irts_rl_3, IRTS_RL_3, is_smsg, true);
            break;
        }
        case SECONDARY_IRTS_RR_0_FRAME_ID: {
            STEER_CAN_UNPACK(secondary, SECONDARY, irts_rr_0, IRTS_RR_0, is_smsg, true);
            break;
        }
        case SECONDARY_IRTS_RR_1_FRAME_ID: {
            STEER_CAN_UNPACK(secondary, SECONDARY, irts_rr_1, IRTS_RR_1, is_smsg, true);
            break;
        }
        case SECONDARY_IRTS_RR_2_FRAME_ID: {
            STEER_CAN_UNPACK(secondary, SECONDARY, irts_rr_2, IRTS_RR_2, is_smsg, true);
            break;
        }
        case SECONDARY_IRTS_RR_3_FRAME_ID: {
            STEER_CAN_UNPACK(secondary, SECONDARY, irts_rr_3, IRTS_RR_3, is_smsg, true);
            break;
        }
        case SECONDARY_ODOMETER_FRAME_ID: {
            STEER_CAN_UNPACK(secondary, SECONDARY, odometer, ODOMETER, is_smsg, true);
            break;
        }
        case SECONDARY_TLM_NETWORK_INTERFACE_FRAME_ID: {
            STEER_CAN_UNPACK(secondary, SECONDARY, tlm_network_interface, TLM_NETWORK_INTERFACE, is_smsg, true);
            /* uint32_t interface_name = converted.iname_3;
            interface_name |= ((uint32_t)converted.iname_2) << 8;
            interface_name |= ((uint32_t)converted.iname_1) << 16;
            interface_name |= ((uint32_t)converted.iname_0) << 24;
            int tlm_ntw_index = -1;
            for (size_t i = 0; i < tlm_ntw_interfaces_current_size; i++) {
                if (tlm_ntw_interfaces[i] == interface_name) {
                    tlm_ntw_index = i;
                    break;
                }
            }
            if (tlm_ntw_index == -1) {
                if (tlm_ntw_interfaces_current_size == TLM_NTW_INTERFACE_MAX_N)
                    return;
                tlm_ntw_index = tlm_ntw_interfaces_current_size;
                tlm_ntw_interfaces_current_size++;
            }
            tlm_ntw_interfaces[tlm_ntw_index] = interface_name;
            tlm_ntw_ips[tlm_ntw_index]        = converted.ip_address; */
            break;
        }
        default:
            break;
    }
}

void message_parser(uint8_t *msg, size_t msg_siz) {
    msg[msg_siz] = 0;
    uint32_t msgi = 0, msgl = 0;
    uint8_t msg_data[8] = { 0 };
    msgi = strtol((char *)msg, NULL, 16);
    msgl = strtol((char *)msg + 2, NULL, 16);
    for (size_t i = 0; i < 8; ++i) {
        char m[3] = { msg[6 + i * 2], msg[6 + i * 2 + 1], 0 };
        msg_data[i] = strtol(m, NULL, 16);
    }
    can_message_t parsed_msg = {
        .id = msgl,
        .size = 0, // unused
    };
    memcpy(&parsed_msg.data, msg_data, 8);
    if (msgi == 0) {
        handle_primary(&parsed_msg);
    } else if (msgi == 1) {
        handle_secondary(&parsed_msg);
    }
}
