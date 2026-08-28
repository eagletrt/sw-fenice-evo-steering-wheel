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

#include "eagletrt.h"

/* USER CODE END 0 */

DMA2D_HandleTypeDef hdma2d;

/* DMA2D init function */
void MX_DMA2D_Init(void) {
    /* USER CODE BEGIN DMA2D_Init 0 */

    /* USER CODE END DMA2D_Init 0 */

    /* USER CODE BEGIN DMA2D_Init 1 */

    /* USER CODE END DMA2D_Init 1 */
    hdma2d.Instance                      = DMA2D;
    hdma2d.Init.Mode                     = DMA2D_M2M;
    hdma2d.Init.ColorMode                = DMA2D_OUTPUT_ARGB8888;
    hdma2d.Init.OutputOffset             = 0;
    hdma2d.LayerCfg[1].InputOffset       = 0;
    hdma2d.LayerCfg[1].InputColorMode    = DMA2D_INPUT_ARGB8888;
    hdma2d.LayerCfg[1].AlphaMode         = DMA2D_NO_MODIF_ALPHA;
    hdma2d.LayerCfg[1].InputAlpha        = 0;
    hdma2d.LayerCfg[1].AlphaInverted     = DMA2D_REGULAR_ALPHA;
    hdma2d.LayerCfg[1].RedBlueSwap       = DMA2D_RB_REGULAR;
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

#define DMA2D_FRAMEBUFFER_WIDTH (800U)  /*!< Framebuffer stride, in pixels */
#define DMA2D_FRAMEBUFFER_HEIGHT (480U) /*!< Framebuffer height, in lines */

EAGLETRT_STATIC volatile uint32_t dma2d_error_counter = 0; /*!< Count of DMA2D errors (TEIF or CEIF) since boot */

enum RasterReturnCode dma2d_enqueue_rectangle(uint32_t *framebuffer, uint16_t x, uint16_t y, uint16_t w, uint16_t h, struct Color color) {
    if (framebuffer == NULL) {
        return RASTER_RC_NULL_POINTER;
    }
    if (w == 0 || h == 0 || color.a == 0) {
        return RASTER_RC_OK; /* nothing to draw */
    }

    /* Wait for any previous fill, then clear stale flags before starting. */
    while (DMA2D->CR & DMA2D_CR_START) {
    }
    DMA2D->IFCR = DMA2D_IFCR_CTCIF | DMA2D_IFCR_CTEIF | DMA2D_IFCR_CCEIF;

    uint32_t dst = (uint32_t)(framebuffer + (uint32_t)y * DMA2D_FRAMEBUFFER_WIDTH + x);
    uint32_t oor = DMA2D_FRAMEBUFFER_WIDTH - w;

    if (color.a == 0xFF) {
        DMA2D->OCOLR = color.argb;
        DMA2D->OPFCCR = DMA2D_OUTPUT_ARGB8888;
        DMA2D->OMAR = dst;
        DMA2D->OOR = oor;
        DMA2D->NLR = ((uint32_t)w << DMA2D_NLR_PL_Pos) | h; // PL=width [29:16], NL=height [15:0]
        DMA2D->CR = (0x3UL << DMA2D_CR_MODE_Pos);
    } else {
        /* Constant colour + constant alpha blended over the framebuffer. Foreground is an A8
         * source whose per-pixel alpha is REPLACED (AM_0) by the constant alpha, so the colour
         * comes from FGCOLR and the FG memory content is irrelevant. ARGB8888 here would read
         * the framebuffer as the colour -> garbage. */
        DMA2D->FGCOLR = color.argb & 0x00FFFFFF;
        DMA2D->FGPFCCR = DMA2D_INPUT_A8 | DMA2D_FGPFCCR_AM_0 | ((uint32_t)color.a << DMA2D_FGPFCCR_ALPHA_Pos);
        DMA2D->FGMAR = dst;
        DMA2D->FGOR = oor;

        DMA2D->BGPFCCR = DMA2D_INPUT_ARGB8888;
        DMA2D->BGMAR = dst;
        DMA2D->BGOR = oor;

        DMA2D->OPFCCR = DMA2D_OUTPUT_ARGB8888;
        DMA2D->OMAR = dst;
        DMA2D->OOR = oor;
        DMA2D->NLR = ((uint32_t)w << DMA2D_NLR_PL_Pos) | h; // PL=width [29:16], NL=height [15:0]
        DMA2D->CR = (0x2UL << DMA2D_CR_MODE_Pos);
    }

    DMA2D->CR |= DMA2D_CR_START;
    while (DMA2D->CR & DMA2D_CR_START) {
    }
    if (DMA2D->ISR & (DMA2D_ISR_TEIF | DMA2D_ISR_CEIF)) {
        dma2d_error_counter++;
    }

    return RASTER_RC_OK;
}

uint32_t dma2d_get_error_count(void) {
    return dma2d_error_counter;
}

void dma2d_draw_drain(void) {
    while (DMA2D->CR & DMA2D_CR_START) {
    }
}

enum RasterReturnCode dma2d_enqueue_framebuffer_copy(uint32_t *dst, const uint32_t *src) {
    if (dst == NULL || src == NULL) {
        return RASTER_RC_NULL_POINTER;
    }

    /* Wait for any previous fill, then clear stale flags before starting. */
    while (DMA2D->CR & DMA2D_CR_START) {
    }
    DMA2D->IFCR = DMA2D_IFCR_CTCIF | DMA2D_IFCR_CTEIF | DMA2D_IFCR_CCEIF;

    /* Foreground = source framebuffer, straight ARGB8888, no PFC, no offset. */
    DMA2D->FGMAR = (uint32_t)src;
    DMA2D->FGOR = 0;
    DMA2D->FGPFCCR = DMA2D_INPUT_ARGB8888;

    DMA2D->OMAR = (uint32_t)dst;
    DMA2D->OOR = 0;
    DMA2D->OPFCCR = DMA2D_OUTPUT_ARGB8888;

    DMA2D->NLR = ((uint32_t)DMA2D_FRAMEBUFFER_WIDTH << DMA2D_NLR_PL_Pos) | DMA2D_FRAMEBUFFER_HEIGHT;
    DMA2D->CR = (0x0UL << DMA2D_CR_MODE_Pos); /* M2M, no PFC, no blend */
    DMA2D->CR |= DMA2D_CR_START;
    while (DMA2D->CR & DMA2D_CR_START) {
    }
    if (DMA2D->ISR & (DMA2D_ISR_TEIF | DMA2D_ISR_CEIF)) {
        dma2d_error_counter++;
    }

    return RASTER_RC_OK;
}

/* USER CODE END 1 */
