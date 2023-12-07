#include "tab_shutdown_circuit.h"

#if ENGINEERING_TAB_ENABLED == 1

lv_style_t tab_shutdown_circuit_style;
lv_obj_t *tab_shutdown_circuit_labels[SHUTDOWN_CIRCUIT_ROWS][2];

char shutdown_circuit_strings[SHUTDOWN_COMPONENT_SIZE][BUFSIZ] = {
    "sd_start",
    "feedbacks_status_feedback_sd_in",
    "feedbacks_status_feedback_sd_out",
    "feedbacks_interlock_fb",
    "ecu_feedbacks_sd_in",
    "ecu_feedbacks_sd_cock_fb",
    "ecu_feedbacks_sd_interial_fb",
    "ecu_feedbacks_sd_bots_fb",
    "feedbacks_invc_lid_fb",
    "feedbacks_hvd_fb",
    "feedbacks_bspd_fb",
    "feedbacks_invc_interlock_fb",
    "sd_end",
    "feedbacks_status_feedback_sd_end",
    "feedbacks_status_feedback_precharge_status",
    "feedbacks_status_feedback_airp_gate",
    "feedbacks_status_feedback_airn_gate",
};

shutdown_component_state_t
    shutdown_circuit_last_state[SHUTDOWN_COMPONENT_SIZE] = {SC_UNKNOWN};
bool shutdown_circuit_active = false;

void init_tab_shutdown_circuit(void) {
  lv_style_init(&tab_shutdown_circuit_style);
  lv_style_set_base_dir(&tab_shutdown_circuit_style, LV_BASE_DIR_LTR);
  lv_style_set_bg_opa(&tab_shutdown_circuit_style, LV_OPA_TRANSP);
  lv_style_set_text_color(&tab_shutdown_circuit_style,
                          lv_color_hex(COLOR_SECONDARY_HEX));
  lv_style_set_text_align(&tab_shutdown_circuit_style, LV_TEXT_ALIGN_CENTER);
  lv_style_set_align(&tab_shutdown_circuit_style, LV_ALIGN_CENTER);
}

void tab_shutdown_circuit_create(lv_obj_t *parent) {
  init_tab_shutdown_circuit();

  static lv_coord_t tab_terminal_cols_ratios[] = {
      SCREEN_WIDTH / 2, SCREEN_WIDTH / 2, LV_GRID_TEMPLATE_LAST};
  static lv_coord_t tab_terminal_rows_ratios[] = {
      SCREEN_HEIGHT / (SHUTDOWN_CIRCUIT_ROWS + 1),
      SCREEN_HEIGHT / (SHUTDOWN_CIRCUIT_ROWS + 1),
      SCREEN_HEIGHT / (SHUTDOWN_CIRCUIT_ROWS + 1),
      SCREEN_HEIGHT / (SHUTDOWN_CIRCUIT_ROWS + 1),
      SCREEN_HEIGHT / (SHUTDOWN_CIRCUIT_ROWS + 1),
      SCREEN_HEIGHT / (SHUTDOWN_CIRCUIT_ROWS + 1),
      SCREEN_HEIGHT / (SHUTDOWN_CIRCUIT_ROWS + 1),
      SCREEN_HEIGHT / (SHUTDOWN_CIRCUIT_ROWS + 1),
      SCREEN_HEIGHT / (SHUTDOWN_CIRCUIT_ROWS + 1),
      SCREEN_HEIGHT / (SHUTDOWN_CIRCUIT_ROWS + 1),
      SCREEN_HEIGHT / (SHUTDOWN_CIRCUIT_ROWS + 1),
      SCREEN_HEIGHT / (SHUTDOWN_CIRCUIT_ROWS + 1),
      SCREEN_HEIGHT / (SHUTDOWN_CIRCUIT_ROWS + 1),
      LV_GRID_TEMPLATE_LAST};

  /*Create a container with grid*/
  lv_obj_t *cont = lv_obj_create(parent);

  lv_obj_set_layout(cont, LV_LAYOUT_GRID);
  lv_obj_set_size(cont, SCREEN_WIDTH, SCREEN_HEIGHT);
  lv_obj_add_style(cont, &grid_style, 0);
  lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_center(cont);

  lv_obj_set_grid_dsc_array(cont, tab_terminal_cols_ratios,
                            tab_terminal_rows_ratios);

  lv_obj_t *cs_l1 = lv_label_create(cont);
  lv_label_set_text(cs_l1, "TAB SHUTDOWN CIRCUIT");
  lv_obj_add_style(cs_l1, &tab_shutdown_circuit_style, 0);
  lv_obj_set_grid_cell(cs_l1, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER,
                       0, 1);

  lv_obj_t *cs_l2 = lv_label_create(cont);
  lv_label_set_text(
      cs_l2,
      "premere bottone verde in alto a sinistra\n per attivare e disattivare");
  lv_obj_add_style(cs_l2, &tab_shutdown_circuit_style, 0);
  lv_obj_set_grid_cell(cs_l2, LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER,
                       0, 1);

  for (int i = 0; i < SHUTDOWN_CIRCUIT_ROWS; i++) {
    tab_shutdown_circuit_labels[i][0] = lv_label_create(cont);
    lv_label_set_text_fmt(tab_shutdown_circuit_labels[i][0], "-----");
    lv_obj_add_style(tab_shutdown_circuit_labels[i][0],
                     &tab_shutdown_circuit_style, 0);
    lv_obj_set_grid_cell(tab_shutdown_circuit_labels[i][0],
                         LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER,
                         i + 1, 1);

    tab_shutdown_circuit_labels[i][1] = lv_label_create(cont);
    lv_label_set_text_fmt(tab_shutdown_circuit_labels[i][1], "-----");
    lv_obj_add_style(tab_shutdown_circuit_labels[i][1],
                     &tab_shutdown_circuit_style, 0);
    lv_obj_set_grid_cell(tab_shutdown_circuit_labels[i][1],
                         LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER,
                         i + 1, 1);
  }
}

void shutdown_circuit_turn_on_off() {
  shutdown_circuit_active = !shutdown_circuit_active;
}

void update_shutdown_component(lv_obj_t *lab, const char *text,
                               const shutdown_component_state_t elem) {
  switch (elem) {
  case SC_LOW: {
    lv_label_set_text_fmt(lab, "%s OPEN", text);
    break;
  }
  case SC_HIGH: {
    lv_label_set_text_fmt(lab, "%s CLOSE", text);
    break;
  }
  case SC_UNKNOWN: {
    lv_label_set_text_fmt(lab, "%s unknown", text);
    break;
  }
  }
}

void update_shutdown_circuit_ui(lv_timer_t *tim) {
  if (!shutdown_circuit_active)
    return;
  for (uint32_t iindex = 0; iindex < SHUTDOWN_COMPONENT_SIZE; ++iindex) {
    update_shutdown_component(
        tab_shutdown_circuit_labels[iindex % SHUTDOWN_CIRCUIT_ROWS]
                                   [iindex / SHUTDOWN_CIRCUIT_ROWS],
        shutdown_circuit_strings[iindex], shutdown_circuit_last_state[iindex]);
  }
}

void update_shutdown_circuit(shutdown_component_state_t *comps) {
  for (uint32_t iindex = 0; iindex < SHUTDOWN_COMPONENT_SIZE; ++iindex) {
    if (comps[iindex] != SC_UNKNOWN) {
      shutdown_circuit_last_state[iindex] = comps[iindex];
    }
  }
}

#endif
