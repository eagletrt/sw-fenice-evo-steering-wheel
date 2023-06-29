#include "inputs/inputs.h"

#define BUTTONS_N 8
#define MANETTINI_N 3
#define BUTTONS_LONG_PRESS_TIME 1500

MCP23017_HandleTypeDef dev1;
MCP23017_HandleTypeDef dev2;

bool buttons[BUTTONS_N] = {true};
uint32_t buttons_long_press_check[BUTTONS_N] = {0};
bool buttons_long_press_activated[BUTTONS_N] = {false};
uint8_t manettini[MANETTINI_N] = {0};
uint32_t manettini_last_change;
bool tson_button_pressed = false;
lv_timer_t *send_tson_long_press_delay = NULL;

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

void buttons_pressed_actions(uint8_t button) {
  switch (button) {
  case PADDLE_TOP_RIGHT:
    change_tab(true);
    break;
  case PADDLE_TOP_LEFT:
    change_tab(false);
    break;
  case PADDLE_BOTTOM_RIGHT:
    break;
  case PADDLE_BOTTOM_LEFT:
    break;
  case BUTTON_TOP_RIGHT:
    turnon_telemetry();
    break;
  case BUTTON_TOP_LEFT:
    print("ACTIVATE ptt\n");
    activate_ptt();
    break;
  case BUTTON_BOTTOM_RIGHT:
    print("Display notification\n");
    display_notification("test notification popup", 500);
    break;
  case BUTTON_BOTTOM_LEFT:
    break;
  }
}

void buttons_released_actions(uint8_t button) {
  switch (button) {
  case PADDLE_TOP_RIGHT:
    break;
  case PADDLE_TOP_LEFT:
    break;
  case PADDLE_BOTTOM_RIGHT:
    break;
  case PADDLE_BOTTOM_LEFT:
    break;
  case BUTTON_TOP_RIGHT:
    break;
  case BUTTON_TOP_LEFT:
    print("DEACTIVATE ptt\n");
    deactivate_ptt();
    break;
  case BUTTON_BOTTOM_RIGHT:
    break;
  case BUTTON_BOTTOM_LEFT:
    break;
  }
}

void buttons_long_pressed_actions(uint8_t button) {
  switch (button) {
  case PADDLE_TOP_RIGHT:
    break;
  case PADDLE_TOP_LEFT:
    break;
  case PADDLE_BOTTOM_RIGHT:
    break;
  case PADDLE_BOTTOM_LEFT:
    break;
  case BUTTON_TOP_RIGHT:
    break;
  case BUTTON_TOP_LEFT:
    break;
  case BUTTON_BOTTOM_RIGHT:
    break;
  case BUTTON_BOTTOM_LEFT:
    break;
  }
}

void manettino_right_actions(uint8_t val) {
  uint8_t possible_vals[] = MANETTINO_RIGHT_VALS;
  for (uint8_t ival = 0; ival < (sizeof(possible_vals) / sizeof(uint8_t));
       ++ival) {
    if (val == MANETTINO_DEBOUNCE_VALUE)
      continue;
    if (val == possible_vals[ival]) {
      print("manettino right step %u\n", (unsigned int)ival);
      break;
    }
  }
}

void manettino_center_actions(uint8_t val) {
  uint8_t possible_vals[] = MANETTINO_CENTER_VALS;
  for (uint8_t ival = 0; ival < (sizeof(possible_vals) / sizeof(uint8_t));
       ++ival) {
    if (val == MANETTINO_DEBOUNCE_VALUE)
      continue;
    if (val == possible_vals[ival]) {
      print("manettino center step %u\n", (unsigned int)ival);
      break;
    }
  }
}

void manettino_left_actions(uint8_t val) {
  uint8_t possible_vals[] = MANETTINO_LEFT_VALS;
  for (uint8_t ival = 0; ival < (sizeof(possible_vals) / sizeof(uint8_t));
       ++ival) {
    if (val == possible_vals[ival]) {
      print("manettino left step %u\n", (unsigned int)ival);
      break;
    }
  }
}

