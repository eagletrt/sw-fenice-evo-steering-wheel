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
#include "ktd2052-api.h"
#include "leds.h"
#include "mcp23017-api.h"

#include <string.h>

#define I2C_TRANSFER_TIMEOUT_MS (10U)

/* 8-bit (shifted) bus addresses of the two GPIO expanders. */
#define I2C_MCP23017_DEV1_ADDRESS (0x27U << 1U) /* buttons on port B, left manettino on port A */
#define I2C_MCP23017_DEV2_ADDRESS (0x20U << 1U) /* right manettino on port A, center manettino on port B */

/* 8-bit bus addresses of the two KTD2052 LED controllers. */
#define I2C_KTD2052_LEFT_ADDRESS (0xE8U)  /* KTD2052A: left LED triplet */
#define I2C_KTD2052_RIGHT_ADDRESS (0xEAU) /* KTD2052C: right LED triplet */

/* Rotary switches are debounced by rate-limiting their reads. */
#define I2C_MANETTINO_DEBOUNCE_MS (100U)

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
    enum InputsSharedKnobID knob_id;                        /*!< Knob reported to the inputs module */
    uint8_t port_values[I2C_MANETTINO_POSITION_COUNT];      /*!< Raw port value at each detent (one line low per position) */
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
static struct Ktd2052Handler ktd2052_left;
static struct Ktd2052Handler ktd2052_right;

static struct I2cManettinoState manettino_states[I2C_MANETTINO_COUNT];
static struct LedColor leds_last_colors[LEDS_INDEX_COUNT];
static bool leds_last_colors_valid = false;

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
 *     firmware and encode the (scrambled) PCB routing per switch.
 */
