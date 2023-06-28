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
#include "lvgl.h"
#include "lvgl_port/lvgl_port.h"
#include "lvgl_port/screen_driver.h"
#include "primary/primary_network.h"
#include "primary/primary_watchdog.h"
#include "steering.h"
#include "tab_manager.h"
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
#define INT1_Pin GPIO_PIN_2
#define INT1_GPIO_Port GPIOE
#define INT3_Pin GPIO_PIN_3
#define INT3_GPIO_Port GPIOE
#define LCD_BL_DIM_Pin GPIO_PIN_4
#define LCD_BL_DIM_GPIO_Port GPIOA
#define LCD_BL_EN_Pin GPIO_PIN_7
#define LCD_BL_EN_GPIO_Port GPIOD
#define INT4_Pin GPIO_PIN_4
#define INT4_GPIO_Port GPIOB
#define INT2_Pin GPIO_PIN_5
#define INT2_GPIO_Port GPIOB
#define INT2_EXTI_IRQn EXTI9_5_IRQn
#define ExtraButton_Pin GPIO_PIN_9
#define ExtraButton_GPIO_Port GPIOB
#define ExtraButton_EXTI_IRQn EXTI9_5_IRQn
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
