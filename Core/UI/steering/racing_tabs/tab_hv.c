#include "tab_hv.h"

#include <stdio.h>

#define CELL_HEIGHT 200
#define CELL_WIDTH  180

/* Balancing Panel */
#define BP_HEIGHT       150
#define BP_WIDTH        300
#define BP_BORDER_WIDTH 3

#define B_COLUMN_HEIGHT  (BP_HEIGHT - 2 * BP_BORDER_WIDTH)
#define B_COLUMN_WIDTH   ((BP_WIDTH - 2 * BP_BORDER_WIDTH) / N_PORK_CELLBOARD)
#define B_COLUMN_SPACING 6

lv_obj_t *data_panel;
lv_obj_t *precharge_panel;

lv_style_t balancing_panel_style;
lv_style_t balancing_columns_style;
lv_style_t not_balancing_columns_style;

lv_obj_t *tab_hv_precharge_bar;
lv_obj_t *tab_hv_pork_speed_bar;
lv_style_t tab_hv_style_indic;
lv_style_t tab_hv_style_precharge;
lv_style_t tab_hv_style_back_indic;
char tab_hv_new_label_buffer[10];

lv_style_t label_custom_style;

lv_obj_t *tab_hv_labels[tab_hv_labels_n] = {
    [tab_hv_lb_temp_max]       = NULL,
    [tab_hv_lb_temp_min]       = NULL,
    [tab_hv_threshold_setting] = NULL,
    [tab_hv_lb_voltage_max]    = NULL,
    [tab_hv_lb_voltage_min]    = NULL,
    [tab_hv_lb_voltage_delta]  = NULL,
    [tab_hv_lb_pack_voltage]   = NULL,
    [tab_hv_lb_pack_voltage_2] = NULL,
    [tab_hv_lb_bus_voltage]    = NULL,
    [tab_hv_lb_current_state]  = NULL,
    [tab_hv_lb_last_error]     = NULL,
    [tab_hv_pork_speed_value]  = NULL,
    [shutdown_status_lb]       = NULL,
};

lv_obj_t *balancing_columns[N_PORK_CELLBOARD] = {
    [0] = NULL,
    [1] = NULL,
    [2] = NULL,
    [3] = NULL,
    [4] = NULL,
    [5] = NULL,
};

static char shutdown_labels[SHUTDOWN_COMPONENT_SIZE][21] = {
    "SD START",
    "TSAC LV IN",
    "TSAC LV OUT",
    "TSAC HV",
    "MUSHROOM L R",
    "MUSHROOM COCK",
    "INERTIAL SWITCH",
    "BOTS",
    "INVC LID",
    "HVD",
    "BSPD",
    "INVC MOT",
    "TSMS LV",
    "TSMS HV"};

static shutdown_circuit_component_state_t shutdown_status_lb_array[SHUTDOWN_COMPONENT_SIZE] = {SC_UNKNOWN};

const char *debug_signal_error_labels[] = {
    "cell low voltage",
    "cell under voltage",
    "cell over voltage",
    "cell high temperature",
    "cell over temperature",
    "over current",
    "can error",
    "internal voltage mismatch",
    "cellboard communication",
    "cellboard internal",
    "connector disconnected",
    "fans disconnected",
    "feedback error",
    "feedback circuitry error",
    "eeprom communication error",
    "eeprom write error"};

bool balancing_status[N_PORK_CELLBOARD] = {0};

bool debug_signal_error_status[DEBUG_SIGNAL_ERROR_SIZE] = {false};

void custom_balancing_column(lv_obj_t *bar, bool balancing);

void set_tab_hv_label_text(const char *s, tab_hv_labels_enum idx) {
    CHECK_CURRENT_TAB(engineer_mode, racing, STEERING_WHEEL_TAB_HV);
    lv_label_set_text(tab_hv_labels[idx], s);
}

