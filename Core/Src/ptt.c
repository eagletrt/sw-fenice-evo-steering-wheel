#include "ptt.h"

bool ptt_status = false;
bool ptt_button_pressed = false;
extern I2C_HandleTypeDef hi2c4;

void set_ptt_button_pressed(bool pressed) { ptt_button_pressed = pressed; }

void ptt_tasks_fn(lv_timer_t *timer) {
  UNUSED(timer);
  if (!ptt_status && ptt_button_pressed) {
    primary_set_ptt_status_converted_t converted = {0};
    converted.status = primary_set_ptt_status_status_ON;
    STEER_CAN_PACK(primary, PRIMARY, set_ptt_status, SET_PTT_STATUS);
    can_send(&msg, true);
  } else if (ptt_status && !ptt_button_pressed) {
    primary_set_ptt_status_converted_t converted = {0};
    converted.status = primary_set_ptt_status_status_OFF;
    STEER_CAN_PACK(primary, PRIMARY, set_ptt_status, SET_PTT_STATUS);
    can_send(&msg, true);
  } else {
    // do nothing
  }
}

void show_ptt_off(void) {
  display_notification("PTT OFF", 400);
  led_control_set(&hi2c4, (uint32_t[6]){COLOR_OFF, COLOR_OFF, COLOR_OFF,
                                            COLOR_OFF, COLOR_OFF, COLOR_OFF});
}

void show_ptt_on(void) {
  display_notification("PTT ON", 400);
  led_control_set(&hi2c4, (uint32_t[6]){COLOR_GREEN, COLOR_GREEN, COLOR_GREEN,
                                    COLOR_OFF, COLOR_OFF, COLOR_OFF});
}

void handle_ptt_message(primary_ptt_status_status val) {
  if (val == primary_ptt_status_status_OFF) {
    if (ptt_status) {
      show_ptt_off();
    }
    ptt_status = 0;
  } else if (val == primary_ptt_status_status_ON) {
    if (!ptt_status) {
      show_ptt_on();
    }
    ptt_status = 1;
  }
}
