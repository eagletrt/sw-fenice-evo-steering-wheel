#ifndef TAB_MANAGER_H
#define TAB_MANAGER_H

#include "steering.h"
#include "lvgl.h"
#include "tab_calibration.h"
#include "tab_debug.h"
#include "tab_racing.h"
#include "utils.h"

#define N_SCREENS 3
extern lv_group_t *g;

typedef enum { FORWARD, BACKWARD } scroll;

void tab_manager(void);
void change_tab(scroll direction);

#endif