void set_balancing_column(bool balancing, uint8_t idx) {
    CHECK_CURRENT_TAB(engineer_mode, racing, STEERING_WHEEL_TAB_HV);
    if (engineer_mode || current_racing_tab != STEERING_WHEEL_TAB_HV)
        return;

    if (balancing_status[idx] != balancing) {
        balancing_status[idx] = balancing;
        custom_balancing_column(balancing_columns[idx], balancing);
    }
}

void tab_hv_set_pork_speed_bar(int32_t val, bool auto_mode) {
    auto_mode ? lv_style_set_bg_color(&tab_hv_style_indic, lv_palette_main(LV_PALETTE_BLUE))
              : lv_style_set_bg_color(&tab_hv_style_indic, lv_palette_main(LV_PALETTE_GREEN));
    lv_bar_set_value(tab_hv_pork_speed_bar, val, LV_ANIM_OFF);
}

void tab_hv_set_error_status(debug_signal_error_t error, bool status) {
    debug_signal_error_status[error] = status;
}

void tab_hv_update_error_label() {
    CHECK_CURRENT_TAB(engineer_mode, racing, STEERING_WHEEL_TAB_HV);
    uint8_t last_error_index = -1;
    uint8_t error_count      = 0;
    for (uint8_t i = 0; i < DEBUG_SIGNAL_ERROR_SIZE; i++) {
        if (debug_signal_error_status[i]) {
            last_error_index = i;
            error_count++;
            return;
        }
    }
    if (error_count == 0) {
        lv_label_set_text(tab_hv_labels[tab_hv_lb_last_error], "NO");
        return;
    } else if (error_count == 1) {
        lv_label_set_text(tab_hv_labels[tab_hv_lb_last_error], debug_signal_error_labels[last_error_index]);
        return;
    } else {
        lv_label_set_text(tab_hv_labels[tab_hv_lb_last_error], "MULTIPLE ERRORS");
        return;
    }
    return;
}

bool precharge_bar_active = false;

void __precharge_bar_update(int32_t val) {
    CHECK_CURRENT_TAB(engineer_mode, racing, STEERING_WHEEL_TAB_HV);
    if (precharge_bar_active) {
        lv_bar_set_value(tab_hv_precharge_bar, val, LV_ANIM_OFF);
    }
}

void __precharge_bar_insert(bool precharge) {
    CHECK_CURRENT_TAB(engineer_mode, racing, STEERING_WHEEL_TAB_HV);
    if (precharge && !precharge_bar_active) {
        static lv_coord_t precharge_panel_cols[] = {DATA_PANEL_WIDTH, LV_GRID_TEMPLATE_LAST};
        static lv_coord_t precharge_panel_rows[] = {150, LV_GRID_TEMPLATE_LAST};

        precharge_panel = lv_obj_create(data_panel);
        lv_obj_set_layout(precharge_panel, LV_LAYOUT_GRID);
        lv_obj_remove_style_all(precharge_panel);
        lv_obj_clear_flag(precharge_panel, LV_OBJ_FLAG_SCROLLABLE);

        lv_obj_add_style(precharge_panel, &box_panels_style, 0);
        lv_obj_center(precharge_panel);
        lv_obj_set_style_base_dir(precharge_panel, LV_BASE_DIR_LTR, 0);
        lv_obj_set_grid_dsc_array(precharge_panel, precharge_panel_cols, precharge_panel_rows);

        lv_obj_set_grid_cell(precharge_panel, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 1, 1);

        tab_hv_precharge_bar = lv_bar_create(precharge_panel);
        lv_obj_remove_style_all(tab_hv_precharge_bar);
        lv_obj_add_style(tab_hv_precharge_bar, &tab_hv_style_precharge, LV_PART_INDICATOR);
        lv_obj_add_style(tab_hv_precharge_bar, &tab_hv_style_back_indic, LV_PART_MAIN);
        lv_obj_set_size(tab_hv_precharge_bar, 700, 100);
        lv_bar_set_range(tab_hv_precharge_bar, 0, HV_BUS_VOLTAGE_LIMIT);
        lv_bar_set_value(tab_hv_precharge_bar, 0, LV_ANIM_OFF);

        lv_obj_set_grid_cell(tab_hv_precharge_bar, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 0, 1);
    } else if (!precharge && precharge_bar_active) {
        lv_obj_del(precharge_panel);
        precharge_panel = NULL;
    }
    precharge_bar_active = precharge;
}

