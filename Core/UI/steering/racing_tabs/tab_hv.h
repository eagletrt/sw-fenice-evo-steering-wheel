#ifndef TAB_HV_H
#define TAB_HV_H

#include "lvgl.h"
#include "steering.h"

#define TAB_HV_CENTRAL_PANEL_WIDTH 660
#define BOTTOM_BAR_HEIGHT          120
#define TOP_BAR_HEIGHT             55
#define DATA_PANEL_HEIGHT          425
#define DATA_PANEL_WIDTH           800
#define ROW_PADDING                20
#define COLUMN_PADDING             20

#define DATA_LEFT_WIDTH   185
#define DATA_RIGHT_WIDTH  185
#define DATA_CENTER_WIDTH 290

extern const char *debug_signal_error_labels[];

// true: error, false: no error
extern bool debug_signal_error_status[DEBUG_SIGNAL_ERROR_SIZE];

void tab_hv_create(lv_obj_t *parent);

void set_tab_hv_label_text(const char *s, tab_hv_labels_enum idx);

void init_hv_styles(void);
void tab_hv_resync(void);

#endif  // TAB_HV_H