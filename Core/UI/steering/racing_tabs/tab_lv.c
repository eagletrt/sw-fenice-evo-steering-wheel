#include "tab_lv.h"

#include <stdio.h>

#define CELL_HEIGHT 200
#define CELL_WIDTH  180

lv_obj_t *tab_lv_pumps_speed_bar;
lv_obj_t *tab_lv_radiators_speed_bar;
lv_style_t tab_lv_style_indic_pumps;
lv_style_t tab_lv_style_indic_radiators;
lv_style_t tab_lv_style_back_indic;

char tab_lv_new_label_buffer[10];

lv_obj_t *tab_lv_labels[tab_lv_labels_n] = {
    [tab_lv_lb_temp_max]        = NULL,
    [tab_lv_lb_temp_min]        = NULL,
    [tab_lv_lb_temp_avg]        = NULL,
    [tab_lv_lb_voltage_max]     = NULL,
    [tab_lv_lb_voltage_min]     = NULL,
    [tab_lv_lb_voltage_delta]   = NULL,
    [tab_lv_lb_pack_voltage]    = NULL,
    [tab_lv_lb_pack_voltage_2]  = NULL,
    [tab_lv_lb_bus_voltage]     = NULL,
    [tab_lv_lb_current_state]   = NULL,
    [tab_lv_lb_last_error]      = NULL,
    [tab_lv_lb_state]           = NULL,
    [tab_lv_lb_pumps_name]      = NULL,
    [tab_lv_lb_pumps_value]     = NULL,
    [tab_lv_lb_radiators_name]  = NULL,
    [tab_lv_lb_radiators_value] = NULL,
};

void set_tab_lv_label_text(const char *s, tab_lv_labels_enum idx) {
    CHECK_CURRENT_TAB(engineer_mode, racing, STEERING_WHEEL_TAB_LV);
    lv_label_set_text(tab_lv_labels[idx], s);
}

void lv_set_pumps_speed_bar(int32_t val, bool auto_mode) {
    auto_mode ? lv_style_set_bg_color(&tab_lv_style_indic_pumps, lv_palette_main(LV_PALETTE_BLUE))
              : lv_style_set_bg_color(&tab_lv_style_indic_pumps, lv_palette_main(LV_PALETTE_GREEN));
    lv_bar_set_value(tab_lv_pumps_speed_bar, val, LV_ANIM_OFF);
}

void lv_set_radiators_speed_bar(int32_t val, bool auto_mode) {
    auto_mode ? lv_style_set_bg_color(&tab_lv_style_indic_radiators, lv_palette_main(LV_PALETTE_BLUE))
              : lv_style_set_bg_color(&tab_lv_style_indic_radiators, lv_palette_main(LV_PALETTE_GREEN));
    lv_bar_set_value(tab_lv_radiators_speed_bar, val, LV_ANIM_OFF);
}

void init_lv_styles(void) {
    lv_style_init(&tab_lv_style_indic_pumps);
    lv_style_set_radius(&tab_lv_style_indic_pumps, 0);
    lv_style_set_bg_opa(&tab_lv_style_indic_pumps, LV_OPA_COVER);
    lv_style_set_bg_color(&tab_lv_style_indic_pumps, lv_palette_main(LV_PALETTE_BLUE));
    // lv_style_set_bg_grad_color(&tab_lv_style_indic_pumps, lv_palette_main(LV_PALETTE_RED));
    // lv_style_set_bg_grad_dir(&tab_lv_style_indic_pumps, LV_GRAD_DIR_VER);

    lv_style_init(&tab_lv_style_indic_radiators);
    lv_style_set_radius(&tab_lv_style_indic_radiators, 0);
    lv_style_set_bg_opa(&tab_lv_style_indic_radiators, LV_OPA_COVER);
    lv_style_set_bg_color(&tab_lv_style_indic_radiators, lv_palette_main(LV_PALETTE_BLUE));
    // lv_style_set_bg_grad_color(&tab_lv_style_indic_radiators, lv_palette_main(LV_PALETTE_RED));
    // lv_style_set_bg_grad_dir(&tab_lv_style_indic_radiators, LV_GRAD_DIR_VER);

    lv_style_init(&tab_lv_style_back_indic);
    lv_style_set_radius(&tab_lv_style_back_indic, 0);
    lv_style_set_bg_opa(&tab_lv_style_back_indic, LV_OPA_COVER);
    lv_style_set_bg_color(&tab_lv_style_back_indic, lv_color_hex(COLOR_SECONDARY_HEX));
}

