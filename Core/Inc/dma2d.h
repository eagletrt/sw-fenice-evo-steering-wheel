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
#include "raster.h"

/* USER CODE END Includes */

extern DMA2D_HandleTypeDef hdma2d;

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

void MX_DMA2D_Init(void);

/* USER CODE BEGIN Prototypes */

/*!
 * \brief Enqueues a rectangle fill operation to the DMA2D engine.
 *
 * \param framebuffer Pointer to the framebuffer where the rectangle will be drawn.
 * \param x The x-coordinate of the top-left corner of the rectangle.
 * \param y The y-coordinate of the top-left corner of the rectangle.
 * \param w The width of the rectangle.
 * \param h The height of the rectangle.
 * \param color The color to fill the rectangle with, in ARGB format.
 *
 * \return A RasterReturnCode indicating the success or failure of the operation.
 */
enum RasterReturnCode dma2d_enqueue_rectangle(uint32_t *framebuffer, uint16_t x, uint16_t y, uint16_t w, uint16_t h, struct Color color);

/*!
 * \brief Retrieves the count of DMA2D errors (TEIF or CEIF) that have occurred since boot.
 *
 * \return The number of DMA2D errors encountered.
 */
uint32_t dma2d_get_error_count(void);

/*!
 * \brief Waits for all queued DMA2D operations to complete.
 *
 * This function blocks until the DMA2D engine has finished processing all pending operations.
 */
void dma2d_draw_drain(void);

/*!
 * \brief Copy a full framebuffer (SCREEN_WIDTH * SCREEN_HEIGHT ARGB8888 pixels) via DMA2D M2M.
 *
 * \details Used right after a framebuffer swap to seed the freshly-becoming-draw
 *     buffer with the currently-visible frame, so the next partial-mode render
 *     starts on top of the frame the user is looking at instead of a two-frames-old
 *     back buffer. Busy-waits until the DMA2D engine reports completion.
 *
 * \param dst Destination framebuffer (write-only).
 * \param src Source framebuffer (read-only).
 *
 * \retval RASTER_RC_OK on success.
 * \retval RASTER_RC_NULL_POINTER if \p dst or \p src is NULL.
 */
enum RasterReturnCode dma2d_enqueue_framebuffer_copy(uint32_t *dst, const uint32_t *src);

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __DMA2D_H__ */
