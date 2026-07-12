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

void dma2d_draw_rectangle(uint32_t *framebuffer, uint16_t x, uint16_t y, uint16_t w, uint16_t h, struct Color color) {
    if (w == 0U || h == 0U) {
        return;
    }

    while (DMA2D->CR & DMA2D_CR_START)
        ;

    uint32_t dst = (uint32_t)(framebuffer + ((uint32_t)y * SCREEN_WIDTH + x));
    uint32_t oor = SCREEN_WIDTH - w;

    if (color.a == 0xFFU) {
        /* Register-to-memory: plain opaque fill. */
        DMA2D->CR = (0x3UL << DMA2D_CR_MODE_Pos);
        DMA2D->OCOLR = color.argb;
        DMA2D->OPFCCR = DMA2D_OUTPUT_ARGB8888;
        DMA2D->OMAR = dst;
        DMA2D->OOR = oor;
        DMA2D->NLR = ((uint32_t)h << 16U) | w;
    } else if (color.a > 0U) {
        /* Memory-to-memory with blending: mix the fill color over the
         * current framebuffer content. */
        DMA2D->CR = (0x2UL << DMA2D_CR_MODE_Pos);

        DMA2D->FGCOLR = color.argb & 0x00FFFFFFU;
        DMA2D->FGPFCCR = DMA2D_INPUT_ARGB8888 | DMA2D_FGPFCCR_AM_0 | ((uint32_t)color.a << DMA2D_FGPFCCR_ALPHA_Pos);
        DMA2D->FGMAR = dst;
        DMA2D->FGOR = oor;

        DMA2D->BGPFCCR = DMA2D_INPUT_ARGB8888;
        DMA2D->BGMAR = dst;
        DMA2D->BGOR = oor;

        DMA2D->OPFCCR = DMA2D_OUTPUT_ARGB8888;
        DMA2D->OMAR = dst;
        DMA2D->OOR = oor;
        DMA2D->NLR = ((uint32_t)h << 16U) | w;
    } else {
        /* Fully transparent: nothing to draw. */
        return;
    }
    DMA2D->CR |= DMA2D_CR_START;
}

/* USER CODE END 1 */
