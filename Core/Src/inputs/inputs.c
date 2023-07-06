#include "inputs/inputs.h"

MCP23017_HandleTypeDef dev1;
MCP23017_HandleTypeDef dev2;

bool buttons[BUTTONS_N] = {true};
uint32_t buttons_long_press_check[BUTTONS_N] = {0};
bool buttons_long_press_activated[BUTTONS_N] = {false};

uint8_t manettini[MANETTINI_N] = {0};
uint32_t manettini_last_change;
bool manettini_initialized[MANETTINI_N] = {false};

bool tson_button_pressed = false;
lv_timer_t *send_set_car_status_long_press_delay = NULL;
bool calibration_min_sent_request[CALBOX_N];
bool calibration_max_sent_request[CALBOX_N];
uint32_t calibration_min_request_timestamp[CALBOX_N];
uint32_t calibration_max_request_timestamp[CALBOX_N];

extern tab_t current_tab;
extern lv_obj_t *set_min_btn;
extern lv_obj_t *set_max_btn;

extern primary_tlm_status_t tlm_status_last_message;
extern primary_car_status_t car_status_last_message;
extern primary_steer_status_converted_t steer_status_last_message;
extern primary_cooling_status_converted_t cooling_status_last_message;

// const static uint8_t button_mapping[BUTTONS_N] = BUTTON_MAPPING;
const static uint8_t manettino_right_possible_vals[BUTTONS_N] =
    MANETTINO_RIGHT_VALS;
const static uint8_t manettino_center_possible_vals[BUTTONS_N] =
    MANETTINO_CENTER_VALS;
const static uint8_t manettino_left_possible_vals[BUTTONS_N] =
    MANETTINO_LEFT_VALS;

const static float val_power_map_mapping[MANETTINO_STEPS_N] = POWER_MAP_MAPPING;
const static float val_torque_map_index[MANETTINO_STEPS_N] = TORQUE_MAP_MAPPING;
const static float val_slip_map_index[MANETTINO_STEPS_N] = SLIP_MAP_MAPPING;

void calibration_tool_set_min_max(bool maxv);

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

// TORQUE
void manettino_right_actions(uint8_t val) {
  for (uint8_t ival = 0;
       ival < (sizeof(manettino_right_possible_vals) / sizeof(uint8_t));
       ++ival) {
    if (val == MANETTINO_DEBOUNCE_VALUE)
      continue;
    if (val == manettino_right_possible_vals[ival]) {
      steer_status_last_message.map_tv = (float)val_torque_map_index[ival];
      char title[100];
      uint16_t map_val = (uint16_t)(steer_status_last_message.map_tv * 100.0f);
      sprintf(title, "%u", map_val);
      STEER_UPDATE_LABEL(steering.control.lb_torque, title)
      sprintf(title, "TORQUE VECTORING %u", map_val);
      print("%s\n", title);
      display_notification(title, 750);
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
      steer_status_last_message.map_pw = (float)val_power_map_mapping[ival];
      uint16_t map_val = (uint16_t)(steer_status_last_message.map_pw * 100.0f);
      char title[100];
      sprintf(title, "%u", map_val);
      STEER_UPDATE_LABEL(steering.control.lb_power, title)
      sprintf(title, "POWER MAP %u", map_val);
      print("%s\n", title);
      display_notification(title, 750);
      break;
    }
  }
}

