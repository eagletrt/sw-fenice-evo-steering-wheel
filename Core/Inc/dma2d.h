/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    dma2d.h
 * @brief   This file contains all the function prototypes for
 *          the dma2d.c file
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
#ifndef __DMA2D_H__
#define __DMA2D_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

#include "colors.h"
#include "screen.h"

/* USER CODE END Includes */

extern DMA2D_HandleTypeDef hdma2d;

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

void MX_DMA2D_Init(void);

/* USER CODE BEGIN Prototypes */

/*!
 * \brief Fill (or blend, when alpha < 255) a rectangle of a framebuffer.
 *
 * \details Blocks until any previous DMA2D transfer is done, then starts
 *     the new one without waiting for it: the next call (or the LTDC scan
 *     of an already-consistent area) synchronizes on the START bit.
 *
 * \param framebuffer Target ARGB8888 framebuffer, SCREEN_WIDTH pixels wide.
 * \param x Top-left X position in pixels.
 * \param y Top-left Y position in pixels.
 * \param w Width in pixels.
 * \param h Height in pixels.
 * \param color Fill color; alpha selects opaque fill vs blending.
 */
void dma2d_draw_rectangle(uint32_t *framebuffer, uint16_t x, uint16_t y, uint16_t w, uint16_t h, struct Color color);

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __DMA2D_H__ */
