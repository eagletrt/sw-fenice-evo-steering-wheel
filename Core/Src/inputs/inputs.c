#include "inputs/inputs.h"

MCP23017_HandleTypeDef dev1;
MCP23017_HandleTypeDef dev2;

bool buttons[BUTTONS_N] = {true};
uint32_t buttons_long_press_check[BUTTONS_N] = {0};
bool buttons_long_press_activated[BUTTONS_N] = {false};

uint8_t manettini[MANETTINI_N] = {0};
uint32_t manettini_last_change;
bool manettini_initialized[MANETTINI_N] = {false};

extern bool tson_button_pressed;
lv_timer_t *send_set_car_status_long_press_delay = NULL;

extern bool engineer_mode;

/***
 * Manettini mapping
 *
 */
const static uint8_t manettino_right_possible_vals[BUTTONS_N] =
    MANETTINO_RIGHT_VALS;
const static uint8_t manettino_center_possible_vals[BUTTONS_N] =
    MANETTINO_CENTER_VALS;
const static uint8_t manettino_left_possible_vals[BUTTONS_N] =
    MANETTINO_LEFT_VALS;

const static float val_power_map_mapping[MANETTINO_STEPS_N] = POWER_MAP_MAPPING;
const static float val_torque_map_index[MANETTINO_STEPS_N] = TORQUE_MAP_MAPPING;
const static float val_slip_map_index[MANETTINO_STEPS_N] = SLIP_MAP_MAPPING;
const static float val_pumps_speed_index[MANETTINO_STEPS_N] = PUMPS_MAPPING;
const static float val_radiators_speed_index[MANETTINO_STEPS_N] =
    RADIATORS_MAPPING;


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
  case PADDLE_TOP_RIGHT: {
    if (!engineer_mode)
      change_tab(true);
    break;
  }
  case PADDLE_TOP_LEFT: {
    if (!engineer_mode)
      change_tab(false);
    break;
  }
  case PADDLE_BOTTOM_RIGHT:
    print("SHIFT BOX FOCUS RIGHT\n");
    shift_box_focus(true);
    change_errors_view(false);
    break;
  case PADDLE_BOTTOM_LEFT:
    print("SHIFT BOX FOCUS LEFT\n");
    shift_box_focus(false);
    change_errors_view(true);
    break;
  case BUTTON_TOP_RIGHT:
    print("TURN TELEMETRY ON/OFF\n");
    turn_telemetry_on_off();
    break;
  case BUTTON_TOP_LEFT:
    print("ACTIVATE ptt\n");
    activate_ptt();
    break;
  case BUTTON_BOTTOM_RIGHT:
    print("CALIBRATION TOOL: SET MAX\n");
    calibration_tool_set_min_max(true);
    break;
  case BUTTON_BOTTOM_LEFT:
    print("CALIBRATION TOOL: SET MIN\n");
    calibration_tool_set_min_max(false);
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
    switch_mode();
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


// TORQUE | RADIATORS
void manettino_right_actions(uint8_t val) {
  for (uint8_t ival = 0;
       ival < (sizeof(manettino_right_possible_vals) / sizeof(uint8_t));
       ++ival) {
    if (val == MANETTINO_DEBOUNCE_VALUE)
      continue;
    if (val == manettino_right_possible_vals[ival]) {
      if (!engineer_mode)
        manettino_send_torque_vectoring(val_torque_map_index[ival]);
      else
        manettino_send_set_radiators(val_radiators_speed_index[ival]);
      break;
    }
  }
}

// POWER MAP
void manettino_center_actions(uint8_t val) {
  for (uint8_t ival = 0;
       ival < (sizeof(manettino_center_possible_vals) / sizeof(uint8_t));
       ++ival) {
    if (val == MANETTINO_DEBOUNCE_VALUE)
      continue;
    if (val == manettino_center_possible_vals[ival]) {
      manettino_send_power_map(val_power_map_mapping[ival]);
      break;
    }
  }
}

