#include "display_buf.h"

void display_buf_init(LTDC_HandleTypeDef *hltdc) {
  HAL_LTDC_SetWindowSize(hltdc, SCREEN_WIDTH, SCREEN_HEIGHT, LTDC_LAYER_1);
  HAL_LTDC_SetWindowPosition(hltdc, 0, 0, LTDC_LAYER_1);
  HAL_LTDC_SetAlpha(hltdc, 255, LTDC_LAYER_1);
  HAL_LTDC_SetAddress(hltdc, SDRAM_BASE_ADDRESS, LTDC_LAYER_1);
}
