#include "dma2d_utils.h"
#include <dma2d.h>

void dma2d_m2m(uint32_t src, uint32_t dest, uint32_t width, uint32_t height) {
    hdma2d.Init.Mode         = DMA2D_M2M;
    hdma2d.Init.ColorMode    = DMA2D_OUTPUT_ARGB8888;
    hdma2d.Init.OutputOffset = 0;

    HAL_DMA2D_Init(&hdma2d);

    hdma2d.LayerCfg[1].InputColorMode = DMA2D_INPUT_ARGB8888;
    hdma2d.LayerCfg[1].InputOffset    = 0;

    HAL_DMA2D_ConfigLayer(&hdma2d, 1);

    HAL_DMA2D_Start(&hdma2d, src, dest, width, height);
    HAL_DMA2D_PollForTransfer(&hdma2d, 10);
}

void dma2d_r2m(uint32_t dest, uint32_t color, uint32_t width, uint32_t height) {
    hdma2d.Init.Mode         = DMA2D_R2M;
    hdma2d.Init.ColorMode    = DMA2D_OUTPUT_ARGB8888;
    hdma2d.Init.OutputOffset = SCREEN_WIDTH - width;

    HAL_DMA2D_Init(&hdma2d);

    HAL_DMA2D_Start(&hdma2d, color, dest, width, height);
    HAL_DMA2D_PollForTransfer(&hdma2d, 30);
}
