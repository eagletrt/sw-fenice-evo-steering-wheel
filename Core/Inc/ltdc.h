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
 * \brief Present the freshly-drawn frame and start drawing the next one.
 *
 * \details Waits for the DMA2D to go idle, swaps the display and draw
 *     framebuffers and points the LTDC layer at the new display buffer. The
 *     new draw buffer is one frame behind: the screen module redraws into it
 *     the boxes it drew last time (no full-frame copy).
 */
void ltdc_swap_framebuffers(void);

/*!
 * \brief Fill a rectangle of the draw framebuffer with one color.
 *
 * \details Registered as the raster draw callback; the rectangle is
 *     clipped to the screen and the fill is offloaded to the DMA2D.
 *
 * \param x Top-left X position in pixels.
 * \param y Top-left Y position in pixels.
 * \param w Width in pixels.
 * \param h Height in pixels.
 * \param color Fill color.
 *
 * \retval RASTER_RC_OK on success.
 * \retval RASTER_RC_NULL_POINTER if the target framebuffer is NULL.
 */
enum RasterReturnCode ltdc_draw_rectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, struct Color color);

/*!
 * \brief Paint the whole draw framebuffer black.
 *
 * \retval RASTER_RC_OK on success.
 * \retval RASTER_RC_NULL_POINTER if the target framebuffer is NULL.
 */
enum RasterReturnCode ltdc_clear_screen(void);

/*!
 * \brief Blend one colour through a coverage mask into the draw framebuffer.
 *
 * \details Registered as the raster mask callback: libraster renders a whole
 *     label into the mask scratch and this blends it in one DMA2D transfer.
 *     The rectangle is clipped to the screen.
 *
 * \param x Top-left X position in pixels.
 * \param y Top-left Y position in pixels.
 * \param w Width in pixels (also the mask stride).
 * \param h Height in pixels.
 * \param mask Coverage bytes, \p w * \p h.
 * \param color Text colour; its alpha scales the coverage.
 *
 * \retval RASTER_RC_OK on success.
 * \retval RASTER_RC_NULL_POINTER if the mask is NULL.
 */
enum RasterReturnCode ltdc_draw_mask(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint8_t *mask, struct Color color);

/*!
 * \brief Scratch buffer for ltdc_draw_mask (DMA2D-readable AXI SRAM).
 */
uint8_t *ltdc_mask_buffer(void);

/*!
 * \brief Size of the scratch returned by ltdc_mask_buffer, in bytes.
 */
uint32_t ltdc_mask_size(void);

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __LTDC_H__ */
