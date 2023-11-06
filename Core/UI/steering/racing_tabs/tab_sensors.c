#include "tab_sensors.h"
#include <stdio.h>

#define CELL_HEIGHT 200
#define CELL_WIDTH 180

lv_obj_t *lb_fl_temp;
lv_obj_t *lb_fr_temp;
lv_obj_t *lb_rl_temp;
lv_obj_t *lb_rr_temp;
lv_obj_t *lb_fl_press;
lv_obj_t *lb_fr_press;
lv_obj_t *lb_rl_press;
lv_obj_t *lb_rr_press;
lv_obj_t *tab_sensors_lb_right_inverter_temp;
lv_obj_t *tab_sensors_lb_right_motor_temp;
lv_obj_t *tab_sensors_lb_average_temperature;
lv_obj_t *tab_sensors_lb_left_inverter_temp;
lv_obj_t *tab_sensors_lb_left_motor_temp;
lv_obj_t *tab_sensors_lb_pack_voltage;
lv_obj_t *tab_sensors_lb_hv_current;
lv_obj_t *tab_sensors_lb_min_cell_voltage;
lv_obj_t *tab_sensors_lb_battery_temperature;
lv_obj_t *tab_sensors_lb_voltage;
lv_obj_t *tab_sensors_lb_lv_current;
lv_obj_t *tab_sensors_lb_hv_delta;
lv_obj_t *tab_sensors_extra_value0;
lv_obj_t *tab_sensors_extra_value1;
lv_obj_t *tab_sensors_extra_value2;
lv_obj_t *tab_sensors_lb_tlm_status;

lv_style_t bar_brake_style;
lv_style_t bar_accel_style;
lv_style_t bar_calib_back_style;

lv_obj_t *get_tab_sensors_extra_value0() { return tab_sensors_extra_value0; }

lv_obj_t *get_tab_sensors_extra_value1() { return tab_sensors_extra_value1; }

lv_obj_t *get_tab_sensors_extra_value2() { return tab_sensors_extra_value2; }

void set_tab_sensors_lb_right_inverter_temp(const char *s) {
  lv_label_set_text(tab_sensors_lb_right_inverter_temp, s);
}

void set_tab_sensors_lb_right_motor_temp(const char *s) {
  lv_label_set_text(tab_sensors_lb_right_motor_temp, s);
}

void set_tab_sensors_lb_average_temperature(const char *s) {
  lv_label_set_text(tab_sensors_lb_average_temperature, s);
}

void set_tab_sensors_lb_left_inverter_temp(const char *s) {
  lv_label_set_text(tab_sensors_lb_left_inverter_temp, s);
}

void set_tab_sensors_lb_left_motor_temp(const char *s) {
  lv_label_set_text(tab_sensors_lb_left_motor_temp, s);
}

void set_tab_sensors_lb_pack_voltage(const char *s) {
  lv_label_set_text(tab_sensors_lb_pack_voltage, s);
}

void set_tab_sensors_lb_hv_current(const char *s) {
  lv_label_set_text(tab_sensors_lb_hv_current, s);
}

void set_tab_sensors_lb_min_cell_voltage(const char *s) {
  lv_label_set_text(tab_sensors_lb_min_cell_voltage, s);
}

void set_tab_sensors_lb_battery_temperature(const char *s) {
  lv_label_set_text(tab_sensors_lb_battery_temperature, s);
}

void set_tab_sensors_lb_voltage(const char *s) {
  lv_label_set_text(tab_sensors_lb_voltage, s);
}

void set_tab_sensors_lb_lv_current(const char *s) {
  lv_label_set_text(tab_sensors_lb_lv_current, s);
}

void set_tab_sensors_lb_hv_delta(const char *s) {
  lv_label_set_text(tab_sensors_lb_hv_delta, s);
}

void set_tab_sensors_lb_tlm_status(const char *s) {
  lv_label_set_text(tab_sensors_lb_tlm_status, s);
}

void init_sensors_styles(void) {
  lv_style_init(&bar_brake_style);
  lv_style_set_bg_opa(&bar_brake_style, LV_OPA_COVER);
  lv_style_set_bg_color(&bar_brake_style, lv_color_hex(COLOR_RED_STATUS_HEX));

  lv_style_init(&bar_accel_style);
  lv_style_set_bg_opa(&bar_accel_style, LV_OPA_COVER);
  lv_style_set_bg_color(&bar_accel_style, lv_color_hex(COLOR_GREEN_STATUS_HEX));

  lv_style_init(&bar_calib_back_style);
  lv_style_set_bg_opa(&bar_calib_back_style, LV_OPA_COVER);
  lv_style_set_bg_color(&bar_calib_back_style,
                        lv_color_hex(COLOR_SECONDARY_HEX));
  lv_style_set_width(&bar_calib_back_style, 25);
  lv_style_set_height(&bar_calib_back_style, 405);
}

