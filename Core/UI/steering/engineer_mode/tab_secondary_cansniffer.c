#include "steering_config.h"

#if CANSNIFFER_ENABLED == 1

#include "tab_secondary_cansniffer.h"

lv_style_t secondary_cansniffer_label_style;

lv_timer_t *secondary_cansniffer_update_task;
bool listening_to_secondary_cansniffer = false;

extern int secondary_cansniffer_ids[CAN_POSSIBLE_IDS];
extern size_t secondary_cansniffer_ids_size;

extern cansniffer_elem_t *secondary_cansniffer_buffer;

extern int secondary_cansniffer_start_index;

void init_secondary_cansniffer_tab_styles() {
  lv_style_init(&secondary_cansniffer_label_style);
  lv_style_set_base_dir(&secondary_cansniffer_label_style, LV_BASE_DIR_LTR);
  lv_style_set_bg_opa(&secondary_cansniffer_label_style, LV_OPA_TRANSP);
  lv_style_set_text_color(&secondary_cansniffer_label_style,
                          lv_color_hex(COLOR_SECONDARY_HEX));
  lv_style_set_text_align(&secondary_cansniffer_label_style,
                          LV_TEXT_ALIGN_CENTER);
  lv_style_set_align(&secondary_cansniffer_label_style, LV_ALIGN_CENTER);
}

lv_obj_t
    *secondary_cansniffer_timestamp_labels[TAB_CANSNIFFER_N_MESSAGES_SHOWN];
lv_obj_t *secondary_cansniffer_id_labels[TAB_CANSNIFFER_N_MESSAGES_SHOWN];
lv_obj_t
    *secondary_cansniffer_message_name_labels[TAB_CANSNIFFER_N_MESSAGES_SHOWN];
lv_obj_t *secondary_cansniffer_len_labels[TAB_CANSNIFFER_N_MESSAGES_SHOWN];
lv_obj_t *secondary_cansniffer_data_labels[TAB_CANSNIFFER_N_MESSAGES_SHOWN];

char secondary_cansniffer_id_name[128] = {0};
char secondary_cansniffer_data_string[128];

void clear_secondary_cansniffer_ui_item(size_t i) {
  lv_label_set_text_fmt(secondary_cansniffer_timestamp_labels[i], "-");
  lv_label_set_text(secondary_cansniffer_id_labels[i], "0x000");
  lv_label_set_text_fmt(secondary_cansniffer_message_name_labels[i], "-");
  lv_label_set_text(secondary_cansniffer_len_labels[i], "-");
  lv_label_set_text(secondary_cansniffer_data_labels[i], "-");
}

void clear_secondary_cansniffer_ui() {
  lv_label_set_text(secondary_cansniffer_timestamp_labels[0], "Time");
  lv_label_set_text(secondary_cansniffer_id_labels[0], "ID");
  lv_label_set_text(secondary_cansniffer_message_name_labels[0],
                    "Secondary Network Message Name");
  lv_label_set_text(secondary_cansniffer_len_labels[0], "Len");
  lv_label_set_text(secondary_cansniffer_data_labels[0], "Data");

  for (size_t i = 0; i < TAB_CANSNIFFER_N_MESSAGES_SHOWN; i++) {
    clear_secondary_cansniffer_ui_item(i);
  }
}

void update_secondary_cansniffer_value(size_t iindex, size_t id) {
  if (iindex >= TAB_CANSNIFFER_N_MESSAGES_SHOWN)
    return;
  lv_label_set_text_fmt(secondary_cansniffer_timestamp_labels[iindex], "%u",
                        (unsigned int)secondary_cansniffer_buffer[id].delta);
  lv_label_set_text_fmt(secondary_cansniffer_id_labels[iindex], "%04X",
                        secondary_cansniffer_buffer[id].id);
  lv_label_set_text_fmt(secondary_cansniffer_message_name_labels[iindex], "%s",
                        secondary_cansniffer_id_name);
  lv_label_set_text_fmt(secondary_cansniffer_len_labels[iindex], "%u",
                        (unsigned int)secondary_cansniffer_buffer[id].len);
  lv_label_set_text_fmt(secondary_cansniffer_data_labels[iindex], "%s",
                        secondary_cansniffer_data_string);
}

