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

#include "arena-allocator.h"
#include "colors.h"
#include "raster.h"

#include <stdbool.h>

/* USER CODE END Includes */

extern DMA2D_HandleTypeDef hdma2d;

/* USER CODE BEGIN Private defines */

/*!
 * \brief Kinds of DMA2D transfer, for the timing statistics.
 */
enum Dma2dOperation {
    DMA2D_OPERATION_FILL,  /*!< Opaque rectangle fill (register to memory) */
    DMA2D_OPERATION_BLEND, /*!< Translucent rectangle blend */
    DMA2D_OPERATION_MASK,  /*!< A8 coverage mask blend (one label) */
    DMA2D_OPERATION_COPY,  /*!< Full framebuffer copy */
    DMA2D_OPERATION_COUNT,
};

/*!
 * \brief Timing of one kind of DMA2D transfer since the last reset.
 */
struct Dma2dOperationStats {
    uint32_t count;    /*!< Transfers */
    uint32_t total_us; /*!< Sum of their durations */
    uint32_t max_us;   /*!< Longest one */
};

/*!
 * \brief Queue behaviour since the last reset.
 */
struct Dma2dQueueStats {
    struct Dma2dOperationStats frames; /*!< Rendered frames: first queued transfer to the idle callback (the swap) */
    uint32_t queue_waits;              /*!< Times a producer waited for a free queue slot */
    uint32_t mask_waits;               /*!< Times a mask waited for the queued ones to run, to get its space */
};

/* USER CODE END Private defines */

void MX_DMA2D_Init(void);

/* USER CODE BEGIN Prototypes */

/*!
 * \brief Allocate the transfer queue. Call it before the first draw.
 *
 * \param arena Arena the queue is allocated from.
 *
 * \retval true on success.
 * \retval false if the allocation failed.
 */
bool dma2d_queue_init(struct ArenaAllocatorHandler *arena);

/*!
 * \brief DMA2D interrupt: start the next queued transfer, or run the idle callback after the last.
 *
 * \details Replaces HAL_DMA2D_IRQHandler: the transfers are started on the
 *     registers, without the HAL's state, and the next one starts in here, so
 *     the HAL handler must not look at the flags afterwards.
 */
void dma2d_irq_handler(void);

/*!
 * \brief Whether transfers are queued or running, or a when-idle callback is pending.
 *
 * \return true until the queue has drained and its idle callback has run.
 */
bool dma2d_is_busy(void);

/*!
 * \brief Run \p callback once the queue has drained (right away if it is idle).
 *
 * \details From the DMA2D interrupt when the last transfer completes. One
 *     callback at a time: a second call replaces the pending one.
 *
 * \param callback Function to call (interrupt context when deferred).
 */
void dma2d_when_idle(void (*callback)(void));

/*!
 * \brief Frame timings and waits since the last dma2d_reset_stats.
 *
 * \return The statistics.
 */
const struct Dma2dQueueStats *dma2d_get_queue_stats(void);

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
 * \return A RasterReturnCode indicating the success or failure of the operation
 *     (RASTER_RC_ERROR if the queue had no room in time).
 */
enum RasterReturnCode dma2d_enqueue_rectangle(uint32_t *framebuffer, uint16_t x, uint16_t y, uint16_t w, uint16_t h, struct Color color);

/*!
 * \brief Retrieves the count of DMA2D errors (TEIF or CEIF) that have occurred since boot.
 *
 * \return The number of DMA2D errors encountered.
 */
uint32_t dma2d_get_error_count(void);

/*!
 * \brief Transfer timings since the last dma2d_reset_stats, indexed by Dma2dOperation.
 *
 * \details Each transfer is timed from start to completion with the DWT cycle counter.
 *
 * \return Array of DMA2D_OPERATION_COUNT entries.
 */
const struct Dma2dOperationStats *dma2d_get_stats(void);

/*!
 * \brief Zero the transfer timings.
 */
void dma2d_reset_stats(void);

/*!
 * \brief Waits for all queued DMA2D operations to complete, for at most 100 ms.
 */
void dma2d_draw_drain(void);

/*!
 * \brief Copy a full framebuffer (SCREEN_WIDTH * SCREEN_HEIGHT ARGB8888 pixels) via DMA2D M2M.
 *
 * \details Used right after a framebuffer swap to seed the freshly-becoming-draw
 *     buffer with the currently-visible frame, so the next partial-mode render
 *     starts on top of the frame the user is looking at instead of a two-frames-old
 *     back buffer. Queued: returns before the copy has run.
 *
 * \param dst Destination framebuffer (write-only).
 * \param src Source framebuffer (read-only).
 *
 * \retval RASTER_RC_OK on success.
 * \retval RASTER_RC_NULL_POINTER if \p dst or \p src is NULL.
 * \retval RASTER_RC_ERROR if the queue had no room in time.
 */
enum RasterReturnCode dma2d_enqueue_framebuffer_copy(uint32_t *dst, const uint32_t *src);

/*!
 * \brief Blend one color through an A8 coverage mask into a framebuffer.
 *
 * \details One DMA2D memory-to-memory-with-blending transfer: the foreground
 *     is the A8 mask (per-pixel alpha multiplied by \p color's alpha, colour
 *     from \p color), the background and the output are the framebuffer
 *     rectangle. Queued: the mask is copied into AXI SRAM, so the caller may
 *     reuse \p mask as soon as this returns.
 *
 * \param framebuffer Destination framebuffer (stride DMA2D_FRAMEBUFFER_WIDTH).
 * \param x Top-left X of the destination rectangle (already clipped).
 * \param y Top-left Y of the destination rectangle (already clipped).
 * \param w Width to blend.
 * \param h Height to blend.
 * \param mask First coverage byte of the rectangle.
 * \param mask_stride Bytes between two mask rows (>= \p w).
 * \param color Blend colour; its alpha scales the coverage.
 *
 * \retval RASTER_RC_OK on success.
 * \retval RASTER_RC_NULL_POINTER if \p framebuffer or \p mask is NULL.
 * \retval RASTER_RC_ERROR if the mask is too large or the queue had no room in time.
 */
enum RasterReturnCode dma2d_enqueue_mask(uint32_t *framebuffer, uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint8_t *mask, uint16_t mask_stride, struct Color color);

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __DMA2D_H__ */