static const struct I2cManettinoWiring manettino_wiring[I2C_MANETTINO_COUNT] = {
    [I2C_MANETTINO_LEFT] = {
        .knob_id = INPUTS_SHARED_KNOB_ID_FRONT_LEFT,
        .port_values = { 127U, 191U, 247U, 251U, 253U, 254U, 239U, 223U },
    },
    [I2C_MANETTINO_CENTER] = {
        .knob_id = INPUTS_SHARED_KNOB_ID_FRONT_RIGHT,
        .port_values = { 239U, 247U, 251U, 254U, 223U, 191U, 127U, 253U },
    },
    [I2C_MANETTINO_RIGHT] = {
        .knob_id = INPUTS_SHARED_KNOB_ID_SIDE_LEFT,
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
 * \brief KTD2052 register-write callback wrapping the HAL.
 */
static enum Ktd2052ReturnCode prv_i2c_ktd2052_write_register(uint8_t device_address, uint8_t register_address, uint8_t value) {
    if (HAL_I2C_Mem_Write(&hi2c4, device_address, register_address, 1U, &value, 1U, I2C_TRANSFER_TIMEOUT_MS) != HAL_OK) {
        return KTD2052_RC_BUS_ERROR;
    }
    return KTD2052_RC_OK;
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
 */
static void prv_i2c_manettino_poll(enum I2cManettino manettino, struct Mcp23017Handler *handler, enum Mcp23017Port port) {
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

    if (!state->initialized) {
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
    hi2c4.Instance              = I2C4;
    hi2c4.Init.Timing           = 0x10707DBC;
    hi2c4.Init.OwnAddress1      = 0;
    hi2c4.Init.AddressingMode   = I2C_ADDRESSINGMODE_7BIT;
    hi2c4.Init.DualAddressMode  = I2C_DUALADDRESS_DISABLE;
    hi2c4.Init.OwnAddress2      = 0;
    hi2c4.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
    hi2c4.Init.GeneralCallMode  = I2C_GENERALCALL_DISABLE;
    hi2c4.Init.NoStretchMode    = I2C_NOSTRETCH_DISABLE;
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
    GPIO_InitTypeDef GPIO_InitStruct             = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
    if (i2cHandle->Instance == I2C4) {
        /* USER CODE BEGIN I2C4_MspInit 0 */

        /* USER CODE END I2C4_MspInit 0 */

        /** Initializes the peripherals clock
  */
        PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_I2C4;
        PeriphClkInitStruct.I2c4ClockSelection   = RCC_I2C4CLKSOURCE_HSI;
        if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
            Error_Handler();
        }

        __HAL_RCC_GPIOD_CLK_ENABLE();
        /**I2C4 GPIO Configuration
    PD12     ------> I2C4_SCL
    PD13     ------> I2C4_SDA
    */
        GPIO_InitStruct.Pin       = GPIO_PIN_12 | GPIO_PIN_13;
        GPIO_InitStruct.Mode      = GPIO_MODE_AF_OD;
        GPIO_InitStruct.Pull      = GPIO_NOPULL;
        GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_LOW;
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

void i2c_inputs_init(void) {
    if (mcp23017_api_init(&mcp23017_dev1, I2C_MCP23017_DEV1_ADDRESS, prv_i2c_mcp23017_read_register, prv_i2c_mcp23017_write_register) != MCP23017_RC_OK) {
        Error_Handler();
    }
    if (mcp23017_api_init(&mcp23017_dev2, I2C_MCP23017_DEV2_ADDRESS, prv_i2c_mcp23017_read_register, prv_i2c_mcp23017_write_register) != MCP23017_RC_OK) {
        Error_Handler();
    }
    memset(manettino_states, 0, sizeof(manettino_states));
}

void i2c_leds_init(void) {
    if (ktd2052_api_init(&ktd2052_left, I2C_KTD2052_LEFT_ADDRESS, prv_i2c_ktd2052_write_register) != KTD2052_RC_OK) {
        Error_Handler();
    }
    if (ktd2052_api_init(&ktd2052_right, I2C_KTD2052_RIGHT_ADDRESS, prv_i2c_ktd2052_write_register) != KTD2052_RC_OK) {
        Error_Handler();
    }
    leds_last_colors_valid = false;
}

void i2c_inputs_poll(uint32_t current_tick_ms) {
    uint8_t buttons_port = 0U;
    if (mcp23017_api_read_port(&mcp23017_dev1, MCP23017_PORT_B, &buttons_port) == MCP23017_RC_OK) {
        for (size_t i = 0U; i < sizeof(button_wiring) / sizeof(button_wiring[0]); i++) {
            const bool pressed = ((buttons_port >> button_wiring[i].bit) & 1U) == 0U;
            inputs_api_update_button(button_wiring[i].button_id, pressed, current_tick_ms);
        }
    }

    static uint32_t last_manettino_poll_tick = 0U;
    if (current_tick_ms - last_manettino_poll_tick >= I2C_MANETTINO_DEBOUNCE_MS) {
        last_manettino_poll_tick = current_tick_ms;
        prv_i2c_manettino_poll(I2C_MANETTINO_LEFT, &mcp23017_dev1, MCP23017_PORT_A);
        prv_i2c_manettino_poll(I2C_MANETTINO_CENTER, &mcp23017_dev2, MCP23017_PORT_B);
        prv_i2c_manettino_poll(I2C_MANETTINO_RIGHT, &mcp23017_dev2, MCP23017_PORT_A);
    }
}

enum LedsReturnCode i2c_leds_transmit(const struct LedColor *colors, uint16_t count) {
    if (colors == NULL) {
        return LEDS_RC_NULL_POINTER;
    }
    if (count != LEDS_INDEX_COUNT) {
        return LEDS_RC_TRANSMISSION_ERROR;
    }

    /* The strip is retransmitted every FSM tick; skip the (slow, blocking)
     * bus traffic when nothing changed. */
    if (leds_last_colors_valid && memcmp(colors, leds_last_colors, sizeof(leds_last_colors)) == 0) {
        return LEDS_RC_OK;
    }

    for (uint16_t i = 0U; i < count; i++) {
        struct Ktd2052Handler *controller = (i < LEDS_INDEX_RIGHT_0) ? &ktd2052_left : &ktd2052_right;
        const uint8_t module = (uint8_t)(i % 3U);
        if (ktd2052_api_set_color(controller, module, colors[i].r, colors[i].g, colors[i].b) != KTD2052_RC_OK) {
            leds_last_colors_valid = false;
            return LEDS_RC_TRANSMISSION_ERROR;
        }
    }

    memcpy(leds_last_colors, colors, sizeof(leds_last_colors));
    leds_last_colors_valid = true;
    return LEDS_RC_OK;
}

/* USER CODE END 1 */