shutdown_circuit_component_state_t global_shutdown_status = SC_UNKNOWN;

shutdown_circuit_indexes_t last_open_shutdown_circuit(void) {
    for (int sdi = 0; sdi < SHUTDOWN_COMPONENT_SIZE; sdi++) {
        if (shutdown_status_lb_array[sdi] == SC_OPEN)
            return sdi;
    }
    return shutdown_circuit_no_element_index;
}

shutdown_circuit_indexes_t last_shutdown_element_unknown(void) {
    for (int sdi = 0; sdi < SHUTDOWN_COMPONENT_SIZE; sdi++) {
        if (shutdown_status_lb_array[sdi] == SC_UNKNOWN)
            return sdi;
    }
    return shutdown_circuit_no_element_index;
}

//TODO
void update_shutdown_circuit_component(shutdown_circuit_indexes_t idx, bool is_close) {
    // TODO: cambiare le soglie
    shutdown_status_lb_array[idx] = is_close ? SC_CLOSE : SC_OPEN;
    shutdown_circuit_indexes_t last_opend_index;
    if ((last_opend_index = last_shutdown_element_unknown()) != shutdown_circuit_no_element_index) {
        endurance_screen_set_color(COLOR_WHITE_STATUS_HEX, COLOR_BLACK_STATUS_HEX, shutdown_status_idx);
        char buf[64];
        snprintf(buf, 64, "unknown %s", shutdown_labels[last_opend_index]);
        set_tab_hv_label_text(buf, shutdown_status_lb);
        return;
    }
    if ((last_opend_index = last_open_shutdown_circuit()) != shutdown_circuit_no_element_index) {
        set_tab_hv_label_text(shutdown_labels[last_opend_index], shutdown_status_lb);
        endurance_screen_set_label(shutdown_labels[last_opend_index], shutdown_status_idx);
        endurance_screen_set_color(COLOR_BLACK_STATUS_HEX, COLOR_BRIGHT_RED_HEX, shutdown_status_idx);
        if (global_shutdown_status == SC_CLOSE) {
            char buf[128];
            int to_display = snprintf(buf, 128, "SHUTDOWN \nCIRCUIT \nOPENED\n");  // %s", shutdown_labels[last_opend_index]);
            if (to_display < 128 && to_display > 0) {
                display_notification(buf, 1500, COLOR_RED_STATUS_HEX, COLOR_PRIMARY_HEX);
            }
        }
        global_shutdown_status = SC_OPEN;
        return;
    }
    if (global_shutdown_status != SC_CLOSE) {
        display_notification("SHUTDOWN \nCIRCUIT \nCLOSED", 1500, COLOR_GREEN_STATUS_HEX, COLOR_PRIMARY_HEX);
    }
    set_tab_hv_label_text("SHUTDOWN CLOSE", shutdown_status_lb);
    endurance_screen_set_label("OK", shutdown_status_idx);
    endurance_screen_set_color(COLOR_BLACK_STATUS_HEX, COLOR_BRIGHT_GREEN_HEX, shutdown_status_idx);
    global_shutdown_status = SC_CLOSE;
}

bool is_shutdown_closed(void) {
    return (global_shutdown_status == SC_CLOSE);
}

