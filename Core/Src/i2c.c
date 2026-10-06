/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    i2c.c
 * @brief   This file provides code for the configuration
 *          of the I2C instances.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2023 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "i2c.h"

/* USER CODE BEGIN 0 */

#include "inputs-api.h"
#include "logger-api.h"
#include "mcp23017-api.h"

#include <string.h>

#define I2C_TRANSFER_TIMEOUT_MS (10U)

/* 8-bit (shifted) bus addresses of the two GPIO expanders. */
#define I2C_MCP23017_DEV1_ADDRESS (0x27U << 1U) /* buttons on port B, left manettino on port A */
#define I2C_MCP23017_DEV2_ADDRESS (0x20U << 1U) /* right manettino on port A, center manettino on port B */

/*!
 * \brief Gap above which a manettino sample is treated as a re-sync.
 *
 * \details The switches are absolute and prv_i2c_manettino_wrap_delta folds
 *     every step onto the shortest path around the 8-position ring, so a
 *     movement of 5..7 detents between two samples is indistinguishable from
 *     3..1 detents the other way. Sampling every main-loop pass makes that
 *     impossible at any human speed, but a long blocking render (the full
 *     dashboard redraw when the popup times out) can still starve the poll.
 *     After a gap this long the reading is adopted as the new baseline
 *     without emitting a rotation: losing a click beats inventing a
 *     backwards one.
 */
#define I2C_MANETTINO_RESYNC_GAP_MS (50U)

/* Number of detents of each rotary switch. */
#define I2C_MANETTINO_POSITION_COUNT (8U)

/*!
 * \brief Physical rotary switches (manettini) of the wheel.
 */
enum I2cManettino {
    I2C_MANETTINO_LEFT,
    I2C_MANETTINO_CENTER,
    I2C_MANETTINO_RIGHT,
    I2C_MANETTINO_COUNT,
};

/*!
 * \brief Static wiring of one manettino.
 */
struct I2cManettinoWiring {
    enum InputsSharedKnobID knob_id;                   /*!< Knob reported to the inputs module */
    uint8_t port_values[I2C_MANETTINO_POSITION_COUNT]; /*!< Raw port value at each detent (one line low per position) */
};

/*!
 * \brief Runtime tracking of one manettino.
 */
struct I2cManettinoState {
    bool initialized;          /*!< First read done: position is meaningful */
    uint8_t position;          /*!< Last decoded detent, 0..7 */
    int16_t accumulated_steps; /*!< Wrap-corrected running position fed to the inputs module */
    uint8_t last_port_value;   /*!< Last raw port value, to skip decoding unchanged reads */
};

static struct Mcp23017Handler mcp23017_dev1;
static struct Mcp23017Handler mcp23017_dev2;

static struct I2cManettinoState manettino_states[I2C_MANETTINO_COUNT];

/*!
 * \brief Mapping of the raw button port (MCP dev1, port B) to input button IDs.
 *
 * \details Bit positions come from the wheel PCB routing; all buttons are
 *     active-low thanks to the expander pull-ups.
 */
static const struct {
    uint8_t bit;
    enum InputsSharedButtonID button_id;
} button_wiring[] = {
    { 7U, INPUTS_SHARED_BUTTON_ID_TOP_LEFT_1 },
    { 5U, INPUTS_SHARED_BUTTON_ID_PADDLE_TOP_LEFT },
    { 3U, INPUTS_SHARED_BUTTON_ID_PADDLE_TOP_RIGHT },
    { 1U, INPUTS_SHARED_BUTTON_ID_TOP_RIGHT_1 },
    { 0U, INPUTS_SHARED_BUTTON_ID_PADDLE_BOTTOM_RIGHT },
    { 6U, INPUTS_SHARED_BUTTON_ID_PADDLE_BOTTOM_LEFT },
    { 4U, INPUTS_SHARED_BUTTON_ID_BOTTOM_LEFT },
    { 2U, INPUTS_SHARED_BUTTON_ID_BOTTOM_RIGHT },
};

/*!
 * \brief Manettino wiring: knob mapping and per-detent raw port values.
 *
 * \details Each detent grounds exactly one expander line, so the raw port
 *     value has a single bit low. The value tables come from the previous
 *     firmware (MANETTINO_*_VALS in old-porcs-can Core/Inc/inputs/inputs.h)
 *     and encode the (scrambled) PCB routing per switch, so they are not
 *     interchangeable between switches.
 */
static const struct I2cManettinoWiring manettino_wiring[I2C_MANETTINO_COUNT] = {
    [I2C_MANETTINO_LEFT] = {
        .knob_id = INPUTS_SHARED_KNOB_ID_FRONT_CENTER,
        .port_values = { 127U, 191U, 247U, 251U, 253U, 254U, 239U, 223U },
    },
    [I2C_MANETTINO_CENTER] = {
        .knob_id = INPUTS_SHARED_KNOB_ID_FRONT_LEFT,
        .port_values = { 239U, 247U, 251U, 254U, 223U, 191U, 127U, 253U },
    },
    [I2C_MANETTINO_RIGHT] = {
        .knob_id = INPUTS_SHARED_KNOB_ID_FRONT_RIGHT,
        .port_values = { 253U, 251U, 239U, 127U, 191U, 223U, 247U, 254U },
    },
};

