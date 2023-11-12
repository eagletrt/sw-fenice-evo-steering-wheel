#ifndef TAB_COOLING_H
#define TAB_COOLING_H

#include "steering.h"
#include "utils.h"

void tab_cooling_create(lv_obj_t *parent);

void set_pumps_speed_bar(int32_t);
void set_radiators_speed_bar(int32_t);
void set_pork_speed_bar(int32_t);

void set_pumps_speed_value_label(float);
void set_radiators_speed_value_label(float);
void set_pork_speed_value_label(float);

#endif // TAB_COOLING_H