// SLIP CONTROL
void manettino_left_actions(uint8_t val) {
  for (uint8_t ival = 0;
       ival < (sizeof(manettino_left_possible_vals) / sizeof(uint8_t));
       ++ival) {
    if (val == manettino_left_possible_vals[ival]) {
      steer_status_last_message.map_sc = (float)val_slip_map_index[ival];
      char title[100];
      uint16_t map_val = (uint16_t)(steer_status_last_message.map_sc * 100.0f);
      sprintf(title, "%u", map_val);
      STEER_UPDATE_LABEL(steering.control.lb_slip, title)
      sprintf(title, "SLIP CONTROL %u", map_val);
      print("%s\n", title);
      display_notification(title, 750);
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

void turn_telemetry_on_off(void) {
  primary_set_tlm_status_converted_t converted = {0};
  if (tlm_status_last_message.tlm_status ==
      (primary_set_tlm_status_tlm_status) primary_set_tlm_status_tlm_status_ON) {
    print("Sending Telemetry OFF\n");
    converted.tlm_status = primary_set_tlm_status_tlm_status_OFF;
  } else {
    print("Sending Telemetry ON\n");
    converted.tlm_status = primary_set_tlm_status_tlm_status_ON;
  }
  STEER_CAN_PACK(primary, PRIMARY, set_tlm_status, SET_TLM_STATUS);
  can_send(&msg, &hfdcan1);
}

void pedal_calibration_ack(primary_pedal_calibration_ack_converted_t *data) {
  primary_pedal_calibration_ack_bound bound = data->bound;

  lv_obj_set_style_bg_color(bound == primary_pedal_calibration_ack_bound_SET_MAX
                                ? set_max_btn
                                : set_min_btn,
                            lv_color_hex(COLOR_GREEN_STATUS_HEX), LV_PART_MAIN);
}

void send_calibration(bool accel, bool max) {
  primary_set_pedal_calibration_converted_t converted = {0};
  converted.pedal = accel ? primary_set_pedal_calibration_pedal_ACCELERATOR
                          : primary_set_pedal_calibration_pedal_BRAKE;
  converted.bound = max ? primary_set_pedal_calibration_bound_SET_MAX
                        : primary_set_pedal_calibration_bound_SET_MIN;
  STEER_CAN_PACK(primary, PRIMARY, set_pedal_calibration,
                 SET_PEDAL_CALIBRATION);
  can_send(&msg, &hfdcan1);
}

void calibration_request_timeout_check(uint32_t current_time) {
  for (uint8_t iel = 0; iel < CALBOX_N; ++iel) {
    if (calibration_min_sent_request[iel] &&
        calibration_min_request_timestamp[iel] + CALIBRATION_TIMEOUT_RESPONSE <
            current_time) {
      calibration_min_sent_request[iel] = false;
      lv_obj_set_style_bg_color(set_min_btn, lv_color_hex(COLOR_RED_STATUS_HEX),
                                LV_PART_MAIN);
    }
    if (calibration_max_sent_request[iel] &&
        calibration_max_request_timestamp[iel] + CALIBRATION_TIMEOUT_RESPONSE <
            current_time) {
      calibration_max_sent_request[iel] = false;
      lv_obj_set_style_bg_color(set_min_btn, lv_color_hex(COLOR_RED_STATUS_HEX),
                                LV_PART_MAIN);
    }
  }
}

void calibration_tool_set_min_max(bool maxv) {
  if (current_tab == TAB_CALIBRATION) {
    calibration_box_t curr_focus = steering.curr_focus;
    if (curr_focus == STEER)
      return;
    switch (curr_focus) {
    case BSE: {
      send_calibration(false, maxv);
      if (maxv) {
        calibration_max_sent_request[BSE] = true;
        calibration_max_request_timestamp[BSE] = HAL_GetTick();
      } else {
        calibration_min_sent_request[BSE] = true;
        calibration_min_request_timestamp[BSE] = HAL_GetTick();
      }
      break;
    }
    case APPS: {
      send_calibration(true, maxv);
      if (maxv) {
        calibration_max_sent_request[APPS] = true;
        calibration_max_request_timestamp[APPS] = HAL_GetTick();
      } else {
        calibration_min_sent_request[APPS] = true;
        calibration_min_request_timestamp[APPS] = HAL_GetTick();
      }
      break;
    }
    default:
      return;
    }
    lv_obj_set_style_bg_color(maxv ? set_max_btn : set_min_btn,
                              lv_color_hex(COLOR_ORANGE_STATUS_HEX),
                              LV_PART_MAIN);
  }
}

void send_set_car_status(void) {
  primary_set_car_status_converted_t converted = {0};
  switch (car_status_last_message.car_status) {
  case primary_car_status_car_status_IDLE: {
    converted.car_status_set = primary_set_car_status_car_status_set_READY;
    display_notification("TSON", 1500);
    print("Sending SET CAR STATUS: TSON\n");
    break;
  }
  case primary_car_status_car_status_WAIT_DRIVER: {
    converted.car_status_set = primary_set_car_status_car_status_set_DRIVE;
    display_notification("DRIVE", 1500);
    print("Sending SET CAR STATUS: DRIVE\n");
    break;
  }
  default: {
    converted.car_status_set = primary_set_car_status_car_status_set_IDLE;
    display_notification("IDLE", 1500);
    print("Sending SET CAR STATUS: IDLE\n");
    break;
  }
  }
  STEER_CAN_PACK(primary, PRIMARY, set_car_status, SET_CAR_STATUS);
  can_send(&msg, &hfdcan1);
}

void send_set_car_status_check(lv_timer_t *tim) {
  if (tson_button_pressed) {
    print("TSON TIMER: sending CAR STATUS SET\n");
    send_set_car_status();
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
      send_set_car_status_long_press_delay =
          lv_timer_create(send_set_car_status_check, 1000, NULL);
      lv_timer_set_repeat_count(send_set_car_status_long_press_delay, 1);
      lv_timer_reset(send_set_car_status_long_press_delay);
    } else {
      print("tson button released and possible timer deleted\n");
      tson_button_pressed = false;
      lv_timer_set_repeat_count(send_set_car_status_long_press_delay, 0);
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