void init_hv_styles(void) {
    /* BALANCING PANEL */
    lv_style_init(&balancing_panel_style);
    lv_style_set_pad_all(&balancing_panel_style, 0);
    lv_style_set_pad_bottom(&balancing_panel_style, 0);
    lv_style_set_pad_column(&balancing_panel_style, 0);
    lv_style_set_pad_top(&balancing_panel_style, 0);
    lv_style_set_pad_row(&balancing_panel_style, 0);
    lv_style_set_bg_opa(&balancing_panel_style, LV_OPA_TRANSP);
    lv_style_set_border_color(&balancing_panel_style, lv_color_hex(COLOR_SECONDARY_HEX));
    lv_style_set_border_width(&balancing_panel_style, BP_BORDER_WIDTH);
    lv_style_set_radius(&balancing_panel_style, 0);

    /* BALANCING COLUMN */
    lv_style_init(&balancing_columns_style);
    lv_style_set_bg_opa(&balancing_columns_style, LV_OPA_COVER);
    lv_style_set_bg_color(&balancing_columns_style, lv_color_hex(COLOR_YELLOW_STATUS_HEX));

    /* NOT BALANCING COLUMN */
    lv_style_init(&not_balancing_columns_style);
    lv_style_set_bg_opa(&not_balancing_columns_style, LV_OPA_COVER);
    lv_style_set_bg_color(&not_balancing_columns_style, lv_color_hex(COLOR_SECONDARY_HEX));

    /* BAR STYLE INDICATOR */
    lv_style_init(&tab_hv_style_indic);
    lv_style_set_radius(&tab_hv_style_indic, 0);
    lv_style_set_bg_opa(&tab_hv_style_indic, LV_OPA_COVER);
    lv_style_set_bg_color(&tab_hv_style_indic, lv_palette_main(LV_PALETTE_BLUE));
    // Gradient makes performances much worse
    // lv_style_set_bg_grad_color(&tab_hv_style_indic, lv_palette_main(LV_PALETTE_RED));
    // lv_style_set_bg_grad_dir(&tab_hv_style_indic, LV_GRAD_DIR_HOR);

    lv_style_init(&tab_hv_style_precharge);
    lv_style_set_radius(&tab_hv_style_precharge, 0);
    lv_style_set_bg_opa(&tab_hv_style_precharge, LV_OPA_COVER);
    lv_style_set_bg_color(&tab_hv_style_precharge, lv_palette_main(LV_PALETTE_YELLOW));
    // Gradient makes performances much worse
    // lv_style_set_bg_grad_color(&tab_hv_style_precharge, lv_palette_main(LV_PALETTE_RED));
    // lv_style_set_bg_grad_dir(&tab_hv_style_precharge, LV_GRAD_DIR_HOR);

    lv_style_init(&tab_hv_style_back_indic);
    lv_style_set_radius(&tab_hv_style_back_indic, 0);
    lv_style_set_bg_opa(&tab_hv_style_back_indic, LV_OPA_COVER);
    lv_style_set_bg_color(&tab_hv_style_back_indic, lv_color_hex(COLOR_SECONDARY_HEX));

    lv_style_init(&label_custom_style);
    lv_style_set_base_dir(&label_custom_style, LV_BASE_DIR_LTR);
    lv_style_set_bg_opa(&label_custom_style, LV_OPA_TRANSP);
    lv_style_set_text_color(&label_custom_style, lv_color_hex(COLOR_TERTIARY_HEX));
    lv_style_set_text_align(&label_custom_style, LV_TEXT_ALIGN_CENTER);
}

