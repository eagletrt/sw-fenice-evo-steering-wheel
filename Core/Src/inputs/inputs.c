#include "inputs/inputs.h"

#define BUTTONS_N 8

MCP23017_HandleTypeDef dev1;
MCP23017_HandleTypeDef dev2;

bool buttons[BUTTONS_N];
uint8_t manettino_1;
uint8_t manettino_2;
uint8_t manettino_3;

void inputs_init() {
  dev1 = (MCP23017_HandleTypeDef){
      .addr = MCP23017_DEV1_ADDR, .hi2c = &hi2c4, .gpio = {0}};
  dev2 = (MCP23017_HandleTypeDef){
      .addr = MCP23017_DEV2_ADDR, .hi2c = &hi2c4, .gpio = {0}};
  mcp23017_init(&dev1);
  mcp23017_init(&dev2);
}

void print_buttons() {
  char buffer[100];
  uint32_t len = 0;
  len += sprintf(buffer + len, "Buttons: ");
  for (int i = 0; i < BUTTONS_N; i++) {
    len += sprintf(buffer + len, "%d ", buttons[i]);
  }
  len += sprintf(buffer + len, "\n");
  print("%s", buffer);
}

void from_gpio_to_buttons(uint8_t gpio) {
  uint8_t mapping[8] = BUTTON_MAPPING;
  for (int i = 0; i < BUTTONS_N; i++) {
    buttons[i] = (gpio >> mapping[i]) & 1;
  }
}

void read_buttons() {
  uint8_t button_input;
  if (HAL_I2C_Mem_Read(&hi2c4, MCP23017_DEV1_ADDR << 1, REGISTER_GPIOB, 1, &button_input,
                       1, 100) != HAL_OK) {
    print("Error\n");
  }
  from_gpio_to_buttons(button_input);
  if (button_input != dev1.gpio[0]) {
    print_buttons();
  }
  dev1.gpio[0] = button_input;
}
