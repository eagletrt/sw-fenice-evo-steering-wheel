#include "steering_config.h"

#if CANSNIFFER_ENABLED == 1

#include "tab_inverters_cansniffer.h"

#define INVERTERS_CANSNIFFER_ID_NAME_SIZE     128
#define INVERTERS_CANSNIFFER_DATA_STRING_SIZE 128

lv_style_t inverters_cansniffer_label_style;
lv_timer_t *inverters_cansniffer_update_task;

cansniffer_elem_t inverters_cansniffer_buffer[inverters_MESSAGE_COUNT] = {0};
extern int inverters_cansniffer_start_index;
can_id_t inverters_cansniffer_ordered_ids[inverters_MESSAGE_COUNT];
MinHeap(int, inverters_MESSAGE_COUNT) inverters_cansniffer_ids_heap;

void init_inverters_cansniffer_tab_styles() {
    min_heap_init(&inverters_cansniffer_ids_heap, int, inverters_MESSAGE_COUNT, min_heap_compare_indexes);
    lv_style_init(&inverters_cansniffer_label_style);
    lv_style_set_base_dir(&inverters_cansniffer_label_style, LV_BASE_DIR_LTR);
    lv_style_set_bg_opa(&inverters_cansniffer_label_style, LV_OPA_TRANSP);
    lv_style_set_text_color(&inverters_cansniffer_label_style, lv_color_hex(COLOR_SECONDARY_HEX));
    lv_style_set_text_align(&inverters_cansniffer_label_style, LV_TEXT_ALIGN_CENTER);
    lv_style_set_align(&inverters_cansniffer_label_style, LV_ALIGN_CENTER);
}

lv_obj_t *inverters_cansniffer_timestamp_labels[TAB_CANSNIFFER_N_MESSAGES_SHOWN];
lv_obj_t *inverters_cansniffer_id_labels[TAB_CANSNIFFER_N_MESSAGES_SHOWN];
lv_obj_t *inverters_cansniffer_message_name_labels[TAB_CANSNIFFER_N_MESSAGES_SHOWN];
lv_obj_t *inverters_cansniffer_len_labels[TAB_CANSNIFFER_N_MESSAGES_SHOWN];
lv_obj_t *inverters_cansniffer_data_labels[TAB_CANSNIFFER_N_MESSAGES_SHOWN];

char inverters_cansniffer_id_name[INVERTERS_CANSNIFFER_ID_NAME_SIZE] = {0};
char inverters_cansniffer_data_string[INVERTERS_CANSNIFFER_DATA_STRING_SIZE];

void clear_inverters_cansniffer_ui_item(size_t i) {
    lv_label_set_text_fmt(inverters_cansniffer_timestamp_labels[i], "-");
    lv_label_set_text(inverters_cansniffer_id_labels[i], "0x000");
    lv_label_set_text_fmt(inverters_cansniffer_message_name_labels[i], "-");
    lv_label_set_text(inverters_cansniffer_len_labels[i], "-");
    lv_label_set_text(inverters_cansniffer_data_labels[i], "-");
}

void clear_inverters_cansniffer_ui() {
    lv_label_set_text(inverters_cansniffer_timestamp_labels[0], "Time");
    lv_label_set_text(inverters_cansniffer_id_labels[0], "ID");
    lv_label_set_text(inverters_cansniffer_message_name_labels[0], "Inverters Network Message Name");
    lv_label_set_text(inverters_cansniffer_len_labels[0], "Len");
    lv_label_set_text(inverters_cansniffer_data_labels[0], "Data");

    for (size_t i = 0; i < TAB_CANSNIFFER_N_MESSAGES_SHOWN; i++) {
        clear_inverters_cansniffer_ui_item(i);
    }
}

