#include "inputs/inputs.h"

MCP23017_HandleTypeDef dev1;
MCP23017_HandleTypeDef dev2;

extern bool engineer_mode;

bool buttons[BUTTONS_N] = {true};
uint32_t buttons_long_press_check[BUTTONS_N] = {0};
bool buttons_long_press_activated[BUTTONS_N] = {false};

uint8_t manettini[MANETTINI_N] = {0};
uint32_t manettini_last_change;
bool manettini_initialized[MANETTINI_N] = {false};

extern bool tson_button_pressed;
lv_timer_t *send_set_car_status_long_press_delay = NULL;

float power_map_last_state = 0.0f;

/***
 * Manettini mapping
 *
 */
const static uint8_t MANETTINO_VALS_MAPPING[MANETTINI_N][BUTTONS_N] = {
  MANETTINO_LEFT_VALS,
  MANETTINO_CENTER_VALS,
  MANETTINO_RIGHT_VALS
};

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

float min(float a, float b) {
  return a < b ? a : b;
}
float max(float a, float b) {
  return a > b ? a : b;
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

uint8_t from_manettino_value_to_index(uint8_t value, uint8_t manettino) {
  for (uint8_t ival = 0; ival < BUTTONS_N; ++ival) {
    if (value == MANETTINO_VALS_MAPPING[manettino][ival]) {
      return ival;
    }
  }
  return MANETTINO_INVALID_VALUE;
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
    bool tson_pin_state =
        HAL_GPIO_ReadPin(ExtraButton_GPIO_Port, ExtraButton_Pin) == GPIO_PIN_SET ? true : false;
      if (!tson_pin_state && !tson_button_pressed) {
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

void manettini_actions(uint8_t value, uint8_t manettino) {
  if (!manettini_initialized[manettino]) {
    manettini_initialized[manettino] = true;
    manettini[manettino] = from_manettino_value_to_index(value, manettino) == MANETTINO_INVALID_VALUE ? 0 : value;
    return;
  }
  uint8_t new_manettino_index = from_manettino_value_to_index(value, manettino);
  if (new_manettino_index == MANETTINO_INVALID_VALUE)
    return;
  switch (manettino)
  {
  case MANETTINO_RIGHT_INDEX: {
    if (!engineer_mode)
      manettino_send_torque_vectoring(val_torque_map_index[new_manettino_index]);
    else
      manettino_send_set_radiators(val_radiators_speed_index[new_manettino_index]);
    break;
  }
  case MANETTINO_CENTER_INDEX: {
    power_map_last_state += ((new_manettino_index - manettini[MANETTINO_CENTER_INDEX]) * 0.1f);
    power_map_last_state = min(power_map_last_state, 1.0f);
    power_map_last_state = max(power_map_last_state, -0.1f);
    manettino_send_power_map(power_map_last_state);
    break;
  }
  case MANETTINO_LEFT_INDEX: {
    if (!engineer_mode)
      manettino_send_slip_control(val_slip_map_index[new_manettino_index]);
    else
      manettino_send_set_pumps_speed(val_pumps_speed_index[new_manettino_index]);
    break;
  }
  }
  manettini[manettino] = new_manettino_index;
}

void read_manettino_left(void) {
  uint8_t manettino_input;
  if (HAL_I2C_Mem_Read(&hi2c4, MCP23017_DEV1_ADDR << 1, REGISTER_GPIOA, 1,
                       &manettino_input, 1, 100) != HAL_OK) {
    print("Error\n");
    return;
  }
  if (manettino_input != dev1.gpio[0]) {
    manettini_actions(manettino_input, MANETTINO_LEFT_INDEX);
    dev1.gpio[0] = manettino_input;
  }
}

void read_manettino_center(void) {
  uint8_t manettino_input;
  if (HAL_I2C_Mem_Read(&hi2c4, MCP23017_DEV2_ADDR << 1, REGISTER_GPIOB, 1,
                       &manettino_input, 1, 100) != HAL_OK) {
    print("Error\n");
    return;
  }
  if (manettino_input != dev2.gpio[1] && manettini_initialized[MANETTINO_CENTER_INDEX]) {
    manettini_actions(manettino_input, MANETTINO_CENTER_INDEX);
    dev2.gpio[1] = manettino_input;
  }
}

void read_manettino_right(void) {
  uint8_t manettino_input;
  if (HAL_I2C_Mem_Read(&hi2c4, MCP23017_DEV2_ADDR << 1, REGISTER_GPIOA, 1,
                       &manettino_input, 1, 100) != HAL_OK) {
    print("Error\n");
    return;
  }
  if (manettino_input != dev2.gpio[0] && manettini_initialized[MANETTINO_RIGHT_INDEX]) {
    manettini_actions(manettino_input, MANETTINO_RIGHT_INDEX);
    dev2.gpio[0] = manettino_input;
  }
}

void read_inputs(lv_timer_t *tim) {
  UNUSED(tim);
  read_buttons();
  if (HAL_GetTick() - manettini_last_change > MANETTINO_DEBOUNCE) {
    manettini_last_change = HAL_GetTick();
    read_manettino_left();
    read_manettino_center();
    read_manettino_right();
  }
}
