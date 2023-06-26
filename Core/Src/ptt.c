#include "ptt.h"

bool status = 0;

void activate_ptt(void) {
  can_message_t msg = {0};
  msg.id = PRIMARY_SET_PTT_STATUS_FRAME_ID;
  msg.size = PRIMARY_SET_PTT_STATUS_BYTE_SIZE;
  msg.data[0] = primary_set_ptt_status_status_ON;
  can_send(&msg, &hfdcan1);
}

void deactivate_ptt(void) {
  can_message_t msg = {0};
  msg.id = PRIMARY_SET_PTT_STATUS_FRAME_ID;
  msg.size = PRIMARY_SET_PTT_STATUS_BYTE_SIZE;
  msg.data[0] = primary_set_ptt_status_status_OFF;
  can_send(&msg, &hfdcan1);
}

void handle_ptt_message(primary_ptt_status_status val) {
  if (val == primary_ptt_status_status_OFF) {
    if (status)
      display_notification("Push-To-Talk OFF");
    status = 0;
  } else if (val == primary_ptt_status_status_ON) {
    if (!status)
      display_notification("Push-To-Talk ON");
    status = 1;
  }
}