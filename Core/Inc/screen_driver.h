#ifndef SCREEN_DRIVER_H_
#define SCREEN_DRIVER_H_

#include "lvgl.h"
#include "usart.h"

#include <stdio.h>

void screen_driver_init();
void steering_log(const char *buf);

#endif
