#include "tab_cooling.h"

#if ENGINEERING_TAB_ENABLED == 1

lv_style_t cooling_label_style;
lv_style_t style_indic;

char new_label_buffer[10];

lv_obj_t *pumps_speed_bar;
lv_obj_t *radiators_speed_bar;
lv_obj_t *pork_speed_bar;
lv_obj_t *pumps_speed_label;
lv_obj_t *radiators_speed_label;
lv_obj_t *pork_speed_label;
lv_obj_t *pumps_speed_value_label;
lv_obj_t *radiators_speed_value_label;
lv_obj_t *pork_speed_value_label;

void set_pumps_speed_bar(int32_t val) {
  lv_bar_set_value(pumps_speed_bar, val, LV_ANIM_OFF);
}

void pumps_speed_bar_invalidate() {
  lv_bar_set_value(pumps_speed_bar, 0, LV_ANIM_OFF);
  lv_style_set_bg_color(&style_indic, lv_palette_main(LV_PALETTE_RED));
  lv_obj_add_style(pumps_speed_bar, &style_indic, LV_PART_INDICATOR);
}

void set_radiators_speed_bar(int32_t val) {
  lv_bar_set_value(radiators_speed_bar, val, LV_ANIM_OFF);
}

void radiators_speed_bar_invalidate() {
  lv_bar_set_value(radiators_speed_bar, 0, LV_ANIM_OFF);
  lv_style_set_bg_color(&style_indic, lv_palette_main(LV_PALETTE_RED));
  lv_obj_add_style(radiators_speed_bar, &style_indic, LV_PART_INDICATOR);
}

void set_pork_speed_bar(int32_t val) {
  lv_bar_set_value(pork_speed_bar, val, LV_ANIM_OFF);
}

void pork_speed_bar_invalidate() {
  lv_bar_set_value(pork_speed_bar, 0, LV_ANIM_OFF);
  lv_style_set_bg_color(&style_indic, lv_palette_main(LV_PALETTE_RED));
  lv_obj_add_style(pork_speed_bar, &style_indic, LV_PART_INDICATOR);
}

void set_pumps_speed_value_label(float val) {
  sprintf(new_label_buffer, "%.2f", val);
  lv_label_set_text(pumps_speed_value_label, new_label_buffer);
}

void pumps_speed_value_label_invalidate() {
  lv_label_set_text(pumps_speed_value_label, "NA");
}

void set_radiators_speed_value_label(float val) {
  sprintf(new_label_buffer, "%.2f", val);
  lv_label_set_text(radiators_speed_value_label, new_label_buffer);
}

void radiators_speed_value_label_invalidate() {
  lv_label_set_text(pumps_speed_value_label, "NA");
}

void set_pork_speed_value_label(float val) {
  sprintf(new_label_buffer, "%.2f", val);
  lv_label_set_text(pork_speed_value_label, new_label_buffer);
}

void pork_speed_value_label_invalidate() {
  lv_label_set_text(pork_speed_value_label, "NA");
}

void init_cooling_tab_styles() {
  lv_style_init(&cooling_label_style);
  lv_style_set_base_dir(&cooling_label_style, LV_BASE_DIR_LTR);
  lv_style_set_bg_opa(&cooling_label_style, LV_OPA_TRANSP);
  lv_style_set_text_color(&cooling_label_style,
                          lv_color_hex(COLOR_SECONDARY_HEX));
  lv_style_set_text_align(&cooling_label_style, LV_TEXT_ALIGN_CENTER);
  lv_style_set_align(&cooling_label_style, LV_ALIGN_CENTER);

  lv_style_init(&style_indic);
  lv_style_set_bg_opa(&style_indic, LV_OPA_COVER);
  lv_style_set_bg_color(&style_indic, lv_palette_main(LV_PALETTE_RED));
  lv_style_set_bg_grad_color(&style_indic, lv_palette_main(LV_PALETTE_BLUE));
  lv_style_set_bg_grad_dir(&style_indic, LV_GRAD_DIR_VER);
}

