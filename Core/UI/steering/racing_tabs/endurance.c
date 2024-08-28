#include "endurance.h"

#ifdef ENDURANCE_MODE_ENABLED

lv_obj_t *endurance_screen_labels[endurance_screen_n_labels];
lv_obj_t *endurance_screen_labels_aux[endurance_screen_n_labels];
lv_obj_t *endurance_screen_objects[endurance_screen_n_labels];
lv_style_t endurance_screen_objects_styles[endurance_screen_n_labels];
bool endurance_screen_first_changed[endurance_screen_n_labels] = {0};
uint32_t endurance_screen_colors[endurance_screen_n_labels];

static lv_style_t label_style_small;
static lv_style_t label_style_large;

void endurance_screen_set_label(const char *text, endurance_screen_labels_idx_t label_idx) {
    lv_label_set_text(endurance_screen_labels[label_idx], text);
}

void endurance_screen_set_color(uint32_t fg_color_hex, uint32_t bg_color_hex, endurance_screen_labels_idx_t label_idx) {
    if (!endurance_screen_first_changed[label_idx] || endurance_screen_colors[label_idx] != bg_color_hex) {
        endurance_screen_first_changed[label_idx] = true;
        endurance_screen_colors[label_idx] = bg_color_hex;
        lv_obj_set_style_text_color(endurance_screen_labels[label_idx], lv_color_hex(fg_color_hex), LV_PART_MAIN);
        lv_obj_set_style_text_color(endurance_screen_labels_aux[label_idx], lv_color_hex(fg_color_hex), LV_PART_MAIN);

        lv_style_set_bg_color(&endurance_screen_objects_styles[label_idx], lv_color_hex(bg_color_hex));
        lv_style_set_bg_opa(&endurance_screen_objects_styles[label_idx], LV_OPA_COVER);
        lv_obj_refresh_style(endurance_screen_objects[label_idx], LV_PART_MAIN, LV_STYLE_PROP_ANY);
    }
}

void endurance_screen_init_styles() {
    lv_style_init(&label_style_small);
    lv_style_set_text_color(&label_style_small, lv_color_hex(COLOR_BLACK_STATUS_HEX));
    lv_style_set_text_font(&label_style_small, &lv_font_inter_bold_38);

    lv_style_init(&label_style_large);
    lv_style_set_text_color(&label_style_large, lv_color_hex(COLOR_BLACK_STATUS_HEX));
    lv_style_set_text_font(&label_style_large, &lv_font_inter_bold_70);
}

