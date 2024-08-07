#if 0

#include "steering_config.h"

#if STEER_TAB_DEBUG_ENABLED == 1

#include "tab_debug.h"

#include <stdio.h>

lv_obj_t *hv_feedbacks_status[20];
lv_obj_t *hv_errors[16];
lv_obj_t *das_errors[9];
lv_obj_t *lv_errors[17];

lv_style_t box_debug_style_yellow;
lv_style_t box_debug_label_style;

lv_obj_t *scr_debug;

lv_obj_t *hv_feedbacks_panel;
lv_obj_t *hv_errors_panel;
lv_obj_t *lv_errors_panel;
lv_obj_t *das_errors_panel;

extern racing_tab_t current_racing_tab;

primary_hv_errors_converted_t hv_errors_last_state                   = {0};
primary_hv_feedback_status_converted_t hv_feedback_status_last_state = {0};
primary_das_errors_converted_t das_errors_last_state                 = {0};
primary_lv_errors_converted_t lv_errors_last_state                   = {0};

lv_obj_t *cell_create(lv_obj_t *parent, const char *text, uint8_t pos_col, uint8_t pos_row, lv_style_t *style) {
    lv_obj_t *cell = lv_obj_create(parent);
    lv_obj_remove_style_all(cell);
    lv_obj_clear_flag(cell, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(cell, TAB_DEBUG_CELL_WIDTH, TAB_DEBUG_CELL_HEIGHT);

    lv_obj_add_style(cell, style, 0);
    lv_obj_center(cell);
    lv_obj_set_style_base_dir(cell, LV_BASE_DIR_LTR, 0);
    lv_obj_set_grid_cell(cell, LV_GRID_ALIGN_CENTER, pos_col, 1, LV_GRID_ALIGN_CENTER, pos_row, 1);

    lv_obj_t *label = lv_label_create(cell);
    lv_obj_add_style(label, &box_debug_label_style, LV_PART_MAIN);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_font(label, &lv_font_inter_bold_14, LV_STATE_DEFAULT);

    return cell;
}

void init_debug_styles(void) {
    /*setting box_debug_style*/
    lv_style_init(&box_debug_style_yellow);
    lv_style_set_pad_all(&box_debug_style_yellow, 0);
    lv_style_set_pad_bottom(&box_debug_style_yellow, 0);
    lv_style_set_pad_column(&box_debug_style_yellow, 0);
    lv_style_set_pad_top(&box_debug_style_yellow, 0);
    lv_style_set_pad_row(&box_debug_style_yellow, 0);
    lv_style_set_bg_opa(&box_debug_style_yellow, LV_OPA_COVER);
    lv_style_set_border_color(&box_debug_style_yellow, lv_color_hex(COLOR_GREEN_STATUS_HEX));
    lv_style_set_border_width(&box_debug_style_yellow, 3);
    lv_style_set_bg_color(&box_debug_style_yellow, lv_color_hex(COLOR_GREEN_STATUS_HEX));
    lv_style_set_radius(&box_debug_style_yellow, 10);

    /*setting label style*/
    lv_style_init(&box_debug_label_style);
    lv_style_set_base_dir(&box_debug_label_style, LV_BASE_DIR_LTR);
    lv_style_set_bg_opa(&box_debug_label_style, LV_OPA_TRANSP);
    lv_style_set_text_color(&box_debug_label_style, lv_color_hex(COLOR_PRIMARY_HEX));
    lv_style_set_text_align(&box_debug_label_style, LV_TEXT_ALIGN_CENTER);
    lv_style_set_text_font(&box_debug_label_style, &lv_font_inter_bold_30);
    lv_style_set_align(&box_debug_label_style, LV_ALIGN_CENTER);
}

void tab_debug_create(lv_obj_t *parent) {
    init_debug_styles();

    scr_debug = parent;

    /*---creating MAIN GRID ---*/

    static lv_coord_t main_panel_cols[] = {SCREEN_WIDTH, LV_GRID_TEMPLATE_LAST};
    static lv_coord_t main_panel_rows[] = {TAB_DEBUG_TOP_BAR_HEIGHT, TAB_DEBUG_DATA_PANEL_HEIGHT, LV_GRID_TEMPLATE_LAST};

    lv_obj_t *main_panel = lv_obj_create(parent);
    lv_obj_add_style(main_panel, &grid_style, 0);
    lv_obj_set_layout(main_panel, LV_LAYOUT_GRID);
    lv_obj_clear_flag(main_panel, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(main_panel, SCREEN_WIDTH, SCREEN_HEIGHT);

    lv_obj_center(main_panel);
    lv_obj_set_style_base_dir(main_panel, LV_BASE_DIR_LTR, 0);
    lv_obj_set_grid_dsc_array(main_panel, main_panel_cols, main_panel_rows);

    /*--- inserting TOP NOTCH ---*/

    lv_obj_t *notch = create_notch(main_panel, TAB_DEBUG);
    lv_obj_align(lv_obj_get_child(notch, 0), LV_ALIGN_TOP_MID, 0, 10);
    lv_obj_set_grid_cell(notch, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_START, 0, 1);

    /*--- creating HV FEEDBACKS PANEL ---*/

    static lv_coord_t hvFeedbacks_panel_cols[] = {
        TAB_DEBUG_CELL_WIDTH, TAB_DEBUG_CELL_WIDTH, TAB_DEBUG_CELL_WIDTH, TAB_DEBUG_CELL_WIDTH, LV_GRID_TEMPLATE_LAST};
    static lv_coord_t hvFeedbacks_panel_rows[] = {
        TAB_DEBUG_DATA_TITLE_HEIGHT,
        TAB_DEBUG_CELL_HEIGHT,
        TAB_DEBUG_CELL_HEIGHT,
        TAB_DEBUG_CELL_HEIGHT,
        TAB_DEBUG_CELL_HEIGHT,
        TAB_DEBUG_CELL_HEIGHT,
        LV_GRID_TEMPLATE_LAST};

    hv_feedbacks_panel = lv_obj_create(main_panel);
    lv_obj_remove_style_all(hv_feedbacks_panel);
    lv_obj_set_layout(hv_feedbacks_panel, LV_LAYOUT_GRID);
    lv_obj_clear_flag(hv_feedbacks_panel, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(hv_feedbacks_panel, TAB_DEBUG_CELL_WIDTH * 4 + TAB_DEBUG_COLUMN_PADDING * 3, TAB_DEBUG_DATA_PANEL_HEIGHT);

    lv_obj_add_style(hv_feedbacks_panel, &grid_style, 0);
    lv_obj_align(hv_feedbacks_panel, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_base_dir(hv_feedbacks_panel, LV_BASE_DIR_LTR, 0);
    lv_obj_set_style_opa(hv_feedbacks_panel, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_grid_dsc_array(hv_feedbacks_panel, hvFeedbacks_panel_cols, hvFeedbacks_panel_rows);

    lv_obj_set_style_pad_row(hv_feedbacks_panel, TAB_DEBUG_ROW_PADDING, 0);
    lv_obj_set_style_pad_column(hv_feedbacks_panel, TAB_DEBUG_COLUMN_PADDING, 0);

    lv_obj_set_grid_cell(hv_feedbacks_panel, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 1, 1);

    lv_obj_t *hv_feedbacks_title_container = lv_obj_create(hv_feedbacks_panel);
    lv_obj_remove_style_all(hv_feedbacks_title_container);
    lv_obj_set_size(hv_feedbacks_title_container, 250, 50);
    lv_obj_add_style(hv_feedbacks_title_container, &box_panels_style, LV_PART_MAIN);
    lv_obj_set_grid_cell(hv_feedbacks_title_container, LV_GRID_ALIGN_CENTER, 0, 4, LV_GRID_ALIGN_END, 0, 1);

    lv_obj_t *hv_feedbacks_title = lv_label_create(hv_feedbacks_title_container);
    lv_obj_remove_style_all(hv_feedbacks_title);
    lv_obj_add_style(hv_feedbacks_title, &label_style, LV_PART_MAIN);
    // lv_obj_add_style(hv_feedbacks_title, &box_panels_style, LV_PART_MAIN);
    lv_obj_align(hv_feedbacks_title, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_text_font(hv_feedbacks_title, &lv_font_inter_bold_30, LV_PART_MAIN);
    lv_obj_set_style_text_align(hv_feedbacks_title, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_align(hv_feedbacks_title, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text(hv_feedbacks_title, "HV FEEDBACKS");

    hv_feedbacks_status[0]  = cell_create(hv_feedbacks_panel, "IMPLAUS. DETECTED", 0, 2, &box_debug_style_yellow);
    hv_feedbacks_status[1]  = cell_create(hv_feedbacks_panel, "IMD COCKPIT", 0, 1, &box_debug_style_yellow);
    hv_feedbacks_status[2]  = cell_create(hv_feedbacks_panel, "TSAL GREEN F LATCHED", 0, 3, &box_debug_style_yellow);
    hv_feedbacks_status[3]  = cell_create(hv_feedbacks_panel, "BMS COCKPIT", 0, 4, &box_debug_style_yellow);
    hv_feedbacks_status[4]  = cell_create(hv_feedbacks_panel, "EXT LATCHED", 3, 2, &box_debug_style_yellow);
    hv_feedbacks_status[5]  = cell_create(hv_feedbacks_panel, "TSAL GREEN", 1, 1, &box_debug_style_yellow);
    hv_feedbacks_status[6]  = cell_create(hv_feedbacks_panel, "TS OVER 60V STATUS", 1, 2, &box_debug_style_yellow);
    hv_feedbacks_status[7]  = cell_create(hv_feedbacks_panel, "AIRN STATUS", 1, 3, &box_debug_style_yellow);
    hv_feedbacks_status[8]  = cell_create(hv_feedbacks_panel, "AIRP STATUS", 1, 4, &box_debug_style_yellow);
    hv_feedbacks_status[9]  = cell_create(hv_feedbacks_panel, "AIRP GATE", 1, 5, &box_debug_style_yellow);
    hv_feedbacks_status[10] = cell_create(hv_feedbacks_panel, "AIRN GATE", 2, 1, &box_debug_style_yellow);
    hv_feedbacks_status[11] = cell_create(hv_feedbacks_panel, "PRECHARGE STATUS", 2, 2, &box_debug_style_yellow);
    hv_feedbacks_status[12] = cell_create(hv_feedbacks_panel, "TSP OVER 60V STATUS", 2, 3, &box_debug_style_yellow);
    hv_feedbacks_status[13] = cell_create(hv_feedbacks_panel, "IMD FAULT", 3, 3, &box_debug_style_yellow);
    hv_feedbacks_status[14] = cell_create(hv_feedbacks_panel, "CHECK MUX", 2, 4, &box_debug_style_yellow);
    hv_feedbacks_status[15] = cell_create(hv_feedbacks_panel, "SD END", 3, 4, &box_debug_style_yellow);
    hv_feedbacks_status[16] = cell_create(hv_feedbacks_panel, "SD OUT", 3, 1, &box_debug_style_yellow);
    hv_feedbacks_status[17] = cell_create(hv_feedbacks_panel, "SD IN", 2, 5, &box_debug_style_yellow);
    hv_feedbacks_status[18] = cell_create(hv_feedbacks_panel, "SD BMS", 3, 5, &box_debug_style_yellow);
    hv_feedbacks_status[19] = cell_create(hv_feedbacks_panel, "SD IMD", 0, 5, &box_debug_style_yellow);

    /*--- creating HV ERRORS PANEL ---*/

    static lv_coord_t hvErrors_panel_cols[] = {TAB_DEBUG_CELL_WIDTH, TAB_DEBUG_CELL_WIDTH, TAB_DEBUG_CELL_WIDTH, TAB_DEBUG_CELL_WIDTH, LV_GRID_TEMPLATE_LAST};
    static lv_coord_t hvErrors_panel_rows[] = {
        TAB_DEBUG_DATA_TITLE_HEIGHT, TAB_DEBUG_CELL_HEIGHT, TAB_DEBUG_CELL_HEIGHT, TAB_DEBUG_CELL_HEIGHT, TAB_DEBUG_CELL_HEIGHT, LV_GRID_TEMPLATE_LAST};

    hv_errors_panel = lv_obj_create(main_panel);
    lv_obj_remove_style_all(hv_errors_panel);
    lv_obj_set_layout(hv_errors_panel, LV_LAYOUT_GRID);
    lv_obj_clear_flag(hv_errors_panel, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(hv_errors_panel, TAB_DEBUG_CELL_WIDTH * 4 + TAB_DEBUG_COLUMN_PADDING * 3, TAB_DEBUG_DATA_PANEL_HEIGHT);

    lv_obj_add_style(hv_errors_panel, &grid_style, 0);
    lv_obj_align(hv_errors_panel, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_base_dir(hv_errors_panel, LV_BASE_DIR_LTR, 0);
    lv_obj_set_grid_dsc_array(hv_errors_panel, hvErrors_panel_cols, hvErrors_panel_rows);

    lv_obj_set_style_pad_row(hv_errors_panel, TAB_DEBUG_ROW_PADDING, 0);
    lv_obj_set_style_pad_column(hv_errors_panel, TAB_DEBUG_COLUMN_PADDING, 0);

    lv_obj_set_style_opa(hv_errors_panel, LV_OPA_0, LV_PART_MAIN);

    lv_obj_set_grid_cell(hv_errors_panel, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 1, 1);

    lv_obj_t *hv_errors_title_container = lv_obj_create(hv_errors_panel);
    lv_obj_remove_style_all(hv_errors_title_container);
    lv_obj_set_size(hv_errors_title_container, 250, 50);
    lv_obj_add_style(hv_errors_title_container, &box_panels_style, LV_PART_MAIN);
    lv_obj_set_grid_cell(hv_errors_title_container, LV_GRID_ALIGN_CENTER, 0, 4, LV_GRID_ALIGN_END, 0, 1);

    lv_obj_t *hv_errors_title = lv_label_create(hv_errors_title_container);
    lv_obj_remove_style_all(hv_errors_title);
    lv_obj_add_style(hv_errors_title, &label_style, LV_PART_MAIN);
    lv_obj_align(hv_errors_title, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_text_font(hv_errors_title, &lv_font_inter_bold_30, LV_PART_MAIN);
    lv_obj_set_style_text_align(hv_errors_title, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_align(hv_errors_title, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text(hv_errors_title, "HV ERRORS");

    hv_errors[0]  = cell_create(hv_errors_panel, "CELL LOW VOLTAGE", 0, 1, &box_debug_style_yellow);
    hv_errors[1]  = cell_create(hv_errors_panel, "CELL UNDER VOLT", 0, 2, &box_debug_style_yellow);
    hv_errors[2]  = cell_create(hv_errors_panel, "CELL OVER VOLT", 0, 3, &box_debug_style_yellow);
    hv_errors[3]  = cell_create(hv_errors_panel, "CELL HIGH TEMP", 0, 4, &box_debug_style_yellow);
    hv_errors[4]  = cell_create(hv_errors_panel, "CELL OVER TEMP", 3, 1, &box_debug_style_yellow);
    hv_errors[5]  = cell_create(hv_errors_panel, "OVER CURRENT", 1, 1, &box_debug_style_yellow);
    hv_errors[6]  = cell_create(hv_errors_panel, "CAN", 1, 2, &box_debug_style_yellow);
    hv_errors[7]  = cell_create(hv_errors_panel, "INT VOLTAGE MISMATCH", 1, 3, &box_debug_style_yellow);
    hv_errors[8]  = cell_create(hv_errors_panel, "CELLBOARD COMM", 1, 4, &box_debug_style_yellow);
    hv_errors[9]  = cell_create(hv_errors_panel, "CELLBOARD INTERNAL", 3, 2, &box_debug_style_yellow);
    hv_errors[10] = cell_create(hv_errors_panel, "CONNECTOR DISCONN.", 3, 3, &box_debug_style_yellow);
    hv_errors[11] = cell_create(hv_errors_panel, "FANS DISCONN.", 3, 4, &box_debug_style_yellow);
    hv_errors[12] = cell_create(hv_errors_panel, "FEEDBACK", 2, 1, &box_debug_style_yellow);
    hv_errors[13] = cell_create(hv_errors_panel, "FEEDBACK CIRCUITRY", 2, 2, &box_debug_style_yellow);
    hv_errors[14] = cell_create(hv_errors_panel, "EEPROM COMM", 2, 3, &box_debug_style_yellow);
    hv_errors[15] = cell_create(hv_errors_panel, "EEPROM WRITE", 2, 4, &box_debug_style_yellow);

    /*--- creating DAS ERRORS PANEL ---*/

    static lv_coord_t dasErrors_panel_cols[] = {TAB_DEBUG_CELL_WIDTH, TAB_DEBUG_CELL_WIDTH, LV_GRID_TEMPLATE_LAST};
    static lv_coord_t dasErrors_panel_rows[] = {
        TAB_DEBUG_DATA_TITLE_HEIGHT,
        TAB_DEBUG_CELL_HEIGHT,
        TAB_DEBUG_CELL_HEIGHT,
        TAB_DEBUG_CELL_HEIGHT,
        TAB_DEBUG_CELL_HEIGHT,
        TAB_DEBUG_CELL_HEIGHT,
        LV_GRID_TEMPLATE_LAST};

    das_errors_panel = lv_obj_create(main_panel);
    lv_obj_remove_style_all(das_errors_panel);
    lv_obj_set_layout(das_errors_panel, LV_LAYOUT_GRID);
    lv_obj_clear_flag(das_errors_panel, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(das_errors_panel, TAB_DEBUG_CELL_WIDTH * 2 + TAB_DEBUG_COLUMN_PADDING * 1, TAB_DEBUG_DATA_PANEL_HEIGHT);

    lv_obj_add_style(das_errors_panel, &grid_style, 0);
    lv_obj_align(das_errors_panel, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_base_dir(das_errors_panel, LV_BASE_DIR_LTR, 0);
    lv_obj_set_grid_dsc_array(das_errors_panel, dasErrors_panel_cols, dasErrors_panel_rows);

    lv_obj_set_style_pad_row(das_errors_panel, TAB_DEBUG_ROW_PADDING, 0);
    lv_obj_set_style_pad_column(das_errors_panel, TAB_DEBUG_COLUMN_PADDING, 0);

    lv_obj_set_style_opa(das_errors_panel, LV_OPA_0, LV_PART_MAIN);
    lv_obj_set_grid_cell(das_errors_panel, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 1, 1);

    lv_obj_t *das_errors_title_container = lv_obj_create(das_errors_panel);
    lv_obj_remove_style_all(das_errors_title_container);
    lv_obj_set_size(das_errors_title_container, 250, 50);
    lv_obj_add_style(das_errors_title_container, &box_panels_style, LV_PART_MAIN);
    lv_obj_set_grid_cell(das_errors_title_container, LV_GRID_ALIGN_CENTER, 0, 2, LV_GRID_ALIGN_END, 0, 1);

    lv_obj_t *das_errors_title = lv_label_create(das_errors_title_container);
    lv_obj_remove_style_all(das_errors_title);
    lv_obj_add_style(das_errors_title, &label_style, LV_PART_MAIN);
    lv_obj_align(das_errors_title, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_text_font(das_errors_title, &lv_font_inter_bold_30, LV_PART_MAIN);
    lv_obj_set_style_text_align(das_errors_title, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_align(das_errors_title, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text(das_errors_title, "DAS ERRORS");

    das_errors[0] = cell_create(das_errors_panel, "PEDAL ADC", 0, 1, &box_debug_style_yellow);
    das_errors[1] = cell_create(das_errors_panel, "PEDAL IMPLAUS", 0, 2, &box_debug_style_yellow);
    das_errors[2] = cell_create(das_errors_panel, "IMU TOUT", 0, 3, &box_debug_style_yellow);
    das_errors[3] = cell_create(das_errors_panel, "IRTS TOUT", 0, 4, &box_debug_style_yellow);
    das_errors[4] = cell_create(das_errors_panel, "TS TOUT", 0, 5, &box_debug_style_yellow);
    das_errors[5] = cell_create(das_errors_panel, "INVL TOUT", 1, 1, &box_debug_style_yellow);
    das_errors[6] = cell_create(das_errors_panel, "INVR TOUT", 1, 2, &box_debug_style_yellow);
    das_errors[7] = cell_create(das_errors_panel, "STEER TOUT", 1, 3, &box_debug_style_yellow);
    das_errors[8] = cell_create(das_errors_panel, "FSM", 1, 4, &box_debug_style_yellow);

    /*--- creating LV ERRORS PANEL ---*/

    static lv_coord_t lvErrors_panel_cols[] = {TAB_DEBUG_CELL_WIDTH, TAB_DEBUG_CELL_WIDTH, TAB_DEBUG_CELL_WIDTH, TAB_DEBUG_CELL_WIDTH, LV_GRID_TEMPLATE_LAST};
    static lv_coord_t lvErrors_panel_rows[] = {
        TAB_DEBUG_DATA_TITLE_HEIGHT,
        TAB_DEBUG_CELL_HEIGHT,
        TAB_DEBUG_CELL_HEIGHT,
        TAB_DEBUG_CELL_HEIGHT,
        TAB_DEBUG_CELL_HEIGHT,
        TAB_DEBUG_CELL_HEIGHT,
        LV_GRID_TEMPLATE_LAST};

    lv_errors_panel = lv_obj_create(main_panel);
    lv_obj_remove_style_all(lv_errors_panel);
    lv_obj_set_layout(lv_errors_panel, LV_LAYOUT_GRID);
    lv_obj_clear_flag(lv_errors_panel, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(lv_errors_panel, TAB_DEBUG_CELL_WIDTH * 4 + TAB_DEBUG_COLUMN_PADDING * 3, TAB_DEBUG_DATA_PANEL_HEIGHT);

    lv_obj_add_style(lv_errors_panel, &grid_style, 0);
    lv_obj_align(lv_errors_panel, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_base_dir(lv_errors_panel, LV_BASE_DIR_LTR, 0);
    lv_obj_set_grid_dsc_array(lv_errors_panel, lvErrors_panel_cols, lvErrors_panel_rows);

    lv_obj_set_style_pad_row(lv_errors_panel, TAB_DEBUG_ROW_PADDING, 0);
    lv_obj_set_style_pad_column(lv_errors_panel, TAB_DEBUG_COLUMN_PADDING, 0);
    lv_obj_set_style_opa(lv_errors_panel, LV_OPA_0, LV_PART_MAIN);

    lv_obj_set_grid_cell(lv_errors_panel, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 1, 1);

    lv_obj_t *lv_errors_title_container = lv_obj_create(lv_errors_panel);
    lv_obj_remove_style_all(lv_errors_title_container);
    lv_obj_set_size(lv_errors_title_container, 250, 50);
    lv_obj_add_style(lv_errors_title_container, &box_panels_style, LV_PART_MAIN);
    lv_obj_set_grid_cell(lv_errors_title_container, LV_GRID_ALIGN_CENTER, 0, 4, LV_GRID_ALIGN_END, 0, 1);

    lv_obj_t *lv_errors_title = lv_label_create(lv_errors_title_container);
    lv_obj_remove_style_all(lv_errors_title);
    lv_obj_add_style(lv_errors_title, &label_style, LV_PART_MAIN);
    lv_obj_align(lv_errors_title, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_text_font(lv_errors_title, &lv_font_inter_bold_30, LV_PART_MAIN);
    lv_obj_set_style_text_align(lv_errors_title, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_align(lv_errors_title, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text(lv_errors_title, "LV ERRORS");

    lv_errors[0] = cell_create(lv_errors_panel, "CELL UNDERVOLT", 0, 1, &box_debug_style_yellow);

    lv_errors[1] = cell_create(lv_errors_panel, "CELL OVERVOLT", 0, 2, &box_debug_style_yellow);

    lv_errors[2] = cell_create(lv_errors_panel, "OPEN WIRE", 0, 3, &box_debug_style_yellow);

    lv_errors[3] = cell_create(lv_errors_panel, "CAN", 0, 4, &box_debug_style_yellow);

    lv_errors[4] = cell_create(lv_errors_panel, "SPI", 2, 1, &box_debug_style_yellow);

    lv_errors[5] = cell_create(lv_errors_panel, "OVER CURRENT", 2, 3, &box_debug_style_yellow);

    lv_errors[6] = cell_create(lv_errors_panel, "CELL UNDER TEMP", 1, 1, &box_debug_style_yellow);

    lv_errors[7] = cell_create(lv_errors_panel, "CELL OVER TEMP", 1, 2, &box_debug_style_yellow);

    lv_errors[8] = cell_create(lv_errors_panel, "RELAY", 1, 3, &box_debug_style_yellow);

    lv_errors[9] = cell_create(lv_errors_panel, "BMS MONITOR", 2, 4, &box_debug_style_yellow);

    lv_errors[10] = cell_create(lv_errors_panel, "VOLTS NOT READY", 1, 4, &box_debug_style_yellow);

    lv_errors[11] = cell_create(lv_errors_panel, "MCP23017", 3, 1, &box_debug_style_yellow);

    lv_errors[12] = cell_create(lv_errors_panel, "RADIATOR", 3, 2, &box_debug_style_yellow);

    lv_errors[13] = cell_create(lv_errors_panel, "FAN", 3, 3, &box_debug_style_yellow);

    lv_errors[14] = cell_create(lv_errors_panel, "PUMP", 3, 4, &box_debug_style_yellow);

    lv_errors[15] = cell_create(lv_errors_panel, "ADC INIT", 2, 2, &box_debug_style_yellow);

    lv_errors[16] = cell_create(lv_errors_panel, "MUX", 0, 5, &box_debug_style_yellow);
}

ErrorTabID errors_focus = TAB_HV_ERRORS;

void change_errors_view(bool dir_left) {
    if (current_racing_tab == TAB_DEBUG) {
        if (dir_left) {
            if (errors_focus == 0)
                errors_focus = TAB_LV_ERRORS;
            else
                errors_focus--;

        } else if (!dir_left) {
            errors_focus++;
            errors_focus %= NUM_TYPE_ERRORS;
        }

        switch (errors_focus) {
            case TAB_HV_FEEDBACKS:
                lv_obj_set_style_opa(hv_feedbacks_panel, LV_OPA_COVER, LV_PART_MAIN);
                lv_obj_set_style_opa(hv_errors_panel, LV_OPA_0, LV_PART_MAIN);
                lv_obj_set_style_opa(lv_errors_panel, LV_OPA_0, LV_PART_MAIN);
                break;

            case TAB_HV_ERRORS:
                lv_obj_set_style_opa(hv_errors_panel, LV_OPA_COVER, LV_PART_MAIN);
                lv_obj_set_style_opa(hv_feedbacks_panel, LV_OPA_0, LV_PART_MAIN);
                lv_obj_set_style_opa(das_errors_panel, LV_OPA_0, LV_PART_MAIN);
                break;

            case TAB_DAS_ERRORS:
                lv_obj_set_style_opa(das_errors_panel, LV_OPA_COVER, LV_PART_MAIN);
                lv_obj_set_style_opa(hv_errors_panel, LV_OPA_0, LV_PART_MAIN);
                lv_obj_set_style_opa(lv_errors_panel, LV_OPA_0, LV_PART_MAIN);
                break;

            case TAB_LV_ERRORS:
                lv_obj_set_style_opa(lv_errors_panel, LV_OPA_COVER, LV_PART_MAIN);
                lv_obj_set_style_opa(das_errors_panel, LV_OPA_0, LV_PART_MAIN);
                lv_obj_set_style_opa(hv_feedbacks_panel, LV_OPA_0, LV_PART_MAIN);
                break;

            default:
                break;
        }
    }
}

#endif

void set_label_color_hv_feedbacks(int label, int i) {
#if STEER_TAB_DEBUG_ENABLED == 1
    // PRIMARY_HV_FEEDBACK_STATUS_FEEDBACK_BMS_COCKPIT_FEEDBACK_STATE_LOW_CHOICE
    // it's better to use the enum canlib, but we use 0 or 1 or 2 for demplicity
    // sake. If canlib specs will change, then we need to change also this code
    if (label == 0 || label == 2) {
        lv_obj_set_style_border_color(hv_errors[i], lv_color_hex(COLOR_GREEN_STATUS_HEX), LV_PART_MAIN);
        lv_obj_set_style_bg_color(hv_errors[i], lv_color_hex(COLOR_GREEN_STATUS_HEX), LV_PART_MAIN);
    } else {
        if (label == 1) {
            lv_obj_set_style_border_color(hv_errors[i], lv_color_hex(COLOR_RED_STATUS_HEX), LV_PART_MAIN);
            lv_obj_set_style_bg_color(hv_errors[i], lv_color_hex(COLOR_RED_STATUS_HEX), LV_PART_MAIN);
        } else {
            lv_obj_set_style_border_color(hv_errors[i], lv_color_hex(COLOR_YELLOW_STATUS_HEX), LV_PART_MAIN);
            lv_obj_set_style_bg_color(hv_errors[i], lv_color_hex(COLOR_YELLOW_STATUS_HEX), LV_PART_MAIN);
        }
    }
#endif
}

void debug_hv_feedbacks_status_update() {
#if 0
  set_label_color_hv_feedbacks(
      hv_feedback_status_last_state.feedback_implausibility_detected, 0);
  set_label_color_hv_feedbacks(
      hv_feedback_status_last_state.feedback_imd_cockpit, 1);
  set_label_color_hv_feedbacks(
      hv_feedback_status_last_state.feedback_tsal_green_fault_latched, 2);
  set_label_color_hv_feedbacks(
      hv_feedback_status_last_state.feedback_bms_cockpit, 3);
  set_label_color_hv_feedbacks(
      hv_feedback_status_last_state.feedback_ext_latched, 4);
  set_label_color_hv_feedbacks(
      hv_feedback_status_last_state.feedback_tsal_green, 5);
  set_label_color_hv_feedbacks(
      hv_feedback_status_last_state.feedback_ts_over_60v_status, 6);
  set_label_color_hv_feedbacks(
      hv_feedback_status_last_state.feedback_airn_status, 7);
  set_label_color_hv_feedbacks(
      hv_feedback_status_last_state.feedback_airp_status, 8);
  set_label_color_hv_feedbacks(hv_feedback_status_last_state.feedback_airp_gate,
                               9);
  set_label_color_hv_feedbacks(hv_feedback_status_last_state.feedback_airn_gate,
                               10);
  set_label_color_hv_feedbacks(
      hv_feedback_status_last_state.feedback_precharge_status, 11);
  set_label_color_hv_feedbacks(
      hv_feedback_status_last_state.feedback_tsp_over_60v_status, 12);
  set_label_color_hv_feedbacks(hv_feedback_status_last_state.feedback_imd_fault,
                               13);
  set_label_color_hv_feedbacks(hv_feedback_status_last_state.feedback_check_mux,
                               14);
  set_label_color_hv_feedbacks(hv_feedback_status_last_state.feedback_sd_end,
                               15);
  set_label_color_hv_feedbacks(hv_feedback_status_last_state.feedback_sd_out,
                               16);
  set_label_color_hv_feedbacks(hv_feedback_status_last_state.feedback_sd_in,
                               17);
  set_label_color_hv_feedbacks(hv_feedback_status_last_state.feedback_sd_bms,
                               18);
  set_label_color_hv_feedbacks(hv_feedback_status_last_state.feedback_sd_imd,
                               19);
#endif
}

void set_label_color_hv_errors(int label, int i) {
#if STEER_TAB_DEBUG_ENABLED == 1
    if (label) {
        lv_obj_set_style_border_color(hv_errors[i], lv_color_hex(COLOR_RED_STATUS_HEX), LV_PART_MAIN);
        lv_obj_set_style_bg_color(hv_errors[i], lv_color_hex(COLOR_RED_STATUS_HEX), LV_PART_MAIN);
    } else {
        lv_obj_set_style_border_color(hv_errors[i], lv_color_hex(COLOR_GREEN_STATUS_HEX), LV_PART_MAIN);
        lv_obj_set_style_bg_color(hv_errors[i], lv_color_hex(COLOR_GREEN_STATUS_HEX), LV_PART_MAIN);
    }
#endif
}

void set_label_color_das_errors(bool label, int i) {
#if STEER_TAB_DEBUG_ENABLED == 1
    if (label) {
        lv_obj_set_style_border_color(das_errors[i], lv_color_hex(COLOR_RED_STATUS_HEX), LV_PART_MAIN);
        lv_obj_set_style_bg_color(das_errors[i], lv_color_hex(COLOR_RED_STATUS_HEX), LV_PART_MAIN);
    } else {
        lv_obj_set_style_border_color(das_errors[i], lv_color_hex(COLOR_GREEN_STATUS_HEX), LV_PART_MAIN);
        lv_obj_set_style_bg_color(das_errors[i], lv_color_hex(COLOR_GREEN_STATUS_HEX), LV_PART_MAIN);
    }
#endif
}

void das_errors_update() {
#if STEER_TAB_DEBUG_ENABLED
    set_label_color_das_errors(das_errors_last_state.das_error_pedal_adc, 0);
    set_label_color_das_errors(das_errors_last_state.das_error_pedal_implausibility, 1);
    set_label_color_das_errors(das_errors_last_state.das_error_imu_tout, 2);
    set_label_color_das_errors(das_errors_last_state.das_error_irts_tout, 3);
    set_label_color_das_errors(das_errors_last_state.das_error_ts_tout, 4);
    set_label_color_das_errors(das_errors_last_state.das_error_invl_tout, 5);
    set_label_color_das_errors(das_errors_last_state.das_error_invr_tout, 6);
    set_label_color_das_errors(das_errors_last_state.das_error_steer_tout, 7);
    set_label_color_das_errors(das_errors_last_state.das_error_fsm, 8);
#endif
}

void set_label_color_lv_errors(bool label, int i) {
#if STEER_TAB_DEBUG_ENABLED
    if (label) {
        lv_obj_set_style_border_color(lv_errors[i], lv_color_hex(COLOR_RED_STATUS_HEX), LV_PART_MAIN);
        lv_obj_set_style_bg_color(lv_errors[i], lv_color_hex(COLOR_RED_STATUS_HEX), LV_PART_MAIN);
    } else {
        lv_obj_set_style_border_color(lv_errors[i], lv_color_hex(COLOR_GREEN_STATUS_HEX), LV_PART_MAIN);
        lv_obj_set_style_bg_color(lv_errors[i], lv_color_hex(COLOR_GREEN_STATUS_HEX), LV_PART_MAIN);
    }
#endif
}

void lv_errors_update() {
#if STEER_TAB_DEBUG_ENABLED
    set_label_color_lv_errors(lv_errors_last_state.errors_cell_undervoltage, 0);
    set_label_color_lv_errors(lv_errors_last_state.errors_cell_overvoltage, 1);
    set_label_color_lv_errors(lv_errors_last_state.errors_battery_open_wire, 2);
    set_label_color_lv_errors(lv_errors_last_state.errors_can, 3);
    set_label_color_lv_errors(lv_errors_last_state.errors_spi, 4);
    set_label_color_lv_errors(lv_errors_last_state.errors_over_current, 5);
    set_label_color_lv_errors(lv_errors_last_state.errors_cell_under_temperature, 6);
    set_label_color_lv_errors(lv_errors_last_state.errors_cell_over_temperature, 7);
    set_label_color_lv_errors(lv_errors_last_state.errors_relay, 8);
    set_label_color_lv_errors(lv_errors_last_state.errors_bms_monitor, 9);
    set_label_color_lv_errors(lv_errors_last_state.errors_voltages_not_ready, 10);
    set_label_color_lv_errors(lv_errors_last_state.errors_mcp23017, 11);
    set_label_color_lv_errors(lv_errors_last_state.errors_radiator, 12);
    set_label_color_lv_errors(lv_errors_last_state.errors_fan, 13);
    set_label_color_lv_errors(lv_errors_last_state.errors_pump, 14);
    set_label_color_lv_errors(lv_errors_last_state.errors_adc_init, 15);
    set_label_color_lv_errors(lv_errors_last_state.errors_mux, 16);
#endif
}

#endif  // #if 0