void update_inverters_cansniffer_value(size_t iindex, size_t id) {
    if (iindex >= TAB_CANSNIFFER_N_MESSAGES_SHOWN)
        return;
    lv_label_set_text_fmt(inverters_cansniffer_timestamp_labels[iindex], "%u", (unsigned int)inverters_cansniffer_buffer[id].delta);
    lv_label_set_text_fmt(inverters_cansniffer_id_labels[iindex], "%04X", inverters_cansniffer_buffer[id].id);
    lv_label_set_text_fmt(inverters_cansniffer_message_name_labels[iindex], "%s", inverters_cansniffer_id_name);
    lv_label_set_text_fmt(inverters_cansniffer_len_labels[iindex], "%u", (unsigned int)inverters_cansniffer_buffer[id].len);
    lv_label_set_text_fmt(inverters_cansniffer_data_labels[iindex], "%s", inverters_cansniffer_data_string);
}
void update_inverters_cansniffer_ui() {
    size_t n_msg = min_heap_size(&inverters_cansniffer_ids_heap);
    if (TAB_CANSNIFFER_N_MESSAGES_SHOWN * inverters_cansniffer_start_index > n_msg && n_msg > 1)
        inverters_cansniffer_start_index--;
    for (size_t iindex = TAB_CANSNIFFER_N_MESSAGES_SHOWN * inverters_cansniffer_start_index;
         iindex < (TAB_CANSNIFFER_N_MESSAGES_SHOWN * (inverters_cansniffer_start_index + 1));
         ++iindex) {
        if (iindex >= n_msg) {
            clear_inverters_cansniffer_ui_item(iindex - (TAB_CANSNIFFER_N_MESSAGES_SHOWN * inverters_cansniffer_start_index));
        } else {
            int index = (inverters_cansniffer_ordered_ids[iindex]);
            int res   = inverters_message_name_from_id(inverters_id_from_index(index), inverters_cansniffer_id_name);
            if (res == 0) {
                snprintf(inverters_cansniffer_id_name, INVERTERS_CANSNIFFER_ID_NAME_SIZE, "unknown");
            }
            for (size_t jindex = 0; jindex < inverters_cansniffer_buffer[index].len; ++jindex) {
                snprintf(
                    inverters_cansniffer_data_string + jindex * 3,
                    INVERTERS_CANSNIFFER_DATA_STRING_SIZE,
                    "%02X ",
                    inverters_cansniffer_buffer[index].data[jindex]);
            }
            update_inverters_cansniffer_value(iindex - (TAB_CANSNIFFER_N_MESSAGES_SHOWN * inverters_cansniffer_start_index), index);
        }
    }
}

void cansniffer_inverters_new_message(can_message_t *msg) {
    int index                        = inverters_index_from_id(msg->id);
    bool has_this_msg_ever_been_seen = true;
    if (index == -1) {
        return;
    }
    if (inverters_cansniffer_buffer[index].id == 0) {  // TODO use another bool to indicate if it is used or not
        min_heap_insert(&inverters_cansniffer_ids_heap, &index);
        has_this_msg_ever_been_seen = false;
    }
    uint32_t current_time                        = get_current_time_ms();
    inverters_cansniffer_buffer[index].delta     = current_time - inverters_cansniffer_buffer[index].timestamp;
    inverters_cansniffer_buffer[index].timestamp = current_time;
    inverters_cansniffer_buffer[index].id        = msg->id;
    inverters_cansniffer_buffer[index].len       = msg->size;
    memcpy(inverters_cansniffer_buffer[index].data, msg->data, msg->size);

    if (!has_this_msg_ever_been_seen) {
        MinHeap(int, inverters_MESSAGE_COUNT) inverters_heap_copy_because_toni_is_sbored = min_heap_new(int, inverters_MESSAGE_COUNT, min_heap_compare_indexes);
        memcpy(&inverters_heap_copy_because_toni_is_sbored, &inverters_cansniffer_ids_heap, sizeof(MinHeap(int, inverters_MESSAGE_COUNT)));
        int cindex = 0, i = 0;
        while (!min_heap_is_empty(&inverters_heap_copy_because_toni_is_sbored)) {
            min_heap_remove(&inverters_heap_copy_because_toni_is_sbored, 0, &cindex);
            inverters_cansniffer_ordered_ids[i] = cindex;
            i++;
        }
    }
}

