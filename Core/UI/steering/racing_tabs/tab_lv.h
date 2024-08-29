#ifndef TAB_LV_H
#define TAB_LV_H

#include "steering_config.h"

#if STEERING_WHEEL_MODE == STEERING_WHEEL_LVGL_MODE

#include "lvgl.h"
#include "steering.h"
#include "utils.h"

#define TAB_LV_CENTRAL_PANEL_WIDTH 660
#define BOTTOM_BAR_HEIGHT          120
#define TOP_BAR_HEIGHT             55
#define DATA_PANEL_HEIGHT          425
#define DATA_PANEL_WIDTH           800
#define CENTRAL_PANEL_HEIGHT       (DATA_PANEL_HEIGHT - 100)
#define ROW_PADDING                20
#define COLUMN_PADDING             20

#define CP_COLUMN_WIDTH (DATA_PANEL_WIDTH / 3)

#define SUB_PANEL_HEADER_HEIGHT ((CENTRAL_PANEL_HEIGHT) * 1 / 6)
#define BAR_HEIGHT              (CENTRAL_PANEL_HEIGHT - SUB_PANEL_HEADER_HEIGHT)

#define DATA_LEFT_WIDTH   185
#define DATA_RIGHT_WIDTH  185
#define DATA_CENTER_WIDTH 290

void tab_lv_create(lv_obj_t *parent);

void set_tab_lv_label_text(const char *s, tab_lv_labels_enum idx);

void init_lv_styles(void);

void tab_lv_resync(void);

#endif  // STEERING_WHEEL_MODE == STEERING_WHEEL_LVGL_MODE

#endif  // TAB_LV_H