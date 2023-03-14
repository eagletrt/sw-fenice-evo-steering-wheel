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

#define RED (uint8_t)4
#define GREEN (uint8_t)3
#define BLUE (uint8_t)5

#define COLOR_WHITE (uint32_t)0xFFFFFF
#define COLOR_RED (uint32_t)0xFF0000
#define COLOR_ORANGE (uint32_t)0xFFA500
#define COLOR_YELLOW (uint32_t)0xFFFF00
#define COLOR_GREEN (uint32_t)0x00FF00
#define COLOR_BLUE (uint32_t)0x0000FF
#define COLOR_PURPLE (uint32_t)0x800080
#define COLOR_OFF (uint32_t)0x000000

void led_control_init();
void led_control_set_all(I2C_HandleTypeDef *hi2c4, uint32_t color);
void led_control_set(I2C_HandleTypeDef *hi2c4, uint32_t colors[LED_N]);

#endif /* INC_LED_CONTROL_H_ */
