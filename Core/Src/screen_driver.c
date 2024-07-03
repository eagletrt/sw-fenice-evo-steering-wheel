#include "steering_config.h"

#include <screen_driver.h>
#include <string.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))

#define BYTE_DIMENSION 2

#define LCD_SCREEN_WIDTH  SCREEN_WIDTH
#define LCD_SCREEN_HEIGHT SCREEN_HEIGHT
#define FRAMEBUFFER_SIZE  (uint32_t)(LCD_SCREEN_HEIGHT * LCD_SCREEN_WIDTH * BYTE_DIMENSION)

/* We need half as many transfers because the buffer is
an array of 16 bits but the transfers are 32 bits. */
#define DMA_XFERS_NEEDED FRAMEBUFFER_SIZE

extern LTDC_HandleTypeDef hltdc;
extern DMA_HandleTypeDef hdma_memtomem_dma2_stream0;
extern DMA2D_HandleTypeDef hdma2d;

//Frame buffers
extern lv_color_t *framebuffer_1;
extern lv_color_t *framebuffer_2;

void stm32_flush_cb(lv_display_t * disp, const lv_area_t * area, uint8_t * px_map);

void screen_driver_init() {
    lv_display_t *disp = lv_display_create(LCD_SCREEN_WIDTH, LCD_SCREEN_HEIGHT);
    lv_display_set_flush_cb(disp, stm32_flush_cb);
    lv_display_set_buffers(disp, framebuffer_1, framebuffer_2, FRAMEBUFFER_SIZE, LV_DISPLAY_RENDER_MODE_DIRECT);
}

uint32_t last_tick = 0;
extern bool dma2d_transfer_completed;

void stm32_flush_cb(lv_display_t * disp, const lv_area_t * area, uint8_t * px_map) {
    if (!lv_disp_flush_is_last(disp)) {
        HAL_LTDC_Reload(&hltdc, LTDC_RELOAD_VERTICAL_BLANKING);
    }
    // HAL_LTDC_Reload(&hltdc, LTDC_RELOAD_VERTICAL_BLANKING);
    lv_disp_flush_ready(disp);
}