void update_secondary_cansniffer_ui(lv_timer_t *unused_tim) {
  if (!listening_to_secondary_cansniffer)
    return;
  if (TAB_CANSNIFFER_N_MESSAGES_SHOWN * secondary_cansniffer_start_index >
          secondary_cansniffer_ids_size &&
      secondary_cansniffer_ids_size > 1)
    secondary_cansniffer_start_index--;
  for (size_t iindex =
           TAB_CANSNIFFER_N_MESSAGES_SHOWN * secondary_cansniffer_start_index;
       iindex < (TAB_CANSNIFFER_N_MESSAGES_SHOWN *
                 (secondary_cansniffer_start_index + 1));
       ++iindex) {
    if (iindex >= secondary_cansniffer_ids_size) {
      clear_secondary_cansniffer_ui_item(
          iindex -
          (TAB_CANSNIFFER_N_MESSAGES_SHOWN * secondary_cansniffer_start_index));
    } else {
      can_id_t id = (can_id_t)(secondary_cansniffer_ids[iindex]);
      secondary_message_name_from_id(id, secondary_cansniffer_id_name);
      for (size_t jindex = 0; jindex < secondary_cansniffer_buffer[id].len;
           ++jindex) {
        sprintf(secondary_cansniffer_data_string + jindex * 3, "%02X ",
                secondary_cansniffer_buffer[id].data[jindex]);
      }
      update_secondary_cansniffer_value(iindex -
                                            (TAB_CANSNIFFER_N_MESSAGES_SHOWN *
                                             secondary_cansniffer_start_index),
                                        id);
      sprintf(secondary_cansniffer_id_name, "unknown");
    }
  }
}

void switch_secondary_cansniffer() {
  if (listening_to_secondary_cansniffer) {
    clear_secondary_cansniffer_ui();
    lv_timer_set_repeat_count(secondary_cansniffer_update_task, 0);
  } else {
    secondary_cansniffer_update_task =
        lv_timer_create(update_secondary_cansniffer_ui, 200, NULL);
    lv_timer_set_repeat_count(secondary_cansniffer_update_task, -1);
    lv_timer_reset(secondary_cansniffer_update_task);
  }
  listening_to_secondary_cansniffer = !listening_to_secondary_cansniffer;
}

void cansniffer_secondary_new_message(can_message_t *msg) {
  can_id_t id = msg->id;
  if (id >= CAN_POSSIBLE_IDS) {
    return;
  }
  size_t old = secondary_cansniffer_ids_size;
  if (secondary_cansniffer_buffer[id].id == 0) {
    secondary_cansniffer_ids[secondary_cansniffer_ids_size] = (int)(id);
    secondary_cansniffer_ids_size++;
  }
  uint32_t current_time = get_current_time_ms();
  secondary_cansniffer_buffer[id].delta =
      current_time - secondary_cansniffer_buffer[id].timestamp;
  secondary_cansniffer_buffer[id].timestamp = current_time;
  secondary_cansniffer_buffer[id].id = msg->id;
  secondary_cansniffer_buffer[id].len = msg->size;
  memcpy(secondary_cansniffer_buffer[id].data, msg->data, 8);
  if (old != secondary_cansniffer_ids_size)
    heap_sort(secondary_cansniffer_ids, secondary_cansniffer_ids_size);
}

