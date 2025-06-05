/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.h
 * @brief          : Header for main.c file.
 *                   This file contains the common defines of the application.
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
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "inputs/inputs.h"
#include "led_control.h"
#include "test/i2c_test.h"
#include "test/sdram_test.h"

#include <string.h>
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define PADDLE_TOP_LEFT_Pin GPIO_PIN_2
#define PADDLE_TOP_LEFT_GPIO_Port GPIOE
#define IMUX_C3_Pin GPIO_PIN_3
#define IMUX_C3_GPIO_Port GPIOE
#define LCD_BL_DIM_Pin GPIO_PIN_4
#define LCD_BL_DIM_GPIO_Port GPIOA
#define GREEN_BUTTON_RIGHT_Pin GPIO_PIN_11
#define GREEN_BUTTON_RIGHT_GPIO_Port GPIOC
#define GREEN_BUTTON_LEFT_Pin GPIO_PIN_12
#define GREEN_BUTTON_LEFT_GPIO_Port GPIOC
#define PAD_BOTTOM_RIGHT_Pin GPIO_PIN_4
#define PAD_BOTTOM_RIGHT_GPIO_Port GPIOD
#define PAD_BOTTOM_LEFT_Pin GPIO_PIN_5
#define PAD_BOTTOM_LEFT_GPIO_Port GPIOD
#define PAD_BOTTOM_LEFT_EXTI_IRQn EXTI9_5_IRQn
#define PAD_TOP_RIGHT_Pin GPIO_PIN_6
#define PAD_TOP_RIGHT_GPIO_Port GPIOD
#define PAD_TOP_RIGHT_EXTI_IRQn EXTI9_5_IRQn
#define LCD_BL_EN_Pin GPIO_PIN_7
#define LCD_BL_EN_GPIO_Port GPIOD
#define IMUX_C1_Pin GPIO_PIN_9
#define IMUX_C1_GPIO_Port GPIOG
#define IMUX_S4_Pin GPIO_PIN_11
#define IMUX_S4_GPIO_Port GPIOG
#define IMUX_S3_Pin GPIO_PIN_12
#define IMUX_S3_GPIO_Port GPIOG
#define IMUX_S2_Pin GPIO_PIN_14
#define IMUX_S2_GPIO_Port GPIOG
#define IMUX_S1_Pin GPIO_PIN_4
#define IMUX_S1_GPIO_Port GPIOB
#define IMUX_C2_Pin GPIO_PIN_5
#define IMUX_C2_GPIO_Port GPIOB
#define TSON_BUTTON_Pin GPIO_PIN_9
#define TSON_BUTTON_GPIO_Port GPIOB
#define TSON_BUTTON_EXTI_IRQn EXTI9_5_IRQn

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
