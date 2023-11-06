#include "graphics_manager.h"

lv_color_t *framebuffer_1 = (lv_color_t *)FRAMEBUFFER1_ADDR;
lv_color_t *framebuffer_2 = (lv_color_t *)FRAMEBUFFER2_ADDR;

// TODO
// lv_timer_t *shutdown_circuit_task =
// lv_timer_create(update_shutdown_circuit_ui, 100, NULL); lv_timer_t *ptt_tasks
// = lv_timer_create(ptt_tasks_fn, 1000, NULL);

bool is_pmsg_new[primary_MESSAGE_COUNT];
bool is_smsg_new[secondary_MESSAGE_COUNT];
bool is_imsg_new[inverters_MESSAGE_COUNT];

void init_graphics_manager(void) {
  lv_init();
  screen_driver_init();
  tab_manager();
}

void refresh_graphics(void) { lv_tasks(); }

void update_graphics(void) {
  for (uint16_t iindex = 0; iindex < primary_MESSAGE_COUNT; iindex++) {
    if (is_pmsg_new[iindex]) {
      can_id_t id = primary_id_from_index(iindex);
      switch (id) {

      default:
        break;
      }
    }
  }

  for (uint16_t iindex = 0; iindex < inverters_MESSAGE_COUNT; iindex++) {
    if (is_imsg_new[iindex]) {
      can_id_t id = inverters_id_from_index(iindex);
      switch (id) {

      default:
        break;
      }
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
      case SECONDARY_TIMESTAMP_FRAME_ID: {
        timestamp_update();
      }
      default:
        break;
      }
    }
  }
}
