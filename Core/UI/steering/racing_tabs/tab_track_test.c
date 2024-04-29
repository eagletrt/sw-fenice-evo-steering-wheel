#include "tab_track_test.h"

#include <stdio.h>

#define CELL_HEIGHT 200
#define CELL_WIDTH  700

lv_obj_t *tab_track_test_lb_target;
lv_obj_t *tab_track_test_lb_steering_angle;
lv_obj_t *tab_track_test_lb_speed;
lv_obj_t *tab_track_test_steering_angle_bar;

float dmt_steering_angle_target = 0.0f;
#define STEERING_ANGLE_TARGET_DELTA  2.0f
#define STEERING_ANGLE_TARGET_LIMITS 20.0f

void set_tab_track_test_lb_speed(const char *s) {
    CHECK_CURRENT_TAB(engineer, TAB_TRACK_TEST);
    lv_label_set_text(tab_track_test_lb_speed, s);
}

void set_tab_track_test_steering_angle_bar(float v) {
    char b[100];
    snprintf(b, 100, "%.1f", v);
    update_sensors_extra_value(b, 2);
    lv_label_set_text(tab_track_test_lb_steering_angle, b);
    v = fmax(v, dmt_steering_angle_target - STEERING_ANGLE_TARGET_LIMITS);
    v = fmin(v, dmt_steering_angle_target + STEERING_ANGLE_TARGET_LIMITS);
    if ((dmt_steering_angle_target + STEERING_ANGLE_TARGET_DELTA) > v && (dmt_steering_angle_target - STEERING_ANGLE_TARGET_DELTA) < v) {
        lv_obj_set_style_bg_color(tab_track_test_steering_angle_bar, lv_color_hex(COLOR_GREEN_STATUS_HEX), LV_PART_INDICATOR);
    } else {
        lv_obj_set_style_bg_color(tab_track_test_steering_angle_bar, lv_color_hex(COLOR_RED_STATUS_HEX), LV_PART_INDICATOR);
    }
    lv_bar_set_value(tab_track_test_steering_angle_bar, v, LV_ANIM_OFF);
}

void set_tab_track_test_dmt_steering_angle_target(float v) {
    CHECK_CURRENT_TAB(engineer, TAB_TRACK_TEST);
    dmt_steering_angle_target = v;
    lv_bar_set_range(tab_track_test_steering_angle_bar, v - STEERING_ANGLE_TARGET_LIMITS - 10.0f, v + STEERING_ANGLE_TARGET_LIMITS + 10.0f);
    char b[100];
    snprintf(b, 100, "%.1f", dmt_steering_angle_target);
    //update_sensors_extra_value(b, 1);
    lv_label_set_text(tab_track_test_lb_target, b);
    snprintf(b, 100, "new target at: %.2f", v);
    display_notification(b, 1000);
}

void tab_track_test_lb_steering_angle_invalidate() {
    CHECK_CURRENT_TAB(engineer, TAB_TRACK_TEST);
    lv_label_set_text(tab_track_test_lb_steering_angle, "NA");
}

void tab_track_test_lb_speed_invalidate() {
    CHECK_CURRENT_TAB(engineer, TAB_TRACK_TEST);
    lv_label_set_text(tab_track_test_lb_speed, "NA");
}

