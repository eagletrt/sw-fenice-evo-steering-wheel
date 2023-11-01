#include "tab_racing.h"

//refactoring
lv_obj_t *tab_racing_lb_pack_voltage;
lv_obj_t *racing_lv_bar;
lv_obj_t *tab_racing_lb_hv_current;
lv_obj_t *racing_hv_bar;
lv_obj_t *lb_best_time;
lv_obj_t *lb_last_time;
lv_obj_t *lb_delta_time;
lv_obj_t *tab_racing_lb_torque;
lv_obj_t *lb_slip;
lv_obj_t *tab_racing_lb_left_inverter_temp;
lv_obj_t *tab_racing_lb_left_motor_temp;
lv_obj_t *tab_racing_lb_battery_temperature;
lv_obj_t *tab_racing_lb_average_temperature;
lv_obj_t *tab_racing_lb_power;
lv_obj_t *tab_racing_lb_speed;
lv_obj_t *bottom_lb_speed;
lv_obj_t *custom_meter;
lv_meter_indicator_t *indicator_blue;
lv_meter_indicator_t *indicator_white;
lv_obj_t *lb_lap_count;

lv_style_t bar_hv_style;
lv_style_t bar_lv_style;
lv_style_t bar_back_style;

lv_obj_t *bottom_bar;

