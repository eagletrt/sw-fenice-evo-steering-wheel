#include "graphics_manager.h"

#include "src/core/lv_obj.h"
#include "src/core/lv_obj_pos.h"

lv_color_t *framebuffer_1 = (lv_color_t *)FRAMEBUFFER1_ADDR;
lv_color_t *framebuffer_2 = (lv_color_t *)FRAMEBUFFER2_ADDR;

// extern primary_watchdog m_primary_watchdog;
// extern secondary_watchdog m_secondary_watchdog;
// extern inverters_watchdog m_inverters_watchdog;

bool is_pmsg_new[primary_MESSAGE_COUNT];
bool is_smsg_new[secondary_MESSAGE_COUNT];
bool is_imsg_new[inverters_MESSAGE_COUNT];

void init_graphics_manager(void) {
    lv_init();
#ifdef STM32H723xx
    screen_driver_init();
#endif
    tab_manager();

    // lv_obj_t * label1 = lv_label_create(NULL);
    // lv_label_set_long_mode(label1, LV_LABEL_LONG_WRAP);     /*Break the long lines*/
    // lv_label_set_text(label1, "Recolor is not supported for v9 now.");
    // lv_obj_set_width(label1, 150);  /*Set smaller width to make the lines wrap*/
    // lv_obj_set_style_text_align(label1, LV_TEXT_ALIGN_CENTER, 0);
    // lv_obj_align(label1, LV_ALIGN_CENTER, 0, -40);
    // lv_scr_load(label1);
}

void refresh_graphics(void) {
#ifdef STM32H723xx
    lv_timer_handler();
#endif
}

