#include "graphic_callbacks.h"
#include "dma2d.h"
#include "ltdc.h"
#include "stm32h7xx_hal_dma2d.h"

uint32_t active_framebuffer = FRAMEBUFFER1_ADDR;
uint32_t writable_framebuffer = FRAMEBUFFER2_ADDR;

volatile bool dma2d_transfer_complete = false;

void HAL_DMA2D_TransferCompleteCallback(DMA2D_HandleTypeDef *hdma2d_handle) {
    // Check if the callback is for the correct DMA2D instance (if you have multiple)
    if (hdma2d_handle->Instance == DMA2D) {
        dma2d_transfer_complete = true; // Signal completion
        // You can add logic here that needs to run immediately after a batch is done,
        // or just rely on the flag for the main loop to check.
    }
}

void clean_alphas_buffer(uint8_t *alphas) {
    memset(alphas, 0x00, 150 * 150);
}

static void DMA2D_ReconfigureForBlendMode(void) {
    while (!dma2d_transfer_complete);

    hdma2d.Init.Mode = DMA2D_M2M_BLEND;
    hdma2d.Init.ColorMode = DMA2D_OUTPUT_ARGB8888;
    hdma2d.Init.OutputOffset = 0;

    if (HAL_DMA2D_Init(&hdma2d) != HAL_OK) {
        Error_Handler();
    }

    hdma2d.LayerCfg[DMA2D_FOREGROUND_LAYER].InputOffset = 0;
    hdma2d.LayerCfg[DMA2D_FOREGROUND_LAYER].InputColorMode = DMA2D_INPUT_A8;
    hdma2d.LayerCfg[DMA2D_FOREGROUND_LAYER].AlphaMode = DMA2D_COMBINE_ALPHA;
    hdma2d.LayerCfg[DMA2D_FOREGROUND_LAYER].InputAlpha = 0xFF;

    if (HAL_DMA2D_ConfigLayer(&hdma2d, DMA2D_FOREGROUND_LAYER) != HAL_OK) {
        Error_Handler();
    }

    hdma2d.LayerCfg[DMA2D_BACKGROUND_LAYER].InputOffset = 0;
    hdma2d.LayerCfg[DMA2D_BACKGROUND_LAYER].InputColorMode = DMA2D_INPUT_ARGB8888;
    hdma2d.LayerCfg[DMA2D_BACKGROUND_LAYER].AlphaMode = DMA2D_NO_MODIF_ALPHA;
    hdma2d.LayerCfg[DMA2D_BACKGROUND_LAYER].InputAlpha = 0xFF;

    if (HAL_DMA2D_ConfigLayer(&hdma2d, DMA2D_BACKGROUND_LAYER) != HAL_OK) {
        Error_Handler();
    }
}

void dma2d_m2m(uint32_t src, uint32_t dest, uint32_t width, uint32_t height) {
    while (!dma2d_transfer_complete);

    hdma2d.Init.Mode = DMA2D_M2M;
    hdma2d.Init.ColorMode = DMA2D_OUTPUT_ARGB8888;
    hdma2d.Init.OutputOffset = 0;

    HAL_DMA2D_Init(&hdma2d);

    hdma2d.LayerCfg[1].InputColorMode = DMA2D_INPUT_ARGB8888;
    hdma2d.LayerCfg[1].InputOffset = 0;

    HAL_DMA2D_ConfigLayer(&hdma2d, 1);

    HAL_DMA2D_Start(&hdma2d, src, dest, width, height);
    HAL_DMA2D_PollForTransfer(&hdma2d, 10);
}

void dma2d_r2m(uint32_t dest, uint32_t color, uint32_t width, uint32_t height) {
    while (!dma2d_transfer_complete);

    hdma2d.Init.Mode = DMA2D_R2M;
    hdma2d.Init.ColorMode = DMA2D_OUTPUT_ARGB8888;
    hdma2d.Init.OutputOffset = SCREEN_WIDTH - width;

    HAL_DMA2D_Init(&hdma2d);

    HAL_DMA2D_Start(&hdma2d, color, dest, width, height);
    HAL_DMA2D_PollForTransfer(&hdma2d, 30);
}

void draw_rectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint32_t color) {
    dma2d_r2m(writable_framebuffer + (x + y * SCREEN_WIDTH) * 4, color, w, h);
}

void draw_line(uint16_t x, uint16_t y, uint16_t lenght, uint32_t color) {
    draw_rectangle(x, y, lenght, 1, color);
}

void copy_batch(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint32_t color, uint8_t *alphas) {
    while (!dma2d_transfer_complete);

    if (w == 0 || h == 0) {
        return; // Nothing to draw
    }

    // Calculate the target address in the writable framebuffer
    uint32_t framebuffer_target_addr = writable_framebuffer + (y * SCREEN_WIDTH + x) * 4;

    // 1. Ensure DMA2D is configured for M2M_BLEND mode
    DMA2D_ReconfigureForBlendMode();

    // 2. Set the Foreground color for the blending operation
    hdma2d.Instance->FGCOLR = color;

    // 3. Cache Coherency: Clean D-Cache for the 'alphas' buffer
    SCB_CleanDCache_by_Addr((uint32_t *)alphas, w * h);

    // 4. Reset the completion flag before starting a new transfer
    dma2d_transfer_complete = false;

    // 5. Start the DMA2D transfer for blending using the INTERRUPT-driven function
    // Parameters for HAL_DMA2D_BlendingStart_IT:
    //  - hdma2d handle
    //  - pdata (Foreground Address - alpha mask)
    //  - pbackgrounddata (Background Address - current framebuffer content)
    //  - poutputdata (Output Address - SAME AS BACKGROUND for in-place blending)
    //  - xsize (Width)
    //  - ysize (Height)
    if (HAL_DMA2D_BlendingStart_IT(&hdma2d,
                                   (uint32_t)alphas,        // pdata (Foreground)
                                   framebuffer_target_addr, // pbackgrounddata (Background)
                                   framebuffer_target_addr, // poutputdata (Output)
                                   w,
                                   h) != HAL_OK) { // xsize, ysize
        Error_Handler();
    }
}

void clear_screen() {
    draw_rectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0xffffffff);
}

void fix_print_interface() {
    draw_rectangle(3, 3, 1, 1, 0x00ff0000);
    draw_rectangle(402, 242, 1, 1, 0x00ff0000);
}

void swap_framebuffers() {
    dma2d_m2m(writable_framebuffer, active_framebuffer, SCREEN_WIDTH, SCREEN_HEIGHT);
    uint32_t tmp = active_framebuffer;
    active_framebuffer = writable_framebuffer;
    writable_framebuffer = tmp;
    HAL_LTDC_SetAddress(&hltdc, active_framebuffer, LTDC_LAYER_1);
}
