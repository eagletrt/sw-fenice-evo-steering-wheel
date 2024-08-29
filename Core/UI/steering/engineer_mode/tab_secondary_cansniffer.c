#include "steering_config.h"

#if STEERING_WHEEL_MODE == STEERING_WHEEL_LVGL_MODE

#if CANSNIFFER_ENABLED == 1

#include "tab_secondary_cansniffer.h"
#define SECONDARY_CANSNIFFER_ID_NAME_SIZE     128
#define SECONDARY_CANSNIFFER_DATA_STRING_SIZE 128

lv_style_t secondary_cansniffer_label_style;
lv_timer_t *secondary_cansniffer_update_task;

cansniffer_elem_t secondary_cansniffer_buffer[secondary_MESSAGE_COUNT] = {0};
extern int secondary_cansniffer_start_index;
can_id_t secondary_cansniffer_ordered_ids[secondary_MESSAGE_COUNT];
MinHeap(int, secondary_MESSAGE_COUNT) secondary_cansniffer_ids_heap;

void init_secondary_cansniffer_tab_styles() {
    min_heap_init(&secondary_cansniffer_ids_heap, int, secondary_MESSAGE_COUNT, min_heap_compare_indexes);
    lv_style_init(&secondary_cansniffer_label_style);
    lv_style_set_base_dir(&secondary_cansniffer_label_style, LV_BASE_DIR_LTR);
    lv_style_set_bg_opa(&secondary_cansniffer_label_style, LV_OPA_TRANSP);
    lv_style_set_text_font(&secondary_cansniffer_label_style, &lv_font_inter_bold_14);
    lv_style_set_text_color(&secondary_cansniffer_label_style, lv_color_hex(COLOR_SECONDARY_HEX));
    lv_style_set_text_align(&secondary_cansniffer_label_style, LV_TEXT_ALIGN_CENTER);
    lv_style_set_align(&secondary_cansniffer_label_style, LV_ALIGN_CENTER);
}

lv_obj_t *secondary_cansniffer_timestamp_labels[TAB_CANSNIFFER_N_MESSAGES_SHOWN];
lv_obj_t *secondary_cansniffer_id_labels[TAB_CANSNIFFER_N_MESSAGES_SHOWN];
lv_obj_t *secondary_cansniffer_message_name_labels[TAB_CANSNIFFER_N_MESSAGES_SHOWN];
lv_obj_t *secondary_cansniffer_len_labels[TAB_CANSNIFFER_N_MESSAGES_SHOWN];
lv_obj_t *secondary_cansniffer_data_labels[TAB_CANSNIFFER_N_MESSAGES_SHOWN];

char secondary_cansniffer_id_name[SECONDARY_CANSNIFFER_ID_NAME_SIZE] = {0};
char secondary_cansniffer_data_string[SECONDARY_CANSNIFFER_DATA_STRING_SIZE];

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
    lv_label_set_text(secondary_cansniffer_message_name_labels[0], "Secondary Network Message Name");
    lv_label_set_text(secondary_cansniffer_len_labels[0], "Len");
    lv_label_set_text(secondary_cansniffer_data_labels[0], "Data");

    for (size_t i = 0; i < TAB_CANSNIFFER_N_MESSAGES_SHOWN; i++) {
        clear_secondary_cansniffer_ui_item(i);
    }
}

void update_secondary_cansniffer_value(size_t iindex, size_t id) {
    if (iindex >= TAB_CANSNIFFER_N_MESSAGES_SHOWN)
        return;
    lv_label_set_text_fmt(secondary_cansniffer_timestamp_labels[iindex], "%u", (unsigned int)secondary_cansniffer_buffer[id].delta);
    lv_label_set_text_fmt(secondary_cansniffer_id_labels[iindex], "%04X", secondary_cansniffer_buffer[id].id);
    lv_label_set_text_fmt(secondary_cansniffer_message_name_labels[iindex], "%s", secondary_cansniffer_id_name);
    lv_label_set_text_fmt(secondary_cansniffer_len_labels[iindex], "%u", (unsigned int)secondary_cansniffer_buffer[id].len);
    lv_label_set_text_fmt(secondary_cansniffer_data_labels[iindex], "%s", secondary_cansniffer_data_string);
}

