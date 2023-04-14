#include "lvgl_port/lvgl_port.h"

void steering_log(const char *buf) { print(buf); }

void lv_tasks(uint32_t *ptick) {
  uint32_t ctick = HAL_GetTick();
  lv_tick_inc(*ptick - ctick);
  *ptick = ctick;
  lv_timer_handler();
}