#include "endurance.h"

#ifdef ENDURANCE_MODE_ENABLED

lv_obj_t *endurance_screen_labels[endurance_screen_n_labels];
lv_obj_t *endurance_screen_objects[endurance_screen_n_labels];
lv_style_t endurance_screen_objects_styles[endurance_screen_n_labels];
static lv_style_t label_style_small;
static lv_style_t label_style_large;

void endurance_screen_set_label(const char *text, endurance_screen_labels_idx_t label_idx) {
    lv_label_set_text(endurance_screen_labels[label_idx], text);
}

void endurance_screen_set_color(uint32_t color_hex, endurance_screen_labels_idx_t label_idx) {
    lv_style_set_bg_color(&endurance_screen_objects_styles[label_idx], lv_color_hex(color_hex));
    lv_style_set_bg_opa(&endurance_screen_objects_styles[label_idx], LV_OPA_COVER);
    lv_obj_refresh_style(endurance_screen_objects[label_idx], LV_PART_MAIN, LV_STYLE_PROP_ANY);
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
    lv_obj_t *clabel;
    lv_obj_t *description;

    for (size_t ilabel = 0; ilabel < endurance_screen_n_labels; ilabel++) {
        lv_style_init(&endurance_screen_objects_styles[ilabel]);
        lv_style_set_bg_color(&endurance_screen_objects_styles[ilabel], lv_color_hex(COLOR_PRIMARY_HEX));
        lv_style_set_bg_opa(&endurance_screen_objects_styles[ilabel], LV_OPA_COVER);
    }

    cobj = lv_horizontal_pair_label(endurance_screen, &clabel, "NA", &lv_font_inter_bold_22, "", &lv_font_inter_bold_14);
    lv_obj_set_grid_cell(cobj, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
    lv_obj_add_style(cobj, &endurance_screen_objects_styles[shutdown_status_idx], LV_PART_MAIN);
    endurance_screen_objects[shutdown_status_idx] = cobj;
    endurance_screen_labels[shutdown_status_idx]  = clabel;

    cobj = lv_horizontal_pair_label(endurance_screen, &clabel, "NA", &lv_font_inter_bold_70, "", &lv_font_inter_bold_14);
    lv_obj_set_grid_cell(cobj, LV_GRID_ALIGN_STRETCH, 2, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
    lv_obj_add_style(cobj, &endurance_screen_objects_styles[car_status_idx], LV_PART_MAIN);
    endurance_screen_objects[car_status_idx] = cobj;
    endurance_screen_labels[car_status_idx]  = clabel;

    cobj = lv_horizontal_pair_label(endurance_screen, &clabel, "OFF", &lv_font_inter_bold_70, "", &lv_font_inter_bold_14);
    lv_obj_set_grid_cell(cobj, LV_GRID_ALIGN_STRETCH, 3, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
    lv_obj_add_style(cobj, &endurance_screen_objects_styles[ptt_status_idx], LV_PART_MAIN);
    endurance_screen_objects[ptt_status_idx] = cobj;
    endurance_screen_labels[ptt_status_idx]  = clabel;

    cobj = lv_horizontal_pair_label(endurance_screen, &clabel, "NA", &lv_font_inter_bold_90, "MOTOR\nTEMPERATURE", &lv_font_inter_bold_14);
    lv_obj_set_grid_cell(cobj, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 1, 1);
    lv_obj_add_style(cobj, &endurance_screen_objects_styles[motor_temp_idx], LV_PART_MAIN);
    endurance_screen_objects[motor_temp_idx] = cobj;
    endurance_screen_labels[motor_temp_idx]  = clabel;

    cobj = lv_vertical_pair_two_labels(endurance_screen, &clabel, "NA", &lv_font_inter_bold_200, &description, "HV SOC", &lv_font_inter_bold_14);
    lv_obj_set_grid_cell(cobj, LV_GRID_ALIGN_STRETCH, 2, 1, LV_GRID_ALIGN_STRETCH, 1, 2);
    lv_obj_add_style(cobj, &endurance_screen_objects_styles[hv_soc_idx], LV_PART_MAIN);
    endurance_screen_objects[hv_soc_idx] = cobj;
    endurance_screen_labels[hv_soc_idx]  = clabel;

    cobj = lv_horizontal_pair_label(endurance_screen, &clabel, "NA", &lv_font_inter_bold_90, "LV SOC", &lv_font_inter_bold_14);
    lv_obj_set_grid_cell(cobj, LV_GRID_ALIGN_STRETCH, 3, 1, LV_GRID_ALIGN_STRETCH, 1, 1);
    lv_obj_add_style(cobj, &endurance_screen_objects_styles[lv_soc_idx], LV_PART_MAIN);
    endurance_screen_objects[lv_soc_idx] = cobj;
    endurance_screen_labels[lv_soc_idx]  = clabel;

    cobj = lv_horizontal_pair_label(endurance_screen, &clabel, "NA", &lv_font_inter_bold_90, "INVERTER\nTEMPERATURE", &lv_font_inter_bold_14);
    lv_obj_set_grid_cell(cobj, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 2, 1);
    lv_obj_add_style(cobj, &endurance_screen_objects_styles[inverter_temp_idx], LV_PART_MAIN);
    endurance_screen_objects[inverter_temp_idx] = cobj;
    endurance_screen_labels[inverter_temp_idx]  = clabel;

    cobj = lv_horizontal_pair_label(endurance_screen, &clabel, "NA", &lv_font_inter_bold_90, "PT\nCOOLING", &lv_font_inter_bold_14);
    lv_obj_set_grid_cell(cobj, LV_GRID_ALIGN_STRETCH, 3, 1, LV_GRID_ALIGN_STRETCH, 2, 1);
    lv_obj_add_style(cobj, &endurance_screen_objects_styles[pt_cooling_idx], LV_PART_MAIN);
    endurance_screen_objects[pt_cooling_idx] = cobj;
    endurance_screen_labels[pt_cooling_idx]  = clabel;

    cobj = lv_horizontal_pair_label(endurance_screen, &clabel, "NA", &lv_font_inter_bold_90, "HV\nTEMPERATURE", &lv_font_inter_bold_14);
    lv_obj_set_grid_cell(cobj, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 3, 1);
    lv_obj_add_style(cobj, &endurance_screen_objects_styles[hv_temp_idx], LV_PART_MAIN);
    endurance_screen_objects[hv_temp_idx] = cobj;
    endurance_screen_labels[hv_temp_idx]  = clabel;

    cobj = lv_horizontal_pair_label(endurance_screen, &clabel, "NA", &lv_font_inter_bold_90, "HV\nCOOLING", &lv_font_inter_bold_14);
    lv_obj_set_grid_cell(cobj, LV_GRID_ALIGN_STRETCH, 3, 1, LV_GRID_ALIGN_STRETCH, 3, 1);
    lv_obj_add_style(cobj, &endurance_screen_objects_styles[hv_cooling_idx], LV_PART_MAIN);
    endurance_screen_objects[hv_cooling_idx] = cobj;
    endurance_screen_labels[hv_cooling_idx]  = clabel;

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

    cobj = lv_vertical_pair_label(container, &clabel, "NA", &lv_font_inter_bold_38, "SL", &lv_font_inter_bold_14);
    lv_obj_set_grid_cell(cobj, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
    lv_obj_add_style(cobj, &endurance_screen_objects_styles[slip_map_idx], LV_PART_MAIN);
    endurance_screen_objects[slip_map_idx] = cobj;
    endurance_screen_labels[slip_map_idx]  = clabel;

    cobj = lv_horizontal_pair_label(container, &clabel, "", &lv_font_inter_bold_38, "", &lv_font_inter_bold_14);
    lv_obj_set_grid_cell(cobj, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
    lv_obj_add_style(cobj, &endurance_screen_objects_styles[regen_on_off_idx], LV_PART_MAIN);
    endurance_screen_objects[regen_on_off_idx] = cobj;
    endurance_screen_labels[regen_on_off_idx]  = clabel;

    cobj = lv_vertical_pair_label(container, &clabel, "NA", &lv_font_inter_bold_38, "TQ", &lv_font_inter_bold_14);
    lv_obj_set_grid_cell(cobj, LV_GRID_ALIGN_STRETCH, 2, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
    lv_obj_add_style(cobj, &endurance_screen_objects_styles[torque_map_idx], LV_PART_MAIN);
    endurance_screen_objects[torque_map_idx] = cobj;
    endurance_screen_labels[torque_map_idx]  = clabel;

    lv_scr_load(endurance_screen);
}

#endif  // ENDURANCE_MODE_ENABLED
