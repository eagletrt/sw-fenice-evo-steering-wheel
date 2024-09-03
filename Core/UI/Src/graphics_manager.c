#include "graphics_manager.h"

uint8_t *framebuffer_1 = (uint8_t *)FRAMEBUFFER1_ADDR;
uint8_t *framebuffer_2 = (uint8_t *)FRAMEBUFFER2_ADDR;

void update_graphics(UI_t *scr) {
    for (uint16_t iindex = 0; iindex < primary_MESSAGE_COUNT; iindex++) {
        if (is_pmsg_new[iindex]) {
            is_pmsg_new[iindex] = false;
            can_id_t id         = primary_id_from_index(iindex);
            switch (id) {
                case PRIMARY_ECU_STATUS_FRAME_ID: {
                    ecu_status_update(scr, is_pmsg_valid[iindex]);
                    break;
                }
                case PRIMARY_ECU_POWER_MAPS_FRAME_ID:
                    ecu_power_maps_update(scr, is_pmsg_valid[iindex]);
                    break;
                case PRIMARY_ECU_FEEDBACKS_FRAME_ID: {
                    ecu_feedbacks_update(scr, is_pmsg_valid[iindex]);
                    break;
                }
                case PRIMARY_ECU_ERRORS_FRAME_ID: {
                    ecu_errors_update(scr, is_pmsg_valid[iindex]);
                    break;
                }
                case PRIMARY_TLM_STATUS_FRAME_ID: {
                    tlm_status_update(scr, is_pmsg_valid[iindex]);
                    break;
                }
                case PRIMARY_HV_STATUS_FRAME_ID: {
                    hv_status_update(scr, is_pmsg_valid[iindex]);
                    break;
                }
                case PRIMARY_HV_DEBUG_SIGNALS_FRAME_ID: {
                    hv_debug_signals_update(scr, is_pmsg_valid[iindex]);
                    break;
                }
                case PRIMARY_HV_TOTAL_VOLTAGE_FRAME_ID: {
                    hv_total_voltage_update(scr, is_pmsg_valid[iindex]);
                    break;
                }
                case PRIMARY_HV_CURRENT_FRAME_ID: {
                    hv_current_update(scr, is_pmsg_valid[iindex]);
                    break;
                }
                case PRIMARY_HV_CELLS_TEMP_STATS_FRAME_ID: {
                    hv_cells_temp_stats_update(scr, is_pmsg_valid[iindex]);
                    break;
                }
                case PRIMARY_HV_ERRORS_FRAME_ID: {
                    hv_errors_update(scr, is_pmsg_valid[iindex]);
                    break;
                }
                case PRIMARY_HV_FEEDBACK_TS_VOLTAGE_FRAME_ID: {
                    hv_feedback_ts_voltage_update(scr, is_pmsg_valid[iindex]);
                    break;
                }
                case PRIMARY_HV_FEEDBACK_MISC_VOLTAGE_FRAME_ID: {
                    hv_feedback_misc_voltage_update(scr, is_pmsg_valid[iindex]);
                    break;
                }
                case PRIMARY_HV_FEEDBACK_SD_VOLTAGE_FRAME_ID: {
                    hv_feedback_sd_voltage_update(scr, is_pmsg_valid[iindex]);
                    break;
                }
                case PRIMARY_HV_BALANCING_STATUS_FRAME_ID: {
                    hv_cell_balancing_status_update(scr, is_pmsg_valid[iindex]);
                    break;
                }
                case PRIMARY_HV_FANS_STATUS_FRAME_ID: {
                    hv_fans_override_status_update(scr, is_pmsg_valid[iindex]);
                    break;
                }
                case PRIMARY_HV_CELLS_VOLTAGE_STATS_FRAME_ID: {
                    hv_cells_voltage_stats_update(scr, is_pmsg_valid[iindex]);
                    break;
                }
                case PRIMARY_HV_FEEDBACK_STATUS_FRAME_ID: {
                    hv_feedbacks_status_update(scr, is_pmsg_valid[iindex]);
                    break;
                }
                case PRIMARY_LV_FEEDBACK_SD_VOLTAGE_FRAME_ID: {
                    lv_feedback_sd_voltage_update(scr, is_pmsg_valid[iindex]);
                    break;
                }
                case PRIMARY_LV_FEEDBACK_TS_VOLTAGE_FRAME_ID: {
                    lv_feedback_ts_voltage_update(scr, is_pmsg_valid[iindex]);
                    break;
                }
                case PRIMARY_LV_FEEDBACK_ENCLOSURE_VOLTAGE_FRAME_ID: {
                    lv_feedback_enclosure_voltage_update(scr, is_pmsg_valid[iindex]);
                    break;
                }
                case PRIMARY_LV_ERRORS_FRAME_ID: {
                    lv_errors_update(scr, is_pmsg_valid[iindex]);
                    break;
                }
                case PRIMARY_LV_COOLING_AGGRESSIVENESS_FRAME_ID: {
                    lv_cooling_aggressiveness_update(scr, is_pmsg_valid[iindex]);
                    break;
                }
                case PRIMARY_LV_RADIATOR_SPEED_FRAME_ID: {
                    lv_radiator_speed_update(scr, is_pmsg_valid[iindex]);
                    break;
                }
                case PRIMARY_LV_PUMPS_SPEED_FRAME_ID: {
                    lv_pumps_speed_update(scr, is_pmsg_valid[iindex]);
                    break;
                }
                case PRIMARY_LV_CURRENT_BATTERY_FRAME_ID: {
                    lv_currents_update(scr, is_pmsg_valid[iindex]);
                    break;
                }
                case PRIMARY_LV_TOTAL_VOLTAGE_FRAME_ID: {
                    lv_total_voltage_update(scr, is_pmsg_valid[iindex]);
                    break;
                }
                case PRIMARY_LV_CELLS_VOLTAGE_FRAME_ID: {
                    lv_cells_voltage_update(scr, is_pmsg_valid[iindex]);
                    break;
                }
                case PRIMARY_LV_CELLS_VOLTAGE_STATS_FRAME_ID: {
                    lv_cells_voltage_stats_update(scr, is_pmsg_valid[iindex]);
                    break;
                }
                case PRIMARY_LV_CELLS_TEMP_FRAME_ID: {
                    lv_cells_temp_update(scr, is_pmsg_valid[iindex]);
                    break;
                }
                case PRIMARY_LV_CELLS_TEMP_STATS_FRAME_ID: {
                    lv_cells_temp_stats_update(scr, is_pmsg_valid[iindex]);
                    break;
                }
                case PRIMARY_ECU_VERSION_FRAME_ID: {
                    ecu_version_update(scr, is_pmsg_valid[iindex]);
                    break;
                }
                case PRIMARY_LV_VERSION_FRAME_ID: {
                    lv_version_update(scr, is_pmsg_valid[iindex]);
                    break;
                }
                case PRIMARY_HV_CELLBOARD_VERSION_FRAME_ID: {
                    hv_cellboard_version_update(scr, is_pmsg_valid[iindex]);
                    break;
                }
                case PRIMARY_HV_MAINBOARD_VERSION_FRAME_ID: {
                    hv_mainboard_version_update(scr, is_pmsg_valid[iindex]);
                    break;
                }
                case PRIMARY_TLM_VERSION_FRAME_ID: {
                    tlm_version_update(scr, is_pmsg_valid[iindex]);
                    break;
                }
                case PRIMARY_CONTROL_STATUS_FRAME_ID: {
                    control_status_update(scr, is_pmsg_valid[iindex]);
                    break;
                }
                default: {
                    break;
                }
            }
        }
    }

    for (uint16_t iindex = 0; iindex < inverters_MESSAGE_COUNT; iindex++) {
        if (is_imsg_new[iindex]) {
            is_imsg_new[iindex] = false;
            can_id_t id         = inverters_id_from_index(iindex);
            switch (id) {
                case INVERTERS_INV_L_RCV_FRAME_ID:
                    inv_l_rcv_update(scr, is_imsg_valid[iindex]);
                    break;
                case INVERTERS_INV_R_RCV_FRAME_ID:
                    inv_r_rcv_update(scr, is_imsg_valid[iindex]);
                    break;
                default:
                    break;
            }
        }
    }

    for (uint16_t iindex = 0; iindex < secondary_MESSAGE_COUNT; iindex++) {
        if (is_smsg_new[iindex]) {
            is_smsg_new[iindex] = false;
            can_id_t id         = secondary_id_from_index(iindex);
            switch (id) {
                case SECONDARY_FRONT_ANGULAR_VELOCITY_FRAME_ID: {
                    angular_velocity_update(scr, is_smsg_valid[iindex]);
                    break;
                }
                case SECONDARY_STEER_ANGLE_FRAME_ID: {
                    steer_angle_update(scr, is_smsg_valid[iindex]);
                    break;
                }
                case SECONDARY_VEHICLE_SPEED_FRAME_ID: {
                    vehicle_speed_update(scr, is_smsg_valid[iindex]);
                    break;
                }
                case SECONDARY_PEDAL_THROTTLE_FRAME_ID: {
                    pedal_throttle_update(scr, is_smsg_valid[iindex]);
                    break;
                }
                case SECONDARY_PEDAL_BRAKES_PRESSURE_FRAME_ID: {
                    pedal_brakes_pressure_update(scr, is_smsg_valid[iindex]);
                    break;
                }
                case SECONDARY_IMU_ACCELERATION_FRAME_ID: {
                    imu_acceleration_update(scr, is_smsg_valid[iindex]);
                    break;
                }
                case SECONDARY_TLM_NETWORK_INTERFACE_FRAME_ID: {
                    tlm_network_interface_update(scr, is_smsg_valid[iindex]);
                    break;
                }
                case SECONDARY_IRTS_FL_0_FRAME_ID:
                case SECONDARY_IRTS_FL_1_FRAME_ID:
                case SECONDARY_IRTS_FL_2_FRAME_ID:
                case SECONDARY_IRTS_FL_3_FRAME_ID: {
                    irts_fl_update(scr, is_smsg_valid[iindex]);
                    break;
                }
                case SECONDARY_IRTS_FR_0_FRAME_ID:
                case SECONDARY_IRTS_FR_1_FRAME_ID:
                case SECONDARY_IRTS_FR_2_FRAME_ID:
                case SECONDARY_IRTS_FR_3_FRAME_ID: {
                    irts_fr_update(scr, is_smsg_valid[iindex]);
                    break;
                }
                case SECONDARY_IRTS_RL_0_FRAME_ID:
                case SECONDARY_IRTS_RL_1_FRAME_ID:
                case SECONDARY_IRTS_RL_2_FRAME_ID:
                case SECONDARY_IRTS_RL_3_FRAME_ID: {
                    irts_rl_update(scr, is_smsg_valid[iindex]);
                    break;
                }
                case SECONDARY_IRTS_RR_0_FRAME_ID:
                case SECONDARY_IRTS_RR_1_FRAME_ID:
                case SECONDARY_IRTS_RR_2_FRAME_ID:
                case SECONDARY_IRTS_RR_3_FRAME_ID: {
                    irts_rr_update(scr, is_smsg_valid[iindex]);
                    break;
                }
                case SECONDARY_ODOMETER_FRAME_ID: {
                    odometer_update(scr, is_smsg_valid[iindex]);
                    break;
                }
                case SECONDARY_TLM_LAP_TIME_FRAME_ID: {
                    tlm_lap_time_update(scr, is_smsg_valid[iindex]);
                    break;
                }
                case SECONDARY_TLM_LAPS_STATS_FRAME_ID: {
                    tlm_laps_stats_update(scr, is_smsg_valid[iindex]);
                    break;
                }
                case SECONDARY_HV_SOC_ESTIMATION_STATE_FRAME_ID: {
                    hv_soc_estimation_update(scr, is_smsg_valid[iindex]);
                    break;
                }
                case SECONDARY_LV_SOC_ESTIMATION_STATE_FRAME_ID: {
                    lv_soc_estimation_update(scr, is_smsg_valid[iindex]);
                    break;
                }
                default: {
                    break;
                }
            }
        }
    }
}
