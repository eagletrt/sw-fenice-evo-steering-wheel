#include "inputs/inputs.h"

#include <stdint.h>

MCP23017_HandleTypeDef dev1;
MCP23017_HandleTypeDef dev2;

bool buttons[BUTTONS_N]                      = {true};
uint32_t buttons_long_press_check[BUTTONS_N] = {0};
bool buttons_long_press_activated[BUTTONS_N] = {false};

uint8_t manettini[MANETTINI_N] = {0};
uint32_t manettini_last_change;
bool manettini_initialized[MANETTINI_N] = {false};

bool tson_button_pressed;

int hv_fans_override_last_state = 0;

#define ERROR_THRESHOLD           100
#define BUFFER_SIZE_PRINT_BUTTONS (100u)
uint32_t inputs_error_counter = 0;
bool inputs_fatal_error       = false;

int imin(int x, int y) {
    return x > y ? y : x;
}
int imax(int x, int y) {
    return x > y ? x : y;
}

/**
 * Manettini mapping
 */
const static uint8_t MANETTINO_VALS_MAPPING[MANETTINI_N][BUTTONS_N] = {MANETTINO_LEFT_VALS, MANETTINO_CENTER_VALS, MANETTINO_RIGHT_VALS};

void configure_internal_pull_up_resistors() {
    uint8_t cdata = 0xFF;
    mcp23017_write(&dev1, REGISTER_GPPUA, &cdata);
    mcp23017_write(&dev1, REGISTER_GPPUB, &cdata);
    mcp23017_write(&dev2, REGISTER_GPPUA, &cdata);
    mcp23017_write(&dev2, REGISTER_GPPUB, &cdata);

#define CHECK_ERROR(cdata) \
    if (cdata != 0xFF) {   \
        Error_Handler();   \
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
    dev1 = (MCP23017_HandleTypeDef){.addr = MCP23017_DEV1_ADDR, .hi2c = &hi2c4, .gpio = {0, 0}};
    dev2 = (MCP23017_HandleTypeDef){.addr = MCP23017_DEV2_ADDR, .hi2c = &hi2c4, .gpio = {0, 0}};
    mcp23017_init(&dev1);
    mcp23017_init(&dev2);

#if MCP23017_IT_ENABLED == 1
    /**
   * @paragraph Interrupts
   *
   * Enable interrupts on all pins on both ports of both devices.
   *
   * Current configuration:
   * Interrupt on all pins
   * Interrupt mode is on change
   * INT_A and INT_B function independently
   */

    /**
   * Legacy implementation
   */
    // uint8_t gpinten_register_value = 0b11111111;
    // mcp23017_write(&dev1, REGISTER_GPINTENA, &gpinten_register_value);
    // mcp23017_write(&dev1, REGISTER_GPINTENB, &gpinten_register_value);
    // mcp23017_write(&dev2, REGISTER_GPINTENA, &gpinten_register_value);
    // mcp23017_write(&dev2, REGISTER_GPINTENB, &gpinten_register_value);

    HAL_StatusTypeDef HAL_Status   = HAL_ERROR;
    uint8_t gpinten_register_value = 0b00000000;
    uint8_t intcon_register_value  = 0b00000000;
    uint8_t defval_register_value  = 0b00000000;
    uint8_t mcp23017_i2c_timeout   = 10;  // ms

    mcp23017_set_it_on_all_pins(&gpinten_register_value, &intcon_register_value, &defval_register_value, MCP23017_INT_ENABLED, MCP23017_INT_MODE_ON_CHANGE, 0);

    /**
   * With this settings we only need to write the register GPINTEN.
   * This is because the other registers values are equal to their default.
   */

    // Write do device one, port a and b
    HAL_Status = HAL_I2C_Mem_Write(
        dev1.hi2c, dev1.addr, MCP23017_REGISTER_GPINTENA, MCP23017_I2C_SIZE, &gpinten_register_value, MCP23017_I2C_SIZE, mcp23017_i2c_timeout);
    if (HAL_Status != HAL_OK) {
        // TO-DO: Error
    }
    HAL_Status = HAL_I2C_Mem_Write(
        dev1.hi2c, dev1.addr, MCP23017_REGISTER_GPINTENB, MCP23017_I2C_SIZE, &gpinten_register_value, MCP23017_I2C_SIZE, mcp23017_i2c_timeout);
    if (HAL_Status != HAL_OK) {
        // TO-DO: Error
    }

    // Write do device two, port a and b
    HAL_Status = HAL_I2C_Mem_Write(
        dev2.hi2c, dev2.addr, MCP23017_REGISTER_GPINTENA, MCP23017_I2C_SIZE, &gpinten_register_value, MCP23017_I2C_SIZE, mcp23017_i2c_timeout);
    if (HAL_Status != HAL_OK) {
        // TO-DO: Error
    }
    HAL_Status = HAL_I2C_Mem_Write(
        dev2.hi2c, dev2.addr, MCP23017_REGISTER_GPINTENB, MCP23017_I2C_SIZE, &gpinten_register_value, MCP23017_I2C_SIZE, mcp23017_i2c_timeout);
    if (HAL_Status != HAL_OK) {
        // TO-DO: Error
    }
#endif
}

/**
 * @param value: The value of the manettino
 * @param manettino: The manettino index (left, center, right)
 * @brief This function maps the manettino value to the manettino index.
 * If the value is not found, it returns MANETTINO_INVALID_VALUE
 */
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
                buttons_long_press_activated[i] = false;
                buttons_long_press_check[i]     = HAL_GetTick();
                buttons_pressed_actions(i);
            } else {
                buttons_released_actions(i);
            }
            buttons[i] = current_button_val;
        }
        if (!buttons[i] && HAL_GetTick() > buttons_long_press_check[i] + BUTTONS_LONG_PRESS_TIME && !buttons_long_press_activated[i]) {
            buttons_long_pressed_actions(i);
            buttons_long_press_activated[i] = true;
        }
    }
}

