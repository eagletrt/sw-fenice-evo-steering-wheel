/*
 * led_control.c
 *
 *  Created on: Mar 6, 2023
 *      Author: tzarjakob
 */

#include "led_control.h"

void led_control_init() {
	uint8_t control[]= {0x02, 0b10011100};

	HAL_I2C_Master_Transmit(&hi2c4, ledaddr1, control, sizeof(control), 100);
	HAL_I2C_Master_Transmit(&hi2c4, ledaddr2, control, sizeof(control), 100);
}

void led_control_set(I2C_HandleTypeDef* hi2c4, uint32_t colors[LED_N]) {
	for (int icolor = 0; icolor < LED_N / 2; ++icolor) {
		HAL_I2C_Master_Transmit(hi2c4, ledaddr1,
				(uint8_t[2] ) { RED + icolor * 3,
								(uint8_t) (colors[icolor] >> 8) % 8 }, 2, 100);
		HAL_I2C_Master_Transmit(hi2c4, ledaddr1,
				(uint8_t[2] ) { GREEN + icolor * 3,
								(uint8_t) (colors[icolor] >> 4) % 8 }, 2, 100);
		HAL_I2C_Master_Transmit(hi2c4, ledaddr1,
				(uint8_t[2] ) { BLUE + icolor * 3,
								(uint8_t) colors[icolor] % 8 }, 2, 100);
	}

	for (int icolor = 0; icolor < LED_N / 2; ++icolor) {
		HAL_I2C_Master_Transmit(hi2c4, ledaddr2,
				(uint8_t[2] ) { RED + icolor * 3,
								(uint8_t) (colors[icolor + 3] >> 8) % 8 }, 2, 100);
		HAL_I2C_Master_Transmit(hi2c4, ledaddr2,
				(uint8_t[2] ) { GREEN + icolor * 3,
								(uint8_t) (colors[icolor + 3] >> 4) % 8 }, 2, 100);
		HAL_I2C_Master_Transmit(hi2c4, ledaddr2,
				(uint8_t[2] ) { BLUE + icolor * 3,
								(uint8_t) colors[icolor + 3] % 8 }, 2, 100);
	}
}
