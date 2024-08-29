#include "steering_config.h"

#if STEERING_WHEEL_MODE == STEERING_WHEEL_LVGL_MODE

#include "tab_terminal.h"

lv_style_t tab_terminal_style;
uint32_t tab_terminal_message_index = 0;

lv_obj_t *tab_terminal_labels[TERMINAL_ROWS_N];

void init_terminal_tab_styles(void) {
    lv_style_init(&tab_terminal_style);
    lv_style_set_base_dir(&tab_terminal_style, LV_BASE_DIR_LTR);
    lv_style_set_bg_opa(&tab_terminal_style, LV_OPA_TRANSP);
    lv_style_set_text_color(&tab_terminal_style, lv_color_hex(COLOR_SECONDARY_HEX));
    lv_style_set_text_align(&tab_terminal_style, LV_TEXT_ALIGN_CENTER);
    lv_style_set_align(&tab_terminal_style, LV_ALIGN_CENTER);
}

void tab_terminal_create(lv_obj_t *parent) {
    init_terminal_tab_styles();

    static lv_coord_t tab_terminal_cols_ratios[] = {SCREEN_WIDTH, LV_GRID_TEMPLATE_LAST};
    static lv_coord_t tab_terminal_rows_ratios[] = {
        SCREEN_HEIGHT / (TERMINAL_ROWS_N + 1),
        SCREEN_HEIGHT / (TERMINAL_ROWS_N + 1),
        SCREEN_HEIGHT / (TERMINAL_ROWS_N + 1),
        SCREEN_HEIGHT / (TERMINAL_ROWS_N + 1),
        SCREEN_HEIGHT / (TERMINAL_ROWS_N + 1),
        SCREEN_HEIGHT / (TERMINAL_ROWS_N + 1),
        SCREEN_HEIGHT / (TERMINAL_ROWS_N + 1),
        SCREEN_HEIGHT / (TERMINAL_ROWS_N + 1),
        SCREEN_HEIGHT / (TERMINAL_ROWS_N + 1),
        SCREEN_HEIGHT / (TERMINAL_ROWS_N + 1),
        SCREEN_HEIGHT / (TERMINAL_ROWS_N + 1),
        SCREEN_HEIGHT / (TERMINAL_ROWS_N + 1),
        SCREEN_HEIGHT / (TERMINAL_ROWS_N + 1),
        LV_GRID_TEMPLATE_LAST};

    /*Create a container with grid*/
    lv_obj_t *cont = lv_obj_create(parent);

    lv_obj_set_layout(cont, LV_LAYOUT_GRID);
    lv_obj_set_size(cont, SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_obj_add_style(cont, &grid_style, 0);
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_center(cont);

    lv_obj_set_grid_dsc_array(cont, tab_terminal_cols_ratios, tab_terminal_rows_ratios);

    lv_obj_t *cs_l1 = lv_label_create(cont);  // timestamp label
    lv_label_set_text(cs_l1, "TAB TERMINAL");
    lv_obj_add_style(cs_l1, &tab_terminal_style, 0);
    lv_obj_set_grid_cell(cs_l1, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 0, 1);

    for (int i = 0; i < TERMINAL_ROWS_N; i++) {
        tab_terminal_labels[i] = lv_label_create(cont);
        lv_label_set_text_fmt(tab_terminal_labels[i], "-----");
        lv_obj_add_style(tab_terminal_labels[i], &tab_terminal_style, 0);
        lv_obj_set_grid_cell(tab_terminal_labels[i], LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, i + 1, 1);
    }
}

void tab_terminal_new_message(const char *message) {
    if (tab_terminal_message_index >= TERMINAL_ROWS_N) {
        tab_terminal_clear();
        tab_terminal_message_index = 0;
    }
    lv_label_set_text_fmt(tab_terminal_labels[tab_terminal_message_index], "%s", message);
    tab_terminal_message_index++;
}

void tab_terminal_clear() {
    for (int i = 0; i < TERMINAL_ROWS_N; i++) {
        lv_label_set_text_fmt(tab_terminal_labels[i], "-----");
    }
}

#endif  // STEERING_WHEEL_MODE == STEERING_WHEEL_LVGL_MODE
