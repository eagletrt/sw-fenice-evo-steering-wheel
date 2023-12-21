#include "graphics_manager.h"

lv_color_t *framebuffer_1 = (lv_color_t *)FRAMEBUFFER1_ADDR;
lv_color_t *framebuffer_2 = (lv_color_t *)FRAMEBUFFER2_ADDR;

bool is_pmsg_new[primary_MESSAGE_COUNT];
bool is_smsg_new[secondary_MESSAGE_COUNT];
bool is_imsg_new[inverters_MESSAGE_COUNT];

void init_graphics_manager(void) {
  lv_init();
#ifdef STM32H723xx
  screen_driver_init();
#endif
  tab_manager();
}

void refresh_graphics(void) {
#ifdef STM32H723xx
#if LV_TICK_CUSTOM == 1
  lv_timer_handler();
#else
  uint32_t ctick = HAL_GetTick();
  lv_tick_inc(ctick - *ptick);
  *ptick = ctick;
#endif
#endif
}

void update_graphics(lv_timer_t *t) {
  for (uint16_t iindex = 0; iindex < primary_MESSAGE_COUNT; iindex++) {
    if (is_pmsg_new[iindex]) {
      is_pmsg_new[iindex] = false;
      can_id_t id = primary_id_from_index(iindex);
      switch (id) {
      case PRIMARY_CAR_STATUS_FRAME_ID: {
        car_status_update();
        break;
      }
      case PRIMARY_COOLING_STATUS_FRAME_ID: {
        cooling_status_update();
        break;
      }
      case PRIMARY_SPEED_FRAME_ID: {
        speed_update();
        break;
      }
      case PRIMARY_TS_STATUS_FRAME_ID: {
        // ts_status_update();
        break;
      }
      case PRIMARY_HV_VOLTAGE_FRAME_ID: {
        hv_voltage_update();
        break;
      }
      case PRIMARY_HV_CURRENT_FRAME_ID: {
        hv_current_update();
        break;
      }
      case PRIMARY_HV_TEMP_FRAME_ID: {
        hv_temp_update();
        break;
      }
      case PRIMARY_HV_ERRORS_FRAME_ID: {
        hv_errors_update();
        break;
      }
      case PRIMARY_HV_CELL_BALANCING_STATUS_FRAME_ID: {
        hv_cell_balancing_status_update();
        break;
      }
      case PRIMARY_LV_FEEDBACKS_FRAME_ID: {
        lv_feedbacks_update();
        break;
      }
      case PRIMARY_HV_FANS_OVERRIDE_STATUS_FRAME_ID: {
        hv_fans_override_status_update();
        break;
      }
      case PRIMARY_TLM_STATUS_FRAME_ID: {
        tlm_status_update();
        break;
      }
      case PRIMARY_DAS_ERRORS_FRAME_ID: {
        das_errors_update();
        break;
      }
      case PRIMARY_LV_CURRENTS_FRAME_ID: {
        lv_currents_update();
        break;
      }
      case PRIMARY_LV_CELLS_TEMP_FRAME_ID: {
        lv_cells_temp_update();
        break;
      }
      case PRIMARY_LV_CELLS_VOLTAGE_FRAME_ID: {
        lv_cells_voltage_update();
        break;
      }
      case PRIMARY_LV_TOTAL_VOLTAGE_FRAME_ID: {
        lv_total_voltage_update();
        break;
      }
      case PRIMARY_LV_ERRORS_FRAME_ID: {
        lv_errors_update();
        break;
      }
      case PRIMARY_HV_CELL_VOLTAGE_FRAME_ID: {
        hv_cell_voltage_update();
        break;
      }
      case PRIMARY_HV_FEEDBACK_STATUS_FRAME_ID: {
        hv_feedbacks_status_update();
        debug_hv_feedbacks_status_update();
        break;
      }
      default:
        break;
      }
      is_pmsg_new[iindex] = false;
    }
  }

  for (uint16_t iindex = 0; iindex < inverters_MESSAGE_COUNT; iindex++) {
    if (is_imsg_new[iindex]) {
      can_id_t id = inverters_id_from_index(iindex);
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
      is_pmsg_new[iindex] = false;
    }
  }

  for (uint16_t iindex = 0; iindex < secondary_MESSAGE_COUNT; iindex++) {
    if (is_smsg_new[iindex]) {
      can_id_t id = secondary_id_from_index(iindex);
      switch (id) {
      case SECONDARY_STEERING_ANGLE_FRAME_ID: {
        steering_angle_update();
        break;
      }
      case SECONDARY_PEDALS_OUTPUT_FRAME_ID: {
        pedals_output_update();
        break;
      }
      case SECONDARY_IMU_ACCELERATION_FRAME_ID: {
        imu_acceleration_update();
        break;
      }
      case SECONDARY_LAP_COUNT_FRAME_ID: {
        lap_count_update();
        break;
      }
      case SECONDARY_LC_STATUS_FRAME_ID: {
        lc_status_update();
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

      default:
        break;
      }
      is_pmsg_new[iindex] = false;
    }
  }
}
