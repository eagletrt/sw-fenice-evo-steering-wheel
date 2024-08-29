#ifndef TAB_TRACK_TEST_H
#define TAB_TRACK_TEST_H

#include "steering_config.h"

#if STEERING_WHEEL_MODE == STEERING_WHEEL_LVGL_MODE

#include "lvgl.h"
#include "steering.h"
#include "utils.h"

#define CENTRAL_PANEL_WIDTH         660
#define BOTTOM_BAR_HEIGHT           120
#define TOP_BAR_HEIGHT              55
#define TAB_TRACK_DATA_PANEL_HEIGHT 430
#define ROW_PADDING                 20
#define COLUMN_PADDING              20

#define DATA_LEFT_WIDTH   185
#define DATA_RIGHT_WIDTH  185
#define DATA_CENTER_WIDTH 290

void tab_track_test_create(lv_obj_t *parent);
void tab_track_test_resync(void);

#endif  // STEERING_WHEEL_MODE == STEERING_WHEEL_LVGL_MODE

#endif  // TAB_TRACK_TEST_H
