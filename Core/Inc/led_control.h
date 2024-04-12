/*
 * led_control.h
 *
 *  Created on: Mar 6, 2023
 *      Author: tzarjakob
 */

#ifndef INC_LED_CONTROL_H_
#define INC_LED_CONTROL_H_

#include "i2c.h"

#include <stdint.h>

#define ledaddr1 (uint8_t)0xE8
#define ledaddr2 (uint8_t)0xEA

#define LED_N 6

#define RED   (uint8_t)4
#define GREEN (uint8_t)3
#define BLUE  (uint8_t)5

#define LED_CONTROL_REGISTER 0x02
/* Control register:

Enable Mode and Reset
00 = off (fade all to zero and then shutdown) (KTD2052A/C default)
01 = enable Night Mode (0 to 1.5mA range)
10 = enable Normal Mode (0 to 24mA range) (KTD2052B/D
default)
11 = reset all registers to default settings

BrightExtend Enable
0 = disabled (KTD2052A/C default)
1 = enabled (KTD2052B/D default)

CoolExtend Temperature Setting
00 = 135°C (KTD2052A/C default)
01 = 120°C
10 = 105°C
11 = 90°C (KTD2052B/D default)

Fade-Rate 0 Exponential Time-Constant Setting
000 = 32ms (KTD2052A/C default)
001 = 63ms
010 = 125ms
011 = 250ms
100 = 500ms (KTD2052B/D default)
101 = 1s
110 = 2s
111 = 4s */

#define COLOR_WHITE  (uint32_t)0xFFFFFF
#define COLOR_RED    (uint32_t)0xFF0000
#define COLOR_ORANGE (uint32_t)0xFFA500
#define COLOR_YELLOW (uint32_t)0xFFFF00
#define COLOR_GREEN  (uint32_t)0x00FF00
#define COLOR_BLUE   (uint32_t)0x0000FF
#define COLOR_PURPLE (uint32_t)0x800080
#define COLOR_OFF    (uint32_t)0x000000

void led_control_init();
void led_control_set_all(I2C_HandleTypeDef *hi2c4, uint32_t color);
void led_control_set(I2C_HandleTypeDef *hi2c4, uint32_t colors[LED_N]);

#endif /* INC_LED_CONTROL_H_ */
