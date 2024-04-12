#include "tab_fatal_error.h"

lv_style_t tab_fatal_error_style;
lv_obj_t *tab_fatal_error_label;

void set_tab_fatal_error_text(const char *text) {
    lv_label_set_text(tab_fatal_error_label, text);
}

void init_tab_fatal_error_styles() {
    lv_style_init(&tab_fatal_error_style);
    lv_style_set_base_dir(&tab_fatal_error_style, LV_BASE_DIR_LTR);
    lv_style_set_bg_opa(&tab_fatal_error_style, LV_OPA_TRANSP);
    lv_style_set_text_color(&tab_fatal_error_style, lv_color_hex(COLOR_PRIMARY_HEX));
    lv_style_set_text_align(&tab_fatal_error_style, LV_TEXT_ALIGN_CENTER);
    lv_style_set_align(&tab_fatal_error_style, LV_ALIGN_CENTER);
}

void tab_fatal_error_create(lv_obj_t *parent) {
    init_tab_fatal_error_styles();

    lv_obj_t *tab_fe_background = lv_obj_create(parent);
    lv_obj_set_size(tab_fe_background, SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_obj_set_style_bg_color(tab_fe_background, lv_color_hex(COLOR_SECONDARY_HEX), LV_PART_MAIN);

    tab_fatal_error_label = lv_label_create(tab_fe_background);

    lv_obj_add_style(tab_fatal_error_label, &tab_fatal_error_style, LV_PART_MAIN);
    lv_obj_set_style_text_font(tab_fatal_error_label, &lv_font_inter_bold_38, LV_PART_MAIN);
    lv_label_set_text(tab_fatal_error_label, "TAB_FATAL_ERROR");
    lv_obj_align(tab_fatal_error_label, LV_ALIGN_CENTER, 0, 0);
}
