#ifndef TAB_NOTIFICATION_H
#define TAB_NOTIFICATION_H

#include "steering.h"
#include "utils.h"

void tab_notification_screen_create(lv_obj_t *notification_screen);
void set_notification_screen_label(const char *s);
void tab_notification_set_bg_color(uint32_t hex_color);
void tab_notification_set_label_color(uint32_t hex_color);

#endif
