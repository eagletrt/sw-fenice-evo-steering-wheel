/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    dma2d.c
 * @brief   This file provides code for the configuration
 *          of the DMA2D instances.
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
/* Includes ------------------------------------------------------------------*/
#include "dma2d.h"

/* USER CODE BEGIN 0 */

#include "eagletrt-api.h"
#include "ring-buffer-api.h"

/* USER CODE END 0 */

DMA2D_HandleTypeDef hdma2d;

/* DMA2D init function */
void MX_DMA2D_Init(void) {

    /* USER CODE BEGIN DMA2D_Init 0 */

    /* USER CODE END DMA2D_Init 0 */

    /* USER CODE BEGIN DMA2D_Init 1 */

    /* USER CODE END DMA2D_Init 1 */
    hdma2d.Instance = DMA2D;
    hdma2d.Init.Mode = DMA2D_M2M;
    hdma2d.Init.ColorMode = DMA2D_OUTPUT_ARGB8888;
    hdma2d.Init.OutputOffset = 0;
    hdma2d.LayerCfg[1].InputOffset = 0;
    hdma2d.LayerCfg[1].InputColorMode = DMA2D_INPUT_ARGB8888;
    hdma2d.LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
    hdma2d.LayerCfg[1].InputAlpha = 0;
    hdma2d.LayerCfg[1].AlphaInverted = DMA2D_REGULAR_ALPHA;
    hdma2d.LayerCfg[1].RedBlueSwap = DMA2D_RB_REGULAR;
    hdma2d.LayerCfg[1].ChromaSubSampling = DMA2D_NO_CSS;
    if (HAL_DMA2D_Init(&hdma2d) != HAL_OK) {
        Error_Handler();
    }
    if (HAL_DMA2D_ConfigLayer(&hdma2d, 1) != HAL_OK) {
        Error_Handler();
    }
    /* USER CODE BEGIN DMA2D_Init 2 */

    /* USER CODE END DMA2D_Init 2 */
}