void tab_cooling_create(lv_obj_t *parent) {
  init_cooling_tab_styles();

  static lv_coord_t col_dsc[] = {SCREEN_WIDTH / 3, SCREEN_WIDTH / 3,
                                 SCREEN_WIDTH / 3, LV_GRID_TEMPLATE_LAST};
  static lv_coord_t row_dsc[] = {SCREEN_HEIGHT / 4, SCREEN_HEIGHT * 2 / 4,
                                 SCREEN_HEIGHT / 4, LV_GRID_TEMPLATE_LAST};

  /*Create a container with grid*/
  lv_obj_t *cont = lv_obj_create(parent);

  lv_obj_set_layout(cont, LV_LAYOUT_GRID);
  lv_obj_set_size(cont, SCREEN_WIDTH, SCREEN_HEIGHT);
  lv_obj_add_style(cont, &grid_style, 0);
  lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_center(cont);

  lv_obj_set_grid_dsc_array(cont, col_dsc, row_dsc);

  // PUMPS SPEED
  pumps_speed_label = lv_label_create(cont);
  lv_obj_add_style(pumps_speed_label, &label_style, LV_PART_MAIN);
  lv_obj_set_style_text_font(pumps_speed_label, &lv_font_inter_bold_30,
                             LV_STATE_DEFAULT);
  lv_label_set_text(pumps_speed_label, "pumps");
  lv_obj_set_grid_cell(pumps_speed_label, LV_GRID_ALIGN_CENTER, 0, 1,
                       LV_GRID_ALIGN_CENTER, 0, 1);

  pumps_speed_bar = lv_bar_create(cont);
  lv_obj_add_style(pumps_speed_bar, &style_indic, LV_PART_INDICATOR);
  lv_obj_set_size(pumps_speed_bar, 20, 200);
  lv_bar_set_range(pumps_speed_bar, 0, 100);
  lv_bar_set_value(pumps_speed_bar, 0, LV_ANIM_OFF);

  lv_obj_set_grid_cell(pumps_speed_bar, LV_GRID_ALIGN_CENTER, 0, 1,
                       LV_GRID_ALIGN_START, 1, 1);

  pumps_speed_value_label = lv_label_create(cont);
  lv_obj_add_style(pumps_speed_value_label, &label_style, LV_PART_MAIN);
  lv_obj_set_style_text_font(pumps_speed_value_label, &lv_font_inter_bold_30,
                             LV_STATE_DEFAULT);
  lv_label_set_text(pumps_speed_value_label, "pumps val");
  lv_obj_set_grid_cell(pumps_speed_value_label, LV_GRID_ALIGN_CENTER, 0, 1,
                       LV_GRID_ALIGN_CENTER, 2, 1);

  // PORK FAN SPEED
  pork_speed_label = lv_label_create(cont);
  lv_obj_add_style(pork_speed_label, &label_style, LV_PART_MAIN);
  lv_obj_set_style_text_font(pork_speed_label, &lv_font_inter_bold_30,
                             LV_STATE_DEFAULT);
  lv_label_set_text(pork_speed_label, "pork");
  lv_obj_set_grid_cell(pork_speed_label, LV_GRID_ALIGN_CENTER, 1, 1,
                       LV_GRID_ALIGN_CENTER, 0, 1);

  pork_speed_bar = lv_bar_create(cont);
  lv_obj_add_style(pork_speed_bar, &style_indic, LV_PART_INDICATOR);
  lv_obj_set_size(pork_speed_bar, 20, 200);
  lv_bar_set_range(pork_speed_bar, 0, 100);
  lv_bar_set_value(pork_speed_bar, 0, LV_ANIM_OFF);

  lv_obj_set_grid_cell(pork_speed_bar, LV_GRID_ALIGN_CENTER, 1, 1,
                       LV_GRID_ALIGN_START, 1, 1);

  pork_speed_value_label = lv_label_create(cont);
  lv_obj_add_style(pork_speed_value_label, &label_style, LV_PART_MAIN);
  lv_obj_set_style_text_font(pork_speed_value_label, &lv_font_inter_bold_30,
                             LV_STATE_DEFAULT);
  lv_label_set_text(pork_speed_value_label, "LVAC");
  lv_obj_set_grid_cell(pork_speed_value_label, LV_GRID_ALIGN_CENTER, 1, 1,
                       LV_GRID_ALIGN_CENTER, 2, 1);

  // RADIATORS SPEED
  radiators_speed_label = lv_label_create(cont);
  lv_obj_add_style(radiators_speed_label, &label_style, LV_PART_MAIN);
  lv_obj_set_style_text_font(radiators_speed_label, &lv_font_inter_bold_30,
                             LV_STATE_DEFAULT);
  lv_label_set_text(radiators_speed_label, "radiators");
  lv_obj_set_grid_cell(radiators_speed_label, LV_GRID_ALIGN_CENTER, 2, 1,
                       LV_GRID_ALIGN_CENTER, 0, 1);

  radiators_speed_bar = lv_bar_create(cont);
  lv_obj_add_style(radiators_speed_bar, &style_indic, LV_PART_INDICATOR);
  lv_obj_set_size(radiators_speed_bar, 20, 200);
  lv_bar_set_range(radiators_speed_bar, 0, 100);
  lv_bar_set_value(radiators_speed_bar, 0, LV_ANIM_OFF);

  lv_obj_set_grid_cell(radiators_speed_bar, LV_GRID_ALIGN_CENTER, 2, 1,
                       LV_GRID_ALIGN_START, 1, 1);

  radiators_speed_value_label = lv_label_create(cont);
  lv_obj_add_style(radiators_speed_value_label, &label_style, LV_PART_MAIN);
  lv_obj_set_style_text_font(radiators_speed_value_label,
                             &lv_font_inter_bold_30, LV_STATE_DEFAULT);
  lv_label_set_text(radiators_speed_value_label, "radiators val");
  lv_obj_set_grid_cell(radiators_speed_value_label, LV_GRID_ALIGN_CENTER, 2, 1,
                       LV_GRID_ALIGN_CENTER, 2, 1);
}
#endif
