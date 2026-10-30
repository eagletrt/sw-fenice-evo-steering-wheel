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

/* USER CODE END Includes */

extern I2C_HandleTypeDef hi2c4;

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

void MX_I2C4_Init(void);

/* USER CODE BEGIN Prototypes */

/*!
 * \brief Free the bus and configure both GPIO expanders.
 *
 * \details Never fatal: if the expanders don't answer, the failure is logged
 *     and i2c_inputs_poll keeps trying to bring them back.
 */
void i2c_inputs_init(void);

/*!
 * \brief Poll the expanders and feed raw button/knob states to the inputs module.
 *
 * \details Buttons and manettini are read on every call. A failed read, or
 *     an expander that lost its configuration, releases every button and
 *     starts a bus recovery, retried every few polls until the expanders
 *     answer again. Intended to be called every ~10 ms (inputs task).
 *
 * \param current_tick_ms Current tick count in milliseconds.
 */
void i2c_inputs_poll(uint32_t current_tick_ms);

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __I2C_H__ */