void inverters_tab_cansniffer_create(lv_obj_t *parent) {
    init_inverters_cansniffer_tab_styles();

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
    lv_obj_add_style(cs_l1, &inverters_cansniffer_label_style, 0);
    lv_obj_set_grid_cell(cs_l1, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 0, 1);

    lv_obj_t *cs_l2 = lv_label_create(cont);  // id label
    lv_label_set_text(cs_l2, "ID");
    lv_obj_add_style(cs_l2, &inverters_cansniffer_label_style, 0);
    lv_obj_set_grid_cell(cs_l2, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, 0, 1);

    lv_obj_t *cs_l3 = lv_label_create(cont);  // message name label
    lv_label_set_text(cs_l3, "Inverters Network Message Name");
    lv_obj_add_style(cs_l3, &inverters_cansniffer_label_style, 0);
    lv_obj_set_grid_cell(cs_l3, LV_GRID_ALIGN_CENTER, 2, 1, LV_GRID_ALIGN_CENTER, 0, 1);

    lv_obj_t *cs_l4 = lv_label_create(cont);  // len label
    lv_label_set_text(cs_l4, "Len");
    lv_obj_add_style(cs_l4, &inverters_cansniffer_label_style, 0);
    lv_obj_set_grid_cell(cs_l4, LV_GRID_ALIGN_CENTER, 3, 1, LV_GRID_ALIGN_CENTER, 0, 1);

    lv_obj_t *cs_l5 = lv_label_create(cont);  // data label
    lv_label_set_text(cs_l5, "Data");
    lv_obj_add_style(cs_l5, &inverters_cansniffer_label_style, 0);
    lv_obj_set_grid_cell(cs_l5, LV_GRID_ALIGN_CENTER, 4, 1, LV_GRID_ALIGN_CENTER, 0, 1);

    for (int i = 0; i < TAB_CANSNIFFER_N_MESSAGES_SHOWN; i++) {
        inverters_cansniffer_timestamp_labels[i] = lv_label_create(cont);  // timestamp label
        lv_label_set_text_fmt(inverters_cansniffer_timestamp_labels[i], "-");
        lv_obj_add_style(inverters_cansniffer_timestamp_labels[i], &inverters_cansniffer_label_style, 0);
        lv_obj_set_grid_cell(inverters_cansniffer_timestamp_labels[i], LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, i + 1, 1);

        inverters_cansniffer_id_labels[i] = lv_label_create(cont);  // id label
        lv_label_set_text(inverters_cansniffer_id_labels[i], "0x000");
        lv_obj_add_style(inverters_cansniffer_id_labels[i], &inverters_cansniffer_label_style, 0);
        lv_obj_set_grid_cell(inverters_cansniffer_id_labels[i], LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, i + 1, 1);

        inverters_cansniffer_message_name_labels[i] = lv_label_create(cont);  // message name label
        lv_label_set_text_fmt(inverters_cansniffer_message_name_labels[i], "-");
        lv_obj_add_style(inverters_cansniffer_message_name_labels[i], &inverters_cansniffer_label_style, 0);
        lv_obj_set_grid_cell(inverters_cansniffer_message_name_labels[i], LV_GRID_ALIGN_CENTER, 2, 1, LV_GRID_ALIGN_CENTER, i + 1, 1);

        inverters_cansniffer_len_labels[i] = lv_label_create(cont);  // len label
        lv_label_set_text(inverters_cansniffer_len_labels[i], "-");
        lv_obj_add_style(inverters_cansniffer_len_labels[i], &inverters_cansniffer_label_style, 0);
        lv_obj_set_grid_cell(inverters_cansniffer_len_labels[i], LV_GRID_ALIGN_CENTER, 3, 1, LV_GRID_ALIGN_CENTER, i + 1, 1);

        inverters_cansniffer_data_labels[i] = lv_label_create(cont);  // data label
        lv_label_set_text(inverters_cansniffer_data_labels[i], "-");
        lv_obj_add_style(inverters_cansniffer_data_labels[i], &inverters_cansniffer_label_style, 0);
        lv_obj_set_grid_cell(inverters_cansniffer_data_labels[i], LV_GRID_ALIGN_CENTER, 4, 1, LV_GRID_ALIGN_CENTER, i + 1, 1);
    }
}

#endif  // CANSNIFFER_ENABLED == 1
