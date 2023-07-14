#include "ptt.h"

bool status = 0;
extern I2C_HandleTypeDef hi2c4;

void activate_ptt(void) {
  primary_set_ptt_status_converted_t converted = {0};
  converted.status = primary_set_ptt_status_status_ON;
  STEER_CAN_PACK(primary, PRIMARY, set_ptt_status, SET_PTT_STATUS);
  can_send(&msg, &hfdcan1);
  led_control_set(&hi2c4, (uint32_t[6]){COLOR_RED, COLOR_RED, COLOR_RED,
                                          COLOR_RED, COLOR_RED, COLOR_RED});
}

void deactivate_ptt(void) {
  primary_set_ptt_status_converted_t converted = {0};
  converted.status = primary_set_ptt_status_status_OFF;
  STEER_CAN_PACK(primary, PRIMARY, set_ptt_status, SET_PTT_STATUS);
  can_send(&msg, &hfdcan1);
}

void handle_ptt_message(primary_ptt_status_status val) {
  if (val == primary_ptt_status_status_OFF) {
    if (status) {
      display_notification("PTT OFF", 400);
      led_control_init(&hi2c4);
        led_control_set(&hi2c4, (uint32_t[6]){COLOR_OFF, COLOR_OFF, COLOR_OFF,
                                          COLOR_OFF, COLOR_OFF, COLOR_OFF});
    }
    status = 0;
    print("Received status OFF");
  } else if (val == primary_ptt_status_status_ON) {
    if (!status) {
      display_notification("PTT ON", 400);
      led_control_init(&hi2c4);
      led_control_set(&hi2c4, (uint32_t[6]){COLOR_GREEN, COLOR_GREEN, COLOR_GREEN,
                                            COLOR_GREEN, COLOR_GREEN, COLOR_GREEN});
    }
    status = 1;
    print("Received status ON");
  }
}