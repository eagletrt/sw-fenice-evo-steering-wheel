#include "ptt.h"

typedef enum {
    ptt_status_OFF     = 0,
    ptt_status_SET_ON  = 1,
    ptt_status_ON      = 2,
    ptt_status_SET_OFF = 3,
} ptt_status_t;

bool ecu_ack            = false;
bool ptt_button_pressed = false;
ptt_status_t ptt_status = ptt_status_OFF;

void set_ptt_button_pressed(bool val) {
    ptt_button_pressed = val;
}

void send_ptt_status_message(bool on) {
    primary_ecu_set_ptt_status_converted_t converted = {0};
    converted.status                                 = on ? primary_ecu_set_ptt_status_status_on : primary_ecu_set_ptt_status_status_off;
    STEER_CAN_PACK(primary, PRIMARY, ecu_set_ptt_status, ECU_SET_PTT_STATUS);
    can_send(&msg, true);
}

void handle_ptt_message(void) {
    GET_LAST_STATE(primary, ecu_ptt_status, PRIMARY, ECU_PTT_STATUS);
    primary_ecu_ptt_status_status val = primary_ecu_ptt_status_last_state->status;
    if (val == primary_ecu_ptt_status_status_off) {
        ecu_ack = false;
    } else if (val == primary_ecu_ptt_status_status_on) {
        ecu_ack = true;
    }
}

#ifdef ENDURANCE_MODE_ENABLED

void ptt_tasks_fn(lv_timer_t *timer) {
#warning To be implemented
}

#else

void ptt_tasks_fn(lv_timer_t *timer) {
    if (!ecu_ack && ptt_button_pressed) {
        ptt_status = ptt_status_SET_ON;
        send_ptt_status_message(true);
        update_sensors_extra_value("SON", 0);
        set_tab_racing_label_text("SON", tab_rac_ptt_status_idx);
        set_tab_racing_ptt_label_color(true);
    } else if (ecu_ack && !ptt_button_pressed) {
        ptt_status = ptt_status_SET_OFF;
        send_ptt_status_message(false);
        update_sensors_extra_value("SOF", 0);
        set_tab_racing_label_text("SOF", tab_rac_ptt_status_idx);
        set_tab_racing_ptt_label_color(false);
    } else if (ecu_ack && ptt_button_pressed) {
        ptt_status = ptt_status_ON;
        send_ptt_status_message(true);
        update_sensors_extra_value("ON", 0);
        set_tab_racing_label_text("ON", tab_rac_ptt_status_idx);
        set_tab_racing_ptt_label_color(true);
    } else if (!ecu_ack && !ptt_button_pressed) {
        ptt_status = ptt_status_OFF;
        send_ptt_status_message(false);
        update_sensors_extra_value("OFF", 0);
        set_tab_racing_label_text("OFF", tab_rac_ptt_status_idx);
        set_tab_racing_ptt_label_color(false);
    }
}

#endif  // ENDURANCE_MODE_ENABLED
