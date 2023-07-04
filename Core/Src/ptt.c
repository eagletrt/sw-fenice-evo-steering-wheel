#include "ptt.h"

bool status = 0;

void activate_ptt(void) {
  primary_set_ptt_status_converted_t converted = {0};
  converted.status = primary_set_ptt_status_status_ON;
  STEER_CAN_PACK(primary, PRIMARY, set_ptt_status, SET_PTT_STATUS);
  can_send(&msg, &hfdcan1);
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
      // LED
    }
    status = 0;
    print("Received status OFF");
  } else if (val == primary_ptt_status_status_ON) {
    if (!status) {
      // LED
    }
    status = 1;
    print("Received status ON");
  }
}