#include "tab_engineer_mode.h"

extern steering_tabs_t steering;
lv_style_t engineer_label_style;

void init_engineer_tab_styles() {
  lv_style_init(&engineer_label_style);
  lv_style_set_base_dir(&engineer_label_style, LV_BASE_DIR_LTR);
  lv_style_set_bg_opa(&engineer_label_style, LV_OPA_TRANSP);
  lv_style_set_text_color(&engineer_label_style,
                          lv_color_hex(COLOR_SECONDARY_HEX));
  lv_style_set_text_align(&engineer_label_style, LV_TEXT_ALIGN_CENTER);
  lv_style_set_align(&engineer_label_style, LV_ALIGN_CENTER);
}

#define SPACING "\n\n\n"

void tab_engineer_screen_create(lv_obj_t *parent) {
  init_engineer_tab_styles();

  static lv_coord_t tab_engineer_mod_cols_ratios[] = {
      SCREEN_WIDTH / 3, SCREEN_WIDTH / 3, SCREEN_WIDTH / 3,
      LV_GRID_TEMPLATE_LAST};
  static lv_coord_t tab_engineer_mod_rows_ratios[] = {SCREEN_HEIGHT,
                                                      LV_GRID_TEMPLATE_LAST};

  /*Create a container with grid*/
  lv_obj_t *cont = lv_obj_create(parent);

  lv_obj_set_layout(cont, LV_LAYOUT_GRID);
  lv_obj_set_size(cont, SCREEN_WIDTH, SCREEN_HEIGHT);
  lv_obj_add_style(cont, &grid_style, 0);
  lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_center(cont);

  lv_obj_set_grid_dsc_array(cont, tab_engineer_mod_cols_ratios,
                            tab_engineer_mod_rows_ratios);

  lv_obj_t *cs_l4 = lv_label_create(cont);
  lv_label_set_text(
      cs_l4, "Controlli" SPACING "PADDLE ALTI" SPACING
             "PADDLE ALTO DX LONG PRESS" SPACING "PADDLE BASSI" SPACING
             "BOTTONE VERDE ALTO SX" SPACING "BOTTONE VERDE ALTO DX" SPACING
             "BOTTONE GIALLO BASSO SX" SPACING "BOTTONE GIALLO BASSO DX" SPACING
             "BOTTONE CENTRALE");
  lv_obj_add_style(cs_l4, &engineer_label_style, 0);
  lv_obj_set_grid_cell(cs_l4, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER,
                       0, 1);

  lv_obj_t *cs_l5 = lv_label_create(cont);
  lv_label_set_text(
      cs_l5, "racing tabs" SPACING "Cambio di tab" SPACING
             "Cambio a modalita' engineer" SPACING "Cambio tab errori" SPACING
             "PTT" SPACING "Accendere/Spegnere Telemetry" SPACING
             "Aumenta mappa di 2" SPACING
             "Diminuisce mappa di 2" SPACING "SEND CAR STATUS");
  lv_obj_add_style(cs_l5, &engineer_label_style, 0);
  lv_obj_set_grid_cell(cs_l5, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER,
                       0, 1);

  lv_obj_t *cs_l6 = lv_label_create(cont);
  lv_label_set_text(
      cs_l6, "engineer tabs" SPACING "Cambio di tab" SPACING
             "Cambio a modalita' racing" SPACING "Cambio cansniffer tab" SPACING
             "Attivare Shutdown Tab(sulla tab giusta)" SPACING
             "Attivare CanDump (sulla tab giusta)" SPACING "bal on" SPACING
             "bal off" SPACING "SEND CAR STATUS");
  lv_obj_add_style(cs_l6, &engineer_label_style, 0);
  lv_obj_set_grid_cell(cs_l6, LV_GRID_ALIGN_CENTER, 2, 1, LV_GRID_ALIGN_CENTER,
                       0, 1);
}
