#include "graphics_manager.h"

#include "steering.h"
#include "libraster-api.h"

void sw_update_graphics_from_can_messages(Box *scr) {
    for (uint16_t iindex = 0; iindex < primary_MESSAGE_COUNT; iindex++) {
        if (is_pmsg_new[iindex]) {
            is_pmsg_new[iindex] = false;
            can_id_t id = primary_id_from_index(iindex);
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
                    // hv_debug_signals_update(scr, is_pmsg_valid[iindex]);
                    break;
                }
                case PRIMARY_HV_TS_VOLTAGE_FRAME_ID: {
                    hv_ts_voltage_update(scr, is_pmsg_valid[iindex]);
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
                case PRIMARY_HV_FEEDBACK_ANALOG_SD_FRAME_ID: {
                    hv_feedback_analog_sd_update(scr, is_pmsg_valid[iindex]);
                    break;
                }
                case PRIMARY_HV_BALANCING_STATUS_FRAME_ID: {
                    hv_cell_balancing_status_update(scr, is_pmsg_valid[iindex]);
                    break;
                }
                case PRIMARY_HV_FANS_STATUS_FRAME_ID: {
                    // hv_fans_override_status_update(scr, is_pmsg_valid[iindex]);
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
            can_id_t id = inverters_id_from_index(iindex);
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
            can_id_t id = secondary_id_from_index(iindex);
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
                    // tlm_laps_stats_update(scr, is_smsg_valid[iindex]);
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

void sw_init_screen(Box *sw_screen) {
    // TODO SIMULATOR: initialize directly the fields of sw_screen without the temporary variable
    Label shutdown_l;
    create_label(&shutdown_l, "", (Coords){ 132, 30 }, KONEXY_120, 20, FONT_ALIGN_CENTER);

    Label status_l;
    create_label(&status_l, "", (Coords){ 132, 19 }, KONEXY_120, 20, FONT_ALIGN_CENTER);

    Label ptt_l;
    create_label(&ptt_l, "MUTE", (Coords){ 132, 19 }, KONEXY_120, 20, FONT_ALIGN_CENTER);

    Threshold motor_ranges[] = {
        { 0.0f, 89.9f, 0x00FF00, 0x000000 },
        { 90.0f, 99.9f, 0xFFFF00, 0x000000 },
        { 100.0f, 1000.0f, 0xFF0000, 0x000000 }
    };
    Thresholds motor_thresholds[] = {
        { motor_ranges, 3 }
    };

    Value motor_temp_v;
    create_value(&motor_temp_v, 0, false, (Coords){ 88, 20 }, KONEXY_120, 40, FONT_ALIGN_CENTER, (Colors){ .thresholds = motor_thresholds }, THRESHOLDS);
    Label motor_temp_l;
    create_label(&motor_temp_l, "MT", (Coords){ 123, 50 }, KONEXY_120, 25, FONT_ALIGN_CENTER);

    Threshold soc_ranges[] = {
        { 0.0f, 20.0f, 0x00FF00, 0x000000 },
        { 20.1f, 40.0f, 0xFFFF00, 0x000000 },
        { 40.1f, 1000.0f, 0xFF0000, 0x000000 }
    };
    Thresholds soc_thresholds[] = {
        { soc_ranges, 3 }
    };
    Value soc_hv_v;
    create_value(&soc_hv_v, 0, false, (Coords){ 133, 60 }, KONEXY_120, 80, FONT_ALIGN_CENTER, (Colors){ .thresholds = soc_thresholds }, THRESHOLDS);
    Label soc_hv_l;
    create_label(&soc_hv_l, "HV", (Coords){ 133, 304 }, KONEXY_120, 25, FONT_ALIGN_CENTER);

    Value soc_lv_v;
    create_value(&soc_lv_v, 0, false, (Coords){ 89, 20 }, KONEXY_120, 40, FONT_ALIGN_CENTER, (Colors){ .thresholds = soc_thresholds }, THRESHOLDS);
    Label soc_lv_l;
    create_label(&soc_lv_l, "LV", (Coords){ 221, 50 }, KONEXY_120, 25, FONT_ALIGN_CENTER);

    Threshold inv_ranges[] = {
        { 0.0f, 66.9f, 0x00FF00, 0x000000 },
        { 67.0f, 72.9f, 0xFFFF00, 0x000000 },
        { 73.1f, 1000.0f, 0xFF0000, 0x000000 }
    };
    Thresholds inv_thresholds[] = {
        { inv_ranges, 3 }
    };

    Value inv_temp_v;
    create_value(&inv_temp_v, 0, false, (Coords){ 89, 20 }, KONEXY_120, 40, FONT_ALIGN_CENTER, (Colors){ .thresholds = inv_thresholds }, THRESHOLDS);
    Label inv_temp_l;
    create_label(&inv_temp_l, "INV", (Coords){ 221, 50 }, KONEXY_120, 25, FONT_ALIGN_CENTER);

    Threshold hv_temp_ranges[] = {
        { 0.0f, 51.9f, 0x00FF00, 0x000000 },
        { 52.0f, 59.9f, 0xFFFF00, 0x000000 },
        { 60.1f, 1000.0f, 0xFF0000, 0x000000 }
    };
    Thresholds hv_temp_thresholds[] = {
        { hv_temp_ranges, 3 }
    };

    Value hv_temp_v;
    create_value(&hv_temp_v, 0, false, (Coords){ 89, 20 }, KONEXY_120, 40, FONT_ALIGN_CENTER, (Colors){ .thresholds = hv_temp_thresholds }, THRESHOLDS);
    Label hv_temp_l;
    create_label(&hv_temp_l, "HV", (Coords){ 221, 50 }, KONEXY_120, 25, FONT_ALIGN_CENTER);

    Label cooling_l;
    create_label(&cooling_l, "", (Coords){ 132, 45 }, KONEXY_120, 35, FONT_ALIGN_CENTER);

    Label regen_l;
    create_label(&regen_l, "R", (Coords){ 43, 50 }, KONEXY_120, 25, FONT_ALIGN_CENTER);

    Label slip_l;
    create_label(&slip_l, "S", (Coords){ 43, 50 }, KONEXY_120, 25, FONT_ALIGN_CENTER);

    Label torque_l;
    create_label(&torque_l, "T", (Coords){ 43, 50 }, KONEXY_120, 25, FONT_ALIGN_CENTER);

    Value lap_time_v;
    create_value(&lap_time_v, 0, true, (Coords){ 89, 25 }, KONEXY_120, 35, FONT_ALIGN_CENTER, (Colors){ .thresholds = NULL }, 0);
    Label lap_time_l;
    create_label(&lap_time_l, "LAP", (Coords){ 221, 50 }, KONEXY_120, 25, FONT_ALIGN_CENTER);

    Box tmp[] = {
        { 1, swoc_sd, (Rect){ 2, 2, 264, 88 }, 0xff000000, 0xffffffff, &shutdown_l, NULL },
        { 1, swoc_status, (Rect){ 268, 2, 266, 88 }, 0xff000000, 0xffffffff, &status_l, NULL },
        { 1, swoc_ptt, (Rect){ 536, 2, 264, 88 }, 0xff000000, 0xffffffff, &ptt_l, NULL },
        { 1, swoc_temp_mot, (Rect){ 2, 92, 264, 128 }, 0xff000000, 0xffffffff, &motor_temp_l, &motor_temp_v },
        { 1, swoc_soc_hv, (Rect){ 268, 92, 266, 258 }, 0xff000000, 0xffffffff, &soc_hv_l, &soc_hv_v },
        { 1, swoc_soc_lv, (Rect){ 536, 92, 264, 128 }, 0xff000000, 0xffffffff, &soc_lv_l, &soc_lv_v },
        { 1, swoc_temp_inv, (Rect){ 2, 222, 264, 128 }, 0xff000000, 0xffffffff, &inv_temp_l, &inv_temp_v },
        { 1, swoc_temp_hv, (Rect){ 536, 222, 264, 128 }, 0xff000000, 0xffffffff, &hv_temp_l, &hv_temp_v },
        { 1, swoc_pt_cooling, (Rect){ 2, 352, 264, 128 }, 0xff000000, 0xffffffff, &cooling_l, NULL },
        { 1, swoc_regen, (Rect){ 268, 352, 87, 128 }, 0xff000000, 0xffffffff, &regen_l, NULL },
        { 1, swoc_slip, (Rect){ 257, 352, 87, 128 }, 0xff000000, 0xffffffff, &slip_l, NULL },
        { 1, swoc_torque, (Rect){ 447, 352, 87, 128 }, 0xff000000, 0xffffffff, &torque_l, NULL },
        { 1, swoc_lap_time, (Rect){ 536, 352, 264, 128 }, 0xff000000, 0xffffffff, &lap_time_l, &lap_time_v }
    };

    memcpy(sw_screen, &tmp, sizeof(Box));
}
