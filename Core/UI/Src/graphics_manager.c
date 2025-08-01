#include "graphics_manager.h"

#define OLIVEC_IMPLEMENTATION
#include "olive.c"

extern Olivec_Canvas *oc;

uint8_t *framebuffer_1 = (uint8_t *)FRAMEBUFFER1_ADDR;
uint8_t *framebuffer_2 = (uint8_t *)FRAMEBUFFER2_ADDR;

void sw_update_graphics_from_can_messages(UI_t *scr) {
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
                    // hv_soc_estimation_update(scr, is_smsg_valid[iindex]);
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

void sw_init_screen(UI_t *sw_screen) {
    // TODO SIMULATOR: initialize directly the fields of sw_screen without the temporary variable
    UI_t tmp = (UI_t){
        .oc = { 0 },
        .components = {
            (UI_elem_t){ // SHUTDOWN STATUS
                         .swoc_elem_was_updated = true,
                         .swoc_elem_label = "",
                         .swoc_elem_lb_color = 0xFFFFFFFF,
                         .swoc_elem_bg_color = 0xFF000000,
                         .swoc_elem_font_size = 0.4,
                         .swoc_elem_boundaries = (Olivec_Boundaries){ 2, 2, 264, 88 },
                         .swoc_elem_margins = (Olivec_Margins){ 132, 30 },
                         .swoc_elem_align = MF_ALIGN_CENTER },
            (UI_elem_t){ // LAP TIME
                         .swoc_elem_was_updated = true,
                         .swoc_elem_label = "",
                         .swoc_elem_lb_color = 0xFFFFFFFF,
                         .swoc_elem_bg_color = 0xFF000000,
                         .swoc_elem_font_size = 0.5,
                         .swoc_elem_boundaries = (Olivec_Boundaries){ 268, 2, 266, 88 },
                         .swoc_elem_margins = (Olivec_Margins){ 132, 19 },
                         .swoc_elem_align = MF_ALIGN_CENTER },
            (UI_elem_t){ // PTT
                         .swoc_elem_was_updated = true,
                         .swoc_elem_label = "MUTE",
                         .swoc_elem_lb_color = 0xFFFFFFFF,
                         .swoc_elem_bg_color = 0xFF000000,
                         .swoc_elem_font_size = 0.5,
                         .swoc_elem_boundaries = (Olivec_Boundaries){ 536, 2, 264, 88 },
                         .swoc_elem_margins = (Olivec_Margins){ 132, 19 },
                         .swoc_elem_align = MF_ALIGN_CENTER },
            (UI_elem_t){ // MOTOR TEMP
                         .swoc_elem_was_updated = true,
                         .swoc_elem_label = "",
                         .swoc_elem_lb_color = 0xFFFFFFFF,
                         .swoc_elem_bg_color = 0xFF000000,
                         .swoc_elem_font_size = 1.0,
                         .swoc_elem_boundaries = (Olivec_Boundaries){ 2, 92, 178, 128 },
                         .swoc_elem_margins = (Olivec_Margins){ 88, 20 },
                         .swoc_elem_align = MF_ALIGN_CENTER },
            (UI_elem_t){ // MOTOR TEMP LABEL
                         .swoc_elem_was_updated = true,
                         .swoc_elem_label = "MT",
                         .swoc_elem_lb_color = 0xFFFFFFFF,
                         .swoc_elem_bg_color = 0xFF000000,
                         .swoc_elem_font_size = 0.4,
                         .swoc_elem_boundaries = (Olivec_Boundaries){ 180, 92, 86, 128 },
                         .swoc_elem_margins = (Olivec_Margins){ 43, 50 },
                         .swoc_elem_align = MF_ALIGN_CENTER },
            (UI_elem_t){ // SOC HV
                         .swoc_elem_was_updated = true,
                         .swoc_elem_label = "",
                         .swoc_elem_lb_color = 0xFFFFFFFF,
                         .swoc_elem_bg_color = 0xFF000000,
                         .swoc_elem_font_size = 1.4,
                         .swoc_elem_boundaries = (Olivec_Boundaries){ 268, 92, 266, 194 },
                         .swoc_elem_margins = (Olivec_Margins){ 133, 60 },
                         .swoc_elem_align = MF_ALIGN_CENTER },
            (UI_elem_t){ // SOC LV
                         .swoc_elem_was_updated = true,
                         .swoc_elem_label = "",
                         .swoc_elem_lb_color = 0xFFFFFFFF,
                         .swoc_elem_bg_color = 0xFF000000,
                         .swoc_elem_font_size = 1.0,
                         .swoc_elem_boundaries = (Olivec_Boundaries){ 536, 92, 178, 128 },
                         .swoc_elem_margins = (Olivec_Margins){ 89, 20 },
                         .swoc_elem_align = MF_ALIGN_CENTER },
            (UI_elem_t){ // SOC LV LABEL
                         .swoc_elem_was_updated = true,
                         .swoc_elem_label = "LV",
                         .swoc_elem_lb_color = 0xFFFFFFFF,
                         .swoc_elem_bg_color = 0xFF000000,
                         .swoc_elem_font_size = 0.4,
                         .swoc_elem_boundaries = (Olivec_Boundaries){ 714, 92, 86, 128 },
                         .swoc_elem_margins = (Olivec_Margins){ 43, 50 },
                         .swoc_elem_align = MF_ALIGN_CENTER },
            (UI_elem_t){ // INVERTER TEMP
                         .swoc_elem_was_updated = true,
                         .swoc_elem_label = "",
                         .swoc_elem_lb_color = 0xFFFFFFFF,
                         .swoc_elem_bg_color = 0xFF000000,
                         .swoc_elem_font_size = 1.0,
                         .swoc_elem_boundaries = (Olivec_Boundaries){ 2, 222, 178, 128 },
                         .swoc_elem_margins = (Olivec_Margins){ 89, 20 },
                         .swoc_elem_align = MF_ALIGN_CENTER },
            (UI_elem_t){ // INVERTER TEMP LABEL
                         .swoc_elem_was_updated = true,
                         .swoc_elem_label = "INV",
                         .swoc_elem_lb_color = 0xFFFFFFFF,
                         .swoc_elem_bg_color = 0xFF000000,
                         .swoc_elem_font_size = 0.4,
                         .swoc_elem_boundaries = (Olivec_Boundaries){ 180, 222, 86, 128 },
                         .swoc_elem_margins = (Olivec_Margins){ 43, 50 },
                         .swoc_elem_align = MF_ALIGN_CENTER },
            (UI_elem_t){ // SOC HV LABEL
                         .swoc_elem_was_updated = true,
                         .swoc_elem_label = "kmh",
                         .swoc_elem_lb_color = 0xFFFFFFFF,
                         .swoc_elem_bg_color = 0xFF000000,
                         .swoc_elem_font_size = 0.5,
                         .swoc_elem_boundaries = (Olivec_Boundaries){ 268, 286, 266, 64 },
                         .swoc_elem_margins = (Olivec_Margins){ 133, 10 },
                         .swoc_elem_align = MF_ALIGN_CENTER },
            (UI_elem_t){ // HV TEMP
                         .swoc_elem_was_updated = true,
                         .swoc_elem_label = "",
                         .swoc_elem_lb_color = 0xFFFFFFFF,
                         .swoc_elem_bg_color = 0xFF000000,
                         .swoc_elem_font_size = 1.0,
                         .swoc_elem_boundaries = (Olivec_Boundaries){ 536, 222, 178, 128 },
                         .swoc_elem_margins = (Olivec_Margins){ 89, 20 },
                         .swoc_elem_align = MF_ALIGN_CENTER },
            (UI_elem_t){ // HV TEMP LABEL
                         .swoc_elem_was_updated = true,
                         .swoc_elem_label = "HV",
                         .swoc_elem_lb_color = 0xFFFFFFFF,
                         .swoc_elem_bg_color = 0xFF000000,
                         .swoc_elem_font_size = 0.4,
                         .swoc_elem_boundaries = (Olivec_Boundaries){ 714, 222, 86, 128 },
                         .swoc_elem_margins = (Olivec_Margins){ 43, 50 },
                         .swoc_elem_align = MF_ALIGN_CENTER },
            (UI_elem_t){ // PT COOLING
                         .swoc_elem_was_updated = true,
                         .swoc_elem_label = "",
                         .swoc_elem_lb_color = 0xFFFFFFFF,
                         .swoc_elem_bg_color = 0xFF000000,
                         .swoc_elem_font_size = 1.0,
                         .swoc_elem_boundaries = (Olivec_Boundaries){ 2, 352, 178, 128 },
                         .swoc_elem_margins = (Olivec_Margins){ 89, 20 },
                         .swoc_elem_align = MF_ALIGN_CENTER },
            (UI_elem_t){ // PT COOLING LABEL
                         .swoc_elem_was_updated = true,
                         .swoc_elem_label = "PT",
                         .swoc_elem_lb_color = 0xFFFFFFFF,
                         .swoc_elem_bg_color = 0xFF000000,
                         .swoc_elem_font_size = 0.4,
                         .swoc_elem_boundaries = (Olivec_Boundaries){ 180, 352, 86, 128 },
                         .swoc_elem_margins = (Olivec_Margins){ 43, 50 },
                         .swoc_elem_align = MF_ALIGN_CENTER },
            (UI_elem_t){ // REGEN STATUS
                         .swoc_elem_was_updated = true,
                         .swoc_elem_label = "",
                         .swoc_elem_lb_color = 0xFFFFFFFF,
                         .swoc_elem_bg_color = 0xFF000000,
                         .swoc_elem_font_size = 0.4,
                         .swoc_elem_boundaries = (Olivec_Boundaries){ 268, 352, 87, 128 },
                         .swoc_elem_margins = (Olivec_Margins){ 43, 50 },
                         .swoc_elem_align = MF_ALIGN_CENTER },
            (UI_elem_t){ // SLIP STATUS
                         .swoc_elem_was_updated = true,
                         .swoc_elem_label = "",
                         .swoc_elem_lb_color = 0xFFFFFFFF,
                         .swoc_elem_bg_color = 0xFF000000,
                         .swoc_elem_font_size = 0.4,
                         .swoc_elem_boundaries = (Olivec_Boundaries){ 357, 352, 88, 128 },
                         .swoc_elem_margins = (Olivec_Margins){ 44, 50 },
                         .swoc_elem_align = MF_ALIGN_CENTER },
            (UI_elem_t){ // TORQUE STATUS
                         .swoc_elem_was_updated = true,
                         .swoc_elem_label = "",
                         .swoc_elem_lb_color = 0xFFFFFFFF,
                         .swoc_elem_bg_color = 0xFF000000,
                         .swoc_elem_font_size = 0.4,
                         .swoc_elem_boundaries = (Olivec_Boundaries){ 447, 352, 87, 128 },
                         .swoc_elem_margins = (Olivec_Margins){ 43, 50 },
                         .swoc_elem_align = MF_ALIGN_CENTER },
            (UI_elem_t){ // HV COOLING
                         .swoc_elem_was_updated = true,
                         .swoc_elem_label = "",
                         .swoc_elem_lb_color = 0xFFFFFFFF,
                         .swoc_elem_bg_color = 0xFF000000,
                         .swoc_elem_font_size = 0.6,
                         .swoc_elem_boundaries = (Olivec_Boundaries){ 536, 352, 178, 128 },
                         .swoc_elem_margins = (Olivec_Margins){ 89, 20 },
                         .swoc_elem_align = MF_ALIGN_CENTER },
            (UI_elem_t){ // HV COOLING LABEL
                         .swoc_elem_was_updated = true,
                         .swoc_elem_label = "LAP",
                         .swoc_elem_lb_color = 0xFFFFFFFF,
                         .swoc_elem_bg_color = 0xFF000000,
                         .swoc_elem_font_size = 0.4,
                         .swoc_elem_boundaries = (Olivec_Boundaries){ 714, 352, 86, 128 },
                         .swoc_elem_margins = (Olivec_Margins){ 43, 50 },
                         .swoc_elem_align = MF_ALIGN_CENTER },
        }
    };
    memcpy(sw_screen, &tmp, sizeof(UI_t));
}

void sw_set_canvas(UI_t *scr, uint32_t *pixels, size_t width, size_t height, size_t stride) {
    scr->oc = olivec_canvas(pixels, SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_WIDTH);
    oc = &scr->oc;
}

void sw_update_screen(float dt, UI_t *scr) {
    olivec_fix_dma2d(scr->oc);
    for (size_t iswoc = 0; iswoc < swoc_elems_n; iswoc++) {
        scr->components[iswoc].swoc_elem_was_updated = false;
        olivec_dma2d_rect(
            scr->oc,
            scr->components[iswoc].swoc_elem_boundaries.x,
            scr->components[iswoc].swoc_elem_boundaries.y,
            scr->components[iswoc].swoc_elem_boundaries.w,
            scr->components[iswoc].swoc_elem_boundaries.h,
            scr->components[iswoc].swoc_elem_bg_color);
        olivec_text(
            scr->oc,
            scr->components[iswoc].swoc_elem_label,
            scr->components[iswoc].swoc_elem_boundaries.x + scr->components[iswoc].swoc_elem_margins.x,
            scr->components[iswoc].swoc_elem_boundaries.y + scr->components[iswoc].swoc_elem_margins.y +
                (iswoc == 0 ? (scr->components[iswoc].swoc_elem_font_size > 0.2f ? -10 : 10) : 0),
            scr->components[iswoc].swoc_elem_lb_color,
            scr->components[iswoc].swoc_elem_font_size,
            scr->components[iswoc].swoc_elem_align);
    }
}

void sw_screen_white(UI_t *scr) {
    olivec_dma2d_rect(scr->oc, 0, 0, 800, 480, 0xFFFFFFFF);
}

void sw_print_debug_info(UI_t *scr, char *text1, char *text2, float size1, float size2) {
    scr->components[18].swoc_elem_was_updated = false;
    olivec_dma2d_rect(
        scr->oc,
        scr->components[18].swoc_elem_boundaries.x,
        scr->components[18].swoc_elem_boundaries.y,
        scr->components[18].swoc_elem_boundaries.w,
        scr->components[18].swoc_elem_boundaries.h,
        scr->components[18].swoc_elem_bg_color);
    olivec_text(
        scr->oc,
        text1,
        scr->components[18].swoc_elem_boundaries.x + scr->components[18].swoc_elem_margins.x,
        scr->components[18].swoc_elem_boundaries.y + scr->components[18].swoc_elem_margins.y,
        scr->components[18].swoc_elem_lb_color,
        size1,
        scr->components[18].swoc_elem_align);

    scr->components[19].swoc_elem_was_updated = false;
    olivec_dma2d_rect(
        scr->oc,
        scr->components[19].swoc_elem_boundaries.x,
        scr->components[19].swoc_elem_boundaries.y,
        scr->components[19].swoc_elem_boundaries.w,
        scr->components[19].swoc_elem_boundaries.h,
        scr->components[19].swoc_elem_bg_color);
    olivec_text(
        scr->oc,
        text2,
        scr->components[19].swoc_elem_boundaries.x + scr->components[19].swoc_elem_margins.x,
        scr->components[19].swoc_elem_boundaries.y + scr->components[19].swoc_elem_margins.y,
        scr->components[19].swoc_elem_lb_color,
        size2,
        scr->components[19].swoc_elem_align);
}
