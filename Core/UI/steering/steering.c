#include "steering.h"

uint8_t primary_messages_last_state[primary_MESSAGE_COUNT][primary_MAX_STRUCT_SIZE_CONVERSION];
uint8_t secondary_messages_last_state[secondary_MESSAGE_COUNT][secondary_MAX_STRUCT_SIZE_CONVERSION];
uint8_t inverters_messages_last_state[inverters_MESSAGE_COUNT][inverters_MAX_STRUCT_SIZE_CONVERSION];

bool is_pmsg_valid[primary_MESSAGE_COUNT];
bool is_smsg_valid[secondary_MESSAGE_COUNT];
bool is_imsg_valid[inverters_MESSAGE_COUNT];

bool is_pmsg_new[primary_MESSAGE_COUNT];
bool is_smsg_new[secondary_MESSAGE_COUNT];
bool is_imsg_new[inverters_MESSAGE_COUNT];

primary_lv_cells_voltage_converted_t lv_voltages_stock_1 = {0};
primary_lv_cells_voltage_converted_t lv_voltages_stock_2 = {0};
primary_lv_cells_temp_converted_t lv_temps_stock_1       = {0};
primary_lv_cells_temp_converted_t lv_temps_stock_2       = {0};

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