// SLIP CONTROL | COOLING
void manettino_left_actions(uint8_t val) {
  for (uint8_t ival = 0;
       ival < (sizeof(manettino_left_possible_vals) / sizeof(uint8_t));
       ++ival) {
    if (val == manettino_left_possible_vals[ival]) {
      if (!engineer_mode)
        manettino_send_slip_control(val_slip_map_index[ival]);
      else
        manettino_send_set_pumps_speed(val_pumps_speed_index[ival]);
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

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
  if (GPIO_Pin == ExtraButton_Pin) {
    GPIO_PinState tson_pin_state =
        HAL_GPIO_ReadPin(ExtraButton_GPIO_Port, ExtraButton_Pin);
      if (tson_pin_state == GPIO_PIN_RESET && !tson_button_pressed) {
        print("Setting timer to check button state in 2 seconds\n");
        tson_button_pressed = true;
        send_set_car_status_long_press_delay =
            lv_timer_create(send_set_car_status_check, 1000, NULL);
        lv_timer_set_repeat_count(send_set_car_status_long_press_delay, 1);
        lv_timer_reset(send_set_car_status_long_press_delay);
        STEER_UPDATE_COLOR_LABEL(steering.das.lb_speed, COLOR_ORANGE_STATUS_HEX)
      } else {
        print("tson button released and possible timer deleted\n");
        tson_button_pressed = false;
        lv_timer_set_repeat_count(send_set_car_status_long_press_delay, 0);
        STEER_UPDATE_COLOR_LABEL(steering.das.lb_speed, COLOR_TERTIARY_HEX)
      }
  }
}

void read_buttons(void) {
  uint8_t button_input;
  if (HAL_I2C_Mem_Read(&hi2c4, MCP23017_DEV1_ADDR << 1, REGISTER_GPIOB, 1,
                       &button_input, 1, 100) != HAL_OK) {
    print("Error\n");
    return;
  }
  from_gpio_to_buttons(button_input);
  dev1.gpio[1] = button_input;
}

void read_manettino_1(void) {
  uint8_t manettino_input;
  if (HAL_I2C_Mem_Read(&hi2c4, MCP23017_DEV1_ADDR << 1, REGISTER_GPIOA, 1,
                       &manettino_input, 1, 100) != HAL_OK) {
    print("Error\n");
    return;
  }
  if (manettino_input != dev1.gpio[0] && manettini_initialized[0]) {
    manettino_left_actions(manettino_input);
  }
  manettini_initialized[0] = true;
  manettini[0] = manettino_input;
  dev1.gpio[0] = manettino_input;
}

void read_manettino_2(void) {
  uint8_t manettino_input;
  if (HAL_I2C_Mem_Read(&hi2c4, MCP23017_DEV2_ADDR << 1, REGISTER_GPIOB, 1,
                       &manettino_input, 1, 100) != HAL_OK) {
    print("Error\n");
    return;
  }
  if (manettino_input != dev2.gpio[1] && manettini_initialized[1]) {
    manettino_center_actions(manettino_input);
  }
  manettini_initialized[1] = true;
  manettini[1] = manettino_input;
  dev2.gpio[1] = manettino_input;
}

void read_manettino_3(void) {
  uint8_t manettino_input;
  if (HAL_I2C_Mem_Read(&hi2c4, MCP23017_DEV2_ADDR << 1, REGISTER_GPIOA, 1,
                       &manettino_input, 1, 100) != HAL_OK) {
    print("Error\n");
    return;
  }
  if (manettino_input != dev2.gpio[0] && manettini_initialized[2]) {
    manettino_right_actions(manettino_input);
  }
  manettini_initialized[2] = true;
  manettini[2] = manettino_input;
  dev2.gpio[0] = manettino_input;
}

void read_inputs(lv_timer_t *tim) {
  UNUSED(tim);
  read_buttons();
  if (HAL_GetTick() - manettini_last_change > MANETTINO_DEBOUNCE) {
    manettini_last_change = HAL_GetTick();
    read_manettino_1();
    read_manettino_2();
    read_manettino_3();
  }
}