void tab_lv_create(lv_obj_t *parent) {
    init_lv_styles();

    /*---creating MAIN GRID ---*/

    static lv_coord_t main_panel_cols[] = {SCREEN_WIDTH, LV_GRID_TEMPLATE_LAST};
    static lv_coord_t main_panel_rows[] = {TOP_BAR_HEIGHT, DATA_PANEL_HEIGHT, LV_GRID_TEMPLATE_LAST};

    lv_obj_t *main_panel = lv_obj_create(parent);
    lv_obj_remove_style_all(main_panel);
    // lv_obj_set_layout(main_panel, LV_LAYOUT_GRID);
    lv_obj_clear_flag(main_panel, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(main_panel, SCREEN_WIDTH, SCREEN_HEIGHT);

    lv_obj_add_style(main_panel, &grid_style, 0);
    lv_obj_center(main_panel);
    lv_obj_set_style_base_dir(main_panel, LV_BASE_DIR_LTR, 0);
    lv_obj_set_grid_dsc_array(main_panel, main_panel_cols, main_panel_rows);

    /*--- inserting TOP NOTCH ---*/

    lv_obj_t *notch = create_notch(main_panel, STEERING_WHEEL_TAB_LV);
    lv_obj_align(lv_obj_get_child(notch, 0), LV_ALIGN_TOP_MID, 0, 10);
    lv_obj_set_grid_cell(notch, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_START, 0, 1);

    /*--- creating DATA PANEL ---*/

    static lv_coord_t dpanel_cols[] = {SCREEN_WIDTH, LV_GRID_TEMPLATE_LAST};
    static lv_coord_t dpanel_rows[] = {100, DATA_PANEL_HEIGHT - 100, LV_GRID_TEMPLATE_LAST};

    lv_obj_t *data_panel = lv_obj_create(main_panel);
    lv_obj_remove_style_all(data_panel);
    // lv_obj_set_layout(data_panel, LV_LAYOUT_GRID);
    lv_obj_clear_flag(data_panel, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(
        data_panel,
        SCREEN_WIDTH,
        DATA_PANEL_HEIGHT);  // non andrebbe tolto il 20 per farlo
                             // simmetrico(forse qualche errore)

    lv_obj_add_style(data_panel, &grid_style, 0);
    lv_obj_center(data_panel);
    lv_obj_set_style_base_dir(data_panel, LV_BASE_DIR_LTR, 0);
    lv_obj_set_grid_dsc_array(data_panel, dpanel_cols, dpanel_rows);

    // lv_obj_set_style_pad_row(data_panel, ROW_PADDING, 0);
    // lv_obj_set_style_pad_column(data_panel, COLUMN_PADDING, 0);
    lv_obj_set_style_pad_all(data_panel, 0, LV_PART_MAIN);

    lv_obj_set_grid_cell(data_panel, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 1, 1);

    /*--- DATA PANEL: cell 0, 0 ---*/

    static lv_coord_t rxd_panel_cols[] = {
        DATA_PANEL_WIDTH / 8,
        DATA_PANEL_WIDTH / 8,
        DATA_PANEL_WIDTH / 8,
        DATA_PANEL_WIDTH / 8,
        DATA_PANEL_WIDTH / 8,
        DATA_PANEL_WIDTH / 8,
        DATA_PANEL_WIDTH / 8,
        DATA_PANEL_WIDTH / 8,
        LV_GRID_TEMPLATE_LAST};
    static lv_coord_t rxd_panel_rows[] = {DATA_PANEL_HEIGHT / 4, LV_GRID_TEMPLATE_LAST};

    lv_obj_t *top_data_panel = lv_obj_create(data_panel);
    lv_obj_set_layout(top_data_panel, LV_LAYOUT_GRID);
    lv_obj_remove_style_all(top_data_panel);
    lv_obj_clear_flag(top_data_panel, LV_OBJ_FLAG_SCROLLABLE);
    // lv_obj_set_size(left_data_panel, CELL_WIDTH, CELL_HEIGHT);

    // lv_obj_add_style(top_data_panel, &box_panels_style, 0);
    lv_obj_center(top_data_panel);
    lv_obj_set_style_base_dir(top_data_panel, LV_BASE_DIR_LTR, 0);
    lv_obj_set_grid_dsc_array(top_data_panel, rxd_panel_cols, rxd_panel_rows);

    lv_obj_set_grid_cell(top_data_panel, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 0, 1);

    /* 1st row */

    /* TEMP */
    lv_obj_t *inv_l_temp_lb = lv_triple_label(
        top_data_panel, &tab_lv_labels[tab_lv_lb_temp_max], "NA", &lv_font_inter_bold_30, "°C", &lv_font_inter_bold_22, "MAX", &lv_font_inter_bold_20);
    lv_obj_set_grid_cell(inv_l_temp_lb, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 0, 1);

    lv_obj_t *inv_r_temp_lb = lv_triple_label(
        top_data_panel, &tab_lv_labels[tab_lv_lb_temp_min], "NA", &lv_font_inter_bold_30, "°C", &lv_font_inter_bold_22, "MIN", &lv_font_inter_bold_20);
    lv_obj_set_grid_cell(inv_r_temp_lb, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, 0, 1);

    lv_obj_t *motor_l_temp_lb = lv_triple_label(
        top_data_panel, &tab_lv_labels[tab_lv_lb_temp_avg], "NA", &lv_font_inter_bold_30, "°C", &lv_font_inter_bold_22, "AVG", &lv_font_inter_bold_20);
    lv_obj_set_grid_cell(motor_l_temp_lb, LV_GRID_ALIGN_CENTER, 2, 1, LV_GRID_ALIGN_CENTER, 0, 1);

    /* VOLT */
    lv_obj_t *motor_r_temp_lb = lv_triple_label(
        top_data_panel, &tab_lv_labels[tab_lv_lb_voltage_max], "NA", &lv_font_inter_bold_30, " V", &lv_font_inter_bold_22, "MAX", &lv_font_inter_bold_20);
    lv_obj_set_grid_cell(motor_r_temp_lb, LV_GRID_ALIGN_CENTER, 3, 1, LV_GRID_ALIGN_CENTER, 0, 1);

    lv_obj_t *lv_temp_lb = lv_triple_label(
        top_data_panel, &tab_lv_labels[tab_lv_lb_voltage_min], "NA", &lv_font_inter_bold_30, " V", &lv_font_inter_bold_22, "MIN", &lv_font_inter_bold_20);
    lv_obj_set_grid_cell(lv_temp_lb, LV_GRID_ALIGN_CENTER, 4, 1, LV_GRID_ALIGN_CENTER, 0, 1);

    lv_obj_t *lv_volt_lb = lv_triple_label(
        top_data_panel, &tab_lv_labels[tab_lv_lb_voltage_delta], "NA", &lv_font_inter_bold_30, " mV", &lv_font_inter_bold_22, "DELTA", &lv_font_inter_bold_20);
    lv_obj_set_grid_cell(lv_volt_lb, LV_GRID_ALIGN_CENTER, 5, 1, LV_GRID_ALIGN_CENTER, 0, 1);

    lv_obj_t *lv_curr_lb = lv_triple_label(
        top_data_panel, &tab_lv_labels[tab_lv_lb_pack_voltage], "NA", &lv_font_inter_bold_30, " V", &lv_font_inter_bold_22, "PACK", &lv_font_inter_bold_20);
    lv_obj_set_grid_cell(lv_curr_lb, LV_GRID_ALIGN_CENTER, 6, 1, LV_GRID_ALIGN_CENTER, 0, 1);

    lv_obj_t *lv_minCellV_lb = lv_triple_label(
        top_data_panel, &tab_lv_labels[tab_lv_lb_pack_voltage_2], "NA", &lv_font_inter_bold_30, " A", &lv_font_inter_bold_22, "CURR", &lv_font_inter_bold_20);
    lv_obj_set_grid_cell(lv_minCellV_lb, LV_GRID_ALIGN_CENTER, 7, 1, LV_GRID_ALIGN_CENTER, 0, 1);

    /* 2nd row LV */

    /*--- creating CENTER PANEL ---*/
    static lv_coord_t center_panel_cols[] = {CP_COLUMN_WIDTH, CP_COLUMN_WIDTH, CP_COLUMN_WIDTH, LV_GRID_TEMPLATE_LAST};
    static lv_coord_t center_panel_rows[] = {CENTRAL_PANEL_HEIGHT, LV_GRID_TEMPLATE_LAST};

    lv_obj_t *center_data_panel = lv_obj_create(data_panel);
    lv_obj_set_layout(center_data_panel, LV_LAYOUT_GRID);
    lv_obj_remove_style_all(center_data_panel);
    lv_obj_clear_flag(center_data_panel, LV_OBJ_FLAG_SCROLLABLE);
    // lv_obj_set_size(left_data_panel, CELL_WIDTH, CELL_HEIGHT);

    // lv_obj_add_style(center_data_panel, &box_panels_style, 0);
    lv_obj_center(center_data_panel);
    lv_obj_set_style_base_dir(center_data_panel, LV_BASE_DIR_LTR, 0);
    lv_obj_set_grid_dsc_array(center_data_panel, center_panel_cols, center_panel_rows);

    lv_obj_set_grid_cell(center_data_panel, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 1, 1);

    /* PUMPS PANEL */
    static lv_coord_t pumps_panel_cols[] = {CP_COLUMN_WIDTH, LV_GRID_TEMPLATE_LAST};
    static lv_coord_t pumps_panel_rows[] = {SUB_PANEL_HEADER_HEIGHT, (CENTRAL_PANEL_HEIGHT - SUB_PANEL_HEADER_HEIGHT), LV_GRID_TEMPLATE_LAST};

    lv_obj_t *pumps_panel = lv_obj_create(center_data_panel);
    lv_obj_set_layout(pumps_panel, LV_LAYOUT_GRID);
    lv_obj_remove_style_all(pumps_panel);
    lv_obj_clear_flag(pumps_panel, LV_OBJ_FLAG_SCROLLABLE);
    // lv_obj_set_size(left_data_panel, CELL_WIDTH, CELL_HEIGHT);

    lv_obj_center(pumps_panel);
    lv_obj_set_style_base_dir(pumps_panel, LV_BASE_DIR_LTR, 0);
    lv_obj_set_grid_dsc_array(pumps_panel, pumps_panel_cols, pumps_panel_rows);

    lv_obj_set_grid_cell(pumps_panel, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 0, 1);

    lv_obj_t *pumps_label = lv_label_create(pumps_panel);
    lv_label_set_text(pumps_label, "pumps");
    lv_obj_set_style_text_font(pumps_label, &lv_font_inter_bold_30, LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(pumps_label, lv_color_hex(0xffffff), 0);
    lv_obj_set_grid_cell(pumps_label, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 0, 1);

    /* PUMPS BAR PANEL */
    static lv_coord_t pumps_bar_panel_cols[] = {CP_COLUMN_WIDTH / 3, CP_COLUMN_WIDTH / 3, CP_COLUMN_WIDTH / 3, LV_GRID_TEMPLATE_LAST};
    static lv_coord_t pumps_bar_panel_rows[] = {BAR_HEIGHT, LV_GRID_TEMPLATE_LAST};

    lv_obj_t *pumps_bar_panel = lv_obj_create(pumps_panel);
    lv_obj_set_layout(pumps_bar_panel, LV_LAYOUT_GRID);
    lv_obj_remove_style_all(pumps_bar_panel);
    lv_obj_clear_flag(pumps_bar_panel, LV_OBJ_FLAG_SCROLLABLE);
    // lv_obj_set_size(left_data_panel, CELL_WIDTH, CELL_HEIGHT);

    // lv_obj_add_style(pumps_bar_panel, &box_panels_style, 0);
    lv_obj_center(pumps_bar_panel);
    lv_obj_set_style_base_dir(pumps_bar_panel, LV_BASE_DIR_LTR, 0);
    lv_obj_set_grid_dsc_array(pumps_bar_panel, pumps_bar_panel_cols, pumps_bar_panel_rows);

    lv_obj_set_grid_cell(pumps_bar_panel, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 1, 1);

    lv_obj_t *pumps_actual_value =
        lv_horizontal_pair_label(pumps_bar_panel, &tab_lv_labels[tab_lv_lb_pumps_name], "", &lv_font_inter_bold_30, "", &lv_font_inter_bold_20);
    lv_obj_set_grid_cell(pumps_actual_value, LV_GRID_ALIGN_END, 0, 1, LV_GRID_ALIGN_START, 0, 1);

    tab_lv_pumps_speed_bar = lv_bar_create(pumps_bar_panel);
    lv_obj_remove_style_all(tab_lv_pumps_speed_bar);
    lv_obj_add_style(tab_lv_pumps_speed_bar, &tab_lv_style_indic_pumps, LV_PART_INDICATOR);
    lv_obj_add_style(tab_lv_pumps_speed_bar, &tab_lv_style_back_indic, LV_PART_MAIN);
    lv_obj_set_size(tab_lv_pumps_speed_bar, 50, BAR_HEIGHT);
    lv_bar_set_range(tab_lv_pumps_speed_bar, 0, 100);
    lv_bar_set_value(tab_lv_pumps_speed_bar, 0, LV_ANIM_OFF);

    lv_obj_set_grid_cell(tab_lv_pumps_speed_bar, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, 0, 1);

    lv_obj_t *pumps_local_value =
        lv_horizontal_pair_label(pumps_bar_panel, &tab_lv_labels[tab_lv_lb_pumps_value], "NA", &lv_font_inter_bold_30, "%", &lv_font_inter_bold_20);
    lv_obj_set_grid_cell(pumps_local_value, LV_GRID_ALIGN_START, 2, 1, LV_GRID_ALIGN_START, 0, 1);

    /* STATUS PANEL */
    static lv_coord_t status_panel_cols[] = {CP_COLUMN_WIDTH, LV_GRID_TEMPLATE_LAST};
    static lv_coord_t status_panel_rows[] = {((CENTRAL_PANEL_HEIGHT) / 2), ((CENTRAL_PANEL_HEIGHT) / 2), LV_GRID_TEMPLATE_LAST};

    lv_obj_t *status_panel = lv_obj_create(center_data_panel);
    lv_obj_set_layout(status_panel, LV_LAYOUT_GRID);
    lv_obj_remove_style_all(status_panel);
    lv_obj_clear_flag(status_panel, LV_OBJ_FLAG_SCROLLABLE);
    // lv_obj_set_size(left_data_panel, CELL_WIDTH, CELL_HEIGHT);

    // lv_obj_add_style(status_panel, &box_panels_style, 0);
    lv_obj_center(status_panel);
    lv_obj_set_style_base_dir(status_panel, LV_BASE_DIR_LTR, 0);
    lv_obj_set_grid_dsc_array(status_panel, status_panel_cols, status_panel_rows);

    lv_obj_set_grid_cell(status_panel, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 0, 1);

    lv_obj_t *status_label = lv_label_create(status_panel);
    lv_label_set_text(status_label, "Status");
    lv_obj_set_style_text_font(status_label, &lv_font_inter_bold_30, LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(status_label, lv_color_hex(0xffffff), 0);
    lv_obj_set_grid_cell(status_label, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 0, 1);

    tab_lv_labels[tab_lv_lb_state] = lv_label_create(status_panel);
    lv_label_set_text(tab_lv_labels[tab_lv_lb_state], "OFF");
    lv_obj_set_style_text_font(tab_lv_labels[tab_lv_lb_state], &lv_font_inter_bold_30, LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(tab_lv_labels[tab_lv_lb_state], lv_color_hex(0xffffff), 0);
    lv_obj_set_grid_cell(tab_lv_labels[tab_lv_lb_state], LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_START, 1, 1);

    /*--- creating RADIATORS PANEL ---*/
    static lv_coord_t radiators_panel_cols[] = {CP_COLUMN_WIDTH, LV_GRID_TEMPLATE_LAST};
    static lv_coord_t radiators_panel_rows[] = {SUB_PANEL_HEADER_HEIGHT, (CENTRAL_PANEL_HEIGHT - SUB_PANEL_HEADER_HEIGHT), LV_GRID_TEMPLATE_LAST};

    lv_obj_t *radiators_panel = lv_obj_create(center_data_panel);
    lv_obj_set_layout(radiators_panel, LV_LAYOUT_GRID);
    lv_obj_remove_style_all(radiators_panel);
    lv_obj_clear_flag(radiators_panel, LV_OBJ_FLAG_SCROLLABLE);
    // lv_obj_set_size(left_data_panel, CELL_WIDTH, CELL_HEIGHT);

    lv_obj_center(radiators_panel);
    lv_obj_set_style_base_dir(radiators_panel, LV_BASE_DIR_LTR, 0);
    lv_obj_set_grid_dsc_array(radiators_panel, radiators_panel_cols, radiators_panel_rows);

    lv_obj_set_grid_cell(radiators_panel, LV_GRID_ALIGN_STRETCH, 2, 1, LV_GRID_ALIGN_STRETCH, 0, 1);

    lv_obj_t *radiators_label = lv_label_create(radiators_panel);
    lv_label_set_text(radiators_label, "radiators");
    lv_obj_set_style_text_font(radiators_label, &lv_font_inter_bold_30, LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(radiators_label, lv_color_hex(0xffffff), 0);
    lv_obj_set_grid_cell(radiators_label, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 0, 1);

    /* RADIATORS BAR PANEL */
    static lv_coord_t radiators_bar_panel_cols[] = {CP_COLUMN_WIDTH / 3, CP_COLUMN_WIDTH / 3, CP_COLUMN_WIDTH / 3, LV_GRID_TEMPLATE_LAST};
    static lv_coord_t radiators_bar_panel_rows[] = {BAR_HEIGHT, LV_GRID_TEMPLATE_LAST};

    lv_obj_t *radiators_bar_panel = lv_obj_create(radiators_panel);
    lv_obj_set_layout(radiators_bar_panel, LV_LAYOUT_GRID);
    lv_obj_remove_style_all(radiators_bar_panel);
    lv_obj_clear_flag(radiators_bar_panel, LV_OBJ_FLAG_SCROLLABLE);
    // lv_obj_set_size(left_data_panel, CELL_WIDTH, CELL_HEIGHT);

    lv_obj_center(radiators_bar_panel);
    lv_obj_set_style_base_dir(radiators_bar_panel, LV_BASE_DIR_LTR, 0);
    lv_obj_set_grid_dsc_array(radiators_bar_panel, radiators_bar_panel_cols, radiators_bar_panel_rows);

    lv_obj_set_grid_cell(radiators_bar_panel, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 1, 1);

    lv_obj_t *radiators_actual_value =
        lv_horizontal_pair_label(radiators_bar_panel, &tab_lv_labels[tab_lv_lb_radiators_name], "", &lv_font_inter_bold_30, "", &lv_font_inter_bold_20);
    lv_obj_set_grid_cell(radiators_actual_value, LV_GRID_ALIGN_END, 0, 1, LV_GRID_ALIGN_START, 0, 1);

    tab_lv_radiators_speed_bar = lv_bar_create(radiators_bar_panel);
    lv_obj_remove_style_all(tab_lv_radiators_speed_bar);
    lv_obj_add_style(tab_lv_radiators_speed_bar, &tab_lv_style_indic_radiators, LV_PART_INDICATOR);
    lv_obj_add_style(tab_lv_radiators_speed_bar, &tab_lv_style_back_indic, LV_PART_MAIN);
    lv_obj_set_size(tab_lv_radiators_speed_bar, 50, BAR_HEIGHT);
    lv_bar_set_range(tab_lv_radiators_speed_bar, 0, 100);
    lv_bar_set_value(tab_lv_radiators_speed_bar, 0, LV_ANIM_OFF);

    lv_obj_set_grid_cell(tab_lv_radiators_speed_bar, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, 0, 1);

    lv_obj_t *radiators_local_value =
        lv_horizontal_pair_label(radiators_bar_panel, &tab_lv_labels[tab_lv_lb_radiators_value], "NA", &lv_font_inter_bold_30, "%", &lv_font_inter_bold_20);
    lv_obj_set_grid_cell(radiators_local_value, LV_GRID_ALIGN_START, 2, 1, LV_GRID_ALIGN_START, 0, 1);
}

void tab_lv_resync(void) {
    lv_pumps_speed_update();
    lv_radiator_speed_update();
    lv_currents_update();
    lv_total_voltage_update();
    lv_cells_voltage_update();
    lv_cells_voltage_stats_update();
    lv_cells_temp_update();
    lv_cells_temp_stats_update();
}