void from_gpio_to_buttons(uint8_t gpio) {
  uint8_t mapping[8] = BUTTON_MAPPING;
  for (int i = 0; i < BUTTONS_N; i++) {
    uint8_t current_button_val = ((gpio >> mapping[i]) & 1);
    if (buttons[i] != current_button_val) {
      if (current_button_val == 0) {
        // button pressed action
        buttons_long_press_activated[i] = false;
        buttons_long_press_check[i] = HAL_GetTick();
        buttons_pressed_actions(i);
      } else {
        // button released action
        buttons_released_actions(i);
      }
      buttons[i] = current_button_val;
    }
    if (!buttons[i] &&
        HAL_GetTick() > buttons_long_press_check[i] + BUTTONS_LONG_PRESS_TIME &&
        !buttons_long_press_activated[i]) {
      buttons_long_pressed_actions(i);
      buttons_long_press_activated[i] = true;
    }
  }
}

void turnon_telemetry(void) {
  print("Sending Telemetry ON\n");
  can_message_t msg = {0};
  msg.id = PRIMARY_SET_TLM_STATUS_FRAME_ID;
  msg.size = PRIMARY_SET_TLM_STATUS_BYTE_SIZE;
  msg.data[0] = primary_set_tlm_status_tlm_status_ON;
  can_send(&msg, &hfdcan1);
}

void send_tson(void) {
  print("Sending TSON\n");
#if 0
  can_message_t msg = {0};
  msg.id = PRIMARY_SET_CAR_STATUS_FRAME_ID;
  msg.size = PRIMARY_SET_CAR_STATUS_BYTE_SIZE;
  msg.data[0] = primary_set_car_status_car_status_set_READY;
  can_send(&msg, &hfdcan1);
#endif
}

void send_tson_check(lv_timer_t *tim) {
  if (tson_button_pressed) {
    print("TSON TIMER: sending TSON\n");
    display_notification("Sent TSON", 1500);
  } else {
    print("TSON TIMER: not sending tson\n");
  }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
  if (GPIO_Pin == ExtraButton_Pin) {
    GPIO_PinState tson_pin_state =
        HAL_GPIO_ReadPin(ExtraButton_GPIO_Port, ExtraButton_Pin);
    if (tson_pin_state == GPIO_PIN_RESET && !tson_button_pressed) {
      print("Setting timer to check button state in 2 seconds\n");
      tson_button_pressed = true;
      send_tson_long_press_delay = lv_timer_create(send_tson_check, 1000, NULL);
      lv_timer_set_repeat_count(send_tson_long_press_delay, 1);
      lv_timer_reset(send_tson_long_press_delay);
    } else {
      print("tson button released and possible timer deleted\n");
      tson_button_pressed = false;
      lv_timer_set_repeat_count(send_tson_long_press_delay, 0);
    }
  }
}

void read_buttons(void) {
  uint8_t button_input;
  if (HAL_I2C_Mem_Read(&hi2c4, MCP23017_DEV1_ADDR << 1, REGISTER_GPIOB, 1,
                       &button_input, 1, 100) != HAL_OK) {
    print("Error\n");
  }
  from_gpio_to_buttons(button_input);
  dev1.gpio[1] = button_input;
}

void read_manettino_1(void) {
  uint8_t manettino_input;
  if (HAL_I2C_Mem_Read(&hi2c4, MCP23017_DEV1_ADDR << 1, REGISTER_GPIOA, 1,
                       &manettino_input, 1, 100) != HAL_OK) {
    print("Error\n");
  }
  if (manettino_input != dev1.gpio[0]) {
    manettino_left_actions(manettino_input);
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
    manettino_center_actions(manettino_input);
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
    manettino_right_actions(manettino_input);
  }
  manettini[2] = manettino_input;
  dev2.gpio[0] = manettino_input;
}

void read_inputs() {
  read_buttons();
  if (HAL_GetTick() - manettini_last_change > MANETTINO_DEBOUNCE) {
    manettini_last_change = HAL_GetTick();
    read_manettino_1();
    read_manettino_2();
    read_manettino_3();
  }
}
