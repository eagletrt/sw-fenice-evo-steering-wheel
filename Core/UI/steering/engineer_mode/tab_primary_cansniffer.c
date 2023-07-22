#include "tab_primary_cansniffer.h"

extern steering_tabs_t steering;
lv_style_t cansniffer_label_style;

void init_cansniffer_tab_styles() {
  lv_style_init(&cansniffer_label_style);
  lv_style_set_base_dir(&cansniffer_label_style, LV_BASE_DIR_LTR);
  lv_style_set_bg_opa(&cansniffer_label_style, LV_OPA_TRANSP);
  lv_style_set_text_color(&cansniffer_label_style,
                          lv_color_hex(COLOR_SECONDARY_HEX));
  lv_style_set_text_align(&cansniffer_label_style, LV_TEXT_ALIGN_CENTER);
  lv_style_set_align(&cansniffer_label_style, LV_ALIGN_CENTER);
}

lv_obj_t *cansniffer_timestamp_labels[TAB_CANSNIFFER_N_MESSAGES_SHOWN];
lv_obj_t *cansniffer_id_labels[TAB_CANSNIFFER_N_MESSAGES_SHOWN];
lv_obj_t *cansniffer_message_name_labels[TAB_CANSNIFFER_N_MESSAGES_SHOWN];
lv_obj_t *cansniffer_len_labels[TAB_CANSNIFFER_N_MESSAGES_SHOWN];
lv_obj_t *cansniffer_data_labels[TAB_CANSNIFFER_N_MESSAGES_SHOWN];