void tab_track_test_create(lv_obj_t *parent) {
    /*---creating MAIN GRID ---*/

    static lv_coord_t main_panel_cols[] = {SCREEN_WIDTH, LV_GRID_TEMPLATE_LAST};
    static lv_coord_t main_panel_rows[] = {TOP_BAR_HEIGHT, TAB_TRACK_DATA_PANEL_HEIGHT, LV_GRID_TEMPLATE_LAST};

    lv_obj_t *main_panel = lv_obj_create(parent);
    lv_obj_set_layout(main_panel, LV_LAYOUT_GRID);
    lv_obj_clear_flag(main_panel, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(main_panel, SCREEN_WIDTH, SCREEN_HEIGHT);

    lv_obj_add_style(main_panel, &grid_style, 0);
    lv_obj_center(main_panel);
    lv_obj_set_style_base_dir(main_panel, LV_BASE_DIR_LTR, 0);
    lv_obj_set_grid_dsc_array(main_panel, main_panel_cols, main_panel_rows);

    /*--- inserting TOP NOTCH ---*/

    lv_obj_t *notch = create_notch(main_panel, TAB_TRACK_TEST);
    lv_obj_align(lv_obj_get_child(notch, 0), LV_ALIGN_TOP_MID, 0, 10);
    lv_obj_set_grid_cell(notch, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_START, 0, 1);

    /*--- creating DATA PANEL ---*/

    static lv_coord_t dpanel_cols[] = {CELL_WIDTH, CELL_WIDTH, LV_GRID_TEMPLATE_LAST};
    static lv_coord_t dpanel_rows[] = {CELL_HEIGHT, CELL_HEIGHT, LV_GRID_TEMPLATE_LAST};

    lv_obj_t *data_panel = lv_obj_create(main_panel);
    lv_obj_set_layout(data_panel, LV_LAYOUT_GRID);
    lv_obj_clear_flag(data_panel, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(
        data_panel,
        SCREEN_WIDTH - 80,
        TAB_TRACK_DATA_PANEL_HEIGHT);  // non andrebbe tolto il 20 per farlo
                                       // simmetrico(forse qualche errore)

    lv_obj_add_style(data_panel, &grid_style, 0);
    lv_obj_center(data_panel);
    lv_obj_set_style_base_dir(data_panel, LV_BASE_DIR_LTR, 0);
    lv_obj_set_grid_dsc_array(data_panel, dpanel_cols, dpanel_rows);

    lv_obj_set_style_pad_row(data_panel, ROW_PADDING, 0);
    lv_obj_set_style_pad_column(data_panel, COLUMN_PADDING, 0);

    lv_obj_set_grid_cell(data_panel, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 1, 1);

    static lv_coord_t lxd_panel_cols[] = {CELL_WIDTH, LV_GRID_TEMPLATE_LAST};
    static lv_coord_t lxd_panel_rows[] = {CELL_HEIGHT, LV_GRID_TEMPLATE_LAST};

    lv_obj_t *up_left_data_panel = lv_obj_create(data_panel);
    lv_obj_set_layout(up_left_data_panel, LV_LAYOUT_GRID);
    lv_obj_remove_style_all(up_left_data_panel);
    lv_obj_clear_flag(up_left_data_panel, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(up_left_data_panel, CELL_WIDTH, CELL_HEIGHT);

    lv_obj_add_style(up_left_data_panel, &box_panels_style, 0);
    lv_obj_center(up_left_data_panel);
    lv_obj_set_style_base_dir(up_left_data_panel, LV_BASE_DIR_LTR, 0);
    lv_obj_set_grid_dsc_array(up_left_data_panel, lxd_panel_cols, lxd_panel_rows);

    lv_obj_set_grid_cell(up_left_data_panel, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 0, 1);

    static lv_style_t gs;
    lv_style_init(&gs);
    lv_style_set_bg_opa(&gs, LV_OPA_COVER);
    lv_style_set_bg_color(&gs, lv_color_hex(COLOR_GREEN_STATUS_HEX));

    static lv_style_t ggs;
    lv_style_init(&ggs);
    lv_style_set_bg_opa(&ggs, LV_OPA_COVER);
    lv_style_set_bg_color(&ggs, lv_color_hex(COLOR_SECONDARY_HEX));

    tab_track_test_steering_angle_bar = lv_bar_create(up_left_data_panel);
    lv_obj_remove_style_all(tab_track_test_steering_angle_bar);
    lv_obj_add_style(tab_track_test_steering_angle_bar, &gs, LV_PART_INDICATOR);
    lv_obj_add_style(tab_track_test_steering_angle_bar, &ggs, LV_PART_MAIN);

    lv_obj_set_size(tab_track_test_steering_angle_bar, 650, 80);
    lv_obj_align(tab_track_test_steering_angle_bar, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(tab_track_test_steering_angle_bar, lv_color_hex(COLOR_GREEN_STATUS_HEX), LV_PART_INDICATOR);
    lv_bar_set_range(tab_track_test_steering_angle_bar, -(STEERING_ANGLE_TARGET_LIMITS + 10.0f), (STEERING_ANGLE_TARGET_LIMITS + 10.0f));
    lv_bar_set_value(tab_track_test_steering_angle_bar, 0.0f, LV_ANIM_OFF);
    lv_obj_set_grid_cell(tab_track_test_steering_angle_bar, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 0, 1);

#define tab_terminal_col_ration_base (SCREEN_WIDTH - 100) / 3
    static lv_coord_t tab_terminal_cols_ratios[] = {
        tab_terminal_col_ration_base, tab_terminal_col_ration_base, tab_terminal_col_ration_base, LV_GRID_TEMPLATE_LAST};
    static lv_coord_t tab_terminal_rows_ratios[] = {(TAB_TRACK_DATA_PANEL_HEIGHT / 2), LV_GRID_TEMPLATE_LAST};

    lv_obj_t *label_panel = lv_obj_create(data_panel);
    lv_obj_set_layout(label_panel, LV_LAYOUT_GRID);
    lv_obj_remove_style_all(label_panel);
    lv_obj_clear_flag(label_panel, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(label_panel, CELL_WIDTH, CELL_HEIGHT);

    lv_obj_add_style(label_panel, &box_panels_style, 0);
    lv_obj_center(label_panel);
    lv_obj_set_style_base_dir(label_panel, LV_BASE_DIR_LTR, 0);
    lv_obj_set_grid_dsc_array(label_panel, lxd_panel_cols, lxd_panel_rows);

    lv_obj_set_grid_dsc_array(label_panel, tab_terminal_cols_ratios, tab_terminal_rows_ratios);
    lv_obj_set_grid_cell(label_panel, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 1, 1);

    lv_obj_t *x_axis_h = lv_vertical_pair_label(label_panel, &tab_track_test_lb_target, "0", &lv_font_inter_bold_30, "TARGET\ndegrees", &lv_font_inter_bold_22);
    lv_obj_set_grid_cell(x_axis_h, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 0, 1);

    lv_obj_t *x_axis_g = lv_vertical_pair_label(label_panel, &tab_track_test_lb_speed, "0", &lv_font_inter_bold_30, "SPEED\nKM/H", &lv_font_inter_bold_22);
    lv_obj_set_grid_cell(x_axis_g, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, 0, 1);

    lv_obj_t *x_axis_j =
        lv_vertical_pair_label(label_panel, &tab_track_test_lb_steering_angle, "0", &lv_font_inter_bold_30, "STEERING ANGLE\ndegrees", &lv_font_inter_bold_22);
    lv_obj_set_grid_cell(x_axis_j, LV_GRID_ALIGN_CENTER, 2, 1, LV_GRID_ALIGN_CENTER, 0, 1);
}

void tab_track_test_resync(void) {
}