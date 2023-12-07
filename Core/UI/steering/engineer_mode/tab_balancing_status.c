#include "tab_balancing_status.h"

#if ENGINEERING_TAB_ENABLED == 1

lv_style_t bal_status_style;

lv_obj_t *bal_status_label;

void set_bal_status_label_text(char *text) {
  lv_label_set_text(bal_status_label, text);
}

void init_balancing_status_styles() {
  lv_style_init(&bal_status_style);
  lv_style_set_base_dir(&bal_status_style, LV_BASE_DIR_LTR);
  lv_style_set_bg_opa(&bal_status_style, LV_OPA_TRANSP);
  lv_style_set_text_color(&bal_status_style, lv_color_hex(COLOR_SECONDARY_HEX));
  lv_style_set_text_align(&bal_status_style, LV_TEXT_ALIGN_CENTER);
  lv_style_set_align(&bal_status_style, LV_ALIGN_CENTER);
}

void tab_balancing_status_screen_create(lv_obj_t *parent) {
  init_balancing_status_styles();

  lv_obj_t *bal_status_tab_background = lv_obj_create(parent);
  lv_obj_set_size(bal_status_tab_background, SCREEN_WIDTH, SCREEN_HEIGHT);
  lv_obj_set_style_bg_color(bal_status_tab_background,
                            lv_color_hex(COLOR_PRIMARY_HEX), LV_PART_MAIN);

  bal_status_label = lv_label_create(bal_status_tab_background);

  lv_obj_add_style(bal_status_label, &bal_status_style, LV_PART_MAIN);
  lv_obj_set_style_text_font(bal_status_label, &lv_font_inter_bold_38,
                             LV_PART_MAIN);
  lv_label_set_text(bal_status_label, "BAL STATUS: UNKNOWN");
  lv_obj_align(bal_status_label, LV_ALIGN_CENTER, 0, 0);
}
#endif
