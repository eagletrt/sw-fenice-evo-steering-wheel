#include "endurance.h"

#ifdef ENDURANCE_MODE_ENABLED

lv_obj_t *endurance_screen;
lv_group_t *endurance_group;

void endurance_screen_create(void) {
#warning To be implemented

    endurance_screen = lv_obj_create(NULL);
    lv_group_add_obj(endurance_group, endurance_screen);

    /*Change the active screen's background color*/
    lv_obj_set_style_bg_color(endurance_screen, lv_color_hex(0x003a57), LV_PART_MAIN);

    /*Create a white label, set its text and align it to the center*/
    lv_obj_t *label = lv_label_create(endurance_screen);
    lv_label_set_text(label, "Hello world");
    lv_obj_set_style_text_color(endurance_screen, lv_color_hex(0xffffff), LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

    lv_scr_load(endurance_screen);
}

#endif  // ENDURANCE_MODE_ENABLED
