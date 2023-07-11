#include "tab_engineer_mode.h"

extern steering_t steering;
lv_style_t engineer_label_style;

void init_engineer_tab_styles() {
  lv_style_init(&engineer_label_style);
  lv_style_set_base_dir(&engineer_label_style, LV_BASE_DIR_LTR);
  lv_style_set_bg_opa(&engineer_label_style, LV_OPA_TRANSP);
  lv_style_set_text_color(&engineer_label_style,
                          lv_color_hex(COLOR_PRIMARY_HEX));
  lv_style_set_text_align(&engineer_label_style, LV_TEXT_ALIGN_CENTER);
  lv_style_set_align(&engineer_label_style, LV_ALIGN_CENTER);
}

void tab_engineer_screen_create(lv_obj_t *notification_screen) {
  init_engineer_tab_styles();

  lv_obj_t *notification_background = lv_obj_create(notification_screen);
  lv_obj_set_size(notification_background, SCREEN_WIDTH, SCREEN_HEIGHT);
  lv_obj_set_style_bg_color(notification_background,
                            lv_color_hex(COLOR_SECONDARY_HEX), LV_PART_MAIN);

  lv_obj_t *label = lv_label_create(notification_background);

  lv_obj_add_style(label, &engineer_label_style, LV_PART_MAIN);
  lv_obj_set_style_text_font(label, &lv_font_inter_bold_38, LV_PART_MAIN);
  lv_label_set_text(label, "ENGINEER MODE");
  lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
}
