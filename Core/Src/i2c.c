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

#include "eagletrt-api.h"
#include "inputs-api.h"
#include "logger-api.h"
#include "mcp23017-api.h"

#include <string.h>

#define I2C_TRANSFER_TIMEOUT_MS (10U)

/* I2C4 pins, as configured by HAL_I2C_MspInit: driven as GPIOs during a bus recovery. */
#define I2C_BUS_GPIO_PORT (GPIOD)
#define I2C_BUS_SCL_PIN (GPIO_PIN_12)
#define I2C_BUS_SDA_PIN (GPIO_PIN_13)

/* SCL pulses that free any slave stuck in the middle of a byte (8 data bits + ACK). */
#define I2C_RECOVERY_CLOCK_PULSES (9U)

/* Busy-loop length of half an SCL period during a recovery: >= 5 us at 550 MHz
 * (slower than 100 kHz, which any slave accepts). */
#define I2C_RECOVERY_HALF_PERIOD_LOOPS (2000U)

/* Bus recoveries + configurations tried by i2c_inputs_init before giving up. */
#define I2C_INIT_ATTEMPTS (3U)

/*!
 * \brief Polls skipped between two attempts to bring back lost expanders (~100 ms).
 *
 * \details Counted in polls, not milliseconds: what matters is how much of
 *     the inputs task a dead bus may take, and each attempt is one recovery
 *     plus a couple of (failing) transfers.
 */
#define I2C_RETRY_POLLS (10U)

/* Polls between two checks that the expanders still hold their configuration (~1 s). */
#define I2C_CHECK_POLLS (100U)

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
 * \brief Health of the expanders, kept by i2c_inputs_init and i2c_inputs_poll.
 */
static struct {
    bool ready;              /*!< Both expanders configured and answering */
    uint16_t polls_to_retry; /*!< Polls left before the next attempt to bring them back */
    uint16_t polls_to_check; /*!< Polls left before the next configuration check */
} expanders;

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
static bool prv_i2c_manettino_poll(enum I2cManettino manettino, struct Mcp23017Handler *handler, enum Mcp23017Port port, bool resync) {
    uint8_t port_value = 0U;
    if (mcp23017_api_read_port(handler, port, &port_value) != MCP23017_RC_OK) {
        return false;
    }

    struct I2cManettinoState *state = &manettino_states[manettino];
    if (port_value == state->last_port_value && state->initialized) {
        return true;
    }
    state->last_port_value = port_value;

    uint8_t position = 0U;
    if (!prv_i2c_manettino_decode(manettino, port_value, &position)) {
        return true;
    }

    if (!state->initialized || resync) {
        /* No idea how far the switch travelled while we were not looking:
         * adopt where it is now and wait for the next real transition. */
        state->initialized = true;
        state->position = position;
        return true;
    }

    state->accumulated_steps += prv_i2c_manettino_wrap_delta((int16_t)position - (int16_t)state->position);
    state->position = position;
    inputs_api_update_knob(manettino_wiring[manettino].knob_id, state->accumulated_steps);
    return true;
}

/*!
 * \brief Busy-wait half an SCL period of the bus recovery.
 */
static void prv_i2c_bus_delay(void) {
    for (volatile uint32_t i = 0U; i < I2C_RECOVERY_HALF_PERIOD_LOOPS; i++) {
    }
}

/*!
 * \brief Free a bus held by a slave, then restart the I2C peripheral.
 *
 * \details A slave reset (or left behind) in the middle of a read keeps
 *     driving SDA low while it waits for clocks that never come; the
 *     peripheral then sees the bus busy forever and every transfer fails.
 *     Clocking SCL by hand until SDA is released and sending a STOP puts
 *     every slave back to idle (NXP UM10204, 3.1.16 "Bus clear").
 *
 * \retval true if both lines are high (bus free) afterwards.
 * \retval false if SDA or SCL is still held low (short, dead slave).
 */