/*!
 * \brief MCP23017 register-read callback wrapping the HAL.
 */
static enum Mcp23017ReturnCode prv_i2c_mcp23017_read_register(uint8_t device_address, uint8_t register_address, uint8_t *value) {
    if (HAL_I2C_Mem_Read(&hi2c4, device_address, register_address, 1U, value, 1U, I2C_TRANSFER_TIMEOUT_MS) != HAL_OK) {
        return MCP23017_RC_BUS_ERROR;
    }
    return MCP23017_RC_OK;
}

/*!
 * \brief MCP23017 register-write callback wrapping the HAL.
 */
static enum Mcp23017ReturnCode prv_i2c_mcp23017_write_register(uint8_t device_address, uint8_t register_address, uint8_t value) {
    if (HAL_I2C_Mem_Write(&hi2c4, device_address, register_address, 1U, &value, 1U, I2C_TRANSFER_TIMEOUT_MS) != HAL_OK) {
        return MCP23017_RC_BUS_ERROR;
    }
    return MCP23017_RC_OK;
}

/*!
 * \brief Decode a raw manettino port value into a detent index.
 *
 * \param manettino Switch whose value table to use.
 * \param port_value Raw expander port value.
 * \param position Output slot for the decoded detent.
 *
 * \retval true if the value matched a detent.
 * \retval false for transient values read mid-rotation.
 */
static bool prv_i2c_manettino_decode(enum I2cManettino manettino, uint8_t port_value, uint8_t *position) {
    for (uint8_t i = 0U; i < I2C_MANETTINO_POSITION_COUNT; i++) {
        if (manettino_wiring[manettino].port_values[i] == port_value) {
            *position = i;
            return true;
        }
    }
    return false;
}

/*!
 * \brief Fold a raw detent delta onto the shortest path around the 8-position ring.
 */
static int16_t prv_i2c_manettino_wrap_delta(int16_t delta) {
    if (delta > (int16_t)(I2C_MANETTINO_POSITION_COUNT / 2U)) {
        delta -= (int16_t)I2C_MANETTINO_POSITION_COUNT;
    } else if (delta < -(int16_t)(I2C_MANETTINO_POSITION_COUNT / 2U)) {
        delta += (int16_t)I2C_MANETTINO_POSITION_COUNT;
    }
    return delta;
}

/*!
 * \brief Read one manettino port and feed the decoded rotation to the inputs module.
 *
 * \param manettino Switch to update.
 * \param handler Expander carrying the switch.
 * \param port Expander port the switch is wired to.
 * \param resync true to adopt the reading as a new baseline instead of
 *     turning it into a rotation, after a gap long enough to have missed
 *     detents.
 */
static void prv_i2c_manettino_poll(enum I2cManettino manettino, struct Mcp23017Handler *handler, enum Mcp23017Port port, bool resync) {
    uint8_t port_value = 0U;
    if (mcp23017_api_read_port(handler, port, &port_value) != MCP23017_RC_OK) {
        return;
    }

    struct I2cManettinoState *state = &manettino_states[manettino];
    if (port_value == state->last_port_value && state->initialized) {
        return;
    }
    state->last_port_value = port_value;

    uint8_t position = 0U;
    if (!prv_i2c_manettino_decode(manettino, port_value, &position)) {
        return;
    }

    if (!state->initialized || resync) {
        /* No idea how far the switch travelled while we were not looking:
         * adopt where it is now and wait for the next real transition. */
        state->initialized = true;
        state->position = position;
        return;
    }

    state->accumulated_steps += prv_i2c_manettino_wrap_delta((int16_t)position - (int16_t)state->position);
    state->position = position;
    inputs_api_update_knob(manettino_wiring[manettino].knob_id, state->accumulated_steps);
}

/* USER CODE END 0 */

I2C_HandleTypeDef hi2c4;

/* I2C4 init function */
void MX_I2C4_Init(void) {

    /* USER CODE BEGIN I2C4_Init 0 */

    /* USER CODE END I2C4_Init 0 */

    /* USER CODE BEGIN I2C4_Init 1 */

    /* USER CODE END I2C4_Init 1 */
    hi2c4.Instance = I2C4;
    hi2c4.Init.Timing = 0x10707DBC;
    hi2c4.Init.OwnAddress1 = 0;
    hi2c4.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c4.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c4.Init.OwnAddress2 = 0;
    hi2c4.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
    hi2c4.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c4.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    if (HAL_I2C_Init(&hi2c4) != HAL_OK) {
        Error_Handler();
    }

    /** Configure Analogue filter
  */
    if (HAL_I2CEx_ConfigAnalogFilter(&hi2c4, I2C_ANALOGFILTER_ENABLE) != HAL_OK) {
        Error_Handler();
    }

    /** Configure Digital filter
  */
    if (HAL_I2CEx_ConfigDigitalFilter(&hi2c4, 0) != HAL_OK) {
        Error_Handler();
    }
    /* USER CODE BEGIN I2C4_Init 2 */

    /* USER CODE END I2C4_Init 2 */
}

