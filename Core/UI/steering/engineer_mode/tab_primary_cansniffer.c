#include "tab_primary_cansniffer.h"

extern steering_tabs_t steering;
lv_style_t primary_cansniffer_label_style;

lv_timer_t *primary_cansniffer_update_task;
bool listening_to_primary_cansniffer = false;
extern bool engineer_mode;

extern int primary_cansniffer_ids[CAN_POSSIBLE_IDS];
extern size_t primary_cansniffer_ids_size;

extern cansniffer_elem_t primary_cansniffer_buffer[CAN_POSSIBLE_IDS];

extern int primary_cansniffer_start_index;

void init_primary_cansniffer_tab_styles() {
  lv_style_init(&primary_cansniffer_label_style);
  lv_style_set_base_dir(&primary_cansniffer_label_style, LV_BASE_DIR_LTR);
  lv_style_set_bg_opa(&primary_cansniffer_label_style, LV_OPA_TRANSP);
  lv_style_set_text_color(&primary_cansniffer_label_style,
                          lv_color_hex(COLOR_SECONDARY_HEX));
  lv_style_set_text_align(&primary_cansniffer_label_style,
                          LV_TEXT_ALIGN_CENTER);
  lv_style_set_align(&primary_cansniffer_label_style, LV_ALIGN_CENTER);
}

lv_obj_t *primary_cansniffer_timestamp_labels[TAB_CANSNIFFER_N_MESSAGES_SHOWN];
lv_obj_t *primary_cansniffer_id_labels[TAB_CANSNIFFER_N_MESSAGES_SHOWN];
lv_obj_t
    *primary_cansniffer_message_name_labels[TAB_CANSNIFFER_N_MESSAGES_SHOWN];
lv_obj_t *primary_cansniffer_len_labels[TAB_CANSNIFFER_N_MESSAGES_SHOWN];
lv_obj_t *primary_cansniffer_data_labels[TAB_CANSNIFFER_N_MESSAGES_SHOWN];

char primary_cansniffer_id_name[128] = {0};
char primary_cansniffer_data_string[128];

void clear_primary_cansniffer_ui_item(size_t i) {
  lv_label_set_text_fmt(primary_cansniffer_timestamp_labels[i], "-");
  lv_label_set_text(primary_cansniffer_id_labels[i], "0x000");
  lv_label_set_text_fmt(primary_cansniffer_message_name_labels[i], "-");
  lv_label_set_text(primary_cansniffer_len_labels[i], "-");
  lv_label_set_text(primary_cansniffer_data_labels[i], "-");
}

void clear_primary_cansniffer_ui() {
  lv_label_set_text(primary_cansniffer_timestamp_labels[0], "Time");
  lv_label_set_text(primary_cansniffer_id_labels[0], "ID");
  lv_label_set_text(primary_cansniffer_message_name_labels[0],
                    "Primary Network Message Name");
  lv_label_set_text(primary_cansniffer_len_labels[0], "Len");
  lv_label_set_text(primary_cansniffer_data_labels[0], "Data");

  for (size_t i = 0; i < TAB_CANSNIFFER_N_MESSAGES_SHOWN; i++) {
    clear_primary_cansniffer_ui_item(i);
  }
}

void update_primary_cansniffer_value(size_t iindex, size_t id) {
  if (!listening_to_primary_cansniffer)
    return;
  if (iindex >= TAB_CANSNIFFER_N_MESSAGES_SHOWN)
    return;
  lv_label_set_text_fmt(primary_cansniffer_timestamp_labels[iindex], "%u",
                        (unsigned int)primary_cansniffer_buffer[id].delta);
  lv_label_set_text_fmt(primary_cansniffer_id_labels[iindex], "%04X",
                        primary_cansniffer_buffer[id].id);
  lv_label_set_text_fmt(primary_cansniffer_message_name_labels[iindex], "%s",
                        primary_cansniffer_id_name);
  lv_label_set_text_fmt(primary_cansniffer_len_labels[iindex], "%u",
                        (unsigned int)primary_cansniffer_buffer[id].len);
  lv_label_set_text_fmt(primary_cansniffer_data_labels[iindex], "%s",
                        primary_cansniffer_data_string);
}