void update_graphics(lv_timer_t *t) {
    if (engineer_mode) {
        switch (current_engineer_tab) {
            case STEERING_WHEEL_TAB_PRIMARY_CANSNIFFER:
                update_primary_cansniffer_ui();
                break;
            case STEERING_WHEEL_TAB_SECONDARY_CANSNIFFER:
                update_secondary_cansniffer_ui();
                break;
            case STEERING_WHEEL_TAB_INVERTERS_CANSNIFFER:
                update_inverters_cansniffer_ui();
                break;
            default:
                break;
        }
    }

    for (uint16_t iindex = 0; iindex < primary_MESSAGE_COUNT; iindex++) {
        if (is_pmsg_new[iindex]) {
            is_pmsg_new[iindex] = false;
            can_id_t id         = primary_id_from_index(iindex);
            switch (id) {
                case PRIMARY_ECU_STATUS_FRAME_ID: {
                    car_status_update();
                    break;
                }
                case PRIMARY_ECU_FEEDBACKS_FRAME_ID: {
                    ecu_feedbacks_update();
                    break;
                }
                case PRIMARY_ECU_ERRORS_FRAME_ID: {
                    ecu_errors_update();
                    break;
                }
                case PRIMARY_TLM_STATUS_FRAME_ID: {
                    tlm_status_update();
                    break;
                }
                case PRIMARY_HV_STATUS_FRAME_ID: {
                    hv_status_update();
                    break;
                }
                case PRIMARY_HV_DEBUG_SIGNALS_FRAME_ID: {
                    hv_debug_signals_update();
                    break;
                }
                case PRIMARY_HV_TOTAL_VOLTAGE_FRAME_ID: {
                    hv_total_voltage_update();
                    break;
                }
                case PRIMARY_HV_CURRENT_FRAME_ID: {
                    hv_current_update();
                    break;
                }
                case PRIMARY_HV_CELLS_TEMP_STATS_FRAME_ID: {
                    hv_cells_temp_stats_update();
                    break;
                }
                case PRIMARY_HV_ERRORS_FRAME_ID: {
                    hv_errors_update();
                    break;
                }
                case PRIMARY_HV_FEEDBACK_TS_VOLTAGE_FRAME_ID: {
                    hv_feedback_ts_voltage_update();
                    break;
                }
                case PRIMARY_HV_FEEDBACK_MISC_VOLTAGE_FRAME_ID: {
                    hv_feedback_misc_voltage_update();
                    break;
                }
                case PRIMARY_HV_FEEDBACK_SD_VOLTAGE_FRAME_ID: {
                    hv_feedback_sd_voltage_update();
                    break;
                }
                case PRIMARY_HV_BALANCING_STATUS_FRAME_ID: {
                    hv_cell_balancing_status_update();
                    break;
                }
                case PRIMARY_HV_FANS_STATUS_FRAME_ID: {
                    hv_fans_override_status_update();
                    break;
                }
                case PRIMARY_HV_CELLS_VOLTAGE_STATS_FRAME_ID: {
                    hv_cells_voltage_stats_update();
                    break;
                }
                case PRIMARY_HV_FEEDBACK_STATUS_FRAME_ID: {
                    hv_feedbacks_status_update();
                    debug_hv_feedbacks_status_update();
                    break;
                }
                case PRIMARY_LV_FEEDBACK_SD_VOLTAGE_FRAME_ID: {
                    lv_feedback_sd_voltage_update();
                    break;
                }
                case PRIMARY_LV_FEEDBACK_TS_VOLTAGE_FRAME_ID: {
                    lv_feedback_ts_voltage_update();
                    break;
                }
                case PRIMARY_LV_FEEDBACK_ENCLOSURE_VOLTAGE_FRAME_ID: {
                    lv_feedback_enclosure_voltage_update();
                    break;
                }
                case PRIMARY_LV_ERRORS_FRAME_ID: {
                    lv_errors_update();
                    break;
                }
                case PRIMARY_LV_RADIATOR_SPEED_FRAME_ID: {
                    lv_radiator_speed_update();
                    break;
                }
                case PRIMARY_LV_PUMPS_SPEED_FRAME_ID: {
                    lv_pumps_speed_update();
                    break;
                }
                case PRIMARY_LV_CURRENT_BATTERY_FRAME_ID: {
                    lv_currents_update();
                    break;
                }
                case PRIMARY_LV_TOTAL_VOLTAGE_FRAME_ID: {
                    lv_total_voltage_update();
                    break;
                }
                case PRIMARY_LV_CELLS_VOLTAGE_FRAME_ID: {
                    lv_cells_voltage_update();
                    break;
                }
                case PRIMARY_LV_CELLS_VOLTAGE_STATS_FRAME_ID: {
                    lv_cells_voltage_stats_update();
                    break;
                }
                case PRIMARY_LV_CELLS_TEMP_FRAME_ID: {
                    lv_cells_temp_update();
                    break;
                }
                case PRIMARY_LV_CELLS_TEMP_STATS_FRAME_ID: {
                    lv_cells_temp_stats_update();
                    break;
                }
                case PRIMARY_ECU_VERSION_FRAME_ID: {
                    ecu_version_update();
                    break;
                }
                case PRIMARY_LV_VERSION_FRAME_ID: {
                    lv_version_update();
                    break;
                }
                case PRIMARY_HV_CELLBOARD_VERSION_FRAME_ID: {
                    hv_cellboard_version_update();
                    break;
                }
                case PRIMARY_HV_MAINBOARD_VERSION_FRAME_ID: {
                    hv_mainboard_version_update();
                    break;
                }
                case PRIMARY_TLM_VERSION_FRAME_ID: {
                    tlm_version_update();
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
                    inv_l_rcv_update();
                    break;
                case INVERTERS_INV_R_RCV_FRAME_ID:
                    inv_r_rcv_update();
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
                case SECONDARY_ANGULAR_VELOCITY_FRAME_ID: {
                    angular_velocity_update();
                    break;
                }
                case SECONDARY_STEER_ANGLE_FRAME_ID: {
                    steer_angle_update();
                    break;
                }
                case SECONDARY_VEHICLE_SPEED_FRAME_ID: {
                    vehicle_speed_update();
                    break;
                }
                case SECONDARY_PEDAL_THROTTLE_FRAME_ID: {
                    pedal_throttle_update();
                    break;
                }
                case SECONDARY_PEDAL_BRAKES_PRESSURE_FRAME_ID: {
                    pedal_brakes_pressure_update();
                    break;
                }
                case SECONDARY_IMU_ACCELERATION_FRAME_ID: {
                    imu_acceleration_update();
                    break;
                }
                case SECONDARY_TLM_NETWORK_INTERFACE_FRAME_ID: {
                    tlm_network_interface_update();
                    break;
                }
                case SECONDARY_IRTS_FL_0_FRAME_ID:
                case SECONDARY_IRTS_FL_1_FRAME_ID:
                case SECONDARY_IRTS_FL_2_FRAME_ID:
                case SECONDARY_IRTS_FL_3_FRAME_ID: {
                    irts_fl_update();
                    break;
                }
                case SECONDARY_IRTS_FR_0_FRAME_ID:
                case SECONDARY_IRTS_FR_1_FRAME_ID:
                case SECONDARY_IRTS_FR_2_FRAME_ID:
                case SECONDARY_IRTS_FR_3_FRAME_ID: {
                    irts_fr_update();
                    break;
                }
                case SECONDARY_IRTS_RL_0_FRAME_ID:
                case SECONDARY_IRTS_RL_1_FRAME_ID:
                case SECONDARY_IRTS_RL_2_FRAME_ID:
                case SECONDARY_IRTS_RL_3_FRAME_ID: {
                    irts_rl_update();
                    break;
                }
                case SECONDARY_IRTS_RR_0_FRAME_ID:
                case SECONDARY_IRTS_RR_1_FRAME_ID:
                case SECONDARY_IRTS_RR_2_FRAME_ID:
                case SECONDARY_IRTS_RR_3_FRAME_ID: {
                    irts_rr_update();
                    break;
                }
                case SECONDARY_ODOMETER_FRAME_ID: {
                    odometer_update();
                    break;
                }
                case SECONDARY_TLM_LAP_TIME_FRAME_ID: {
                    tlm_lap_time_update();
                    break;
                }
                case SECONDARY_TLM_LAPS_STATS_FRAME_ID: {
                    tlm_laps_stats_update();
                    break;
                }
                case SECONDARY_HV_SOC_ESTIMATION_STATE_FRAME_ID: {
                    hv_soc_estimation_update();
                    break;
                }
                case SECONDARY_LV_SOC_ESTIMATION_STATE_FRAME_ID: {
                    lv_soc_estimation_update();
                    break;
                }
                default: {
                    break;
                }
            }
        }
    }
}
