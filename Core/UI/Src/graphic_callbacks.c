#include "graphic_callbacks.h"
#include "dma2d.h"
#include "ltdc.h"

uint32_t active_framebuffer = FRAMEBUFFER1_ADDR;
uint32_t writable_framebuffer = FRAMEBUFFER2_ADDR;

void dma2d_m2m(uint32_t src, uint32_t dest, uint32_t width, uint32_t height) {
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
