#include "steering_config.h"

#include <screen_driver.h>
#include <string.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))

#define BYTE_DIMENSION 4

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

// lv_disp_drv_t lv_display_driver;

//void stm32_flush_cb(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p);
void stm32_flush_cb(lv_display_t *disp, const lv_area_t *area, lv_color_t *color_p);
void dma2d_copy_area(lv_area_t area, uint32_t src_buffer, uint32_t dst_buffer);

void screen_driver_init() {
    lv_display_t *disp = lv_display_create(LCD_SCREEN_WIDTH, LCD_SCREEN_HEIGHT); /*Basic initialization with horizontal and vertical resolution in pixels*/
    lv_display_set_flush_cb(disp, stm32_flush_cb);                               /*Set a flush callback to draw to the display*/
    lv_display_set_buffers(disp, framebuffer_1, framebuffer_2, FRAMEBUFFER_SIZE, LV_DISPLAY_RENDER_MODE_DIRECT); /*Set an initialized buffer*/

    // static lv_disp_draw_buf_t draw_buf; //OLD 8.0
    //lv_disp_draw_buf_init(&draw_buf, framebuffer_1, framebuffer_2, FRAMEBUFFER_SIZE);
    // lv_disp_drv_init(&lv_display_driver);
    // lv_display_driver.direct_mode = true;
    // lv_display_driver.hor_res     = SCREEN_WIDTH;
    // lv_display_driver.ver_res     = SCREEN_HEIGHT;
    // lv_display_driver.flush_cb    = stm32_flush_cb;
    // lv_display_driver.draw_buf    = &draw_buf;
    // lv_display_driver.dpi         = 1;

    // lv_disp_drv_register(&lv_display_driver);
}

uint32_t last_tick = 0;
extern bool dma2d_transfer_completed;

void stm32_flush_cb(lv_display_t *disp, const lv_area_t *area, lv_color_t *color_p) {
    uint32_t *dma_xfer_src, *dma_xfer_dst;
    if (!lv_disp_flush_is_last(disp)) {
        // lv_disp_flush_ready(disp_drv); //  TODO: check

    HAL_LTDC_Reload(&hltdc, LTDC_RELOAD_VERTICAL_BLANKING);

        // return;
    }

    // int32_t x,y;
    // for (y = area->y1; y <= area->y2; y++) {
    //     for (x = area->x1; x <= area->x2; x++) {
    //         uint32_t lcd_fb_address;
    //         if (color_p == framebuffer_1) {
    //             dma_xfer_dst = (uint32_t *)framebuffer_2;
    //         } else {
    //             dma_xfer_dst = (uint16_t *)framebuffer_1;
    //         }
    //  memcpy       lv_color_t *fb_ptr = (lv_color_t *)(lcd_fb_address);
    //         fb_ptr[y * LCD_SCREEN_WIDTH + x] = *color_p;
    //         color_p++;
    //     }
    // }

    // Swap the buffer for the one to display and reload the screen at the next
    // vertical blanking
    HAL_LTDC_Reload(&hltdc, LTDC_RELOAD_VERTICAL_BLANKING);  // VSYNC

    //Determine source and destination of transfer
    dma_xfer_src = (uint32_t *)color_p;
    if (color_p == framebuffer_1) {
        dma_xfer_dst = (uint32_t *)framebuffer_2;
    } else {
        dma_xfer_dst = framebuffer_1;
    }

    // for (size_t i = 0; i < disp->inv_p; i++) {
    //     dma2d_copy_area(disp->inv_areas[i], (uint32_t)dma_xfer_src, (uint32_t)dma_xfer_dst);
    // }

    //memcpy(dma_xfer_dst, dma_xfer_src, 800*480*4);
    lv_disp_flush_ready(disp);
}

// {
//   //Set the drawing region
//   set_draw_window(area->x1, area->y1, area->x2, area->y2);

//   int height = area->y2 - area->y1 + 1;
//   int width = area->x2 - area->x1 + 1;

//   //We will do the SPI write manually here for speed
//   HAL_GPIO_WritePin(DC_PORT, DC_PIN, GPIO_PIN_SET);
//   //CS low to begin data
//   HAL_GPIO_WritePin(CS_PORT, CS_PIN, GPIO_PIN_RESET);

//   //Write colour to each pixel
//   for (int i = 0; i < width * height; i++) {
//     uint16_t color_full = (color_p->red << 11) | (color_p->green << 5) | (color_p->blue);
//     parallel_write(color_full);

//     color_p++;
//   }

//   //Return CS to high
//   HAL_GPIO_WritePin(CS_PORT, CS_PIN, GPIO_PIN_SET);

//   /* IMPORTANT!!!
//   * Inform the graphics library that you are ready with the flushing*/
//   lv_display_flush_ready(disp);
// }

void dma2d_copy_area(lv_area_t area, uint32_t src_buffer, uint32_t dst_buffer) {
    size_t start_offset  = (SCREEN_WIDTH * (area.y1) + (area.x1)) * COLOR_RESOLUTION;
    size_t area_width    = 1 + area.x2 - area.x1;
    size_t area_height   = 1 + area.y2 - area.y1;
    size_t in_out_offset = SCREEN_WIDTH - area_width;

    // Set up DMA2D to transfer parts of picture to part of picture
    hdma2d.Init.Mode                                       = DMA2D_M2M;  // plain memory to memory
    hdma2d.Init.ColorMode                                  = DMA2D_OUTPUT_RGB565;
    hdma2d.Init.OutputOffset                               = in_out_offset;
    hdma2d.LayerCfg[DMA2D_FOREGROUND_LAYER].InputColorMode = DMA2D_INPUT_RGB565;
    hdma2d.LayerCfg[DMA2D_FOREGROUND_LAYER].InputOffset    = in_out_offset;
    hdma2d.LayerCfg[DMA2D_FOREGROUND_LAYER].AlphaMode      = DMA2D_REPLACE_ALPHA;
    hdma2d.LayerCfg[DMA2D_FOREGROUND_LAYER].InputAlpha     = 0;

    HAL_DMA2D_Init(&hdma2d);
    HAL_DMA2D_ConfigLayer(&hdma2d, DMA2D_FOREGROUND_LAYER);
    HAL_DMA2D_Start(&hdma2d, src_buffer + start_offset, dst_buffer + start_offset, area_width, area_height);
    HAL_DMA2D_PollForTransfer(&hdma2d, 10);  // TODO: use the callback once checked that they work
    /***
     * maybe this should improve graphics because:
     * now: update -> does not end to update -> another update occurs
     * then: update -> does not end to update -> waits for update completion
     */
    // dma2d_transfer_completed = false;
}
