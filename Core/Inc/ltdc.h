/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    ltdc.h
 * @brief   This file contains all the function prototypes for
 *          the ltdc.c file
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
#ifndef __LTDC_H__
#define __LTDC_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

#include "colors.h"
#include "raster.h"

/* USER CODE END Includes */

extern LTDC_HandleTypeDef hltdc;

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

void MX_LTDC_Init(void);

/* USER CODE BEGIN Prototypes */

/*!
 * \brief Fill a rectangle of the displayed framebuffer with one color.
 *
 * \details Registered as the raster draw callback; drawing is offloaded to
 *     the DMA2D.
 *
 * \param x Top-left X position in pixels.
 * \param y Top-left Y position in pixels.
 * \param w Width in pixels.
 * \param h Height in pixels.
 * \param color Fill color.
 *
 * \retval RASTER_RC_OK always.
 */
enum RasterReturnCode ltdc_draw_rectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, struct Color color);

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __LTDC_H__ */