void read_buttons(void) {
    uint8_t button_input;
    if (HAL_I2C_Mem_Read(&hi2c4, MCP23017_DEV1_ADDR << 1, REGISTER_GPIOB, 1, &button_input, 1, 100) != HAL_OK) {
        // TODO: handle errors
        return;
    }
    from_gpio_to_buttons(button_input);
    dev1.gpio[1] = button_input;
}

// TODO: use a formula lol
int delta_step_position(int delta_step) {
    if (delta_step == -7)
        delta_step = 1;
    if (delta_step == -6)
        delta_step = 2;
    if (delta_step == -5)
        delta_step = 3;
    if (delta_step == 5)
        delta_step = -3;
    if (delta_step == 6)
        delta_step = -2;
    if (delta_step == 7)
        delta_step = -1;
    if (delta_step < -7)
        delta_step = 0;
    return delta_step;
}

void manettini_actions(uint8_t value, uint8_t manettino) {
    if (!manettini_initialized[manettino]) {
        manettini_initialized[manettino] = true;
        manettini[manettino]             = from_manettino_value_to_index(value, manettino) == MANETTINO_INVALID_VALUE ? 0 : value;
        return;
    }
    uint8_t new_manettino_index = from_manettino_value_to_index(value, manettino);
    if (new_manettino_index == MANETTINO_INVALID_VALUE || new_manettino_index < 0 || new_manettino_index >= MANETTINO_STEPS_N) {
        return;
    }
    switch (manettino) {
        case MANETTINO_RIGHT_INDEX: {
            int dstep = new_manettino_index - manettini[MANETTINO_RIGHT_INDEX];
            manettino_right_actions(delta_step_position(dstep));
            break;
        }
        case MANETTINO_CENTER_INDEX: {
            int dstep = new_manettino_index - manettini[MANETTINO_CENTER_INDEX];
            manettino_center_actions(delta_step_position(dstep));
            break;
        }
        case MANETTINO_LEFT_INDEX: {
            int dstep = new_manettino_index - manettini[MANETTINO_LEFT_INDEX];
            manettino_left_actions(delta_step_position(dstep));
            break;
        }
        default: {
            break;
        }
    }
    manettini[manettino] = new_manettino_index;
}