void update_secondary_cansniffer_ui() {
    size_t n_msg = min_heap_size(&secondary_cansniffer_ids_heap);
    if (TAB_CANSNIFFER_N_MESSAGES_SHOWN * secondary_cansniffer_start_index > n_msg && n_msg > 1)
        secondary_cansniffer_start_index--;  // TODO: check, maybe possible bug
    for (size_t iindex = TAB_CANSNIFFER_N_MESSAGES_SHOWN * secondary_cansniffer_start_index;
         iindex < (TAB_CANSNIFFER_N_MESSAGES_SHOWN * (secondary_cansniffer_start_index + 1));
         ++iindex) {
        if (iindex >= n_msg) {
            clear_secondary_cansniffer_ui_item(iindex - (TAB_CANSNIFFER_N_MESSAGES_SHOWN * secondary_cansniffer_start_index));
        } else {
            int index = (secondary_cansniffer_ordered_ids[iindex]);
            int res   = secondary_message_name_from_id(secondary_id_from_index(index), secondary_cansniffer_id_name);
            if (res == 0) {
                snprintf(secondary_cansniffer_id_name, SECONDARY_CANSNIFFER_ID_NAME_SIZE, "unknown");
            }
            for (size_t jindex = 0; jindex < secondary_cansniffer_buffer[index].len; ++jindex) {
                snprintf(
                    secondary_cansniffer_data_string + jindex * 3,
                    SECONDARY_CANSNIFFER_DATA_STRING_SIZE,
                    "%02X ",
                    secondary_cansniffer_buffer[index].data[jindex]);
            }
            update_secondary_cansniffer_value(iindex - (TAB_CANSNIFFER_N_MESSAGES_SHOWN * secondary_cansniffer_start_index), index);
        }
    }
}

void cansniffer_secondary_new_message(can_message_t *msg) {
    int index                        = secondary_index_from_id(msg->id);
    bool has_this_msg_ever_been_seen = true;
    if (index == -1) {
        return;
    }
    if (secondary_cansniffer_buffer[index].id == 0) {  // TODO use another bool to indicate if it is used or not
        min_heap_insert(&secondary_cansniffer_ids_heap, &index);
        has_this_msg_ever_been_seen = false;
    }
    uint32_t current_time                        = get_current_time_ms();
    secondary_cansniffer_buffer[index].delta     = current_time - secondary_cansniffer_buffer[index].timestamp;
    secondary_cansniffer_buffer[index].timestamp = current_time;
    secondary_cansniffer_buffer[index].id        = msg->id;
    secondary_cansniffer_buffer[index].len       = msg->size;
    memcpy(secondary_cansniffer_buffer[index].data, msg->data, msg->size);

    if (!has_this_msg_ever_been_seen) {
        MinHeap(int, secondary_MESSAGE_COUNT) secondary_heap_copy_because_toni_is_sbored = min_heap_new(int, secondary_MESSAGE_COUNT, min_heap_compare_indexes);
        memcpy(&secondary_heap_copy_because_toni_is_sbored, &secondary_cansniffer_ids_heap, sizeof(MinHeap(int, secondary_MESSAGE_COUNT)));
        int cindex = 0, i = 0;
        while (!min_heap_is_empty(&secondary_heap_copy_because_toni_is_sbored)) {
            min_heap_remove(&secondary_heap_copy_because_toni_is_sbored, 0, &cindex);
            secondary_cansniffer_ordered_ids[i] = cindex;
            i++;
        }
    }
}