void tab_cansniffer_create(lv_obj_t *parent) {
  init_cansniffer_tab_styles();

  static lv_coord_t tab_cansniffer_cols_ratios[] = {
      SCREEN_WIDTH * 1 / 10, SCREEN_WIDTH * 1 / 10, SCREEN_WIDTH * 4 / 10, SCREEN_WIDTH * 1 / 10, SCREEN_WIDTH * 3 / 10, LV_GRID_TEMPLATE_LAST};
  static lv_coord_t tab_cansniffer_rows_ratios[] = {
      SCREEN_HEIGHT / TAB_CANSNIFFER_N_MESSAGES_SHOWN,
      SCREEN_HEIGHT / TAB_CANSNIFFER_N_MESSAGES_SHOWN,
      SCREEN_HEIGHT / TAB_CANSNIFFER_N_MESSAGES_SHOWN,
      SCREEN_HEIGHT / TAB_CANSNIFFER_N_MESSAGES_SHOWN,
      SCREEN_HEIGHT / TAB_CANSNIFFER_N_MESSAGES_SHOWN,
      SCREEN_HEIGHT / TAB_CANSNIFFER_N_MESSAGES_SHOWN,
      SCREEN_HEIGHT / TAB_CANSNIFFER_N_MESSAGES_SHOWN,
      SCREEN_HEIGHT / TAB_CANSNIFFER_N_MESSAGES_SHOWN,
      SCREEN_HEIGHT / TAB_CANSNIFFER_N_MESSAGES_SHOWN,
      SCREEN_HEIGHT / TAB_CANSNIFFER_N_MESSAGES_SHOWN,
      SCREEN_HEIGHT / TAB_CANSNIFFER_N_MESSAGES_SHOWN,
      SCREEN_HEIGHT / TAB_CANSNIFFER_N_MESSAGES_SHOWN,
      LV_GRID_TEMPLATE_LAST};

  /*Create a container with grid*/
  lv_obj_t *cont = lv_obj_create(parent);

  lv_obj_set_layout(cont, LV_LAYOUT_GRID);
  lv_obj_set_size(cont, SCREEN_WIDTH, SCREEN_HEIGHT);
  lv_obj_add_style(cont, &grid_style, 0);
  lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_center(cont);

  lv_obj_set_grid_dsc_array(cont, tab_cansniffer_cols_ratios,
                            tab_cansniffer_rows_ratios);

  cansniffer_timestamp_labels[0] =
      lv_label_create(cont); // timestamp label
  lv_label_set_text(cansniffer_timestamp_labels[0], "Time");
  lv_obj_add_style(cansniffer_timestamp_labels[0], &cansniffer_label_style, 0);
  lv_obj_set_grid_cell(cansniffer_timestamp_labels[0], LV_GRID_ALIGN_CENTER, 0,
                       1, LV_GRID_ALIGN_CENTER, 0, 1);
  
  cansniffer_id_labels[0] = lv_label_create(cont); // id label
  lv_label_set_text(cansniffer_id_labels[0], "ID");
  lv_obj_add_style(cansniffer_id_labels[0], &cansniffer_label_style, 0);
  lv_obj_set_grid_cell(cansniffer_id_labels[0], LV_GRID_ALIGN_CENTER, 1, 1,
                       LV_GRID_ALIGN_CENTER, 0, 1);

  cansniffer_message_name_labels[0] =
      lv_label_create(cont); // message name label
  lv_label_set_text(cansniffer_message_name_labels[0], "Message Name");
  lv_obj_add_style(cansniffer_message_name_labels[0], &cansniffer_label_style,
                   0);
  lv_obj_set_grid_cell(cansniffer_message_name_labels[0], LV_GRID_ALIGN_CENTER,
                       2, 1, LV_GRID_ALIGN_CENTER, 0, 1);

  cansniffer_len_labels[0] = lv_label_create(cont); // len label
  lv_label_set_text(cansniffer_len_labels[0], "Len");
  lv_obj_add_style(cansniffer_len_labels[0], &cansniffer_label_style, 0);
  lv_obj_set_grid_cell(cansniffer_len_labels[0], LV_GRID_ALIGN_CENTER, 3, 1,
                       LV_GRID_ALIGN_CENTER, 0, 1);

  cansniffer_data_labels[0] = lv_label_create(cont); // data label
  lv_label_set_text(cansniffer_data_labels[0], "Data");
  lv_obj_add_style(cansniffer_data_labels[0], &cansniffer_label_style, 0);
  lv_obj_set_grid_cell(cansniffer_data_labels[0], LV_GRID_ALIGN_CENTER, 4, 1,
                       LV_GRID_ALIGN_CENTER, 0, 1);

  for (int i = 1; i < TAB_CANSNIFFER_N_MESSAGES_SHOWN; i++) {
    cansniffer_timestamp_labels[i] =
        lv_label_create(cont); // timestamp label
    lv_label_set_text_fmt(cansniffer_timestamp_labels[i], "t", i);
    lv_obj_add_style(cansniffer_timestamp_labels[i], &cansniffer_label_style, 0);
    lv_obj_set_grid_cell(cansniffer_timestamp_labels[i], LV_GRID_ALIGN_CENTER,
                         0, 1, LV_GRID_ALIGN_CENTER, i, 1);

    cansniffer_id_labels[i] = lv_label_create(cont); // id label
    lv_label_set_text(cansniffer_id_labels[i], "0x000");
    lv_obj_add_style(cansniffer_id_labels[i], &cansniffer_label_style, 0);
    lv_obj_set_grid_cell(cansniffer_id_labels[i], LV_GRID_ALIGN_CENTER, 1, 1,
                         LV_GRID_ALIGN_CENTER, i, 1);

    cansniffer_message_name_labels[i] = lv_label_create(cont); // message name label
    lv_label_set_text_fmt(cansniffer_message_name_labels[i], "placeholder %d", i);
    lv_obj_add_style(cansniffer_message_name_labels[i], &cansniffer_label_style,
                     0);
    lv_obj_set_grid_cell(cansniffer_message_name_labels[i],
                         LV_GRID_ALIGN_CENTER, 2, 1, LV_GRID_ALIGN_CENTER, i,
                         1);

    cansniffer_len_labels[i] = lv_label_create(cont); // len label
    lv_label_set_text(cansniffer_len_labels[i], "0");
    lv_obj_add_style(cansniffer_len_labels[i], &cansniffer_label_style, 0);
    lv_obj_set_grid_cell(cansniffer_len_labels[i], LV_GRID_ALIGN_CENTER, 3, 1,
                         LV_GRID_ALIGN_CENTER, i, 1);

    cansniffer_data_labels[i] = lv_label_create(cont); // data label
    lv_label_set_text(cansniffer_data_labels[i], "********");
    lv_obj_add_style(cansniffer_data_labels[i], &cansniffer_label_style, 0);
    lv_obj_set_grid_cell(cansniffer_data_labels[i], LV_GRID_ALIGN_CENTER, 4, 1,
                         LV_GRID_ALIGN_CENTER, i, 1);
  }

}