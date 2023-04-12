#ifndef LVGL_PORT_H
#define LVGL_PORT_H

#include "lvgl.h"
#include "usart.h"

// void my_flush_cb(lv_disp_t * disp, const lv_area_t * area, lv_color_t * buf);
void steering_log(const char *buf);

#endif