void HAL_DMA2D_MspInit(DMA2D_HandleTypeDef *dma2dHandle) {

    if (dma2dHandle->Instance == DMA2D) {
        /* USER CODE BEGIN DMA2D_MspInit 0 */

        /* USER CODE END DMA2D_MspInit 0 */
        /* DMA2D clock enable */
        __HAL_RCC_DMA2D_CLK_ENABLE();

        /* DMA2D interrupt Init */
        HAL_NVIC_SetPriority(DMA2D_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(DMA2D_IRQn);
        /* USER CODE BEGIN DMA2D_MspInit 1 */

        /* USER CODE END DMA2D_MspInit 1 */
    }
}

void HAL_DMA2D_MspDeInit(DMA2D_HandleTypeDef *dma2dHandle) {

    if (dma2dHandle->Instance == DMA2D) {
        /* USER CODE BEGIN DMA2D_MspDeInit 0 */

        /* USER CODE END DMA2D_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_DMA2D_CLK_DISABLE();

        /* DMA2D interrupt Deinit */
        HAL_NVIC_DisableIRQ(DMA2D_IRQn);
        /* USER CODE BEGIN DMA2D_MspDeInit 1 */

        /* USER CODE END DMA2D_MspDeInit 1 */
    }
}

/* USER CODE BEGIN 1 */

#include <string.h>

#define DMA2D_FRAMEBUFFER_WIDTH (800U)  /*!< Framebuffer stride, in pixels */
#define DMA2D_FRAMEBUFFER_HEIGHT (480U) /*!< Framebuffer height, in lines */

/*!
 * \brief Transfers that can wait for the running one.
 *
 * \details A full dashboard redraw is ~66 transfers; a full queue makes the
 *     producer wait for the oldest one to start.
 */
#define DMA2D_QUEUE_LENGTH (128U)

/*!
 * \brief Longest wait for a queue slot or mask space, in milliseconds.
 *
 * \details A deadline on the HAL tick, not a timeout from the pool: the pool
 *     doesn't run while the main loop is spinning here. Far longer than any
 *     transfer (~17 ms for a full screen).
 */
#define DMA2D_WAIT_TIMEOUT_MS (100U)

/*!
 * \brief Where queued coverage masks are copied, until the queue drains.
 *
 * \details AXI SRAM (RAM_D1, unused by the linker script) right after the
 *     128 KB libraster scratch at 0x24000000: the scratch is overwritten by
 *     the next label before a queued mask has been blended. The copies are
 *     filled front to back and all given back when the queue is empty.
 */
#define DMA2D_MASK_BUFFER_ADDRESS (0x24020000U)
#define DMA2D_MASK_BUFFER_SIZE (192U * 1024U)

/*!
 * \brief D-cache line size: mask copies start on a line so cleaning them touches nothing else.
 */
#define DMA2D_CACHE_LINE (32U)

/*!
 * \brief Interrupts the queue runs on (transfer complete, transfer error, configuration error).
 */
#define DMA2D_CR_IRQS (DMA2D_CR_TCIE | DMA2D_CR_TEIE | DMA2D_CR_CEIE)

/*!
 * \brief One queued transfer: the register values to program.
 */
struct Dma2dCommand {
    enum Dma2dOperation operation; /*!< Kind, for the statistics */
    uint32_t mode;                 /*!< CR.MODE */
    uint32_t omar;                 /*!< Output address */
    uint32_t oor;                  /*!< Output line offset */
    uint32_t nlr;                  /*!< Pixels per line and lines */
    uint32_t ocolr;                /*!< Fill colour (register to memory) */
    uint32_t fgmar;                /*!< Foreground address */
    uint32_t fgor;                 /*!< Foreground line offset */
    uint32_t fgpfccr;              /*!< Foreground format and alpha */
    uint32_t fgcolr;               /*!< Foreground colour (A8 inputs) */
    uint32_t bgmar;                /*!< Background address (blends) */
    uint32_t bgor;                 /*!< Background line offset */
};

EAGLETRT_STATIC volatile uint32_t dma2d_error_counter = 0; /*!< Count of DMA2D errors (TEIF or CEIF) since boot */

EAGLETRT_STATIC struct Dma2dOperationStats dma2d_stats[DMA2D_OPERATION_COUNT]; /*!< Transfer timings since the last reset */
EAGLETRT_STATIC struct Dma2dQueueStats dma2d_queue_stats;                      /*!< Frame timings and waits since the last reset */

EAGLETRT_STATIC struct RingBufferHandler dma2d_queue;        /*!< Transfers waiting for the running one; shared with the interrupt, always under prv_dma2d_lock */
EAGLETRT_STATIC volatile bool dma2d_running;                 /*!< A transfer is in progress */
EAGLETRT_STATIC enum Dma2dOperation dma2d_running_operation; /*!< Kind of the running transfer */
EAGLETRT_STATIC uint32_t dma2d_transfer_start;               /*!< Cycle counter at the start of the running transfer */
EAGLETRT_STATIC uint32_t dma2d_frame_start;                  /*!< Cycle counter at the first transfer queued since the last idle callback */
EAGLETRT_STATIC volatile bool dma2d_frame_open;              /*!< Transfers were queued since the last idle callback */
EAGLETRT_STATIC void (*volatile dma2d_idle_callback)(void);  /*!< Called once when the queue drains (the swap) */
EAGLETRT_STATIC uint32_t dma2d_mask_used;                    /*!< Bytes of the mask buffer holding queued masks (main context only) */

/*!
 * \brief Enter a short critical section shared with the DMA2D interrupt.
 *
 * \return The PRIMASK to restore.
 */
EAGLETRT_STATIC uint32_t prv_dma2d_lock(void) {
    const uint32_t primask = __get_PRIMASK();
    __disable_irq();
    return primask;
}

/*!
 * \brief Leave a critical section entered with prv_dma2d_lock.
 *
 * \param primask Value returned by prv_dma2d_lock.
 */
EAGLETRT_STATIC void prv_dma2d_unlock(uint32_t primask) {
    __set_PRIMASK(primask);
}

/*!
 * \brief Whether the queue has a free slot.
 */
EAGLETRT_STATIC bool prv_dma2d_has_slot(void) {
    return !ring_buffer_api_is_full(&dma2d_queue);
}

/*!
 * \brief Whether no transfer is running, so nothing is queued either.
 */
EAGLETRT_STATIC bool prv_dma2d_is_idle(void) {
    return !dma2d_running;
}

/*!
 * \brief Whether the last frame is completely done, swap included.
 */
EAGLETRT_STATIC bool prv_dma2d_is_drained(void) {
    return !dma2d_is_busy();
}

/*!
 * \brief Wait (interrupts enabled) for the interrupt to make a condition true.
 *
 * \param condition The condition.
 *
 * \retval true if it came true.
 * \retval false if it didn't within DMA2D_WAIT_TIMEOUT_MS: the engine isn't making progress.
 */
EAGLETRT_STATIC bool prv_dma2d_wait_for(bool (*condition)(void)) {
    const uint32_t start = HAL_GetTick();
    while (!condition()) {
        if (HAL_GetTick() - start >= DMA2D_WAIT_TIMEOUT_MS) {
            return false;
        }
    }
    return true;
}

/*!
 * \brief Program and start one transfer. The engine must be idle.
 *
 * \param command The transfer.
 */
EAGLETRT_STATIC void prv_dma2d_start(const struct Dma2dCommand *command) {
    DMA2D->IFCR = DMA2D_IFCR_CTCIF | DMA2D_IFCR_CTEIF | DMA2D_IFCR_CCEIF;
    DMA2D->OPFCCR = DMA2D_OUTPUT_ARGB8888;
    DMA2D->OMAR = command->omar;
    DMA2D->OOR = command->oor;
    DMA2D->NLR = command->nlr;
    if (command->operation == DMA2D_OPERATION_FILL) {
        DMA2D->OCOLR = command->ocolr;
    } else {
        DMA2D->FGMAR = command->fgmar;
        DMA2D->FGOR = command->fgor;
        DMA2D->FGPFCCR = command->fgpfccr;
        DMA2D->FGCOLR = command->fgcolr;
        if (command->operation != DMA2D_OPERATION_COPY) {
            DMA2D->BGPFCCR = DMA2D_INPUT_ARGB8888;
            DMA2D->BGMAR = command->bgmar;
            DMA2D->BGOR = command->bgor;
        }
    }
    dma2d_running_operation = command->operation;
    dma2d_transfer_start = DWT->CYCCNT;
    DMA2D->CR = (command->mode << DMA2D_CR_MODE_Pos) | DMA2D_CR_IRQS | DMA2D_CR_START;
}

/*!
 * \brief Microseconds since a cycle counter value.
 */
EAGLETRT_STATIC uint32_t prv_dma2d_elapsed_us(uint32_t start) {
    return (DWT->CYCCNT - start) / (SystemCoreClock / 1000000U);
}

/*!
 * \brief Add a duration to a statistics entry.
 */
EAGLETRT_STATIC void prv_dma2d_account(struct Dma2dOperationStats *stats, uint32_t elapsed_us) {
    stats->count++;
    stats->total_us += elapsed_us;
    if (elapsed_us > stats->max_us) {
        stats->max_us = elapsed_us;
    }
}

/*!
 * \brief The frame's transfers have all run and its idle callback (the swap) is about to: time it.
 *
 * \details The queue may drain several times during a frame (the DMA2D
 *     catching up with the CPU): a frame spans from its first queued
 *     transfer to the idle callback.
 */
EAGLETRT_STATIC void prv_dma2d_close_frame(void) {
    if (dma2d_frame_open) {
        dma2d_frame_open = false;
        prv_dma2d_account(&dma2d_queue_stats.frames, prv_dma2d_elapsed_us(dma2d_frame_start));
    }
}

/*!
 * \brief Append a transfer, or start it if the engine is idle.
 *
 * \details Waits while the queue is full.
 *
 * \param command The transfer, copied.
 *
 * \retval RASTER_RC_OK on success.
 * \retval RASTER_RC_ERROR if no slot freed up in time.
 */
EAGLETRT_STATIC enum RasterReturnCode prv_dma2d_push(const struct Dma2dCommand *command) {
    if (!prv_dma2d_has_slot()) {
        dma2d_queue_stats.queue_waits++;
        if (!prv_dma2d_wait_for(prv_dma2d_has_slot)) {
            return RASTER_RC_ERROR;
        }
    }

    enum RasterReturnCode return_code = RASTER_RC_OK;
    const uint32_t primask = prv_dma2d_lock();
    if (!dma2d_frame_open) {
        dma2d_frame_open = true;
        dma2d_frame_start = DWT->CYCCNT;
    }
    if (dma2d_running) {
        if (ring_buffer_api_push_back(&dma2d_queue, command) != RING_BUFFER_RC_OK) {
            return_code = RASTER_RC_ERROR;
        }
    } else {
        dma2d_running = true;
        prv_dma2d_start(command);
    }
    prv_dma2d_unlock(primask);
    return return_code;
}

/*!
 * \brief Reserve space for a mask copy, on a cache line.
 *
 * \details The interrupt only ever stops the engine and only this context
 *     starts it, so once it is idle nothing reads the buffer: it is all free
 *     again. If a frame's masks don't fit, waits for the queued ones to run.
 *
 * \param size Bytes needed, whole cache lines.
 *
 * \return The block, NULL if it is larger than the whole buffer or the queue didn't drain in time.
 */
EAGLETRT_STATIC uint8_t *prv_dma2d_mask_alloc(uint32_t size) {
    if (size > DMA2D_MASK_BUFFER_SIZE) {
        return NULL;
    }
    if (dma2d_running && dma2d_mask_used + size > DMA2D_MASK_BUFFER_SIZE) {
        dma2d_queue_stats.mask_waits++;
        if (!prv_dma2d_wait_for(prv_dma2d_is_idle)) {
            return NULL;
        }
    }
    if (!dma2d_running) {
        dma2d_mask_used = 0U;
    }
    uint8_t *block = (uint8_t *)(DMA2D_MASK_BUFFER_ADDRESS + dma2d_mask_used);
    dma2d_mask_used += size;
    return block;
}

bool dma2d_queue_init(struct ArenaAllocatorHandler *arena) {
    /* No ring buffer critical section: its cs_exit would re-enable the interrupt
     * inside prv_dma2d_lock. Every access is either under that lock or in the
     * interrupt itself. */
    return ring_buffer_api_init(&dma2d_queue, sizeof(struct Dma2dCommand), DMA2D_QUEUE_LENGTH, NULL, NULL, arena) == RING_BUFFER_RC_OK;
}

void dma2d_irq_handler(void) {
    /* One snapshot, one completion: a transfer error can come with the complete
     * flag, and the next transfer starts below (its flags aren't these). */
    const uint32_t flags = DMA2D->ISR & (DMA2D_ISR_TEIF | DMA2D_ISR_TCIF | DMA2D_ISR_CEIF);
    if (flags == 0U) {
        return;
    }
    DMA2D->IFCR = flags; /* Same bit positions as the ISR */
    if (!dma2d_running) {
        return;
    }

    if ((flags & (DMA2D_ISR_TEIF | DMA2D_ISR_CEIF)) != 0U) {
        dma2d_error_counter++;
    }
    prv_dma2d_account(&dma2d_stats[dma2d_running_operation], prv_dma2d_elapsed_us(dma2d_transfer_start));

    struct Dma2dCommand next;
    if (ring_buffer_api_pop_front(&dma2d_queue, &next) == RING_BUFFER_RC_OK) {
        prv_dma2d_start(&next);
        return;
    }

    dma2d_running = false;
    void (*callback)(void) = dma2d_idle_callback;
    dma2d_idle_callback = NULL;
    if (callback != NULL) {
        prv_dma2d_close_frame();
        callback();
    }
}

enum RasterReturnCode dma2d_enqueue_rectangle(uint32_t *framebuffer, uint16_t x, uint16_t y, uint16_t w, uint16_t h, struct Color color) {
    if (framebuffer == NULL) {
        return RASTER_RC_NULL_POINTER;
    }
    if (w == 0 || h == 0 || color.a == 0) {
        return RASTER_RC_OK; /* nothing to draw */
    }

    const uint32_t dst = (uint32_t)(framebuffer + (uint32_t)y * DMA2D_FRAMEBUFFER_WIDTH + x);
    struct Dma2dCommand command = {
        .omar = dst,
        .oor = DMA2D_FRAMEBUFFER_WIDTH - w,
        .nlr = ((uint32_t)w << DMA2D_NLR_PL_Pos) | h, // PL=width [29:16], NL=height [15:0]
    };
    if (color.a == 0xFF) {
        command.operation = DMA2D_OPERATION_FILL;
        command.mode = 0x3UL; /* register to memory */
        command.ocolr = color.argb;
    } else {
        /* Constant colour + constant alpha blended over the framebuffer. Foreground is an A8
         * source whose per-pixel alpha is REPLACED (AM_0) by the constant alpha, so the colour
         * comes from FGCOLR and the FG memory content is irrelevant. ARGB8888 here would read
         * the framebuffer as the colour -> garbage. */
        command.operation = DMA2D_OPERATION_BLEND;
        command.mode = 0x2UL; /* memory to memory with blending */
        command.fgcolr = color.argb & 0x00FFFFFFU;
        command.fgpfccr = DMA2D_INPUT_A8 | DMA2D_FGPFCCR_AM_0 | ((uint32_t)color.a << DMA2D_FGPFCCR_ALPHA_Pos);
        command.fgmar = dst;
        command.fgor = command.oor;
        command.bgmar = dst;
        command.bgor = command.oor;
    }
    return prv_dma2d_push(&command);
}

const struct Dma2dOperationStats *dma2d_get_stats(void) {
    return dma2d_stats;
}

const struct Dma2dQueueStats *dma2d_get_queue_stats(void) {
    return &dma2d_queue_stats;
}

void dma2d_reset_stats(void) {
    /* The interrupt updates them. */
    const uint32_t primask = prv_dma2d_lock();
    memset(dma2d_stats, 0, sizeof(dma2d_stats));
    memset(&dma2d_queue_stats, 0, sizeof(dma2d_queue_stats));
    prv_dma2d_unlock(primask);
}

uint32_t dma2d_get_error_count(void) {
    return dma2d_error_counter;
}

bool dma2d_is_busy(void) {
    return dma2d_running || dma2d_idle_callback != NULL;
}

void dma2d_when_idle(void (*callback)(void)) {
    const uint32_t primask = prv_dma2d_lock();
    const bool idle = !dma2d_running;
    if (idle) {
        prv_dma2d_close_frame();
    } else {
        dma2d_idle_callback = callback;
    }
    prv_dma2d_unlock(primask);
    if (idle && callback != NULL) {
        callback();
    }
}

void dma2d_draw_drain(void) {
    EAGLETRT_API_UNUSED(prv_dma2d_wait_for(prv_dma2d_is_drained));
}

enum RasterReturnCode dma2d_enqueue_framebuffer_copy(uint32_t *dst, const uint32_t *src) {
    if (dst == NULL || src == NULL) {
        return RASTER_RC_NULL_POINTER;
    }

    /* Foreground = source framebuffer, straight ARGB8888, no PFC, no offset. */
    const struct Dma2dCommand command = {
        .operation = DMA2D_OPERATION_COPY,
        .mode = 0x0UL, /* M2M, no PFC, no blend */
        .omar = (uint32_t)dst,
        .oor = 0U,
        .nlr = ((uint32_t)DMA2D_FRAMEBUFFER_WIDTH << DMA2D_NLR_PL_Pos) | DMA2D_FRAMEBUFFER_HEIGHT,
        .fgmar = (uint32_t)src,
        .fgor = 0U,
        .fgpfccr = DMA2D_INPUT_ARGB8888,
    };
    return prv_dma2d_push(&command);
}

enum RasterReturnCode dma2d_enqueue_mask(uint32_t *framebuffer, uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint8_t *mask, uint16_t mask_stride, struct Color color) {
    if (framebuffer == NULL || mask == NULL) {
        return RASTER_RC_NULL_POINTER;
    }
    if (w == 0 || h == 0 || color.a == 0) {
        return RASTER_RC_OK; /* nothing to draw */
    }

    /* The caller's mask is reused for the next label: keep a copy until the
     * transfer has run, rows packed (stride w), then push it to the SRAM
     * before the DMA2D reads it. */
    const uint32_t size = ((uint32_t)w * h + DMA2D_CACHE_LINE - 1U) & ~(DMA2D_CACHE_LINE - 1U);
    uint8_t *copy = prv_dma2d_mask_alloc(size);
    if (copy == NULL) {
        return RASTER_RC_ERROR;
    }
    for (uint16_t row = 0U; row < h; row++) {
        memcpy(&copy[(uint32_t)row * w], &mask[(uint32_t)row * mask_stride], w);
    }
    SCB_CleanDCache_by_Addr((uint32_t *)copy, (int32_t)size);

    const uint32_t dst = (uint32_t)(framebuffer + (uint32_t)y * DMA2D_FRAMEBUFFER_WIDTH + x);
    /* Foreground: the coverage mask as A8, alpha MULTIPLIED (AM = 10) by the
     * colour's alpha, colour from FGCOLR. Background and output: the
     * framebuffer rectangle, so the text is blended over what is there. */
    const struct Dma2dCommand command = {
        .operation = DMA2D_OPERATION_MASK,
        .mode = 0x2UL, /* memory to memory with blending */
        .omar = dst,
        .oor = DMA2D_FRAMEBUFFER_WIDTH - w,
        .nlr = ((uint32_t)w << DMA2D_NLR_PL_Pos) | h, // PL=width [29:16], NL=height [15:0]
        .fgmar = (uint32_t)copy,
        .fgor = 0U,
        .fgpfccr = DMA2D_INPUT_A8 | DMA2D_FGPFCCR_AM_1 | ((uint32_t)color.a << DMA2D_FGPFCCR_ALPHA_Pos),
        .fgcolr = color.argb & 0x00FFFFFFU,
        .bgmar = dst,
        .bgor = DMA2D_FRAMEBUFFER_WIDTH - w,
    };
    return prv_dma2d_push(&command);
}

/* USER CODE END 1 */