/**
 * @brief This function is called when the interrupt pin of the MCP23017
 * connected to the manettino left is triggered
 * If the input has changed, it calls the manettini_actions function
 * Then it updates the manettino value in the dev1 struct
 */
void read_manettino_left(void) {
    uint8_t manettino_input;
    if (HAL_I2C_Mem_Read(&hi2c4, MCP23017_DEV1_ADDR << 1, REGISTER_GPIOA, 1, &manettino_input, 1, 100) != HAL_OK) {
        // TODO: handle errors
        return;
    }
    if (manettino_input != dev1.gpio[0]) {
        manettini_actions(manettino_input, MANETTINO_LEFT_INDEX);
        dev1.gpio[0] = manettino_input;
    }
}

void read_manettino_center(void) {
    uint8_t manettino_input;
    if (HAL_I2C_Mem_Read(&hi2c4, MCP23017_DEV2_ADDR << 1, REGISTER_GPIOB, 1, &manettino_input, 1, 100) != HAL_OK) {
        // TODO: handle errors
        return;
    }
    if (manettino_input != dev2.gpio[1]) {
        manettini_actions(manettino_input, MANETTINO_CENTER_INDEX);
        dev2.gpio[1] = manettino_input;
    }
}

void read_manettino_right(void) {
    uint8_t manettino_input;
    if (HAL_I2C_Mem_Read(&hi2c4, MCP23017_DEV2_ADDR << 1, REGISTER_GPIOA, 1, &manettino_input, 1, 100) != HAL_OK) {
        // TODO handle errors
        return;
    }
    if (manettino_input != dev2.gpio[0]) {
        manettini_actions(manettino_input, MANETTINO_RIGHT_INDEX);
        dev2.gpio[0] = manettino_input;
    }
}

#if MCP23017_IT_ENABLED == 1
bool int_pins[NUM_INTERRUPT_PINS] = {false};
#endif

/**
 * @brief This function handles EXTI interrupt.
 * @param GPIO_Pin: The pin that triggered the interrupt
 */

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    switch (GPIO_Pin) {
#if MCP23017_IT_ENABLED == 1
        case INT1_Pin:
            int_pins[BUTTONS_INTERRUPT_INDEX] = true;
            break;
        case INT2_Pin:
            int_pins[LEFT_MANETTINO_INTERRUPT_INDEX] = true;
            break;
        case INT3_Pin:
            int_pins[CENTER_MANETTINO_INTERRUPT_INDEX] = true;
            break;
        case INT4_Pin:
            int_pins[RIGHT_MANETTINO_INTERRUPT_INDEX] = true;
            break;
#endif
        case TSON_BUTTON_Pin:
            break;
        default:
            break;
    }
}

/**
 * @brief This function is called periodically to read the inputs
 * and read the value of the interrupt pins
 * @param tim:   Pointer to the LVGL timer
 */
void read_inputs(void) {
    if (HAL_GetTick() - manettini_last_change > MANETTINO_DEBOUNCE) {
        manettini_last_change = HAL_GetTick();

#if MCP23017_IT_ENABLED == 1
        if (int_pins[BUTTONS_INTERRUPT_INDEX]) {
            int_pins[BUTTONS_INTERRUPT_INDEX] = false;
            read_buttons();
        } else if (int_pins[LEFT_MANETTINO_INTERRUPT_INDEX]) {
            int_pins[LEFT_MANETTINO_INTERRUPT_INDEX] = false;
            read_manettino_left();
        } else if (int_pins[CENTER_MANETTINO_INTERRUPT_INDEX]) {
            int_pins[CENTER_MANETTINO_INTERRUPT_INDEX] = false;
            read_manettino_center();
        } else if (int_pins[RIGHT_MANETTINO_INTERRUPT_INDEX]) {
            int_pins[RIGHT_MANETTINO_INTERRUPT_INDEX] = false;
            read_manettino_right();
        }
#else
        read_buttons();
        read_manettino_left();
        read_manettino_center();
        read_manettino_right();
#endif
    }
}
