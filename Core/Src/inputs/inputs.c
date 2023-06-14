#include "inputs/inputs.h"

#define BUTTONS_N 8
#define MANETTINI_N 3

MCP23017_HandleTypeDef dev1;
MCP23017_HandleTypeDef dev2;

bool buttons[BUTTONS_N];
uint8_t manettini[MANETTINI_N];

void configure_internal_pull_up_resistors() {
  uint8_t cdata = 0xFF;
  mcp23017_write(&dev1, REGISTER_GPPUA, &cdata);
  mcp23017_write(&dev1, REGISTER_GPPUB, &cdata);
  mcp23017_write(&dev2, REGISTER_GPPUA, &cdata);
  mcp23017_write(&dev2, REGISTER_GPPUB, &cdata);

#define CHECK_ERROR(cdata)                                                     \
  if (cdata != 0xFF) {                                                         \
    Error_Handler();                                                           \
  }
  mcp23017_read(&dev1, REGISTER_GPPUA, &cdata);
  CHECK_ERROR(cdata);
  mcp23017_read(&dev1, REGISTER_GPPUB, &cdata);
  CHECK_ERROR(cdata);
  mcp23017_read(&dev2, REGISTER_GPPUA, &cdata);
  CHECK_ERROR(cdata);
  mcp23017_read(&dev2, REGISTER_GPPUB, &cdata);
  CHECK_ERROR(cdata);
}

void inputs_init(void) {
  dev1 = (MCP23017_HandleTypeDef){
      .addr = MCP23017_DEV1_ADDR, .hi2c = &hi2c4, .gpio = {0}};
  dev2 = (MCP23017_HandleTypeDef){
      .addr = MCP23017_DEV2_ADDR, .hi2c = &hi2c4, .gpio = {0}};
  mcp23017_init(&dev1);
  mcp23017_init(&dev2);
  configure_internal_pull_up_resistors();
}

void print_buttons(void) {
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

void send_tson() {
  // TS ON
  can_message_t msg = {0};
  msg.id = PRIMARY_SET_CAR_STATUS_FRAME_ID;
  msg.size = PRIMARY_SET_CAR_STATUS_BYTE_SIZE;
  msg.data[0] = primary_set_car_status_car_status_set_READY;
  can_send(&msg, &hfdcan1);
}

void actions(void) {
  if (buttons[0]) {
  }

  if (buttons[1]) {
  }

  if (buttons[2]) {
  }

  if (buttons[3]) {
  }

  if (buttons[4]) {
  }

  if (buttons[5]) {
  }

  if (buttons[6]) {
  }

  if (buttons[7]) {
  }
}

void read_buttons(void) {
  uint8_t button_input;
  if (HAL_I2C_Mem_Read(&hi2c4, MCP23017_DEV1_ADDR << 1, REGISTER_GPIOB, 1,
                       &button_input, 1, 100) != HAL_OK) {
    print("Error\n");
  }
  from_gpio_to_buttons(button_input);
  if (button_input != dev1.gpio[1]) {
    print_buttons();
    actions();
  }
  dev1.gpio[1] = button_input;
}

void read_manettino_1(void) {
  uint8_t manettino_input;
  if (HAL_I2C_Mem_Read(&hi2c4, MCP23017_DEV1_ADDR << 1, REGISTER_GPIOA, 1,
                       &manettino_input, 1, 100) != HAL_OK) {
    print("Error\n");
  }
  if (manettino_input != dev1.gpio[0]) {
    print("Manettino 1: %d\n", manettino_input);
  }
  manettini[0] = manettino_input;
  dev1.gpio[0] = manettino_input;
}

void read_manettino_2(void) {
  uint8_t manettino_input;
  if (HAL_I2C_Mem_Read(&hi2c4, MCP23017_DEV2_ADDR << 1, REGISTER_GPIOB, 1,
                       &manettino_input, 1, 100) != HAL_OK) {
    print("Error\n");
  }
  if (manettino_input != dev2.gpio[1]) {
    print("Manettino 2: %d\n", manettino_input);
  }
  manettini[1] = manettino_input;
  dev2.gpio[1] = manettino_input;
}

void read_manettino_3(void) {
  uint8_t manettino_input;
  if (HAL_I2C_Mem_Read(&hi2c4, MCP23017_DEV2_ADDR << 1, REGISTER_GPIOA, 1,
                       &manettino_input, 1, 100) != HAL_OK) {
    print("Error\n");
  }
  if (manettino_input != dev2.gpio[0]) {
    print("Manettino 3: %d\n", manettino_input);
  }
  manettini[2] = manettino_input;
  dev2.gpio[0] = manettino_input;
}

void test_change_IOCON() {
  print("----------------\n");
  print("Change IOCON bank 0x0A register\n");
  uint8_t cdata[1];
  mcp23017_read(&dev1, 0x0A, cdata);
  print("Before writing: %u\n", (uint32_t)cdata[0]);

  cdata[0] |= 0b10000000;
  mcp23017_write(&dev1, 0x05, cdata);

  mcp23017_read(&dev1, 0x05, cdata);
  print("After writing: %u\n", (uint32_t)cdata[0]);
  print("----------------\n");
}

void read_inputs(void) {
  read_buttons();
  read_manettino_1();
  read_manettino_2();
  read_manettino_3();
}