static bool prv_i2c_bus_recover(void) {
    EAGLETRT_API_UNUSED(HAL_I2C_DeInit(&hi2c4));

    /* Output register high before switching the pins, so they don't glitch low. */
    HAL_GPIO_WritePin(I2C_BUS_GPIO_PORT, I2C_BUS_SCL_PIN | I2C_BUS_SDA_PIN, GPIO_PIN_SET);
    GPIO_InitTypeDef gpio = {
        .Pin = I2C_BUS_SCL_PIN | I2C_BUS_SDA_PIN,
        .Mode = GPIO_MODE_OUTPUT_OD,
        .Pull = GPIO_NOPULL,
        .Speed = GPIO_SPEED_FREQ_LOW,
    };
    HAL_GPIO_Init(I2C_BUS_GPIO_PORT, &gpio);
    prv_i2c_bus_delay();

    for (uint8_t i = 0U; i < I2C_RECOVERY_CLOCK_PULSES && HAL_GPIO_ReadPin(I2C_BUS_GPIO_PORT, I2C_BUS_SDA_PIN) == GPIO_PIN_RESET; i++) {
        HAL_GPIO_WritePin(I2C_BUS_GPIO_PORT, I2C_BUS_SCL_PIN, GPIO_PIN_RESET);
        prv_i2c_bus_delay();
        HAL_GPIO_WritePin(I2C_BUS_GPIO_PORT, I2C_BUS_SCL_PIN, GPIO_PIN_SET);
        prv_i2c_bus_delay();
    }

    /* STOP: SDA rises while SCL is high (SCL goes low first, so this can't be a START). */
    HAL_GPIO_WritePin(I2C_BUS_GPIO_PORT, I2C_BUS_SCL_PIN, GPIO_PIN_RESET);
    prv_i2c_bus_delay();
    HAL_GPIO_WritePin(I2C_BUS_GPIO_PORT, I2C_BUS_SDA_PIN, GPIO_PIN_RESET);
    prv_i2c_bus_delay();
    HAL_GPIO_WritePin(I2C_BUS_GPIO_PORT, I2C_BUS_SCL_PIN, GPIO_PIN_SET);
    prv_i2c_bus_delay();
    HAL_GPIO_WritePin(I2C_BUS_GPIO_PORT, I2C_BUS_SDA_PIN, GPIO_PIN_SET);
    prv_i2c_bus_delay();

    const bool bus_free = HAL_GPIO_ReadPin(I2C_BUS_GPIO_PORT, I2C_BUS_SDA_PIN) == GPIO_PIN_SET &&
                          HAL_GPIO_ReadPin(I2C_BUS_GPIO_PORT, I2C_BUS_SCL_PIN) == GPIO_PIN_SET;

    /* Back to the peripheral (HAL_I2C_MspInit puts the pins in AF again). */
    MX_I2C4_Init();
    return bus_free;
}

/*!
 * \brief Recover the bus and configure both expanders.
 *
 * \param attempts How many recover + configure rounds to try.
 *
 * \retval true if both expanders are configured.
 * \retval false if they still don't answer.
 */
static bool prv_i2c_expanders_start(uint8_t attempts) {
    for (uint8_t i = 0U; i < attempts; i++) {
        /* A bus still held low would make every transfer wait for the HAL's busy timeout: skip it. */
        if (!prv_i2c_bus_recover()) {
            continue;
        }
        if (mcp23017_api_init(&mcp23017_dev1, I2C_MCP23017_DEV1_ADDRESS, prv_i2c_mcp23017_read_register, prv_i2c_mcp23017_write_register) == MCP23017_RC_OK &&
            mcp23017_api_init(&mcp23017_dev2, I2C_MCP23017_DEV2_ADDRESS, prv_i2c_mcp23017_read_register, prv_i2c_mcp23017_write_register) == MCP23017_RC_OK) {
            /* Readings from before are stale: every manettino adopts its next reading as its baseline. */
            memset(manettino_states, 0, sizeof(manettino_states));
            expanders.polls_to_check = I2C_CHECK_POLLS;
            return true;
        }
    }
    return false;
}

