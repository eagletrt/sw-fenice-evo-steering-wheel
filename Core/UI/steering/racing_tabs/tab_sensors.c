#include "tab_sensors.h"

#include <stdio.h>

#define CELL_HEIGHT 200
#define CELL_WIDTH  180

lv_obj_t *throttle_bar1;
// lv_obj_t *throttle_bar2;
lv_obj_t *break_rear_bar;
lv_obj_t *brake_front_bar;

lv_obj_t *tab_sensors_labels[tab_sensors_labels_n] = {
    [tab_sensors_lb_fl_temp]             = NULL,
    [tab_sensors_lb_fr_temp]             = NULL,
    [tab_sensors_lb_rl_temp]             = NULL,
    [tab_sensors_lb_rr_temp]             = NULL,
    [tab_sensors_lb_fl_press]            = NULL,
    [tab_sensors_lb_fr_press]            = NULL,
    [tab_sensors_lb_rl_press]            = NULL,
    [tab_sensors_lb_rr_press]            = NULL,
    [tab_sensors_lb_right_inverter_temp] = NULL,
    [tab_sensors_lb_right_motor_temp]    = NULL,
    [tab_sensors_lb_left_inverter_temp]  = NULL,
    [tab_sensors_lb_left_motor_temp]     = NULL,
    [tab_sensors_lb_tlm_ntw_interface_0] = NULL,
    [tab_sensors_lb_tlm_ntw_interface_1] = NULL,
    [tab_sensors_lb_tlm_ntw_interface_2] = NULL,
    [tab_sensors_lb_tlm_ntw_interface_3] = NULL,
};

void set_tab_sensors_label_text(const char *s, tab_sensors_labels_enum idx) {
    CHECK_CURRENT_TAB(engineer_mode, racing, STEERING_WHEEL_TAB_SENSORS);
    lv_label_set_text(tab_sensors_labels[idx], s);
}

void set_tab_sensors_value_brake_f(float value_in_bar) {
    CHECK_CURRENT_TAB(engineer_mode, racing, STEERING_WHEEL_TAB_SENSORS);

    lv_bar_set_value(brake_front_bar, (int32_t)(value_in_bar), LV_ANIM_OFF);
}

void set_tab_sensors_value_brake_r(float value_in_bar) {
    CHECK_CURRENT_TAB(engineer_mode, racing, STEERING_WHEEL_TAB_SENSORS);

    lv_bar_set_value(break_rear_bar, (int32_t)(value_in_bar), LV_ANIM_OFF);
}

void set_tab_sensors_value_apps(int32_t value_0_to_100) {
    CHECK_CURRENT_TAB(engineer_mode, racing, STEERING_WHEEL_TAB_SENSORS);

    lv_bar_set_value(throttle_bar1, (int32_t)value_0_to_100, LV_ANIM_OFF);
}

lv_style_t throttle_bar_style;
lv_style_t brake_bar_style;
lv_style_t throttle_bar_background_style;
lv_style_t brake_bar_background_style;

void init_sensors_styles(void) {
    lv_style_init(&throttle_bar_style);
    lv_style_set_bg_opa(&throttle_bar_style, LV_OPA_COVER);
    lv_style_set_bg_color(&throttle_bar_style, lv_color_hex(COLOR_GREEN_STATUS_HEX));
    lv_style_set_width(&throttle_bar_style, 50);
    lv_style_set_height(&throttle_bar_style, 405);

    lv_style_init(&brake_bar_style);
    lv_style_set_bg_opa(&brake_bar_style, LV_OPA_COVER);
    lv_style_set_bg_color(&brake_bar_style, lv_color_hex(COLOR_RED_STATUS_HEX));
    lv_style_set_width(&brake_bar_style, 25);
    lv_style_set_height(&brake_bar_style, 405);

    lv_style_init(&throttle_bar_background_style);
    lv_style_set_bg_opa(&throttle_bar_background_style, LV_OPA_COVER);
    lv_style_set_bg_color(&throttle_bar_background_style, lv_color_hex(COLOR_SECONDARY_HEX));
    lv_style_set_width(&throttle_bar_background_style, 25);
    lv_style_set_height(&throttle_bar_background_style, 405);

    lv_style_init(&brake_bar_background_style);
    lv_style_set_bg_opa(&brake_bar_background_style, LV_OPA_COVER);
    lv_style_set_bg_color(&brake_bar_background_style, lv_color_hex(COLOR_SECONDARY_HEX));
    lv_style_set_width(&brake_bar_background_style, 25);
    lv_style_set_height(&brake_bar_background_style, 405);
}

