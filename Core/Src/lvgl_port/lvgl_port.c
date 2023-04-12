#include "lvgl_port/lvgl_port.h"

/*
void my_flush_cb(lv_disp_t * disp, const lv_area_t * area, lv_color_t * buf){
    //The most simple case (but also the slowest) to put all pixels to the screen //one-by-one
    // *`put_px` is just an example, it needs to be implemented by you.
    int32_t x, y;
    for(y = area->y1; y <= area->y2; y++) {
        for(x = area->x1; x <= area->x2; x++) {
            put_px(x, y, *color_p);
            color_p++;
        }
    }

    // IMPORTANT!!!Inform LVGL that you are ready with the flushing and buf is not used anymore
    lv_disp_flush_ready(disp);
}
*/

void steering_log(lv_log_level_t level, const char * buf) {
    print(buf);
}