void tab_hv_create(lv_obj_t *parent) {
    init_hv_styles();

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

    lv_obj_t *notch = create_notch(main_panel, STEERING_WHEEL_TAB_HV);
    lv_obj_align(lv_obj_get_child(notch, 0), LV_ALIGN_TOP_MID, 0, 10);
    lv_obj_set_grid_cell(notch, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_START, 0, 1);

    /*--- creating DATA PANEL ---*/

    static lv_coord_t dpanel_cols[] = {SCREEN_WIDTH, LV_GRID_TEMPLATE_LAST};
    static lv_coord_t dpanel_rows[] = {100, 150, DATA_PANEL_HEIGHT - 250, LV_GRID_TEMPLATE_LAST};

    data_panel = lv_obj_create(main_panel);
    lv_obj_remove_style_all(data_panel);
    // lv_obj_set_layout(data_panel, LV_LAYOUT_GRID);
    lv_obj_clear_flag(data_panel, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(data_panel, SCREEN_WIDTH, DATA_PANEL_HEIGHT);  // TODO non andrebbe tolto il 20 per farlo simmetrico(forse qualche errore)

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
        top_data_panel, &tab_hv_labels[tab_hv_lb_temp_max], "NA", &lv_font_inter_bold_30, "°C", &lv_font_inter_bold_22, "MAX", &lv_font_inter_bold_20);
    lv_obj_set_grid_cell(inv_l_temp_lb, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 0, 1);

    lv_obj_t *inv_r_temp_lb = lv_triple_label(
        top_data_panel, &tab_hv_labels[tab_hv_lb_temp_min], "NA", &lv_font_inter_bold_30, "°C", &lv_font_inter_bold_22, "MIN", &lv_font_inter_bold_20);
    lv_obj_set_grid_cell(inv_r_temp_lb, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, 0, 1);

    lv_obj_t *motor_l_temp_lb = lv_triple_label(
        top_data_panel, &tab_hv_labels[tab_hv_threshold_setting], "50", &lv_font_inter_bold_30, "mV", &lv_font_inter_bold_22, "D THR", &lv_font_inter_bold_20);
    lv_obj_set_grid_cell(motor_l_temp_lb, LV_GRID_ALIGN_CENTER, 2, 1, LV_GRID_ALIGN_CENTER, 0, 1);

    /* VOLT */
    lv_obj_t *motor_r_temp_lb = lv_triple_label(
        top_data_panel, &tab_hv_labels[tab_hv_lb_voltage_max], "NA", &lv_font_inter_bold_30, " V", &lv_font_inter_bold_22, "MAX", &lv_font_inter_bold_20);
    lv_obj_set_grid_cell(motor_r_temp_lb, LV_GRID_ALIGN_CENTER, 3, 1, LV_GRID_ALIGN_CENTER, 0, 1);

    lv_obj_t *hv_temp_lb = lv_triple_label(
        top_data_panel, &tab_hv_labels[tab_hv_lb_voltage_min], "NA", &lv_font_inter_bold_30, " V", &lv_font_inter_bold_22, "MIN", &lv_font_inter_bold_20);
    lv_obj_set_grid_cell(hv_temp_lb, LV_GRID_ALIGN_CENTER, 4, 1, LV_GRID_ALIGN_CENTER, 0, 1);

    lv_obj_t *hv_volt_lb = lv_triple_label(
        top_data_panel, &tab_hv_labels[tab_hv_lb_voltage_delta], "NA", &lv_font_inter_bold_30, " mV", &lv_font_inter_bold_22, "DELTA", &lv_font_inter_bold_20);
    lv_obj_set_grid_cell(hv_volt_lb, LV_GRID_ALIGN_CENTER, 5, 1, LV_GRID_ALIGN_CENTER, 0, 1);

    lv_obj_t *hv_curr_lb = lv_triple_label(
        top_data_panel, &tab_hv_labels[tab_hv_lb_pack_voltage], "NA", &lv_font_inter_bold_30, " V", &lv_font_inter_bold_22, "TOT", &lv_font_inter_bold_20);
    lv_obj_set_grid_cell(hv_curr_lb, LV_GRID_ALIGN_CENTER, 6, 1, LV_GRID_ALIGN_CENTER, 0, 1);

    lv_obj_t *hv_minCellV_lb = lv_triple_label(
        top_data_panel, &tab_hv_labels[tab_hv_lb_pack_voltage_2], "NA", &lv_font_inter_bold_30, " V", &lv_font_inter_bold_22, "SUM", &lv_font_inter_bold_20);
    lv_obj_set_grid_cell(hv_minCellV_lb, LV_GRID_ALIGN_CENTER, 7, 1, LV_GRID_ALIGN_CENTER, 0, 1);

    /* 2nd row HV */

    /*--- DATA PANEL: cell 1, 0 ---*/
    static lv_coord_t center_panel_cols[] = {DATA_PANEL_WIDTH / 2, DATA_PANEL_WIDTH / 2, LV_GRID_TEMPLATE_LAST};
    static lv_coord_t center_panel_rows[] = {150, LV_GRID_TEMPLATE_LAST};

    lv_obj_t *center_data_panel = lv_obj_create(data_panel);
    lv_obj_set_layout(center_data_panel, LV_LAYOUT_GRID);
    lv_obj_remove_style_all(center_data_panel);
    lv_obj_clear_flag(center_data_panel, LV_OBJ_FLAG_SCROLLABLE);
    // lv_obj_set_size(left_data_panel, CELL_WIDTH, CELL_HEIGHT);

    lv_obj_add_style(center_data_panel, &box_panels_style, 0);
    lv_obj_center(center_data_panel);
    lv_obj_set_style_base_dir(center_data_panel, LV_BASE_DIR_LTR, 0);
    lv_obj_set_grid_dsc_array(center_data_panel, center_panel_cols, center_panel_rows);

    lv_obj_set_grid_cell(center_data_panel, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 1, 1);

    /* 1st row */
    lv_obj_t *status_lb = lv_triple_label(
        center_data_panel, &tab_hv_labels[tab_hv_lb_current_state], "NA", &lv_font_inter_bold_30, "", &lv_font_inter_bold_22, "STATUS", &lv_font_inter_bold_20);
    lv_obj_set_grid_cell(status_lb, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 0, 1);

    lv_obj_t *error_lb = lv_triple_label(
        center_data_panel, &tab_hv_labels[tab_hv_lb_last_error], "NA", &lv_font_inter_bold_30, "", &lv_font_inter_bold_22, "ERRORS", &lv_font_inter_bold_20);
    lv_obj_set_grid_cell(error_lb, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, 0, 1);

    /*--- creating BOTTOM PANEL ---*/
    static lv_coord_t bottom_panel_cols[] = {SCREEN_WIDTH / 2, SCREEN_WIDTH / 2, LV_GRID_TEMPLATE_LAST};
    static lv_coord_t bottom_panel_rows[] = {B_COLUMN_HEIGHT, LV_GRID_TEMPLATE_LAST};

    lv_obj_t *bottom_data_panel = lv_obj_create(data_panel);
    lv_obj_set_layout(bottom_data_panel, LV_LAYOUT_GRID);
    lv_obj_remove_style_all(bottom_data_panel);
    lv_obj_clear_flag(bottom_data_panel, LV_OBJ_FLAG_SCROLLABLE);
    // lv_obj_set_size(left_data_panel, CELL_WIDTH, CELL_HEIGHT);
    lv_obj_set_grid_align(bottom_data_panel, LV_GRID_ALIGN_SPACE_BETWEEN, LV_GRID_ALIGN_CENTER);

    // lv_obj_add_style(bottom_data_panel, &box_panels_style, 0);
    lv_obj_center(bottom_data_panel);
    lv_obj_set_style_base_dir(bottom_data_panel, LV_BASE_DIR_LTR, 0);
    lv_obj_set_grid_dsc_array(bottom_data_panel, bottom_panel_cols, bottom_panel_rows);

    lv_obj_set_grid_cell(bottom_data_panel, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 2, 1);

    /*--- creating BALANCING PANEL ---*/
    static lv_coord_t bal_panel_cols[] = {
        B_COLUMN_WIDTH, B_COLUMN_WIDTH, B_COLUMN_WIDTH, B_COLUMN_WIDTH, B_COLUMN_WIDTH, B_COLUMN_WIDTH, LV_GRID_TEMPLATE_LAST};
    static lv_coord_t bal_panel_rows[] = {B_COLUMN_HEIGHT, LV_GRID_TEMPLATE_LAST};

    lv_obj_t *balancing_panel = lv_obj_create(bottom_data_panel);
    lv_obj_set_layout(balancing_panel, LV_LAYOUT_GRID);
    lv_obj_remove_style_all(balancing_panel);
    lv_obj_clear_flag(balancing_panel, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(balancing_panel, BP_WIDTH + (N_PORK_CELLBOARD - 1) * B_COLUMN_SPACING, BP_HEIGHT);
    lv_obj_set_grid_align(balancing_panel, LV_GRID_ALIGN_SPACE_BETWEEN, LV_GRID_ALIGN_CENTER);

    lv_obj_add_style(balancing_panel, &balancing_panel_style, 0);
    lv_obj_center(balancing_panel);
    lv_obj_set_style_base_dir(balancing_panel, LV_BASE_DIR_LTR, 0);
    lv_obj_set_grid_dsc_array(balancing_panel, bal_panel_cols, bal_panel_rows);

    lv_obj_set_grid_cell(balancing_panel, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 0, 1);

    for (uint8_t i = 0; i < N_PORK_CELLBOARD; i++) {
        balancing_columns[i] = lv_obj_create(balancing_panel);
        custom_balancing_column(balancing_columns[i], false);

        lv_obj_set_grid_cell(balancing_columns[i], LV_GRID_ALIGN_CENTER, i, 1, LV_GRID_ALIGN_CENTER, 0, 1);
    }

    /*--- creating PORK PANEL ---*/
    static lv_coord_t pork_panel_cols[] = {SCREEN_WIDTH / 2, LV_GRID_TEMPLATE_LAST};
    static lv_coord_t pork_panel_rows[] = {(B_COLUMN_HEIGHT) / 3, (B_COLUMN_HEIGHT) / 3, (B_COLUMN_HEIGHT) / 3, LV_GRID_TEMPLATE_LAST};

    lv_obj_t *pork_panel = lv_obj_create(bottom_data_panel);
    lv_obj_set_layout(pork_panel, LV_LAYOUT_GRID);
    lv_obj_remove_style_all(pork_panel);
    lv_obj_clear_flag(pork_panel, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_grid_align(pork_panel, LV_GRID_ALIGN_CENTER, LV_GRID_ALIGN_CENTER);

    // lv_obj_add_style(pork_panel, &balancing_panel_style, 0);
    // lv_obj_center(pork_panel);
    // lv_obj_set_style_base_dir(pork_panel, LV_BASE_DIR_LTR, 0);
    lv_obj_set_grid_dsc_array(pork_panel, pork_panel_cols, pork_panel_rows);

    lv_obj_set_grid_cell(pork_panel, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 0, 1);

    tab_hv_labels[shutdown_status_lb] = lv_label_create(pork_panel);
    lv_obj_add_style(tab_hv_labels[shutdown_status_lb], &label_custom_style, LV_PART_MAIN);
    lv_obj_set_style_text_font(tab_hv_labels[shutdown_status_lb], &lv_font_inter_bold_30, LV_STATE_DEFAULT);
    lv_label_set_text(tab_hv_labels[shutdown_status_lb], "SHUTDOWN NA");
    lv_obj_set_grid_cell(tab_hv_labels[shutdown_status_lb], LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_START, 0, 1);

    static lv_coord_t pork_label_cols[] = {(SCREEN_WIDTH / 4), (SCREEN_WIDTH / 4), LV_GRID_TEMPLATE_LAST};
    static lv_coord_t pork_label_row[]  = {(B_COLUMN_HEIGHT) / 3, LV_GRID_TEMPLATE_LAST};

    lv_obj_t *pork_label_grid = lv_obj_create(pork_panel);
    lv_obj_set_layout(pork_label_grid, LV_LAYOUT_GRID);
    lv_obj_remove_style_all(pork_label_grid);
    lv_obj_clear_flag(pork_label_grid, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_grid_align(pork_label_grid, LV_GRID_ALIGN_CENTER, LV_GRID_ALIGN_CENTER);

    // lv_obj_add_style(pork_label_grid, &balancing_panel_style, 0);
    // lv_obj_center(pork_label_grid);
    lv_obj_set_style_base_dir(pork_label_grid, LV_BASE_DIR_LTR, 0);
    lv_obj_set_grid_dsc_array(pork_label_grid, pork_label_cols, pork_label_row);

    lv_obj_set_grid_cell(pork_label_grid, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 1, 1);

    lv_obj_t *pork_label = lv_label_create(pork_label_grid);
    lv_obj_add_style(pork_label, &label_custom_style, LV_PART_MAIN);
    lv_obj_set_style_text_font(pork_label, &lv_font_inter_bold_30, LV_STATE_DEFAULT);
    lv_label_set_text(pork_label, "TSAC");
    lv_obj_set_grid_cell(pork_label, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 0, 1);

    tab_hv_labels[tab_hv_pork_speed_value] = lv_label_create(pork_label_grid);
    lv_obj_add_style(tab_hv_labels[tab_hv_pork_speed_value], &label_custom_style, LV_PART_MAIN);
    lv_obj_set_style_text_font(tab_hv_labels[tab_hv_pork_speed_value], &lv_font_inter_bold_30, LV_STATE_DEFAULT);
    lv_label_set_text(tab_hv_labels[tab_hv_pork_speed_value], "-");
    lv_obj_set_grid_cell(tab_hv_labels[tab_hv_pork_speed_value], LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, 0, 1);

    // Pork fan bar
    tab_hv_pork_speed_bar = lv_bar_create(pork_panel);
    lv_obj_remove_style_all(tab_hv_pork_speed_bar);
    lv_obj_add_style(tab_hv_pork_speed_bar, &tab_hv_style_indic, LV_PART_INDICATOR);
    lv_obj_add_style(tab_hv_pork_speed_bar, &tab_hv_style_back_indic, LV_PART_MAIN);
    lv_obj_set_size(tab_hv_pork_speed_bar, 350, 50);
    lv_bar_set_range(tab_hv_pork_speed_bar, 0, 100);
    lv_bar_set_value(tab_hv_pork_speed_bar, 0, LV_ANIM_OFF);

    lv_obj_set_grid_cell(tab_hv_pork_speed_bar, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_START, 2, 1);
}

/**
 * @brief Custom style applied to the balancing column
 * @param bar pointer to the object
 * @param balancing true if cell is balancing, false otherwise
 */
void custom_balancing_column(lv_obj_t *bar, bool balancing) {
    lv_obj_remove_style_all(bar);

    if (balancing) {
        lv_obj_add_style(bar, &balancing_columns_style, LV_PART_MAIN);
    } else {
        lv_obj_add_style(bar, &not_balancing_columns_style, LV_PART_MAIN);
    }

    lv_obj_set_size(bar, B_COLUMN_WIDTH, B_COLUMN_HEIGHT);
    lv_obj_center(bar);
}

void tab_hv_resync(void) {
#if 0
    // Sets the text about the shutdown
    hv_feedbacks_status_update();
    // Sets the text about the speed of the fans and the bar
    hv_fans_override_status_update();

    hv_cells_voltage_stats_update();
    hv_total_voltage_update();
    hv_cells_temp_stats_update();
    hv_status_update();
    hv_fans_override_status_update();
#endif
}