void secondary_tab_cansniffer_create(lv_obj_t *parent) {
    init_secondary_cansniffer_tab_styles();

    static lv_coord_t tab_cansniffer_cols_ratios[] = {
        SCREEN_WIDTH * 1 / 10, SCREEN_WIDTH * 1 / 10, SCREEN_WIDTH * 4 / 10, SCREEN_WIDTH * 1 / 10, SCREEN_WIDTH * 3 / 10, LV_GRID_TEMPLATE_LAST};
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

    lv_obj_set_grid_dsc_array(cont, tab_cansniffer_cols_ratios, tab_cansniffer_rows_ratios);

    lv_obj_t *cs_l1 = lv_label_create(cont);  // timestamp label
    lv_label_set_text(cs_l1, "Time");
    lv_obj_add_style(cs_l1, &secondary_cansniffer_label_style, 0);
    lv_obj_set_grid_cell(cs_l1, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 0, 1);

    lv_obj_t *cs_l2 = lv_label_create(cont);  // id label
    lv_label_set_text(cs_l2, "ID");
    lv_obj_add_style(cs_l2, &secondary_cansniffer_label_style, 0);
    lv_obj_set_grid_cell(cs_l2, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, 0, 1);

    lv_obj_t *cs_l3 = lv_label_create(cont);  // message name label
    lv_label_set_text(cs_l3, "Secondary Network Message Name");
    lv_obj_add_style(cs_l3, &secondary_cansniffer_label_style, 0);
    lv_obj_set_grid_cell(cs_l3, LV_GRID_ALIGN_CENTER, 2, 1, LV_GRID_ALIGN_CENTER, 0, 1);

    lv_obj_t *cs_l4 = lv_label_create(cont);  // len label
    lv_label_set_text(cs_l4, "Len");
    lv_obj_add_style(cs_l4, &secondary_cansniffer_label_style, 0);
    lv_obj_set_grid_cell(cs_l4, LV_GRID_ALIGN_CENTER, 3, 1, LV_GRID_ALIGN_CENTER, 0, 1);

    lv_obj_t *cs_l5 = lv_label_create(cont);  // data label
    lv_label_set_text(cs_l5, "Data");
    lv_obj_add_style(cs_l5, &secondary_cansniffer_label_style, 0);
    lv_obj_set_grid_cell(cs_l5, LV_GRID_ALIGN_CENTER, 4, 1, LV_GRID_ALIGN_CENTER, 0, 1);

    for (int i = 0; i < TAB_CANSNIFFER_N_MESSAGES_SHOWN; i++) {
        secondary_cansniffer_timestamp_labels[i] = lv_label_create(cont);  // timestamp label
        lv_label_set_text_fmt(secondary_cansniffer_timestamp_labels[i], "-");
        lv_obj_add_style(secondary_cansniffer_timestamp_labels[i], &secondary_cansniffer_label_style, 0);
        lv_obj_set_grid_cell(secondary_cansniffer_timestamp_labels[i], LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, i + 1, 1);

        secondary_cansniffer_id_labels[i] = lv_label_create(cont);  // id label
        lv_label_set_text(secondary_cansniffer_id_labels[i], "0x000");
        lv_obj_add_style(secondary_cansniffer_id_labels[i], &secondary_cansniffer_label_style, 0);
        lv_obj_set_grid_cell(secondary_cansniffer_id_labels[i], LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, i + 1, 1);

        secondary_cansniffer_message_name_labels[i] = lv_label_create(cont);  // message name label
        lv_label_set_text_fmt(secondary_cansniffer_message_name_labels[i], "-");
        lv_obj_add_style(secondary_cansniffer_message_name_labels[i], &secondary_cansniffer_label_style, 0);
        lv_obj_set_grid_cell(secondary_cansniffer_message_name_labels[i], LV_GRID_ALIGN_CENTER, 2, 1, LV_GRID_ALIGN_CENTER, i + 1, 1);

        secondary_cansniffer_len_labels[i] = lv_label_create(cont);  // len label
        lv_label_set_text(secondary_cansniffer_len_labels[i], "-");
        lv_obj_add_style(secondary_cansniffer_len_labels[i], &secondary_cansniffer_label_style, 0);
        lv_obj_set_grid_cell(secondary_cansniffer_len_labels[i], LV_GRID_ALIGN_CENTER, 3, 1, LV_GRID_ALIGN_CENTER, i + 1, 1);

        secondary_cansniffer_data_labels[i] = lv_label_create(cont);  // data label
        lv_label_set_text(secondary_cansniffer_data_labels[i], "-");
        lv_obj_add_style(secondary_cansniffer_data_labels[i], &secondary_cansniffer_label_style, 0);
        lv_obj_set_grid_cell(secondary_cansniffer_data_labels[i], LV_GRID_ALIGN_CENTER, 4, 1, LV_GRID_ALIGN_CENTER, i + 1, 1);
    }
}

#endif  // CANSNIFFER_ENABLED == 1

#endif  // STEERING_WHEEL_MODE == STEERING_WHEEL_LVGL_MODE