void secondary_tab_cansniffer_create(lv_obj_t *parent) {
  init_secondary_cansniffer_tab_styles();

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
  lv_obj_add_style(cs_l1, &secondary_cansniffer_label_style, 0);
  lv_obj_set_grid_cell(cs_l1, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER,
                       0, 1);

  lv_obj_t *cs_l2 = lv_label_create(cont); // id label
  lv_label_set_text(cs_l2, "ID");
  lv_obj_add_style(cs_l2, &secondary_cansniffer_label_style, 0);
  lv_obj_set_grid_cell(cs_l2, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER,
                       0, 1);

  lv_obj_t *cs_l3 = lv_label_create(cont); // message name label
  lv_label_set_text(cs_l3, "Secondary Network Message Name");
  lv_obj_add_style(cs_l3, &secondary_cansniffer_label_style, 0);
  lv_obj_set_grid_cell(cs_l3, LV_GRID_ALIGN_CENTER, 2, 1, LV_GRID_ALIGN_CENTER,
                       0, 1);

  lv_obj_t *cs_l4 = lv_label_create(cont); // len label
  lv_label_set_text(cs_l4, "Len");
  lv_obj_add_style(cs_l4, &secondary_cansniffer_label_style, 0);
  lv_obj_set_grid_cell(cs_l4, LV_GRID_ALIGN_CENTER, 3, 1, LV_GRID_ALIGN_CENTER,
                       0, 1);

  lv_obj_t *cs_l5 = lv_label_create(cont); // data label
  lv_label_set_text(cs_l5, "Data");
  lv_obj_add_style(cs_l5, &secondary_cansniffer_label_style, 0);
  lv_obj_set_grid_cell(cs_l5, LV_GRID_ALIGN_CENTER, 4, 1, LV_GRID_ALIGN_CENTER,
                       0, 1);

  for (int i = 0; i < TAB_CANSNIFFER_N_MESSAGES_SHOWN; i++) {
    secondary_cansniffer_timestamp_labels[i] =
        lv_label_create(cont); // timestamp label
    lv_label_set_text_fmt(secondary_cansniffer_timestamp_labels[i], "-");
    lv_obj_add_style(secondary_cansniffer_timestamp_labels[i],
                     &secondary_cansniffer_label_style, 0);
    lv_obj_set_grid_cell(secondary_cansniffer_timestamp_labels[i],
                         LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER,
                         i + 1, 1);

    secondary_cansniffer_id_labels[i] = lv_label_create(cont); // id label
    lv_label_set_text(secondary_cansniffer_id_labels[i], "0x000");
    lv_obj_add_style(secondary_cansniffer_id_labels[i],
                     &secondary_cansniffer_label_style, 0);
    lv_obj_set_grid_cell(secondary_cansniffer_id_labels[i],
                         LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER,
                         i + 1, 1);

    secondary_cansniffer_message_name_labels[i] =
        lv_label_create(cont); // message name label
    lv_label_set_text_fmt(secondary_cansniffer_message_name_labels[i], "-");
    lv_obj_add_style(secondary_cansniffer_message_name_labels[i],
                     &secondary_cansniffer_label_style, 0);
    lv_obj_set_grid_cell(secondary_cansniffer_message_name_labels[i],
                         LV_GRID_ALIGN_CENTER, 2, 1, LV_GRID_ALIGN_CENTER,
                         i + 1, 1);

    secondary_cansniffer_len_labels[i] = lv_label_create(cont); // len label
    lv_label_set_text(secondary_cansniffer_len_labels[i], "-");
    lv_obj_add_style(secondary_cansniffer_len_labels[i],
                     &secondary_cansniffer_label_style, 0);
    lv_obj_set_grid_cell(secondary_cansniffer_len_labels[i],
                         LV_GRID_ALIGN_CENTER, 3, 1, LV_GRID_ALIGN_CENTER,
                         i + 1, 1);

    secondary_cansniffer_data_labels[i] = lv_label_create(cont); // data label
    lv_label_set_text(secondary_cansniffer_data_labels[i], "-");
    lv_obj_add_style(secondary_cansniffer_data_labels[i],
                     &secondary_cansniffer_label_style, 0);
    lv_obj_set_grid_cell(secondary_cansniffer_data_labels[i],
                         LV_GRID_ALIGN_CENTER, 4, 1, LV_GRID_ALIGN_CENTER,
                         i + 1, 1);
  }
}

#endif // CANSNIFFER_ENABLED == 1