void HAL_I2C_MspInit(I2C_HandleTypeDef *i2cHandle) {

    GPIO_InitTypeDef GPIO_InitStruct = { 0 };
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = { 0 };
    if (i2cHandle->Instance == I2C4) {
        /* USER CODE BEGIN I2C4_MspInit 0 */

        /* USER CODE END I2C4_MspInit 0 */

        /** Initializes the peripherals clock
  */
        PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_I2C4;
        PeriphClkInitStruct.I2c4ClockSelection = RCC_I2C4CLKSOURCE_HSI;
        if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
            Error_Handler();
        }

        __HAL_RCC_GPIOD_CLK_ENABLE();
        /**I2C4 GPIO Configuration
    PD12     ------> I2C4_SCL
    PD13     ------> I2C4_SDA
    */
        GPIO_InitStruct.Pin = GPIO_PIN_12 | GPIO_PIN_13;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF4_I2C4;
        HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

        /* I2C4 clock enable */
        __HAL_RCC_I2C4_CLK_ENABLE();
        /* USER CODE BEGIN I2C4_MspInit 1 */

        /* USER CODE END I2C4_MspInit 1 */
    }
}

void HAL_I2C_MspDeInit(I2C_HandleTypeDef *i2cHandle) {

    if (i2cHandle->Instance == I2C4) {
        /* USER CODE BEGIN I2C4_MspDeInit 0 */

        /* USER CODE END I2C4_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_I2C4_CLK_DISABLE();

        /**I2C4 GPIO Configuration
    PD12     ------> I2C4_SCL
    PD13     ------> I2C4_SDA
    */
        HAL_GPIO_DeInit(GPIOD, GPIO_PIN_12);

        HAL_GPIO_DeInit(GPIOD, GPIO_PIN_13);

        /* USER CODE BEGIN I2C4_MspDeInit 1 */

        /* USER CODE END I2C4_MspDeInit 1 */
    }
}

/* USER CODE BEGIN 1 */

void i2c_scan_bus(void) {
    logger_api_log(LOGGER_LEVEL_DEBUG, "I2C4 scan: start");

    uint8_t found = 0U;
    /* 7-bit addresses 0x08..0x77; the HAL takes the 8-bit (shifted) form. */
    for (uint8_t address = 0x08U; address <= 0x77U; address++) {
        if (HAL_I2C_IsDeviceReady(&hi2c4, (uint16_t)(address << 1U), 2U, I2C_TRANSFER_TIMEOUT_MS) == HAL_OK) {
            logger_api_log(LOGGER_LEVEL_DEBUG, "I2C4 scan: ACK at 7-bit 0x%02X (8-bit 0x%02X)", address, address << 1U);
            found++;
        }
    }

    logger_api_log(LOGGER_LEVEL_DEBUG, "I2C4 scan: done, %u device(s)", found);
}

void i2c_inputs_init(void) {
    if (mcp23017_api_init(&mcp23017_dev1, I2C_MCP23017_DEV1_ADDRESS, prv_i2c_mcp23017_read_register, prv_i2c_mcp23017_write_register) != MCP23017_RC_OK) {
        Error_Handler();
    }
    if (mcp23017_api_init(&mcp23017_dev2, I2C_MCP23017_DEV2_ADDRESS, prv_i2c_mcp23017_read_register, prv_i2c_mcp23017_write_register) != MCP23017_RC_OK) {
        Error_Handler();
    }
    memset(manettino_states, 0, sizeof(manettino_states));
}

void i2c_inputs_poll(uint32_t current_tick_ms) {
    uint8_t buttons_port = 0U;
    if (mcp23017_api_read_port(&mcp23017_dev1, MCP23017_PORT_B, &buttons_port) == MCP23017_RC_OK) {
        for (size_t i = 0U; i < sizeof(button_wiring) / sizeof(button_wiring[0]); i++) {
            const bool pressed = ((buttons_port >> button_wiring[i].bit) & 1U) == 0U;
            inputs_api_update_button(button_wiring[i].button_id, pressed, current_tick_ms);
        }
    }

    /* Sampled on every pass, not rate-limited: the codes between detents do
     * not decode, so they are skipped for free, and reading often is what
     * keeps a fast turn from aliasing into a backwards one. */
    static uint32_t last_manettino_poll_tick = 0U;
    const bool resync = (current_tick_ms - last_manettino_poll_tick) >= I2C_MANETTINO_RESYNC_GAP_MS;
    last_manettino_poll_tick = current_tick_ms;

    prv_i2c_manettino_poll(I2C_MANETTINO_LEFT, &mcp23017_dev1, MCP23017_PORT_A, resync);
    prv_i2c_manettino_poll(I2C_MANETTINO_CENTER, &mcp23017_dev2, MCP23017_PORT_B, resync);
    prv_i2c_manettino_poll(I2C_MANETTINO_RIGHT, &mcp23017_dev2, MCP23017_PORT_A, resync);
}

/* USER CODE END 1 */
