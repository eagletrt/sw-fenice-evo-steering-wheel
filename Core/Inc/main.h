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
#include "led_control.h"
#include "lvgl.h"
#include "lvgl_port/lvgl_port.h"
#include "lvgl_port/screen_driver.h"
#include "steering/steering.h"
#include "steering/tab_manager.h"
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
#define LCD_BL_DIM_Pin GPIO_PIN_4
#define LCD_BL_DIM_GPIO_Port GPIOA
#define INT_4_Pin GPIO_PIN_4
#define INT_4_GPIO_Port GPIOD
#define INT_3_Pin GPIO_PIN_5
#define INT_3_GPIO_Port GPIOD
#define LCD_BL_EN_Pin GPIO_PIN_7
#define LCD_BL_EN_GPIO_Port GPIOD
#define NC4_Pin GPIO_PIN_9
#define NC4_GPIO_Port GPIOG
#define INT_1_Pin GPIO_PIN_11
#define INT_1_GPIO_Port GPIOG
#define INT_2_Pin GPIO_PIN_12
#define INT_2_GPIO_Port GPIOG
/* USER CODE BEGIN Private defines */
#define SDRAM_BASE_ADDRESS 0xC0000000
#define FRAMEBUFFER1_ADDR SDRAM_BASE_ADDRESS
#define FRAMEBUFFER2_ADDR 0xC0200000
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 480

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
