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
