#include <screen_driver.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))

#define LCD_SCREEN_WIDTH SCREEN_WIDTH
#define LCD_SCREEN_HEIGHT SCREEN_HEIGHT
#define FRAMEBUFFER_SIZE (uint32_t)(LCD_SCREEN_HEIGHT * LCD_SCREEN_WIDTH * 4)

/* We need half as many transfers because the buffer is
an array of 16 bits but the transfers are 32 bits. */
#define DMA_XFERS_NEEDED FRAMEBUFFER_SIZE

extern LTDC_HandleTypeDef hltdc;
extern DMA_HandleTypeDef hdma_memtomem_dma2_stream0;
extern DMA2D_HandleTypeDef hdma2d;
extern lv_color_t *framebuffer_1;
extern lv_color_t *framebuffer_2;

lv_disp_drv_t lv_display_driver;

void stm32_flush_cb(lv_disp_drv_t *disp_drv, const lv_area_t *area,
                    lv_color_t *color_p);
void dma2d_copy_area(lv_area_t area, uint32_t src_buffer, uint32_t dst_buffer);

void screen_driver_init() {
  static lv_disp_draw_buf_t draw_buf;
  lv_disp_draw_buf_init(&draw_buf, framebuffer_1, framebuffer_2,
                        FRAMEBUFFER_SIZE);
  lv_disp_drv_init(&lv_display_driver);
  lv_display_driver.direct_mode = true;
  lv_display_driver.hor_res = SCREEN_WIDTH;
  lv_display_driver.ver_res = SCREEN_HEIGHT;
  lv_display_driver.flush_cb = stm32_flush_cb;
  lv_display_driver.draw_buf = &draw_buf;
  lv_display_driver.dpi = 1;

  lv_disp_drv_register(&lv_display_driver);
}

uint32_t last_tick = 0;

void stm32_flush_cb(lv_disp_drv_t *disp_drv, const lv_area_t *area,
                    lv_color_t *color_p) {
  lv_disp_t *disp = _lv_refr_get_disp_refreshing();
  uint16_t *dma_xfer_src, *dma_xfer_dst;
  if (!lv_disp_flush_is_last(disp_drv)) {
    lv_disp_flush_ready(disp_drv);

#if 0
    uint32_t hal_tick = HAL_GetTick();
    print("Flush time: %u\n", (unsigned int) (hal_tick - last_tick));
    last_tick = hal_tick;
#endif

    return;
  }

  // Swap the buffer for the one to display and reload the screen at the next
  // vertical blanking
  HAL_LTDC_Reload(&hltdc, LTDC_RELOAD_VERTICAL_BLANKING); // VSYNC

  // Determine source and destination of transfer
  dma_xfer_src = (uint16_t *)color_p;
  if (color_p == framebuffer_1) {
    dma_xfer_dst = (uint16_t *)framebuffer_2;
  } else {
    dma_xfer_dst = (uint16_t *)framebuffer_1;
  }

  for (size_t i = 0; i < disp->inv_p; i++) {
    dma2d_copy_area(disp->inv_areas[i], (uint32_t)dma_xfer_src,
                    (uint32_t)dma_xfer_dst);
  }
  lv_disp_flush_ready(disp_drv);
}

void dma2d_copy_area(lv_area_t area, uint32_t src_buffer, uint32_t dst_buffer) {
  size_t start_offset =
      (LCD_SCREEN_WIDTH * (area.y1) + (area.x1)) *
      4; // address offset (not pixel offset so it is multiplied by 2)
  size_t area_width = 1 + area.x2 - area.x1;
  size_t area_height = 1 + area.y2 - area.y1;
  size_t in_out_offset = LCD_SCREEN_WIDTH - area_width;

  // Set up DMA2D to transfer parts of picture to part of picture
  hdma2d.Init.Mode = DMA2D_M2M; // plain memory to memory
  hdma2d.Init.ColorMode = DMA2D_OUTPUT_ARGB8888;
  hdma2d.Init.OutputOffset =
      in_out_offset; // nb pixels in buffer between end of area line and start
                     // of next area line
  hdma2d.LayerCfg[DMA2D_FOREGROUND_LAYER].InputColorMode = DMA2D_INPUT_ARGB8888;
  hdma2d.LayerCfg[DMA2D_FOREGROUND_LAYER].InputOffset = in_out_offset;
  hdma2d.LayerCfg[DMA2D_FOREGROUND_LAYER].AlphaMode = DMA2D_REPLACE_ALPHA;
  hdma2d.LayerCfg[DMA2D_FOREGROUND_LAYER].InputAlpha = 0;

  HAL_DMA2D_Init(&hdma2d);
  HAL_DMA2D_ConfigLayer(&hdma2d, DMA2D_FOREGROUND_LAYER);
  HAL_DMA2D_Start(&hdma2d, src_buffer + start_offset, dst_buffer + start_offset,
                  area_width, area_height); // Start transfer
  HAL_DMA2D_PollForTransfer(&hdma2d, 10);   // Wait for transfer to be over
}

void lv_tasks() {
#if LV_TICK_CUSTOM == 1
  lv_timer_handler();
#else
  uint32_t ctick = HAL_GetTick();
  lv_tick_inc(ctick - *ptick);
  *ptick = ctick;
#endif
}