void tab_racing_create(lv_obj_t *parent) {
  init_racing_style();

  static lv_coord_t main_panel_cols[] = {
      RACING_TAB_SIDE_BAR_WIDTH, RACING_TAB_CENTRAL_PANEL_WIDTH,
      RACING_TAB_SIDE_BAR_WIDTH, LV_GRID_TEMPLATE_LAST};
  static lv_coord_t main_panel_rows[] = {SCREEN_HEIGHT, LV_GRID_TEMPLATE_LAST};

  lv_obj_t *tab_racing = lv_obj_create(parent);
  lv_obj_set_layout(tab_racing, LV_LAYOUT_GRID);
  lv_obj_set_size(tab_racing, SCREEN_WIDTH, SCREEN_HEIGHT);
  lv_obj_add_style(tab_racing, &grid_style, 0);
  lv_obj_clear_flag(tab_racing, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_center(tab_racing);

  lv_obj_set_grid_dsc_array(tab_racing, main_panel_cols, main_panel_rows);
  // lv_obj_set_grid_align(tab_racing, LV_GRID_ALIGN_SPACE_EVENLY,
  // LV_GRID_ALIGN_SPACE_EVENLY);

  /*-------LEFT BAR-------*/

  //  bar grid setup
  static lv_coord_t bar_panel_cols[] = {RACING_TAB_SIDE_BAR_WIDTH,
                                        LV_GRID_TEMPLATE_LAST};
  static lv_coord_t bar_panel_rows[] = {70, 360, 70, LV_GRID_TEMPLATE_LAST};

  lv_obj_t *bar_panel_lv = lv_obj_create(tab_racing);
  lv_obj_remove_style_all(bar_panel_lv);
  lv_obj_add_style(bar_panel_lv, &grid_style, 0);
  lv_obj_set_size(bar_panel_lv, RACING_TAB_SIDE_BAR_WIDTH, SCREEN_HEIGHT);
  lv_obj_clear_flag(bar_panel_lv, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_grid_dsc_array(bar_panel_lv, bar_panel_cols, bar_panel_rows);

  lv_obj_set_grid_cell(bar_panel_lv, LV_GRID_ALIGN_CENTER, 0, 1,
                       LV_GRID_ALIGN_START, 0, 1);

  // lv percentage
  lv_obj_t *lv_perc = lv_horizontal_pair_label(
      bar_panel_lv, &tab_racing_lb_pack_voltage[TAB_RACING], "0",
      &lv_font_inter_bold_30, "V", &lv_font_inter_bold_20);
  lv_obj_align(lv_obj_get_child(lv_obj_get_child(lv_perc, 1), 0),
               LV_ALIGN_CENTER, 0, 5); // change "%" position
  lv_obj_set_grid_cell(lv_perc, LV_GRID_ALIGN_CENTER, 0, 1,
                       LV_GRID_ALIGN_CENTER, 0, 1);

  // lv state of charge bar
  racing_lv_bar = lv_bar_create(bar_panel_lv);
  custom_side_bar(racing_lv_bar);
  lv_bar_set_range(racing_lv_bar, 0, 500);
  lv_bar_set_value(racing_lv_bar, 0, LV_ANIM_OFF);

  lv_obj_set_grid_cell(racing_lv_bar, LV_GRID_ALIGN_CENTER, 0, 1,
                       LV_GRID_ALIGN_END, 1, 1);

  // lv label

  lv_obj_t *label_lv = lv_label_create(bar_panel_lv);
  lv_obj_add_style(label_lv, &label_style, LV_PART_MAIN);
  lv_obj_set_style_text_font(label_lv, &lv_font_inter_bold_22,
                             LV_STATE_DEFAULT);
  lv_label_set_text(label_lv, "HV-V");
  lv_obj_set_grid_cell(label_lv, LV_GRID_ALIGN_CENTER, 0, 1,
                       LV_GRID_ALIGN_CENTER, 2, 1);

  /*---------RIGHT BAR-----------*/

  // bar grid setup
  lv_obj_t *bar_panel_hv = lv_obj_create(tab_racing);
  lv_obj_remove_style_all(bar_panel_hv);
  lv_obj_add_style(bar_panel_hv, &grid_style, 0);
  lv_obj_set_size(bar_panel_hv, RACING_TAB_SIDE_BAR_WIDTH, SCREEN_HEIGHT);
  lv_obj_clear_flag(bar_panel_hv, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_grid_dsc_array(bar_panel_hv, bar_panel_cols,
                            bar_panel_rows); // same as LEFT BAR
  lv_obj_set_grid_cell(bar_panel_hv, LV_GRID_ALIGN_CENTER, 2, 1,
                       LV_GRID_ALIGN_CENTER, 0, 1);

  // hv percentage
  lv_obj_t *hv_perc = lv_horizontal_pair_label(
      bar_panel_hv, &tab_racing_lb_hv_current[TAB_RACING], "0",
      &lv_font_inter_bold_30, "A", &lv_font_inter_bold_20);
  lv_obj_align(lv_obj_get_child(lv_obj_get_child(hv_perc, 1), 0),
               LV_ALIGN_CENTER, 0, 5); // change "%" position
  lv_obj_set_grid_cell(hv_perc, LV_GRID_ALIGN_CENTER, 0, 1,
                       LV_GRID_ALIGN_CENTER, 0, 1);

  // hv label
  lv_obj_t *label_hv = lv_label_create(bar_panel_hv);
  lv_obj_add_style(label_hv, &label_style, LV_PART_MAIN);
  lv_obj_set_style_text_font(label_hv, &lv_font_inter_bold_22,
                             LV_STATE_DEFAULT);
  lv_label_set_text(label_hv, "HV-A");
  lv_obj_set_grid_cell(label_hv, LV_GRID_ALIGN_CENTER, 0, 1,
                       LV_GRID_ALIGN_CENTER, 2, 1);

  // hv state of charge bar
  racing_hv_bar = lv_bar_create(bar_panel_hv);
  custom_side_bar(racing_hv_bar);
  lv_bar_set_range(racing_hv_bar, 0, 40);
  lv_bar_set_value(racing_hv_bar, 0, LV_ANIM_OFF);
  lv_obj_set_style_bg_color(racing_hv_bar,
                            lv_color_hex(COLOR_ORANGE_STATUS_HEX),
                            LV_PART_INDICATOR);
  lv_obj_set_grid_cell(racing_hv_bar, LV_GRID_ALIGN_CENTER, 0, 1,
                       LV_GRID_ALIGN_CENTER, 1, 1);

  /*-------------------------------------*/
  // CENTRAL PANEL

  static lv_coord_t cpanel_cols[] = {RACING_TAB_CENTRAL_PANEL_WIDTH,
                                     LV_GRID_TEMPLATE_LAST};
  static lv_coord_t cpanel_rows[] = {
      RACING_TAB_TOP_BAR_HEIGHT, RACING_TAB_DATA_PANEL_HEIGHT,
      RACING_TAB_BOTTOM_BAR_HEIGHT, LV_GRID_TEMPLATE_LAST};

  lv_obj_t *central_panel = lv_obj_create(tab_racing);
  lv_obj_set_layout(central_panel, LV_LAYOUT_GRID);
  lv_obj_clear_flag(central_panel, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_size(central_panel, RACING_TAB_CENTRAL_PANEL_WIDTH, SCREEN_HEIGHT);

  lv_obj_add_style(central_panel, &grid_style, 0);
  lv_obj_center(central_panel);
  lv_obj_set_style_base_dir(central_panel, LV_BASE_DIR_LTR, 0);
  lv_obj_set_grid_dsc_array(central_panel, cpanel_cols, cpanel_rows);

  // NOTCH
  lv_obj_t *notch = create_notch(central_panel, TAB_RACING);
  lv_obj_align(lv_obj_get_child(notch, 0), LV_ALIGN_TOP_MID, 0, 10);
  lv_obj_set_grid_cell(notch, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_START,
                       0, 2);

  // DATA PANEL
  static lv_coord_t dpanel_cols[] = {
      RACING_TAB_DATA_LEFT_WIDTH, RACING_TAB_DATA_CENTER_WIDTH,
      RACING_TAB_DATA_RIGHT_WIDTH, LV_GRID_TEMPLATE_LAST};
  static lv_coord_t dpanel_rows[] = {RACING_TAB_DATA_PANEL_HEIGHT,
                                     LV_GRID_TEMPLATE_LAST};

  lv_obj_t *data_panel = lv_obj_create(central_panel);
  lv_obj_set_layout(data_panel, LV_LAYOUT_GRID);
  lv_obj_clear_flag(data_panel, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_size(data_panel, RACING_TAB_CENTRAL_PANEL_WIDTH,
                  RACING_TAB_DATA_PANEL_HEIGHT);

  lv_obj_add_style(data_panel, &grid_style, 0);
  lv_obj_center(data_panel);
  lv_obj_set_style_base_dir(data_panel, LV_BASE_DIR_LTR, 0);
  lv_obj_set_grid_dsc_array(data_panel, dpanel_cols, dpanel_rows);

  // DATA PANEL LEFT
  static lv_coord_t lxd_panel_cols[] = {RACING_TAB_DATA_LEFT_WIDTH - 5,
                                        LV_GRID_TEMPLATE_LAST};
  static lv_coord_t lxd_panel_rows[] = {100, 100, 100, LV_GRID_TEMPLATE_LAST};

  lv_obj_t *left_data_panel = lv_obj_create(data_panel);
  lv_obj_set_layout(left_data_panel, LV_LAYOUT_GRID);
  lv_obj_remove_style_all(left_data_panel);
  lv_obj_clear_flag(left_data_panel, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_size(left_data_panel, RACING_TAB_DATA_LEFT_WIDTH - 5,
                  RACING_TAB_DATA_PANEL_HEIGHT - 2);

  lv_obj_add_style(left_data_panel, &box_panels_style, 0);
  lv_obj_center(left_data_panel);
  lv_obj_set_style_base_dir(left_data_panel, LV_BASE_DIR_LTR, 0);
  lv_obj_set_grid_dsc_array(left_data_panel, lxd_panel_cols, lxd_panel_rows);

  lv_obj_set_grid_cell(left_data_panel, LV_GRID_ALIGN_END, 0, 1,
                       LV_GRID_ALIGN_CENTER, 0, 1);

  /* inserting data into data left panel */
  lv_obj_t *best_time = lv_vertical_pair_label(
      left_data_panel, &lb_best_time[TAB_RACING], "0:00:00",
      &lv_font_inter_bold_38, "BEST TIME", &lv_font_inter_bold_22);
  lv_obj_align(lv_obj_get_child(lv_obj_get_child(best_time, 1), 0),
               LV_ALIGN_LEFT_MID, 10, 0); // change bottom label position
  lv_obj_align(lv_obj_get_child(lv_obj_get_child(best_time, 0), 0),
               LV_ALIGN_LEFT_MID, 5, 0); // change bottom label position
  lv_obj_set_grid_cell(best_time, LV_GRID_ALIGN_START, 0, 1,
                       LV_GRID_ALIGN_CENTER, 0, 1);

  lv_obj_t *last_time = lv_vertical_pair_label(
      left_data_panel, &lb_last_time[TAB_RACING], "0:00:00",
      &lv_font_inter_bold_38, "LAST TIME", &lv_font_inter_bold_22);
  lv_obj_align(lv_obj_get_child(lv_obj_get_child(last_time, 1), 0),
               LV_ALIGN_LEFT_MID, 10, 0); // change bottom label position
  lv_obj_align(lv_obj_get_child(lv_obj_get_child(last_time, 0), 0),
               LV_ALIGN_LEFT_MID, 5, 0); // change bottom label position
  lv_obj_set_grid_cell(last_time, LV_GRID_ALIGN_START, 0, 1,
                       LV_GRID_ALIGN_CENTER, 1, 1);

  lv_obj_t *delta = lv_vertical_pair_label(
      left_data_panel, &lb_delta_time[TAB_RACING], "NA",
      &lv_font_inter_bold_38, "DELTA", &lv_font_inter_bold_22);
  lv_obj_align(lv_obj_get_child(lv_obj_get_child(delta, 0), 0),
               LV_ALIGN_LEFT_MID, 10, 0); // change upper label position
  lv_obj_align(lv_obj_get_child(lv_obj_get_child(delta, 1), 0),
               LV_ALIGN_LEFT_MID, 10, 0); // change bottom label position
  lv_obj_set_grid_cell(delta, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_CENTER,
                       2, 1);

  // DATA RIGHT PANEL
  static lv_coord_t rxd_panel_cols[] = {90, 90, LV_GRID_TEMPLATE_LAST};
  static lv_coord_t rxd_panel_rows[] = {100, 100, 100, LV_GRID_TEMPLATE_LAST};

  lv_obj_t *right_data_panel = lv_obj_create(data_panel);
  lv_obj_remove_style_all(right_data_panel);

  lv_obj_set_layout(right_data_panel, LV_LAYOUT_GRID);
  lv_obj_clear_flag(right_data_panel, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_size(right_data_panel, RACING_TAB_DATA_RIGHT_WIDTH - 5,
                  RACING_TAB_DATA_PANEL_HEIGHT - 2);

  lv_obj_set_style_pad_left(right_data_panel, 1, LV_PART_MAIN);
  lv_obj_set_style_border_opa(right_data_panel, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_add_style(right_data_panel, &box_panels_style, 0);
  // lv_obj_center(right_data_panel);
  lv_obj_set_style_base_dir(right_data_panel, LV_BASE_DIR_LTR, 0);
  lv_obj_set_grid_dsc_array(right_data_panel, rxd_panel_cols, rxd_panel_rows);

  lv_obj_set_grid_cell(right_data_panel, LV_GRID_ALIGN_START, 2, 1,
                       LV_GRID_ALIGN_CENTER, 0, 1);

  /*inserting data into data right panel*/
  lv_obj_t *trq = lv_vertical_pair_label(
      right_data_panel, &tab_racing_lb_torque[TAB_RACING], "NA",
      &lv_font_inter_bold_38, "TRQ", &lv_font_inter_bold_22);
  lv_obj_set_grid_cell(trq, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 0,
                       1);

  lv_obj_t *slip = lv_vertical_pair_label(
      right_data_panel, &lb_slip[TAB_RACING], "NA",
      &lv_font_inter_bold_38, "SLIP", &lv_font_inter_bold_22);
  lv_obj_set_grid_cell(slip, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER,
                       0, 1);

  // adding separation line for right data panel
  lv_obj_t *sep_line = lv_obj_create(right_data_panel);
  lv_obj_remove_style_all(sep_line);
  lv_obj_set_style_bg_color(sep_line, lv_color_hex(COLOR_SECONDARY_HEX),
                            LV_PART_MAIN);
  lv_obj_set_style_bg_opa(sep_line, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_set_size(sep_line, 185, 3);
  lv_obj_set_grid_cell(sep_line, LV_GRID_ALIGN_CENTER, 0, 2,
                       LV_GRID_ALIGN_START, 1, 1);

  lv_obj_t *inverter_temp = lv_triple_label(
      right_data_panel, &tab_racing_lb_left_inverter_temp[TAB_RACING], "0",
      &lv_font_inter_bold_38, "°C", &lv_font_inter_bold_22, "INV",
      &lv_font_inter_bold_20);
  lv_obj_set_grid_cell(inverter_temp, LV_GRID_ALIGN_CENTER, 0, 1,
                       LV_GRID_ALIGN_CENTER, 1, 1);

  lv_obj_t *motor_temp = lv_triple_label(
      right_data_panel, &tab_racing_lb_left_motor_temp[TAB_RACING], "0",
      &lv_font_inter_bold_38, "°C", &lv_font_inter_bold_22, "MOTOR",
      &lv_font_inter_bold_20);
  lv_obj_set_grid_cell(motor_temp, LV_GRID_ALIGN_CENTER, 1, 1,
                       LV_GRID_ALIGN_CENTER, 1, 1);

  lv_obj_t *lv_temp = lv_triple_label(
      right_data_panel, &tab_racing_lb_battery_temperature[TAB_RACING], "0",
      &lv_font_inter_bold_38, "°C", &lv_font_inter_bold_22, "LV",
      &lv_font_inter_bold_20);
  lv_obj_set_grid_cell(lv_temp, LV_GRID_ALIGN_CENTER, 0, 1,
                       LV_GRID_ALIGN_CENTER, 2, 1);

  lv_obj_t *hv_temp = lv_triple_label(
      right_data_panel, &tab_racing_lb_average_temperature[TAB_RACING], "0",
      &lv_font_inter_bold_38, "°C", &lv_font_inter_bold_22, "HV",
      &lv_font_inter_bold_20);
  lv_obj_set_grid_cell(hv_temp, LV_GRID_ALIGN_CENTER, 1, 1,
                       LV_GRID_ALIGN_CENTER, 2, 1);

  // DATA CENTER

  // power
  lv_obj_t *power = lv_horizontal_pair_label(
      central_panel, &tab_racing_lb_power[TAB_RACING], "0",
      &lv_font_inter_bold_38, " POWER", &lv_font_inter_bold_22);

  lv_obj_set_grid_cell(lv_obj_get_child(power, 1), LV_GRID_ALIGN_CENTER, 1, 1,
                       LV_GRID_ALIGN_CENTER, 0, 1); // center the label "POWER"

  lv_obj_set_grid_cell(power, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_START,
                       0, 2);
  // lv_obj_set_grid_cell(lv_obj_get_child(power, 1), LV_GRID_ALIGN_CENTER, 1,
  // 1, LV_GRID_ALIGN_CENTER, 0, 1);
  // lv_obj_set_style_pad_bottom(lv_obj_get_child(power, 0), -1, 0);
  lv_obj_set_style_pad_top(power, 65, 0);

  // lv_obj_align(power, LV_ALIGN_CENTER, 0,0);

  // speedometer
  lv_obj_t *speed = lv_vertical_pair_two_labels(
      data_panel, &tab_racing_lb_speed[TAB_RACING], "IDLE",
      &lv_font_inter_bold_38, &bottom_lb_speed, "-",
      &lv_font_inter_bold_22);
  lv_obj_set_grid_cell(speed, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER,
                       0, 1);

  lv_obj_t *meter_container = lv_obj_create(data_panel);
  lv_obj_remove_style_all(meter_container);
  lv_obj_set_height(meter_container, LV_SIZE_CONTENT);

  custom_meter = lv_meter_create(meter_container);

  // TODO fix this warning!!!
  lv_custom_meter(&custom_meter, &indicator_blue,
                  &indicator_white);
  lv_obj_align(custom_meter, LV_ALIGN_CENTER, 0, 0);

  lv_obj_set_grid_cell(meter_container, LV_GRID_ALIGN_STRETCH, 1, 1,
                       LV_GRID_ALIGN_CENTER, 0, 1);

  // lap counter

  lv_obj_t *lap_counter = lv_vertical_pair_label(
      data_panel, &lb_lap_count[TAB_RACING], "NA",
      &lv_font_inter_bold_38, "LAP", &lv_font_inter_bold_22);
  lv_obj_set_grid_cell(lap_counter, LV_GRID_ALIGN_START, 1, 1,
                       LV_GRID_ALIGN_END, 0, 1);
  lv_obj_set_style_pad_bottom(lap_counter, 5, 0);
  lv_obj_set_style_pad_left(lap_counter, 40, 0);

  // ???
  lv_obj_t *km_counter = lv_vertical_pair_label(
      data_panel, &lb_lap_count[TAB_RACING], "0",
      &lv_font_inter_bold_38, "KM", &lv_font_inter_bold_22);
  lv_obj_set_grid_cell(km_counter, LV_GRID_ALIGN_END, 1, 1, LV_GRID_ALIGN_END,
                       0, 1);
  lv_obj_set_style_pad_bottom(km_counter, 5, 0);
  lv_obj_set_style_pad_right(km_counter, 40, 0);

  // setting data panel position in central panel
  lv_obj_set_grid_cell(data_panel, LV_GRID_ALIGN_STRETCH, 0, 1,
                       LV_GRID_ALIGN_STRETCH, 1, 1);

  // BOTTOM BAR
  bottom_bar = lv_bar_create(central_panel);
  lv_obj_remove_style_all(bottom_bar);
  // lv_obj_add_style(bottom_bar, &bar_lv_style, LV_PART_INDICATOR);
  lv_obj_add_style(bottom_bar, &bar_back_style, LV_PART_MAIN);
  lv_obj_set_size(bottom_bar, 600, RACING_TAB_BOTTOM_BAR_HEIGHT - 15);
  lv_obj_center(bottom_bar);
  lv_obj_set_style_radius(bottom_bar, 6, LV_PART_MAIN);
  lv_obj_set_grid_cell(bottom_bar, LV_GRID_ALIGN_CENTER, 0, 1,
                       LV_GRID_ALIGN_END, 2, 1);

  // setting central panel position in tab racing
  lv_obj_set_grid_cell(central_panel, LV_GRID_ALIGN_STRETCH, 1, 1,
                       LV_GRID_ALIGN_STRETCH, 0, 1);
}

void init_racing_style(void) {
  /* BAR_HV_STYLE */
  lv_style_init(&bar_hv_style);
  lv_style_set_bg_opa(&bar_hv_style, LV_OPA_COVER);
  lv_style_set_bg_color(&bar_hv_style, lv_color_hex(COLOR_ORANGE_STATUS_HEX));

  /* BAR_LV_STYLE */
  lv_style_init(&bar_lv_style);
  lv_style_set_bg_opa(&bar_lv_style, LV_OPA_COVER);
  lv_style_set_bg_color(&bar_lv_style, lv_color_hex(COLOR_GREEN_STATUS_HEX));

  /* BAR_BACKGROUND_STYLE */
  lv_style_init(&bar_back_style);
  // lv_style_set_bg_opa(&bar_back_style, LV_OPA_COVER);
  lv_style_set_bg_opa(&bar_back_style, LV_OPA_COVER);
  lv_style_set_bg_color(&bar_back_style, lv_color_hex(COLOR_SECONDARY_HEX));
}

//shadow ? of custom meter and indicator blue and white
void lv_custom_meter(lv_obj_t **custom_meter,
                     lv_meter_indicator_t **indicator_blue,
                     lv_meter_indicator_t **indicator_white) {
  /*Remove the circle from the middle*/
  lv_obj_remove_style(*custom_meter, NULL, LV_PART_MAIN);
  lv_obj_remove_style(*custom_meter, NULL, LV_PART_INDICATOR);

  // lv_obj_set_grid_cell(*custom_meter, LV_GRID_ALIGN_STRETCH, 2, 1,
  // LV_GRID_ALIGN_STRETCH, 2, 1);
  lv_obj_set_size(*custom_meter, 280, 290);
  lv_obj_set_style_pad_top(*custom_meter, 12, LV_PART_MAIN);

  /*Add a scale first*/
  lv_meter_scale_t *scale = lv_meter_add_scale(*custom_meter);
  // lv_obj_align(*custom_meter,  )
  // lv_meter_set_scale_ticks(*custom_meter, scale, 11, 2, 10,
  // lv_palette_main(LV_PALETTE_GREY));
  lv_meter_set_scale_ticks(*custom_meter, scale, 0, 0, 0,
                           lv_color_black()); /*to remove spedometer ticks*/
  // lv_meter_set_scale_major_ticks(*custom_meter, scale, 1, 2, 30,
  // lv_color_hex3(0xeee), 10); lv_meter_set_scale_range(*custom_meter, scale,
  // 60, 100, 270, 90);

  lv_meter_set_scale_range(*custom_meter, scale, 0, 100, 240, 150);

  /*Add a three arc indicator*/

  lv_meter_indicator_t *back = lv_meter_add_arc(
      *custom_meter, scale, 22, lv_color_hex(COLOR_SECONDARY_HEX), -5);
  *indicator_white = lv_meter_add_arc(*custom_meter, scale, 16,
                                      lv_color_hex(COLOR_TERTIARY_HEX), -8);
  *indicator_blue = lv_meter_add_arc(*custom_meter, scale, 18,
                                     lv_color_hex(COLOR_BLUE_STATUS_HEX), -7);

  lv_meter_set_indicator_start_value(*custom_meter, back, 0);
  lv_meter_set_indicator_end_value(*custom_meter, back, 100);

  lv_meter_set_indicator_start_value(*custom_meter, *indicator_white, 0);
  lv_meter_set_indicator_end_value(*custom_meter, *indicator_white,
                                   100); /*range values 0-100*/

  lv_meter_set_indicator_start_value(*custom_meter, *indicator_blue, 0);
  lv_meter_set_indicator_end_value(*custom_meter, *indicator_blue,
                                   100); /*range values 0-100*/

  lv_meter_set_indicator_value(*custom_meter, *indicator_white, 0);
  lv_meter_set_indicator_value(*custom_meter, *indicator_blue, 0);

  // lv_meter_indicator_t * indic3 = lv_meter_add_arc(custom_meter, scale, 10,
  // lv_palette_main(LV_PALETTE_BLUE), 0);

  // lv_coord_t indic_w = 0;

  // return custom_meter;
}

void custom_side_bar(lv_obj_t *bar) {
  lv_obj_remove_style_all(bar);
  lv_obj_add_style(bar, &bar_lv_style, LV_PART_INDICATOR);
  lv_obj_add_style(bar, &bar_back_style, LV_PART_MAIN);
  lv_obj_set_size(bar, 66, 360);
  lv_obj_center(bar);

  /* adding levels for side bars */
  lv_obj_t *rect1 = lv_bar_create(bar);
  lv_obj_add_style(rect1, &grid_style, LV_PART_MAIN);
  lv_obj_set_size(rect1, 100, 5);
  lv_obj_align(rect1, LV_ALIGN_CENTER, 0, 36);

  lv_obj_t *rect2 = lv_bar_create(bar);
  lv_obj_add_style(rect2, &grid_style, LV_PART_MAIN);
  lv_obj_set_size(rect2, 100, 5);
  lv_obj_align(rect2, LV_ALIGN_CENTER, 0, -36);

  lv_obj_t *rect3 = lv_bar_create(bar);
  lv_obj_add_style(rect3, &grid_style, LV_PART_MAIN);
  lv_obj_set_size(rect3, 100, 5);
  lv_obj_align(rect3, LV_ALIGN_CENTER, 0, 108);

  lv_obj_t *rect4 = lv_bar_create(bar);
  lv_obj_add_style(rect4, &grid_style, LV_PART_MAIN);
  lv_obj_set_size(rect4, 100, 5);
  lv_obj_align(rect4, LV_ALIGN_CENTER, 0, -108);
}

void racing_error_notification(char *msg) {
  lv_obj_t *label_err = lv_label_create(bottom_bar);
  lv_obj_remove_style_all(label_err);
  lv_obj_add_style(label_err, &label_style, LV_PART_MAIN);
  lv_obj_align(label_err, LV_ALIGN_CENTER, 0, 0);
  lv_label_set_text(label_err, msg);
  lv_obj_set_style_text_color(label_err, lv_color_hex(COLOR_PRIMARY_HEX),
                              LV_PART_MAIN);
  lv_obj_set_style_text_font(label_err, &lv_font_inter_bold_30, LV_PART_MAIN);
  lv_obj_set_style_bg_opa(label_err, LV_OPA_COVER, LV_PART_MAIN);
}