void update_primary_cansniffer_ui(lv_timer_t *unused_tim) {
  if (TAB_CANSNIFFER_N_MESSAGES_SHOWN * primary_cansniffer_start_index >
          primary_cansniffer_ids_size &&
      primary_cansniffer_ids_size > 1)
    primary_cansniffer_start_index--;
  for (size_t iindex =
           TAB_CANSNIFFER_N_MESSAGES_SHOWN * primary_cansniffer_start_index;
       iindex <
       (TAB_CANSNIFFER_N_MESSAGES_SHOWN * (primary_cansniffer_start_index + 1));
       ++iindex) {
    if (iindex >= primary_cansniffer_ids_size) {
      clear_primary_cansniffer_ui_item(
          iindex -
          (TAB_CANSNIFFER_N_MESSAGES_SHOWN * primary_cansniffer_start_index));
    } else {
      can_id_t id = (can_id_t)(primary_cansniffer_ids[iindex]);
      primary_message_name_from_id(id, primary_cansniffer_id_name);
      for (size_t jindex = 0; jindex < primary_cansniffer_buffer[id].len;
           ++jindex) {
        sprintf(primary_cansniffer_data_string + jindex * 3, "%02X ",
                primary_cansniffer_buffer[id].data[jindex]);
      }
      update_primary_cansniffer_value(iindex -
                                          (TAB_CANSNIFFER_N_MESSAGES_SHOWN *
                                           primary_cansniffer_start_index),
                                      id);
      sprintf(primary_cansniffer_id_name, "unknown");
    }
  }
}

void switch_primary_cansniffer() {
  if (listening_to_primary_cansniffer) {
    clear_primary_cansniffer_ui();
    lv_timer_set_repeat_count(primary_cansniffer_update_task, 0);
  } else {
    primary_cansniffer_update_task =
        lv_timer_create(update_primary_cansniffer_ui, 200, NULL);
    lv_timer_set_repeat_count(primary_cansniffer_update_task, -1);
    lv_timer_reset(primary_cansniffer_update_task);
  }
  listening_to_primary_cansniffer = !listening_to_primary_cansniffer;
}

void cansniffer_primary_new_message(can_message_t *msg) {
  can_id_t id = msg->id;
  if (id >= CAN_POSSIBLE_IDS) {
    return;
  }
  size_t old = primary_cansniffer_ids_size;
  if (primary_cansniffer_buffer[id].id == 0) {
    primary_cansniffer_ids[primary_cansniffer_ids_size] = (int)(id);
    primary_cansniffer_ids_size++;
  }
  uint32_t current_time = get_current_time_ms();
  primary_cansniffer_buffer[id].delta =
      current_time - primary_cansniffer_buffer[id].timestamp;
  primary_cansniffer_buffer[id].timestamp = current_time;
  primary_cansniffer_buffer[id].id = msg->id;
  primary_cansniffer_buffer[id].len = msg->size;
  memcpy(primary_cansniffer_buffer[id].data, msg->data, 8);
  if (old != primary_cansniffer_ids_size)
    heap_sort(primary_cansniffer_ids, primary_cansniffer_ids_size);
}

