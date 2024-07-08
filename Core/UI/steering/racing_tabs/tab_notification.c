#include "tab_notification.h"

lv_style_t notif_label_style;

lv_obj_t *notification_screen_label;

void set_notification_screen_label(const char *s) {
    lv_label_set_text(notification_screen_label, s);
}

lv_obj_t *notification_background;

void tab_notification_set_bg_color(uint32_t hex_color) {
    lv_obj_set_style_bg_color(notification_background, lv_color_hex(hex_color), LV_PART_MAIN);
}

void tab_notification_set_label_color(uint32_t hex_color) {
    lv_style_set_text_color(&notif_label_style, lv_color_hex(hex_color));
}

void init_notification_tab_styles() {
    lv_style_init(&notif_label_style);
    lv_style_set_base_dir(&notif_label_style, LV_BASE_DIR_LTR);
    lv_style_set_bg_opa(&notif_label_style, LV_OPA_TRANSP);
    lv_style_set_text_color(&notif_label_style, lv_color_hex(COLOR_PRIMARY_HEX));
    lv_style_set_text_align(&notif_label_style, LV_TEXT_ALIGN_CENTER);
    lv_style_set_align(&notif_label_style, LV_ALIGN_CENTER);
}

void tab_notification_screen_create(lv_obj_t *notification_screen) {
    init_notification_tab_styles();

    notification_background = lv_obj_create(notification_screen);
    lv_obj_set_size(notification_background, SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_obj_set_style_bg_color(notification_background, lv_color_hex(COLOR_SECONDARY_HEX), LV_PART_MAIN);

    notification_screen_label = lv_label_create(notification_background);

    lv_obj_add_style(notification_screen_label, &notif_label_style, LV_PART_MAIN);
    lv_obj_set_style_text_font(notification_screen_label, &lv_font_inter_bold_70, LV_PART_MAIN);
    lv_label_set_text(notification_screen_label, "PLACEHOLDER TEXT");
    lv_obj_align(notification_screen_label, LV_ALIGN_CENTER, 0, 0);
}
