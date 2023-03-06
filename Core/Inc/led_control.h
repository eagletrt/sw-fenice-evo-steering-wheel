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

#define ledaddr1 (uint8_t) 0xE8
#define ledaddr2 (uint8_t) 0xEA

#define RED   (uint8_t) 4
#define GREEN (uint8_t) 3
#define BLUE  (uint8_t) 5

#define COLOR_ORANGE 0xFFA500
#define COLOR_WHITE  0xFFFFFF

#define LED_N 6

void led_control_init();
void led_control_set(I2C_HandleTypeDef* hi2c4, uint32_t colors[LED_N]);


#endif /* INC_LED_CONTROL_H_ */