void primary_tab_cansniffer_create(lv_obj_t *parent) {
  init_primary_cansniffer_tab_styles();

  static lv_coord_t tab_cansniffer_cols_ratios[] = {
      SCREEN_WIDTH * 1 / 10, SCREEN_WIDTH * 1 / 10, SCREEN_WIDTH * 4 / 10,
      SCREEN_WIDTH * 1 / 10, SCREEN_WIDTH * 3 / 10, LV_GRID_TEMPLATE_LAST};
  static lv_coord_t tab_cansniffer_rows_ratios[] = {
      SCREEN_HEIGHT / (TAB_CANSNIFFER_N_MESSAGES_SHOWN + 1),
      SCREEN_HEIGHT / (TAB_CANSNIFFER_N_MESSAGES_SHOWN + 1),
      SCREEN_HEIGHT / (TAB_CANSNIFFER_N_MESSAGES_SHOWN + 1),
      SCREEN_HEIGHT / (TAB_CANSNIFFER_N_MESSAGES_SHOWN + 1),
      SCREEN_HEIGHT / (TAB_CANSNIFFER_N_MESSAGES_SHOWN + 1),
      SCREEN_HEIGHT / (TAB_CANSNIFFER_N_MESSAGES_SHOWN + 1),
      SCREEN_HEIGHT / (TAB_CANSNIFFER_N_MESSAGES_SHOWN + 1),
      SCREEN_HEIGHT / (TAB_CANSNIFFER_N_MESSAGES_SHOWN + 1),
      SCREEN_HEIGHT / (TAB_CANSNIFFER_N_MESSAGES_SHOWN + 1),
      SCREEN_HEIGHT / (TAB_CANSNIFFER_N_MESSAGES_SHOWN + 1),
      SCREEN_HEIGHT / (TAB_CANSNIFFER_N_MESSAGES_SHOWN + 1),
      SCREEN_HEIGHT / (TAB_CANSNIFFER_N_MESSAGES_SHOWN + 1),
      SCREEN_HEIGHT / (TAB_CANSNIFFER_N_MESSAGES_SHOWN + 1),
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

  lv_obj_t *cs_l1 = lv_label_create(cont); // timestamp label
  lv_label_set_text(cs_l1, "Time");
  lv_obj_add_style(cs_l1, &primary_cansniffer_label_style, 0);
  lv_obj_set_grid_cell(cs_l1, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER,
                       0, 1);

  lv_obj_t *cs_l2 = lv_label_create(cont); // id label
  lv_label_set_text(cs_l2, "ID");
  lv_obj_add_style(cs_l2, &primary_cansniffer_label_style, 0);
  lv_obj_set_grid_cell(cs_l2, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER,
                       0, 1);

  lv_obj_t *cs_l3 = lv_label_create(cont); // message name label
  lv_label_set_text(cs_l3, "Primary Network Message Name");
  lv_obj_add_style(cs_l3, &primary_cansniffer_label_style, 0);
  lv_obj_set_grid_cell(cs_l3, LV_GRID_ALIGN_CENTER, 2, 1, LV_GRID_ALIGN_CENTER,
                       0, 1);

  lv_obj_t *cs_l4 = lv_label_create(cont); // len label
  lv_label_set_text(cs_l4, "Len");
  lv_obj_add_style(cs_l4, &primary_cansniffer_label_style, 0);
  lv_obj_set_grid_cell(cs_l4, LV_GRID_ALIGN_CENTER, 3, 1, LV_GRID_ALIGN_CENTER,
                       0, 1);

  lv_obj_t *cs_l5 = lv_label_create(cont); // data label
  lv_label_set_text(cs_l5, "Data");
  lv_obj_add_style(cs_l5, &primary_cansniffer_label_style, 0);
  lv_obj_set_grid_cell(cs_l5, LV_GRID_ALIGN_CENTER, 4, 1, LV_GRID_ALIGN_CENTER,
                       0, 1);

  for (int i = 0; i < TAB_CANSNIFFER_N_MESSAGES_SHOWN; i++) {
    primary_cansniffer_timestamp_labels[i] =
        lv_label_create(cont); // timestamp label
    lv_label_set_text_fmt(primary_cansniffer_timestamp_labels[i], "-");
    lv_obj_add_style(primary_cansniffer_timestamp_labels[i],
                     &primary_cansniffer_label_style, 0);
    lv_obj_set_grid_cell(primary_cansniffer_timestamp_labels[i],
                         LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER,
                         i + 1, 1);

    primary_cansniffer_id_labels[i] = lv_label_create(cont); // id label
    lv_label_set_text(primary_cansniffer_id_labels[i], "0x000");
    lv_obj_add_style(primary_cansniffer_id_labels[i],
                     &primary_cansniffer_label_style, 0);
    lv_obj_set_grid_cell(primary_cansniffer_id_labels[i], LV_GRID_ALIGN_CENTER,
                         1, 1, LV_GRID_ALIGN_CENTER, i + 1, 1);

    primary_cansniffer_message_name_labels[i] =
        lv_label_create(cont); // message name label
    lv_label_set_text_fmt(primary_cansniffer_message_name_labels[i], "-");
    lv_obj_add_style(primary_cansniffer_message_name_labels[i],
                     &primary_cansniffer_label_style, 0);
    lv_obj_set_grid_cell(primary_cansniffer_message_name_labels[i],
                         LV_GRID_ALIGN_CENTER, 2, 1, LV_GRID_ALIGN_CENTER,
                         i + 1, 1);

    primary_cansniffer_len_labels[i] = lv_label_create(cont); // len label
    lv_label_set_text(primary_cansniffer_len_labels[i], "-");
    lv_obj_add_style(primary_cansniffer_len_labels[i],
                     &primary_cansniffer_label_style, 0);
    lv_obj_set_grid_cell(primary_cansniffer_len_labels[i], LV_GRID_ALIGN_CENTER,
                         3, 1, LV_GRID_ALIGN_CENTER, i + 1, 1);

    primary_cansniffer_data_labels[i] = lv_label_create(cont); // data label
    lv_label_set_text(primary_cansniffer_data_labels[i], "-");
    lv_obj_add_style(primary_cansniffer_data_labels[i],
                     &primary_cansniffer_label_style, 0);
    lv_obj_set_grid_cell(primary_cansniffer_data_labels[i],
                         LV_GRID_ALIGN_CENTER, 4, 1, LV_GRID_ALIGN_CENTER,
                         i + 1, 1);
  }
}