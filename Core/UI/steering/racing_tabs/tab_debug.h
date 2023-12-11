#ifndef TAB_DEBUG_H
#define TAB_DEBUG_H

#include "steering_config.h"

#if STEER_TAB_DEBUG_ENABLED == 1

#include "lvgl.h"
#include "steering.h"
#include "utils.h"
#include "../can_messages.h"

#define TAB_DEBUG_TOP_BAR_HEIGHT 55
#define TAB_DEBUG_DATA_PANEL_HEIGHT 425
#define TAB_DEBUG_DATA_PANEL_WIDTH 780
#define TAB_DEBUG_DATA_TITLE_HEIGHT 70

#define ERROR_BOX_HEIGHT 50

#define TAB_DEBUG_ROW_PADDING 10
#define TAB_DEBUG_COLUMN_PADDING 10

#define TAB_DEBUG_CELL_HEIGHT 40
#define TAB_DEBUG_CELL_WIDTH 180

#define NUM_TYPE_ERRORS 4

typedef enum {
  TAB_HV_FEEDBACKS,
  TAB_HV_ERRORS,
  TAB_DAS_ERRORS,
  TAB_LV_ERRORS
} ErrorTabID;

void tab_debug_create(lv_obj_t *parent);
void init_debug_styles(void);

lv_obj_t *cell_create(lv_obj_t *parent, const char *text, uint8_t pos_row,
                      uint8_t pos_col, lv_style_t *style);

void change_errors_view(bool dir_left);

void set_label_color_hv_feedbacks(int label, int i);

void set_label_color(bool label, lv_obj_t *errors[], int i);

void update_tab_debug_hv_feedbacks_labels(primary_hv_feedback_status_converted_t *primary_hv_feedbacks_last_state);
void update_tab_debug_hv_errors_labels(primary_hv_errors_converted_t *primary_hv_errors_last_state);
void update_tab_debug_das_errors_labels(primary_das_errors_converted_t *primary_das_errors_last_state);
void update_tab_debug_lv_errors_labels(primary_lv_errors_converted_t *primary_lv_errors_last_state);

#endif // STEER_TAB_DEBUG_ENABLED

#endif // TAB_DEBUG_H
