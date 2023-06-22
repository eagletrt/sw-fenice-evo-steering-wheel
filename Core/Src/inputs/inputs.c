#include "inputs/inputs.h"

#define BUTTONS_N 8
#define MANETTINI_N 3

MCP23017_HandleTypeDef dev1;
MCP23017_HandleTypeDef dev2;

bool buttons[BUTTONS_N];
uint8_t manettini[MANETTINI_N];
uint8_t tson_button = 0;
uint32_t manettini_last_change;

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

void buttons_actions(uint8_t button) {
  switch (button) {
  case 0:
    // BUTTON_0 ACTION
    print("button 0\n");
    break;
  case 1:
    // BUTTON_1 ACTION
    print("button 1\n");
    break;
  case 2:
    // BUTTON_2 ACTION
    print("button 2\n");
    break;
  case 3:
    // BUTTON_3 ACTION
    print("button 3\n");
    break;
  case 4:
    // BUTTON_4 ACTION
    print("button 4\n");
    break;
  case 5:
    // BUTTON_5 ACTION
    print("button 5\n");
    break;
  case 6:
    change_tab(false);
    break;
  case 7:
    change_tab(true);
    break;
  }
}

void from_gpio_to_buttons(uint8_t gpio) {
  uint8_t mapping[8] = BUTTON_MAPPING;
  for (int i = 0; i < BUTTONS_N; i++) {
    uint8_t current_button_val = ((gpio >> mapping[i]) & 1);
    if (buttons[i] != current_button_val) {
      if (current_button_val == 0) {
        buttons_actions(i);
      }
      buttons[i] = current_button_val;
    }
  }
}

void send_tson(void) {
  // TS ON
  can_message_t msg = {0};
  msg.id = PRIMARY_SET_CAR_STATUS_FRAME_ID;
  msg.size = PRIMARY_SET_CAR_STATUS_BYTE_SIZE;
  msg.data[0] = primary_set_car_status_car_status_set_READY;
  can_send(&msg, &hfdcan1);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
  if (GPIO_Pin == ExtraButton_Pin) {
    print("Send tson\n");
    send_tson();
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
    print("Manettino 1: %u\n", (unsigned int)manettino_input);
    // MANETTINO_1 ACTION
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
    print("Manettino 2: %u\n", (unsigned int)manettino_input);
    // MANETTINO_2 ACTION
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
    print("Manettino 3: %u\n", (unsigned int)manettino_input);
    // MANETTINO_3 ACTION
  }
  manettini[2] = manettino_input;
  dev2.gpio[0] = manettino_input;
}

void read_inputs(void) {
  read_buttons();
  if (HAL_GetTick() - manettini_last_change > MANETTINO_DEBOUNCE) {
    manettini_last_change = HAL_GetTick();
    read_manettino_1();
    read_manettino_2();
    read_manettino_3();
  }
}