void tab_sensors_create(lv_obj_t *parent) {
    init_sensors_styles();

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

    lv_obj_t *notch = create_notch(main_panel, STEERING_WHEEL_TAB_SENSORS);
    lv_obj_align(lv_obj_get_child(notch, 0), LV_ALIGN_TOP_MID, 0, 10);
    lv_obj_set_grid_cell(notch, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_START, 0, 1);

    /*--- creating DATA PANEL ---*/

    static lv_coord_t dpanel_cols[] = {225, SCREEN_WIDTH - 265 - 120, 120, LV_GRID_TEMPLATE_LAST};
    static lv_coord_t dpanel_rows[] = {DATA_PANEL_HEIGHT, LV_GRID_TEMPLATE_LAST};

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

    lv_obj_set_style_pad_row(data_panel, ROW_PADDING, 0);
    lv_obj_set_style_pad_column(data_panel, COLUMN_PADDING, 0);
    lv_obj_set_style_pad_all(data_panel, 0, LV_PART_MAIN);

    lv_obj_set_grid_cell(data_panel, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 1, 1);

    /*--- DATA PANEL: cell 0,0 ---*/

    static lv_coord_t lxd_panel_cols[] = {225, LV_GRID_TEMPLATE_LAST};
    static lv_coord_t lxd_panel_rows[] = {DATA_PANEL_HEIGHT / 2, DATA_PANEL_HEIGHT / 2, LV_GRID_TEMPLATE_LAST};

    lv_obj_t *left_data_panel = lv_obj_create(data_panel);
    lv_obj_set_layout(left_data_panel, LV_LAYOUT_GRID);
    lv_obj_remove_style_all(left_data_panel);
    lv_obj_clear_flag(left_data_panel, LV_OBJ_FLAG_SCROLLABLE);
    // lv_obj_set_size(left_data_panel, CELL_WIDTH, CELL_HEIGHT);

    lv_obj_add_style(left_data_panel, &box_panels_style, 0);
    lv_obj_center(left_data_panel);
    lv_obj_set_style_base_dir(left_data_panel, LV_BASE_DIR_LTR, 0);
    lv_obj_set_grid_dsc_array(left_data_panel, lxd_panel_cols, lxd_panel_rows);

    lv_obj_set_grid_cell(left_data_panel, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 0, 1);

    lv_obj_t *frame_1 = lv_obj_create(left_data_panel);
    lv_obj_remove_style_all(frame_1);
    lv_obj_set_grid_cell(frame_1, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 0, 1);

    lv_obj_t *front_lx_temp =
        lv_horizontal_pair_label(frame_1, &tab_sensors_labels[tab_sensors_lb_fl_temp], NULL, "NA", &lv_font_inter_bold_30, "°C", &lv_font_inter_bold_22);
    lv_obj_align(front_lx_temp, LV_ALIGN_CENTER, -60, -70);

    lv_obj_t *front_rx_temp =
        lv_horizontal_pair_label(frame_1, &tab_sensors_labels[tab_sensors_lb_fr_temp], NULL, "NA", &lv_font_inter_bold_30, "°C", &lv_font_inter_bold_22);
    lv_obj_align(front_rx_temp, LV_ALIGN_CENTER, 60, -70);

    lv_obj_t *rear_lx_temp =
        lv_horizontal_pair_label(frame_1, &tab_sensors_labels[tab_sensors_lb_rl_temp], NULL, "NA", &lv_font_inter_bold_30, "°C", &lv_font_inter_bold_22);
    lv_obj_align(rear_lx_temp, LV_ALIGN_CENTER, -60, 70);

    lv_obj_t *rear_rx_temp =
        lv_horizontal_pair_label(frame_1, &tab_sensors_labels[tab_sensors_lb_rr_temp], NULL, "NA", &lv_font_inter_bold_30, "°C", &lv_font_inter_bold_22);
    lv_obj_align(rear_rx_temp, LV_ALIGN_CENTER, 60, 70);

    lv_obj_t *body_1 = lv_obj_create(frame_1);
    lv_obj_remove_style_all(body_1);
    lv_obj_set_size(body_1, 40, 60);
    lv_obj_set_style_radius(body_1, 5, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(body_1, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_color(body_1, lv_color_hex(COLOR_CAR_HEX), LV_PART_MAIN);
    lv_obj_set_align(body_1, LV_ALIGN_CENTER);

    lv_obj_t *leg_11 = lv_obj_create(frame_1);
    lv_obj_remove_style_all(leg_11);
    lv_obj_set_size(leg_11, 50, 2);
    lv_obj_set_style_radius(leg_11, 5, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(leg_11, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_color(leg_11, lv_color_hex(COLOR_CAR_HEX), LV_PART_MAIN);
    lv_obj_align(leg_11, LV_ALIGN_CENTER, -35, 20);

    lv_obj_t *feet_11 = lv_obj_create(frame_1);
    lv_obj_remove_style_all(feet_11);
    lv_obj_set_size(feet_11, 2, 25);
    lv_obj_set_style_radius(feet_11, 5, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(feet_11, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_color(feet_11, lv_color_hex(COLOR_CAR_HEX), LV_PART_MAIN);
    lv_obj_align(feet_11, LV_ALIGN_CENTER, -60, 31);

    lv_obj_t *leg_12 = lv_obj_create(frame_1);
    lv_obj_remove_style_all(leg_12);
    lv_obj_set_size(leg_12, 50, 2);
    lv_obj_set_style_radius(leg_12, 5, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(leg_12, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_color(leg_12, lv_color_hex(COLOR_CAR_HEX), LV_PART_MAIN);
    lv_obj_align(leg_12, LV_ALIGN_CENTER, 35, 20);

    lv_obj_t *feet_12 = lv_obj_create(frame_1);
    lv_obj_remove_style_all(feet_12);
    lv_obj_set_size(feet_12, 2, 25);
    lv_obj_set_style_radius(feet_12, 5, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(feet_12, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_color(feet_12, lv_color_hex(COLOR_CAR_HEX), LV_PART_MAIN);
    lv_obj_align(feet_12, LV_ALIGN_CENTER, 60, 31);

    lv_obj_t *leg_13 = lv_obj_create(frame_1);
    lv_obj_remove_style_all(leg_13);
    lv_obj_set_size(leg_13, 50, 2);
    lv_obj_set_style_radius(leg_13, 5, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(leg_13, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_color(leg_13, lv_color_hex(COLOR_CAR_HEX), LV_PART_MAIN);
    lv_obj_align(leg_13, LV_ALIGN_CENTER, 35, -20);

    lv_obj_t *feet_13 = lv_obj_create(frame_1);
    lv_obj_remove_style_all(feet_13);
    lv_obj_set_size(feet_13, 2, 25);
    lv_obj_set_style_radius(feet_13, 5, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(feet_13, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_color(feet_13, lv_color_hex(COLOR_CAR_HEX), LV_PART_MAIN);
    lv_obj_align(feet_13, LV_ALIGN_CENTER, -60, -32);

    lv_obj_t *leg_14 = lv_obj_create(frame_1);
    lv_obj_remove_style_all(leg_14);
    lv_obj_set_size(leg_14, 50, 2);
    lv_obj_set_style_radius(leg_14, 5, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(leg_14, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_color(leg_14, lv_color_hex(COLOR_CAR_HEX), LV_PART_MAIN);
    lv_obj_align(leg_14, LV_ALIGN_CENTER, -35, -20);

    lv_obj_t *feet_14 = lv_obj_create(frame_1);
    lv_obj_remove_style_all(feet_14);
    lv_obj_set_size(feet_14, 2, 25);
    lv_obj_set_style_radius(feet_14, 5, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(feet_14, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_color(feet_14, lv_color_hex(COLOR_CAR_HEX), LV_PART_MAIN);
    lv_obj_align(feet_14, LV_ALIGN_CENTER, 60, -32);

    lv_obj_t *frame_2 = lv_obj_create(left_data_panel);
    lv_obj_remove_style_all(frame_2);
    lv_obj_set_grid_cell(frame_2, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 1, 1);

    lv_obj_t *front_lx_press =
        lv_horizontal_pair_label(frame_2, &tab_sensors_labels[tab_sensors_lb_fl_press], NULL, "NA", &lv_font_inter_bold_30, " bar", &lv_font_inter_bold_22);
    lv_obj_align(front_lx_press, LV_ALIGN_CENTER, -60, -70);

    lv_obj_t *front_rx_press =
        lv_horizontal_pair_label(frame_2, &tab_sensors_labels[tab_sensors_lb_fr_press], NULL, "NA", &lv_font_inter_bold_30, " bar", &lv_font_inter_bold_22);
    lv_obj_align(front_rx_press, LV_ALIGN_CENTER, 60, -70);

    lv_obj_t *rear_lx_press =
        lv_horizontal_pair_label(frame_2, &tab_sensors_labels[tab_sensors_lb_rl_press], NULL, "NA", &lv_font_inter_bold_30, " bar", &lv_font_inter_bold_22);
    lv_obj_align(rear_lx_press, LV_ALIGN_CENTER, -60, 70);

    lv_obj_t *rear_rx_press =
        lv_horizontal_pair_label(frame_2, &tab_sensors_labels[tab_sensors_lb_rr_press], NULL, "NA", &lv_font_inter_bold_30, " bar", &lv_font_inter_bold_22);
    lv_obj_align(rear_rx_press, LV_ALIGN_CENTER, 60, 70);

    lv_obj_t *body_2 = lv_obj_create(frame_2);
    lv_obj_remove_style_all(body_2);
    lv_obj_set_size(body_2, 40, 60);
    lv_obj_set_style_radius(body_2, 5, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(body_2, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_color(body_2, lv_color_hex(COLOR_CAR_HEX), LV_PART_MAIN);
    lv_obj_set_align(body_2, LV_ALIGN_CENTER);

    lv_obj_t *leg_21 = lv_obj_create(frame_2);
    lv_obj_remove_style_all(leg_21);
    lv_obj_set_size(leg_21, 50, 2);
    lv_obj_set_style_radius(leg_21, 5, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(leg_21, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_color(leg_21, lv_color_hex(COLOR_CAR_HEX), LV_PART_MAIN);
    lv_obj_align(leg_21, LV_ALIGN_CENTER, -35, 20);

    lv_obj_t *feet_21 = lv_obj_create(frame_2);
    lv_obj_remove_style_all(feet_21);
    lv_obj_set_size(feet_21, 2, 25);
    lv_obj_set_style_radius(feet_21, 5, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(feet_21, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_color(feet_21, lv_color_hex(COLOR_CAR_HEX), LV_PART_MAIN);
    lv_obj_align(feet_21, LV_ALIGN_CENTER, -60, 31);

    lv_obj_t *leg_22 = lv_obj_create(frame_2);
    lv_obj_remove_style_all(leg_22);
    lv_obj_set_size(leg_22, 50, 2);
    lv_obj_set_style_radius(leg_22, 5, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(leg_22, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_color(leg_22, lv_color_hex(COLOR_CAR_HEX), LV_PART_MAIN);
    lv_obj_align(leg_22, LV_ALIGN_CENTER, 35, 20);

    lv_obj_t *feet_22 = lv_obj_create(frame_2);
    lv_obj_remove_style_all(feet_22);
    lv_obj_set_size(feet_22, 2, 25);
    lv_obj_set_style_radius(feet_22, 5, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(feet_22, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_color(feet_22, lv_color_hex(COLOR_CAR_HEX), LV_PART_MAIN);
    lv_obj_align(feet_22, LV_ALIGN_CENTER, 60, 31);

    lv_obj_t *leg_23 = lv_obj_create(frame_2);
    lv_obj_remove_style_all(leg_23);
    lv_obj_set_size(leg_23, 50, 2);
    lv_obj_set_style_radius(leg_23, 5, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(leg_23, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_color(leg_23, lv_color_hex(COLOR_CAR_HEX), LV_PART_MAIN);
    lv_obj_align(leg_23, LV_ALIGN_CENTER, 35, -20);

    lv_obj_t *feet_23 = lv_obj_create(frame_2);
    lv_obj_remove_style_all(feet_23);
    lv_obj_set_size(feet_23, 2, 25);
    lv_obj_set_style_radius(feet_23, 5, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(feet_23, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_color(feet_23, lv_color_hex(COLOR_CAR_HEX), LV_PART_MAIN);
    lv_obj_align(feet_23, LV_ALIGN_CENTER, -60, -32);

    lv_obj_t *leg_24 = lv_obj_create(frame_2);
    lv_obj_remove_style_all(leg_24);
    lv_obj_set_size(leg_24, 50, 2);
    lv_obj_set_style_radius(leg_24, 5, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(leg_24, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_color(leg_24, lv_color_hex(COLOR_CAR_HEX), LV_PART_MAIN);
    lv_obj_align(leg_24, LV_ALIGN_CENTER, -35, -20);

    lv_obj_t *feet_24 = lv_obj_create(frame_2);
    lv_obj_remove_style_all(feet_24);
    lv_obj_set_size(feet_24, 2, 25);
    lv_obj_set_style_radius(feet_24, 5, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(feet_24, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_color(feet_24, lv_color_hex(COLOR_CAR_HEX), LV_PART_MAIN);
    lv_obj_align(feet_24, LV_ALIGN_CENTER, 60, -32);

    /*--- DATA PANEL: cell 0,1 ---*/

    static lv_coord_t rxd_panel_cols[] = {415 / 4, 415 / 4, 415 / 4, 415 / 4, LV_GRID_TEMPLATE_LAST};
    static lv_coord_t rxd_panel_rows[] = {DATA_PANEL_HEIGHT / 4, DATA_PANEL_HEIGHT / 4, DATA_PANEL_HEIGHT / 4, DATA_PANEL_HEIGHT / 4, LV_GRID_TEMPLATE_LAST};

    lv_obj_t *right_data_panel = lv_obj_create(data_panel);
    lv_obj_set_layout(right_data_panel, LV_LAYOUT_GRID);
    lv_obj_remove_style_all(right_data_panel);
    lv_obj_clear_flag(right_data_panel, LV_OBJ_FLAG_SCROLLABLE);
    // lv_obj_set_size(left_data_panel, CELL_WIDTH, CELL_HEIGHT);

    lv_obj_add_style(right_data_panel, &box_panels_style, 0);
    lv_obj_center(right_data_panel);
    lv_obj_set_style_base_dir(right_data_panel, LV_BASE_DIR_LTR, 0);
    lv_obj_set_grid_dsc_array(right_data_panel, rxd_panel_cols, rxd_panel_rows);

    lv_obj_set_grid_cell(right_data_panel, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 0, 1);

    /* 1st row */

    lv_obj_t *inv_l_temp_lb = lv_triple_label(
        right_data_panel,
        &tab_sensors_labels[tab_sensors_lb_left_inverter_temp],
        "NA",
        &lv_font_inter_bold_30,
        "°C",
        &lv_font_inter_bold_22,
        "INV L",
        &lv_font_inter_bold_20);
    lv_obj_set_grid_cell(inv_l_temp_lb, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 0, 1);

    lv_obj_t *inv_r_temp_lb = lv_triple_label(
        right_data_panel,
        &tab_sensors_labels[tab_sensors_lb_right_inverter_temp],
        "NA",
        &lv_font_inter_bold_30,
        "°C",
        &lv_font_inter_bold_22,
        "INV R",
        &lv_font_inter_bold_20);
    lv_obj_set_grid_cell(inv_r_temp_lb, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, 0, 1);

    lv_obj_t *motor_l_temp_lb = lv_triple_label(
        right_data_panel,
        &tab_sensors_labels[tab_sensors_lb_left_motor_temp],
        "NA",
        &lv_font_inter_bold_30,
        "°C",
        &lv_font_inter_bold_22,
        "MOTOR L",
        &lv_font_inter_bold_20);
    lv_obj_set_grid_cell(motor_l_temp_lb, LV_GRID_ALIGN_CENTER, 2, 1, LV_GRID_ALIGN_CENTER, 0, 1);

    lv_obj_t *motor_r_temp_lb = lv_triple_label(
        right_data_panel,
        &tab_sensors_labels[tab_sensors_lb_right_motor_temp],
        "NA",
        &lv_font_inter_bold_30,
        "°C",
        &lv_font_inter_bold_22,
        "MOTOR R",
        &lv_font_inter_bold_20);
    lv_obj_set_grid_cell(motor_r_temp_lb, LV_GRID_ALIGN_CENTER, 3, 1, LV_GRID_ALIGN_CENTER, 0, 1);

    /* 2nd row */

    lv_obj_t *tlm_ntw_intf0 = lv_triple_label(
        right_data_panel,
        &tab_sensors_labels[tab_sensors_lb_tlm_ntw_interface_0],
        "NA",
        &lv_font_inter_bold_20,
        "",
        &lv_font_inter_bold_22,
        "INTERFACE 1",
        &lv_font_inter_bold_20);
    lv_obj_set_grid_cell(tlm_ntw_intf0, LV_GRID_ALIGN_CENTER, 0, 2, LV_GRID_ALIGN_CENTER, 1, 1);

    lv_obj_t *tlm_ntw_intf1 = lv_triple_label(
        right_data_panel,
        &tab_sensors_labels[tab_sensors_lb_tlm_ntw_interface_1],
        "NA",
        &lv_font_inter_bold_20,
        "",
        &lv_font_inter_bold_22,
        "INTERFACE 2",
        &lv_font_inter_bold_20);
    lv_obj_set_grid_cell(tlm_ntw_intf1, LV_GRID_ALIGN_CENTER, 2, 2, LV_GRID_ALIGN_CENTER, 1, 1);

    /* 3rd row */

    lv_obj_t *tlm_ntw_intf2 = lv_triple_label(
        right_data_panel,
        &tab_sensors_labels[tab_sensors_lb_tlm_ntw_interface_2],
        "NA",
        &lv_font_inter_bold_20,
        "",
        &lv_font_inter_bold_22,
        "INTERFACE 3",
        &lv_font_inter_bold_20);
    lv_obj_set_grid_cell(tlm_ntw_intf2, LV_GRID_ALIGN_CENTER, 0, 2, LV_GRID_ALIGN_CENTER, 2, 1);

    lv_obj_t *tlm_ntw_intf3 = lv_triple_label(
        right_data_panel,
        &tab_sensors_labels[tab_sensors_lb_tlm_ntw_interface_3],
        "NA",
        &lv_font_inter_bold_20,
        "",
        &lv_font_inter_bold_22,
        "INTERFACE 4",
        &lv_font_inter_bold_20);
    lv_obj_set_grid_cell(tlm_ntw_intf3, LV_GRID_ALIGN_CENTER, 2, 2, LV_GRID_ALIGN_CENTER, 2, 1);

    /* 4th row */

    lv_obj_t *tlm_ntw_intf4 = lv_triple_label(
        right_data_panel,
        &tab_sensors_labels[tab_sensors_lb_tlm_ntw_interface_4],
        "NA",
        &lv_font_inter_bold_20,
        "",
        &lv_font_inter_bold_22,
        "INTERFACE 5",
        &lv_font_inter_bold_20);
    lv_obj_set_grid_cell(tlm_ntw_intf4, LV_GRID_ALIGN_CENTER, 0, 2, LV_GRID_ALIGN_CENTER, 3, 1);

    lv_obj_t *tlm_ntw_intf5 = lv_triple_label(
        right_data_panel,
        &tab_sensors_labels[tab_sensors_lb_tlm_ntw_interface_5],
        "NA",
        &lv_font_inter_bold_20,
        "",
        &lv_font_inter_bold_22,
        "INTERFACE 6",
        &lv_font_inter_bold_20);
    lv_obj_set_grid_cell(tlm_ntw_intf5, LV_GRID_ALIGN_CENTER, 2, 2, LV_GRID_ALIGN_CENTER, 3, 1);

    /* --- BARS PANEL --- */

    static lv_coord_t bars_panel_cols[] = {120 / 4, 120 / 4, 120 / 4, 120 / 4, LV_GRID_TEMPLATE_LAST};
    static lv_coord_t bars_panel_rows[] = {DATA_PANEL_HEIGHT - 30, 30, LV_GRID_TEMPLATE_LAST};

    lv_obj_t *bars_data_panel = lv_obj_create(data_panel);
    lv_obj_set_layout(bars_data_panel, LV_LAYOUT_GRID);
    lv_obj_remove_style_all(bars_data_panel);
    lv_obj_clear_flag(bars_data_panel, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_center(bars_data_panel);
    lv_obj_set_style_base_dir(bars_data_panel, LV_BASE_DIR_LTR, 0);
    lv_obj_set_grid_dsc_array(bars_data_panel, bars_panel_cols, bars_panel_rows);

    lv_obj_set_grid_cell(bars_data_panel, LV_GRID_ALIGN_STRETCH, 2, 1, LV_GRID_ALIGN_STRETCH, 0, 1);

    lv_obj_t *break_rear_container = lv_label_create(bars_data_panel);
    lv_obj_remove_style_all(break_rear_container);
    lv_obj_align(break_rear_container, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_style(break_rear_container, &label_style, LV_PART_MAIN);
    lv_label_set_text(break_rear_container, "R");
    lv_obj_set_style_text_font(break_rear_container, &lv_font_inter_bold_20, LV_PART_MAIN);
    lv_obj_set_grid_cell(break_rear_container, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 1, 1);

    break_rear_bar = lv_bar_create(bars_data_panel);
    lv_obj_remove_style_all(break_rear_bar);
    lv_obj_add_style(break_rear_bar, &brake_bar_style, LV_PART_INDICATOR);

    lv_obj_add_style(break_rear_bar, &brake_bar_background_style, LV_PART_MAIN);
    lv_bar_set_range(break_rear_bar, 0, 100);
    lv_bar_set_value(break_rear_bar, 0, LV_ANIM_OFF);

    lv_obj_set_grid_cell(break_rear_bar, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 0, 1);

    lv_obj_t *break_front_container = lv_label_create(bars_data_panel);
    lv_obj_remove_style_all(break_front_container);
    lv_obj_add_style(break_front_container, &label_style, LV_PART_MAIN);
    lv_label_set_text(break_front_container, "F");
    lv_obj_set_style_text_font(break_front_container, &lv_font_inter_bold_20, LV_PART_MAIN);
    lv_obj_set_grid_cell(break_front_container, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, 1, 1);

    brake_front_bar = lv_bar_create(bars_data_panel);
    lv_obj_remove_style_all(brake_front_bar);
    lv_obj_add_style(brake_front_bar, &brake_bar_style, LV_PART_INDICATOR);
    lv_style_set_width(&brake_bar_background_style, 25);
    lv_obj_add_style(brake_front_bar, &brake_bar_background_style, LV_PART_MAIN);
    lv_bar_set_range(brake_front_bar, 0, 100);
    lv_bar_set_value(brake_front_bar, 0, LV_ANIM_OFF);
    lv_obj_set_grid_cell(brake_front_bar, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, 0, 1);

    lv_obj_t *throttle_1_container = lv_label_create(bars_data_panel);
    lv_obj_remove_style_all(throttle_1_container);
    lv_obj_add_style(throttle_1_container, &label_style, LV_PART_MAIN);
    lv_label_set_text(throttle_1_container, "THR");
    lv_obj_set_style_text_font(throttle_1_container, &lv_font_inter_bold_20, LV_PART_MAIN);
    lv_obj_set_grid_cell(throttle_1_container, LV_GRID_ALIGN_CENTER, 2, 2, LV_GRID_ALIGN_CENTER, 1, 1);

    throttle_bar1 = lv_bar_create(bars_data_panel);
    lv_obj_remove_style_all(throttle_bar1);
    lv_obj_add_style(throttle_bar1, &throttle_bar_style, LV_PART_INDICATOR);
    lv_style_set_width(&throttle_bar_background_style, 50);
    lv_obj_add_style(throttle_bar1, &throttle_bar_background_style, LV_PART_MAIN);
    lv_bar_set_range(throttle_bar1, 0, 100);
    lv_bar_set_value(throttle_bar1, 0, LV_ANIM_OFF);
    lv_obj_set_grid_cell(throttle_bar1, LV_GRID_ALIGN_CENTER, 2, 2, LV_GRID_ALIGN_CENTER, 0, 1);
}

void tab_sensor_resync(void) {
#if 0
    inv_l_rcv_update();
    inv_r_rcv_update();
    irts_fl_update();
    irts_fr_update();
    irts_rl_update();
    irts_rr_update();
#endif
}