void tab_sensors_create(lv_obj_t *parent) {
  init_sensors_styles();

  /*---creating MAIN GRID ---*/

  static lv_coord_t main_panel_cols[] = {SCREEN_WIDTH, LV_GRID_TEMPLATE_LAST};
  static lv_coord_t main_panel_rows[] = {TOP_BAR_HEIGHT, DATA_PANEL_HEIGHT,
                                         LV_GRID_TEMPLATE_LAST};

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

  lv_obj_t *notch = create_notch(main_panel, TAB_SENSORS);
  lv_obj_align(lv_obj_get_child(notch, 0), LV_ALIGN_TOP_MID, 0, 10);
  lv_obj_set_grid_cell(notch, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_START,
                       0, 1);

  /*--- creating DATA PANEL ---*/

  static lv_coord_t dpanel_cols[] = {225, SCREEN_WIDTH - 265 - 120, 120,
                                     LV_GRID_TEMPLATE_LAST};
  static lv_coord_t dpanel_rows[] = {DATA_PANEL_HEIGHT, LV_GRID_TEMPLATE_LAST};

  lv_obj_t *data_panel = lv_obj_create(main_panel);
  lv_obj_remove_style_all(data_panel);
  // lv_obj_set_layout(data_panel, LV_LAYOUT_GRID);
  lv_obj_clear_flag(data_panel, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_size(data_panel, SCREEN_WIDTH,
                  DATA_PANEL_HEIGHT); // non andrebbe tolto il 20 per farlo
                                      // simmetrico(forse qualche errore)

  lv_obj_add_style(data_panel, &grid_style, 0);
  lv_obj_center(data_panel);
  lv_obj_set_style_base_dir(data_panel, LV_BASE_DIR_LTR, 0);
  lv_obj_set_grid_dsc_array(data_panel, dpanel_cols, dpanel_rows);

  lv_obj_set_style_pad_row(data_panel, ROW_PADDING, 0);
  lv_obj_set_style_pad_column(data_panel, COLUMN_PADDING, 0);
  lv_obj_set_style_pad_all(data_panel, 0, LV_PART_MAIN);

  lv_obj_set_grid_cell(data_panel, LV_GRID_ALIGN_CENTER, 0, 1,
                       LV_GRID_ALIGN_CENTER, 1, 1);

  /*--- DATA PANEL: cell 0,0 ---*/

  static lv_coord_t lxd_panel_cols[] = {225, LV_GRID_TEMPLATE_LAST};
  static lv_coord_t lxd_panel_rows[] = {
      DATA_PANEL_HEIGHT / 2, DATA_PANEL_HEIGHT / 2, LV_GRID_TEMPLATE_LAST};

  lv_obj_t *left_data_panel = lv_obj_create(data_panel);
  lv_obj_set_layout(left_data_panel, LV_LAYOUT_GRID);
  lv_obj_remove_style_all(left_data_panel);
  lv_obj_clear_flag(left_data_panel, LV_OBJ_FLAG_SCROLLABLE);
  // lv_obj_set_size(left_data_panel, CELL_WIDTH, CELL_HEIGHT);

  lv_obj_add_style(left_data_panel, &box_panels_style, 0);
  lv_obj_center(left_data_panel);
  lv_obj_set_style_base_dir(left_data_panel, LV_BASE_DIR_LTR, 0);
  lv_obj_set_grid_dsc_array(left_data_panel, lxd_panel_cols, lxd_panel_rows);

  lv_obj_set_grid_cell(left_data_panel, LV_GRID_ALIGN_STRETCH, 0, 1,
                       LV_GRID_ALIGN_STRETCH, 0, 1);

  lv_obj_t *frame_1 = lv_obj_create(left_data_panel);
  lv_obj_remove_style_all(frame_1);
  lv_obj_set_grid_cell(frame_1, LV_GRID_ALIGN_STRETCH, 0, 1,
                       LV_GRID_ALIGN_STRETCH, 0, 1);

  lv_obj_t *front_lx_temp = lv_horizontal_pair_label(
      frame_1, &lb_fl_temp, "X", &lv_font_inter_bold_30, "°C",
      &lv_font_inter_bold_22);
  lv_obj_align(front_lx_temp, LV_ALIGN_CENTER, -60, -70);

  lv_obj_t *front_rx_temp = lv_horizontal_pair_label(
      frame_1, &lb_fr_temp, "X", &lv_font_inter_bold_30, "°C",
      &lv_font_inter_bold_22);
  lv_obj_align(front_rx_temp, LV_ALIGN_CENTER, 60, -70);

  lv_obj_t *rear_lx_temp = lv_horizontal_pair_label(
      frame_1, &lb_rl_temp, "X", &lv_font_inter_bold_30, "°C",
      &lv_font_inter_bold_22);
  lv_obj_align(rear_lx_temp, LV_ALIGN_CENTER, -60, 70);

  lv_obj_t *rear_rx_temp = lv_horizontal_pair_label(
      frame_1, &lb_rr_temp, "X", &lv_font_inter_bold_30, "°C",
      &lv_font_inter_bold_22);
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
  lv_obj_set_grid_cell(frame_2, LV_GRID_ALIGN_STRETCH, 0, 1,
                       LV_GRID_ALIGN_STRETCH, 1, 1);

  lv_obj_t *front_lx_press = lv_horizontal_pair_label(
      frame_2, &lb_fl_press, "X", &lv_font_inter_bold_30, " bar",
      &lv_font_inter_bold_22);
  lv_obj_align(front_lx_press, LV_ALIGN_CENTER, -60, -70);

  lv_obj_t *front_rx_press = lv_horizontal_pair_label(
      frame_2, &lb_fr_press, "X", &lv_font_inter_bold_30, " bar",
      &lv_font_inter_bold_22);
  lv_obj_align(front_rx_press, LV_ALIGN_CENTER, 60, -70);

  lv_obj_t *rear_lx_press = lv_horizontal_pair_label(
      frame_2, &lb_rl_press, "X", &lv_font_inter_bold_30, " bar",
      &lv_font_inter_bold_22);
  lv_obj_align(rear_lx_press, LV_ALIGN_CENTER, -60, 70);

  lv_obj_t *rear_rx_press = lv_horizontal_pair_label(
      frame_2, &lb_rr_press, "X", &lv_font_inter_bold_30, " bar",
      &lv_font_inter_bold_22);
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

  static lv_coord_t rxd_panel_cols[] = {415 / 4, 415 / 4, 415 / 4, 415 / 4,
                                        LV_GRID_TEMPLATE_LAST};
  static lv_coord_t rxd_panel_rows[] = {
      DATA_PANEL_HEIGHT / 4, DATA_PANEL_HEIGHT / 4, DATA_PANEL_HEIGHT / 4,
      DATA_PANEL_HEIGHT / 4, LV_GRID_TEMPLATE_LAST};

  lv_obj_t *right_data_panel = lv_obj_create(data_panel);
  lv_obj_set_layout(right_data_panel, LV_LAYOUT_GRID);
  lv_obj_remove_style_all(right_data_panel);
  lv_obj_clear_flag(right_data_panel, LV_OBJ_FLAG_SCROLLABLE);
  // lv_obj_set_size(left_data_panel, CELL_WIDTH, CELL_HEIGHT);

  lv_obj_add_style(right_data_panel, &box_panels_style, 0);
  lv_obj_center(right_data_panel);
  lv_obj_set_style_base_dir(right_data_panel, LV_BASE_DIR_LTR, 0);
  lv_obj_set_grid_dsc_array(right_data_panel, rxd_panel_cols, rxd_panel_rows);

  lv_obj_set_grid_cell(right_data_panel, LV_GRID_ALIGN_STRETCH, 1, 1,
                       LV_GRID_ALIGN_STRETCH, 0, 1);

  /* 1st row */

  lv_obj_t *inv_l_temp_lb =
      lv_triple_label(right_data_panel, &tab_sensors_lb_left_inverter_temp, "X",
                      &lv_font_inter_bold_30, "°C", &lv_font_inter_bold_22,
                      "INV L", &lv_font_inter_bold_20);
  lv_obj_set_grid_cell(inv_l_temp_lb, LV_GRID_ALIGN_CENTER, 0, 1,
                       LV_GRID_ALIGN_CENTER, 0, 1);

  lv_obj_t *inv_r_temp_lb =
      lv_triple_label(right_data_panel, &tab_sensors_lb_right_inverter_temp,
                      "X", &lv_font_inter_bold_30, "°C", &lv_font_inter_bold_22,
                      "INV R", &lv_font_inter_bold_20);
  lv_obj_set_grid_cell(inv_r_temp_lb, LV_GRID_ALIGN_CENTER, 1, 1,
                       LV_GRID_ALIGN_CENTER, 0, 1);

  lv_obj_t *motor_l_temp_lb =
      lv_triple_label(right_data_panel, &tab_sensors_lb_left_motor_temp, "X",
                      &lv_font_inter_bold_30, "°C", &lv_font_inter_bold_22,
                      "MOTOR L", &lv_font_inter_bold_20);
  lv_obj_set_grid_cell(motor_l_temp_lb, LV_GRID_ALIGN_CENTER, 2, 1,
                       LV_GRID_ALIGN_CENTER, 0, 1);

  lv_obj_t *motor_r_temp_lb =
      lv_triple_label(right_data_panel, &tab_sensors_lb_right_motor_temp, "X",
                      &lv_font_inter_bold_30, "°C", &lv_font_inter_bold_22,
                      "MOTOR R", &lv_font_inter_bold_20);
  lv_obj_set_grid_cell(motor_r_temp_lb, LV_GRID_ALIGN_CENTER, 3, 1,
                       LV_GRID_ALIGN_CENTER, 0, 1);

  /* 2nd row HV */

  lv_obj_t *hv_temp_lb =
      lv_triple_label(right_data_panel, &tab_sensors_lb_average_temperature,
                      "0", &lv_font_inter_bold_30, "°C", &lv_font_inter_bold_22,
                      "HV", &lv_font_inter_bold_20);
  lv_obj_set_grid_cell(hv_temp_lb, LV_GRID_ALIGN_CENTER, 0, 1,
                       LV_GRID_ALIGN_CENTER, 1, 1);

  lv_obj_t *hv_volt_lb =
      lv_triple_label(right_data_panel, &tab_sensors_lb_pack_voltage, "0",
                      &lv_font_inter_bold_30, " V", &lv_font_inter_bold_22,
                      "HV", &lv_font_inter_bold_20);
  lv_obj_set_grid_cell(hv_volt_lb, LV_GRID_ALIGN_CENTER, 1, 1,
                       LV_GRID_ALIGN_CENTER, 1, 1);

  lv_obj_t *hv_curr_lb = lv_triple_label(
      right_data_panel, &tab_sensors_lb_hv_current, "0", &lv_font_inter_bold_30,
      " A", &lv_font_inter_bold_22, "HV", &lv_font_inter_bold_20);
  lv_obj_set_grid_cell(hv_curr_lb, LV_GRID_ALIGN_CENTER, 2, 1,
                       LV_GRID_ALIGN_CENTER, 1, 1);

  lv_obj_t *hv_minCellV_lb =
      lv_triple_label(right_data_panel, &tab_sensors_lb_min_cell_voltage, "0",
                      &lv_font_inter_bold_30, " V", &lv_font_inter_bold_22,
                      "HV MIN", &lv_font_inter_bold_20);
  lv_obj_set_grid_cell(hv_minCellV_lb, LV_GRID_ALIGN_CENTER, 3, 1,
                       LV_GRID_ALIGN_CENTER, 1, 1);

  /* 3rd row LV */

  lv_obj_t *lv_temp_lb =
      lv_triple_label(right_data_panel, &tab_sensors_lb_battery_temperature,
                      "X", &lv_font_inter_bold_30, "°C", &lv_font_inter_bold_22,
                      "LV", &lv_font_inter_bold_20);
  lv_obj_set_grid_cell(lv_temp_lb, LV_GRID_ALIGN_CENTER, 0, 1,
                       LV_GRID_ALIGN_CENTER, 2, 1);

  lv_obj_t *lv_volt_lb = lv_triple_label(
      right_data_panel, &tab_sensors_lb_voltage, "X", &lv_font_inter_bold_30,
      " V", &lv_font_inter_bold_22, "LV", &lv_font_inter_bold_20);
  lv_obj_set_grid_cell(lv_volt_lb, LV_GRID_ALIGN_CENTER, 1, 1,
                       LV_GRID_ALIGN_CENTER, 2, 1);

  lv_obj_t *lv_curr_lb = lv_triple_label(
      right_data_panel, &tab_sensors_lb_lv_current, "X", &lv_font_inter_bold_30,
      " A", &lv_font_inter_bold_22, "LV", &lv_font_inter_bold_20);
  lv_obj_set_grid_cell(lv_curr_lb, LV_GRID_ALIGN_CENTER, 2, 1,
                       LV_GRID_ALIGN_CENTER, 2, 1);

  lv_obj_t *y2 = lv_triple_label(
      right_data_panel, &tab_sensors_lb_hv_delta, "-", &lv_font_inter_bold_30,
      " V", &lv_font_inter_bold_22, "HV DELTA", &lv_font_inter_bold_20);
  lv_obj_set_grid_cell(y2, LV_GRID_ALIGN_CENTER, 3, 1, LV_GRID_ALIGN_CENTER, 2,
                       1);
/* 4th row */
#if 1
  lv_obj_t *cooling_rad_lb = lv_triple_label(
      right_data_panel, &tab_sensors_extra_value0, "V1", &lv_font_inter_bold_30,
      "-", &lv_font_inter_bold_22, "-", &lv_font_inter_bold_20);
  lv_obj_set_grid_cell(cooling_rad_lb, LV_GRID_ALIGN_CENTER, 0, 1,
                       LV_GRID_ALIGN_CENTER, 3, 1);

  lv_obj_t *cooling_pump_lb = lv_triple_label(
      right_data_panel, &tab_sensors_extra_value1, "V2", &lv_font_inter_bold_30,
      "-", &lv_font_inter_bold_22, "-", &lv_font_inter_bold_20);
  lv_obj_set_grid_cell(cooling_pump_lb, LV_GRID_ALIGN_CENTER, 1, 1,
                       LV_GRID_ALIGN_CENTER, 3, 1);

  lv_obj_t *y11 = lv_triple_label(
      right_data_panel, &tab_sensors_extra_value2, "V3", &lv_font_inter_bold_30,
      "-", &lv_font_inter_bold_22, "-", &lv_font_inter_bold_20);
  lv_obj_set_grid_cell(y11, LV_GRID_ALIGN_CENTER, 2, 1, LV_GRID_ALIGN_CENTER, 3,
                       1);

  lv_obj_t *y21 =
      lv_triple_label(right_data_panel, &tab_sensors_lb_tlm_status, "TLM",
                      &lv_font_inter_bold_30, "", &lv_font_inter_bold_22, "-",
                      &lv_font_inter_bold_20);
  lv_obj_set_grid_cell(y21, LV_GRID_ALIGN_CENTER, 3, 1, LV_GRID_ALIGN_CENTER, 3,
                       1);
#endif

  /* --- BARS PANEL --- */

  static lv_coord_t bars_panel_cols[] = {120 / 4, 120 / 4, 120 / 4, 120 / 4,
                                         LV_GRID_TEMPLATE_LAST};
  static lv_coord_t bars_panel_rows[] = {DATA_PANEL_HEIGHT - 30, 30,
                                         LV_GRID_TEMPLATE_LAST};

  lv_obj_t *bars_data_panel = lv_obj_create(data_panel);
  lv_obj_set_layout(bars_data_panel, LV_LAYOUT_GRID);
  lv_obj_remove_style_all(bars_data_panel);
  lv_obj_clear_flag(bars_data_panel, LV_OBJ_FLAG_SCROLLABLE);

  lv_obj_center(bars_data_panel);
  lv_obj_set_style_base_dir(bars_data_panel, LV_BASE_DIR_LTR, 0);
  lv_obj_set_grid_dsc_array(bars_data_panel, bars_panel_cols, bars_panel_rows);

  lv_obj_set_grid_cell(bars_data_panel, LV_GRID_ALIGN_STRETCH, 2, 1,
                       LV_GRID_ALIGN_STRETCH, 0, 1);

  lv_obj_t *apps_1 = lv_label_create(bars_data_panel);
  lv_obj_remove_style_all(apps_1);
  lv_obj_align(apps_1, LV_ALIGN_CENTER, 0, 0);
  lv_obj_add_style(apps_1, &label_style, LV_PART_MAIN);
  lv_label_set_text(apps_1, "F");
  lv_obj_set_style_text_font(apps_1, &lv_font_inter_bold_20, LV_PART_MAIN);
  lv_obj_set_grid_cell(apps_1, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER,
                       1, 1);

  lv_obj_t *apps_1_bar = lv_bar_create(bars_data_panel);
  lv_obj_remove_style_all(apps_1_bar);
  lv_obj_add_style(apps_1_bar, &bar_accel_style, LV_PART_INDICATOR);
  lv_obj_add_style(apps_1_bar, &bar_calib_back_style, LV_PART_MAIN);
  lv_bar_set_range(apps_1_bar, 0, 100);
  lv_bar_set_value(apps_1_bar, 0, LV_ANIM_OFF);

  lv_obj_set_grid_cell(apps_1_bar, LV_GRID_ALIGN_CENTER, 0, 1,
                       LV_GRID_ALIGN_CENTER, 0, 1);

  lv_obj_t *apps_2 = lv_label_create(bars_data_panel);
  lv_obj_remove_style_all(apps_2);
  lv_obj_add_style(apps_2, &label_style, LV_PART_MAIN);
  lv_label_set_text(apps_2, "R");
  lv_obj_set_style_text_font(apps_2, &lv_font_inter_bold_20, LV_PART_MAIN);
  lv_obj_set_grid_cell(apps_2, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER,
                       1, 1);

  lv_obj_t *apps_2_bar = lv_bar_create(bars_data_panel);
  lv_obj_remove_style_all(apps_2_bar);
  lv_obj_add_style(apps_2_bar, &bar_accel_style, LV_PART_INDICATOR);
  lv_obj_add_style(apps_2_bar, &bar_calib_back_style, LV_PART_MAIN);
  lv_bar_set_range(apps_2_bar, 0, 100);
  lv_bar_set_value(apps_2_bar, 0, LV_ANIM_OFF);
  lv_obj_set_grid_cell(apps_2_bar, LV_GRID_ALIGN_CENTER, 1, 1,
                       LV_GRID_ALIGN_CENTER, 0, 1);

  lv_obj_t *brake_f = lv_label_create(bars_data_panel);
  lv_obj_remove_style_all(brake_f);
  lv_obj_add_style(brake_f, &label_style, LV_PART_MAIN);
  lv_label_set_text(brake_f, "F");
  lv_obj_set_style_text_font(brake_f, &lv_font_inter_bold_20, LV_PART_MAIN);
  lv_obj_set_grid_cell(brake_f, LV_GRID_ALIGN_CENTER, 2, 1,
                       LV_GRID_ALIGN_CENTER, 1, 1);

  lv_obj_t *brake_f_bar = lv_bar_create(bars_data_panel);
  lv_obj_remove_style_all(brake_f_bar);
  lv_obj_add_style(brake_f_bar, &bar_brake_style, LV_PART_INDICATOR);
  lv_obj_add_style(brake_f_bar, &bar_calib_back_style, LV_PART_MAIN);
  lv_bar_set_range(brake_f_bar, 0, 100);
  lv_bar_set_value(brake_f_bar, 0, LV_ANIM_OFF);
  lv_obj_set_grid_cell(brake_f_bar, LV_GRID_ALIGN_CENTER, 2, 1,
                       LV_GRID_ALIGN_CENTER, 0, 1);

  lv_obj_t *brake_r = lv_label_create(bars_data_panel);
  lv_obj_remove_style_all(brake_r);
  lv_obj_add_style(brake_r, &label_style, LV_PART_MAIN);
  lv_label_set_text(brake_r, "R");
  lv_obj_set_style_text_font(brake_r, &lv_font_inter_bold_20, LV_PART_MAIN);

  lv_obj_set_grid_cell(brake_r, LV_GRID_ALIGN_CENTER, 3, 1,
                       LV_GRID_ALIGN_CENTER, 1, 1);

  lv_obj_t *brake_r_bar = lv_bar_create(bars_data_panel);
  lv_obj_remove_style_all(brake_r_bar);
  lv_obj_add_style(brake_r_bar, &bar_brake_style, LV_PART_INDICATOR);
  lv_obj_add_style(brake_r_bar, &bar_calib_back_style, LV_PART_MAIN);
  lv_bar_set_range(brake_r_bar, 0, 100);
  lv_bar_set_value(brake_r_bar, 0, LV_ANIM_OFF);
  lv_obj_set_grid_cell(brake_r_bar, LV_GRID_ALIGN_CENTER, 3, 1,
                       LV_GRID_ALIGN_CENTER, 0, 1);
}