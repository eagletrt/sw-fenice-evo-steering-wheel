/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    i2c.h
 * @brief   This file contains all the function prototypes for
 *          the i2c.c file
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __I2C_H__
#define __I2C_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

#include "leds.h"

/* USER CODE END Includes */

extern I2C_HandleTypeDef hi2c4;

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

void MX_I2C4_Init(void);

/* USER CODE BEGIN Prototypes */

/*!
 * \brief Initialize the two MCP23017 GPIO expanders carrying buttons and manettini.
 *
 * \details Must be called once after MX_I2C4_Init. Calls Error_Handler on a
 *     dead or mis-addressed expander.
 */
void i2c_inputs_init(void);

/*!
 * \brief Initialize the two KTD2052 LED controllers.
 *
 * \details Must be called once after MX_I2C4_Init. Calls Error_Handler on a
 *     bus failure.
 */
void i2c_leds_init(void);

/*!
 * \brief Poll the expanders and feed raw button/knob states to the inputs module.
 *
 * \details Buttons are read on every call; the manettini are rate-limited
 *     internally for debouncing. Intended to be called from the main loop
 *     every ~10 ms.
 *
 * \param current_tick_ms Current tick count in milliseconds.
 */
void i2c_inputs_poll(uint32_t current_tick_ms);

/*!
 * \brief Push the LED colors to the two KTD2052 controllers.
 *
 * \details Registered as the leds module transmit callback. Unchanged
 *     colors are skipped to keep the bus quiet.
 *
 * \param colors Array of colors, one per LED.
 * \param count Number of entries in \p colors, must be LEDS_INDEX_COUNT.
 *
 * \retval LEDS_RC_OK on success (or nothing to do).
 * \retval LEDS_RC_NULL_POINTER if \p colors is NULL.
 * \retval LEDS_RC_TRANSMISSION_ERROR on a bus failure or a wrong \p count.
 */
enum LedsReturnCode i2c_leds_transmit(const struct LedColor *colors, uint16_t count);

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __I2C_H__ */
