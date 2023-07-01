#include "ptt.h"

bool status = 0;

void activate_ptt(void) {
  can_message_t msg = {0};
  primary_set_ptt_status_t ptt_status = {0};
  ptt_status.status = primary_set_ptt_status_status_ON;
  msg.id = PRIMARY_SET_PTT_STATUS_FRAME_ID;
  msg.size = PRIMARY_SET_PTT_STATUS_BYTE_SIZE;
  primary_set_ptt_status_pack(msg.data, &ptt_status,
                              PRIMARY_SET_PTT_STATUS_BYTE_SIZE);
  can_send(&msg, &hfdcan1);
}

void deactivate_ptt(void) {
  can_message_t msg = {0};
  primary_set_ptt_status_t ptt_status = {0};
  ptt_status.status = primary_set_ptt_status_status_OFF;
  msg.id = PRIMARY_SET_PTT_STATUS_FRAME_ID;
  msg.size = PRIMARY_SET_PTT_STATUS_BYTE_SIZE;
  primary_set_ptt_status_pack(msg.data, &ptt_status,
                              PRIMARY_SET_PTT_STATUS_BYTE_SIZE);
  can_send(&msg, &hfdcan1);
}

void handle_ptt_message(primary_ptt_status_status val) {
  if (val == primary_ptt_status_status_OFF) {
    if (status)
      display_notification("Push-To-Talk OFF", 500);
    status = 0;
    print("Received status OFF");
  } else if (val == primary_ptt_status_status_ON) {
    if (!status)
      display_notification("Push-To-Talk ON", 500);
    status = 1;
    print("Received status ON");
  }
}