void endurance_screen_create(lv_obj_t *parent) {
    endurance_screen_init_styles();

    static lv_coord_t main_panel_cols[] = {4, 264, 264, 264, 4, LV_GRID_TEMPLATE_LAST};
    static lv_coord_t main_panel_rows[] = {90, 130, 130, 130, LV_GRID_TEMPLATE_LAST};

    lv_obj_t *endurance_screen = lv_obj_create(parent);
    lv_obj_set_style_bg_color(parent, lv_color_hex(COLOR_BLACK_STATUS_HEX), LV_PART_MAIN);
    lv_obj_set_layout(endurance_screen, LV_LAYOUT_GRID);
    lv_obj_set_size(endurance_screen, SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_obj_clear_flag(endurance_screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_center(endurance_screen);

    lv_obj_set_layout(endurance_screen, LV_LAYOUT_GRID);
    lv_obj_remove_style_all(endurance_screen);
    lv_obj_clear_flag(endurance_screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(endurance_screen, 800, 480);

    lv_obj_add_style(endurance_screen, &box_panels_style, 0);
    lv_obj_center(endurance_screen);
    lv_obj_set_style_base_dir(endurance_screen, LV_BASE_DIR_LTR, 0);

    lv_obj_set_grid_dsc_array(endurance_screen, main_panel_cols, main_panel_rows);

    lv_obj_t *cobj;
    lv_obj_t *clabel, *clabel_aux;
    lv_obj_t *description;

    for (size_t ilabel = 0; ilabel < endurance_screen_n_labels; ilabel++) {
        lv_style_init(&endurance_screen_objects_styles[ilabel]);
        lv_style_set_bg_color(&endurance_screen_objects_styles[ilabel], lv_color_hex(COLOR_PRIMARY_HEX));
        lv_style_set_bg_opa(&endurance_screen_objects_styles[ilabel], LV_OPA_COVER);
    }

    cobj = lv_horizontal_pair_label(endurance_screen, &clabel, &clabel_aux, "NA", &lv_font_inter_bold_38, "", &lv_font_inter_bold_38);
    lv_obj_set_grid_cell(cobj, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
    lv_obj_add_style(cobj, &endurance_screen_objects_styles[shutdown_status_idx], LV_PART_MAIN);
    endurance_screen_objects[shutdown_status_idx] = cobj;
    endurance_screen_labels[shutdown_status_idx]  = clabel;
    endurance_screen_labels_aux[shutdown_status_idx]  = clabel_aux;

    cobj = lv_horizontal_pair_label(endurance_screen, &clabel, &clabel_aux, "NA", &lv_font_inter_bold_70, "", &lv_font_inter_bold_38);
    lv_obj_set_grid_cell(cobj, LV_GRID_ALIGN_STRETCH, 2, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
    lv_obj_add_style(cobj, &endurance_screen_objects_styles[car_status_idx], LV_PART_MAIN);
    endurance_screen_objects[car_status_idx] = cobj;
    endurance_screen_labels[car_status_idx]  = clabel;
    endurance_screen_labels_aux[car_status_idx]  = clabel_aux;

    cobj = lv_horizontal_pair_label(endurance_screen, &clabel, &clabel_aux, "OFF", &lv_font_inter_bold_70, "", &lv_font_inter_bold_38);
    lv_obj_set_grid_cell(cobj, LV_GRID_ALIGN_STRETCH, 3, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
    lv_obj_add_style(cobj, &endurance_screen_objects_styles[ptt_status_idx], LV_PART_MAIN);
    endurance_screen_objects[ptt_status_idx] = cobj;
    endurance_screen_labels[ptt_status_idx]  = clabel;
    endurance_screen_labels_aux[ptt_status_idx]  = clabel_aux;

    cobj = lv_horizontal_pair_label(endurance_screen, &clabel, &clabel_aux, "NA", &lv_font_inter_bold_90, "MOT", &lv_font_inter_bold_38);
    lv_obj_set_grid_cell(cobj, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 1, 1);
    lv_obj_add_style(cobj, &endurance_screen_objects_styles[motor_temp_idx], LV_PART_MAIN);
    endurance_screen_objects[motor_temp_idx] = cobj;
    endurance_screen_labels[motor_temp_idx]  = clabel;
    endurance_screen_labels_aux[motor_temp_idx]  = clabel_aux;

    cobj = lv_vertical_pair_two_labels(endurance_screen, &clabel, "NA", &lv_font_inter_bold_200, &description, "HV", &lv_font_inter_bold_70);
    lv_obj_set_grid_cell(cobj, LV_GRID_ALIGN_STRETCH, 2, 1, LV_GRID_ALIGN_STRETCH, 1, 2);
    lv_obj_add_style(cobj, &endurance_screen_objects_styles[hv_soc_idx], LV_PART_MAIN);
    endurance_screen_objects[hv_soc_idx] = cobj;
    endurance_screen_labels[hv_soc_idx]  = clabel;
    endurance_screen_labels_aux[hv_soc_idx]  = description;

    cobj = lv_horizontal_pair_label(endurance_screen, &clabel, &clabel_aux, "NA", &lv_font_inter_bold_90, "LV", &lv_font_inter_bold_38);
    lv_obj_set_grid_cell(cobj, LV_GRID_ALIGN_STRETCH, 3, 1, LV_GRID_ALIGN_STRETCH, 1, 1);
    lv_obj_add_style(cobj, &endurance_screen_objects_styles[lv_soc_idx], LV_PART_MAIN);
    endurance_screen_objects[lv_soc_idx] = cobj;
    endurance_screen_labels[lv_soc_idx]  = clabel;
    endurance_screen_labels_aux[lv_soc_idx]  = clabel_aux;

    cobj = lv_horizontal_pair_label(endurance_screen, &clabel, &clabel_aux, "NA", &lv_font_inter_bold_90, "INVT", &lv_font_inter_bold_38);
    lv_obj_set_grid_cell(cobj, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 2, 1);
    lv_obj_add_style(cobj, &endurance_screen_objects_styles[inverter_temp_idx], LV_PART_MAIN);
    endurance_screen_objects[inverter_temp_idx] = cobj;
    endurance_screen_labels[inverter_temp_idx]  = clabel;
    endurance_screen_labels_aux[inverter_temp_idx]  = clabel_aux;

    cobj = lv_horizontal_pair_label(endurance_screen, &clabel, &clabel_aux, "NA", &lv_font_inter_bold_90, "PTC", &lv_font_inter_bold_38);
    lv_obj_set_grid_cell(cobj, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 3, 1);
    lv_obj_add_style(cobj, &endurance_screen_objects_styles[pt_cooling_idx], LV_PART_MAIN);
    endurance_screen_objects[pt_cooling_idx] = cobj;
    endurance_screen_labels[pt_cooling_idx]  = clabel;
    endurance_screen_labels_aux[pt_cooling_idx]  = clabel_aux;

    cobj = lv_horizontal_pair_label(endurance_screen, &clabel, &clabel_aux, "NA", &lv_font_inter_bold_90, "HVT", &lv_font_inter_bold_38);
    lv_obj_set_grid_cell(cobj, LV_GRID_ALIGN_STRETCH, 3, 1, LV_GRID_ALIGN_STRETCH, 2, 1);
    lv_obj_add_style(cobj, &endurance_screen_objects_styles[hv_temp_idx], LV_PART_MAIN);
    endurance_screen_objects[hv_temp_idx] = cobj;
    endurance_screen_labels[hv_temp_idx]  = clabel;
    endurance_screen_labels_aux[hv_temp_idx]  = clabel_aux;

    cobj = lv_horizontal_pair_label(endurance_screen, &clabel, &clabel_aux, "NA", &lv_font_inter_bold_90, "HVC", &lv_font_inter_bold_38);
    lv_obj_set_grid_cell(cobj, LV_GRID_ALIGN_STRETCH, 3, 1, LV_GRID_ALIGN_STRETCH, 3, 1);
    lv_obj_add_style(cobj, &endurance_screen_objects_styles[hv_cooling_idx], LV_PART_MAIN);
    endurance_screen_objects[hv_cooling_idx] = cobj;
    endurance_screen_labels[hv_cooling_idx]  = clabel;
    endurance_screen_labels_aux[hv_cooling_idx]  = clabel_aux;

    static lv_coord_t controls_cols[] = {110, 44, 110, LV_GRID_TEMPLATE_LAST};
    static lv_coord_t controls_rows[] = {130, LV_GRID_TEMPLATE_LAST};

    lv_obj_t *container = lv_obj_create(endurance_screen);
    lv_obj_set_style_bg_color(endurance_screen, lv_color_hex(COLOR_BLACK_STATUS_HEX), LV_PART_MAIN);
    lv_obj_set_layout(container, LV_LAYOUT_GRID);
    lv_obj_set_size(container, 264, 130);
    lv_obj_clear_flag(container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_center(container);

    lv_obj_set_layout(container, LV_LAYOUT_GRID);
    lv_obj_remove_style_all(container);
    lv_obj_clear_flag(container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(container, 264, 130);

    lv_obj_add_style(container, &box_panels_style, 0);
    lv_obj_center(container);
    lv_obj_set_style_base_dir(container, LV_BASE_DIR_LTR, 0);

    lv_obj_set_grid_dsc_array(container, controls_cols, controls_rows);
    lv_obj_set_grid_cell(container, LV_GRID_ALIGN_STRETCH, 2, 1, LV_GRID_ALIGN_STRETCH, 3, 1);

    cobj = lv_vertical_pair_label(container, &clabel, &clabel_aux, "", &lv_font_inter_bold_38, "SLIP", &lv_font_inter_bold_22);
    lv_obj_set_grid_cell(cobj, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
    lv_obj_add_style(cobj, &endurance_screen_objects_styles[slip_map_idx], LV_PART_MAIN);
    endurance_screen_objects[slip_map_idx] = cobj;
    endurance_screen_labels[slip_map_idx]  = clabel;
    endurance_screen_labels_aux[slip_map_idx]  = clabel_aux;

    cobj = lv_horizontal_pair_label(container, &clabel, &clabel_aux, "", &lv_font_inter_bold_38, "", &lv_font_inter_bold_22);
    lv_obj_set_grid_cell(cobj, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
    lv_obj_add_style(cobj, &endurance_screen_objects_styles[regen_on_off_idx], LV_PART_MAIN);
    endurance_screen_objects[regen_on_off_idx] = cobj;
    endurance_screen_labels[regen_on_off_idx]  = clabel;
    endurance_screen_labels_aux[regen_on_off_idx]  = clabel_aux;

    cobj = lv_vertical_pair_label(container, &clabel, &clabel_aux, "", &lv_font_inter_bold_38, "TORQUE", &lv_font_inter_bold_22);
    lv_obj_set_grid_cell(cobj, LV_GRID_ALIGN_STRETCH, 2, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
    lv_obj_add_style(cobj, &endurance_screen_objects_styles[torque_map_idx], LV_PART_MAIN);
    endurance_screen_objects[torque_map_idx] = cobj;
    endurance_screen_labels[torque_map_idx]  = clabel;
    endurance_screen_labels_aux[torque_map_idx]  = clabel_aux;

    lv_scr_load(endurance_screen);
}

#endif  // ENDURANCE_MODE_ENABLED
