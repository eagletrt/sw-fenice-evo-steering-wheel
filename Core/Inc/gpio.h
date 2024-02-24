/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    gpio.h
 * @brief   This file contains all the function prototypes for
 *          the gpio.c file
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
#ifndef __GPIO_H__
#define __GPIO_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* USER CODE BEGIN Private defines */
/**
 * 0 = INT1 -> buttons <br>
 * 1 = INT2 -> left manettino<br>
 * 2 = INT3 -> center manettino<br>
 * 3 = INT4 -> right manettino<br>
 * 4 = ExtraButton
 */
#define BUTTONS_INTERRUPT_INDEX 0
#define LEFT_MANETTINO_INTERRUPT_INDEX 1
#define CENTER_MANETTINO_INTERRUPT_INDEX 2
#define RIGHT_MANETTINO_INTERRUPT_INDEX 3
#define EXTRA_BUTTON_INTERRUPT_INDEX 4
#define NUM_INTERRUPT_PINS 5

/* USER CODE END Private defines */

void MX_GPIO_Init(void);

/* USER CODE BEGIN Prototypes */
extern bool int_pins[NUM_INTERRUPT_PINS];

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif
#endif /*__GPIO_H__ */