/*!
 * \brief The expanders stopped answering (or lost their configuration): release every button and retry soon.
 *
 * \details Releasing is the safe default: a button held when the bus died
 *     (PTT, TSon) must not stay pressed until the expanders come back.
 *
 * \param current_tick_ms Current tick, for the button releases.
 */
static void prv_i2c_expanders_lost(uint32_t current_tick_ms) {
    expanders.ready = false;
    expanders.polls_to_retry = 0U;
    for (size_t i = 0U; i < sizeof(button_wiring) / sizeof(button_wiring[0]); i++) {
        inputs_api_update_button(button_wiring[i].button_id, false, current_tick_ms);
    }
    logger_api_log(LOGGER_LEVEL_WARN, "I2C expanders lost, recovering");
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
    memset(&expanders, 0, sizeof(expanders));
    memset(manettino_states, 0, sizeof(manettino_states));
    expanders.ready = prv_i2c_expanders_start(I2C_INIT_ATTEMPTS);
    if (!expanders.ready) {
        /* Not fatal: the rest of the wheel runs and i2c_inputs_poll keeps trying. */
        expanders.polls_to_retry = I2C_RETRY_POLLS;
        logger_api_log(LOGGER_LEVEL_ERROR, "I2C expanders not answering, retrying");
    }
}

void i2c_inputs_poll(uint32_t current_tick_ms) {
    if (!expanders.ready) {
        if (expanders.polls_to_retry > 0U) {
            expanders.polls_to_retry--;
            return;
        }
        expanders.ready = prv_i2c_expanders_start(1U);
        if (!expanders.ready) {
            expanders.polls_to_retry = I2C_RETRY_POLLS;
            return;
        }
        logger_api_log(LOGGER_LEVEL_INFO, "I2C expanders recovered");
    }

    /* An expander reset on its own (supply glitch) answers fine but with its pull-ups off. */
    if (expanders.polls_to_check > 0U) {
        expanders.polls_to_check--;
    } else {
        expanders.polls_to_check = I2C_CHECK_POLLS;
        if (mcp23017_api_check_config(&mcp23017_dev1) != MCP23017_RC_OK || mcp23017_api_check_config(&mcp23017_dev2) != MCP23017_RC_OK) {
            prv_i2c_expanders_lost(current_tick_ms);
            return;
        }
    }

    uint8_t buttons_port = 0U;
    if (mcp23017_api_read_port(&mcp23017_dev1, MCP23017_PORT_B, &buttons_port) != MCP23017_RC_OK) {
        prv_i2c_expanders_lost(current_tick_ms);
        return;
    }
    for (size_t i = 0U; i < sizeof(button_wiring) / sizeof(button_wiring[0]); i++) {
        const bool pressed = ((buttons_port >> button_wiring[i].bit) & 1U) == 0U;
        inputs_api_update_button(button_wiring[i].button_id, pressed, current_tick_ms);
    }

    /* Sampled on every pass, not rate-limited: the codes between detents do
     * not decode, so they are skipped for free, and reading often is what
     * keeps a fast turn from aliasing into a backwards one. */
    static uint32_t last_manettino_poll_tick = 0U;
    const bool resync = (current_tick_ms - last_manettino_poll_tick) >= I2C_MANETTINO_RESYNC_GAP_MS;
    last_manettino_poll_tick = current_tick_ms;

    if (!prv_i2c_manettino_poll(I2C_MANETTINO_LEFT, &mcp23017_dev1, MCP23017_PORT_A, resync) ||
        !prv_i2c_manettino_poll(I2C_MANETTINO_CENTER, &mcp23017_dev2, MCP23017_PORT_B, resync) ||
        !prv_i2c_manettino_poll(I2C_MANETTINO_RIGHT, &mcp23017_dev2, MCP23017_PORT_A, resync)) {
        prv_i2c_expanders_lost(current_tick_ms);
    }
}

/* USER CODE END 1 */
