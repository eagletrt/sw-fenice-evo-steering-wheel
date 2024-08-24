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

#define LED_CONTROL_REGISTER                0x02
#define PATTERN_GENERATION_CONTROL_REGISTER 0x0F
#define PATTERN_GENERATION_FADE_REGISTER    0x10
#define PATTERN_GENERATION_RGB_1            0X11
#define PATTERN_GENERATION_RGB_2            0X12
#define PATTERN_GENERATION_RGB_3            0X13
#define PATTERN_GENERATION_RGB_4            0X14
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

#define COLOR_WHITE  ((uint32_t)0xFFFFFF)
#define COLOR_BLACK  ((uint32_t)0x000000)
#define COLOR_RED    ((uint32_t)0xFF0000)
#define COLOR_ORANGE ((uint32_t)0xFFA500)
#define COLOR_YELLOW ((uint32_t)0xFFFF00)
#define COLOR_GREEN  ((uint32_t)0x00FF00)
#define COLOR_BLUE   ((uint32_t)0x0000FF)
#define COLOR_PURPLE ((uint32_t)0x800080)
#define COLOR_OFF    ((uint32_t)0x000000)
#define KTD2052A     ((uint8_t)0b11101000)
#define KTD2052C     ((uint8_t)0b11101010)

extern uint8_t reply[2];

void led_control_init();
void led_control_init_reset();
void led_control_set_all(I2C_HandleTypeDef *hi2c4, uint32_t color);
void led_control_set(I2C_HandleTypeDef *hi2c4, uint32_t colors[LED_N]);
void led_control_green_test();

void led_mem_write(void);

void led_control_read_value(void);
void try_led_last_time(void);

#endif /* INC_LED_CONTROL_H_ */

/*
What has been done about LEDs (long story short: a lot but little)
-   The current code has been tested. The functions don't work as expected. In fact they don't work at all.
-   Explanation: when using the init_led function, and then the led_control_set_all leds are supposed to turn on, but they don't give any sign of life.
-   KTD2052 manual has been read and the device address wasn't sent as first instruction with the HAL_I2C_Master_Transmit instruction.
    In fact, as the manual explain, you should: 
    1- send the device address. In our case, as the datashet says, the device address is 0b1110100x for the KTD2052A controller and 0b1110101x for the KTD2052C controller. The last bit should be 0 if you want to write data on the controller (i.e. set the color of the leds) and 1 if you want to read data from the controller.
    2- send the register address you want to write or read. In our case, we firstly sent is 0x02, which is the control register. Details above. 
    3- send the data, that is the value we want to set into our register, in our case the control register.
    Idea: if we sent with the HAL_I2C_Master_Transmit the device address into its field and then with each transmit the device address, then the register address, then the data
    Once added the device address into the I2C function and sent the CONTROL register, the leds start working but only for a fraction of second, and then the auto breath pattern starts showing, but with the left leds reds, and the right leds show strange patterns.
-   We tried also to detect the read the HAL_I2C_Master_Transmit returns, but it is always HAL_OK.
-   Then we set, into the CONTROL register the reset of all register to default settings, but nothing worked after this little change (0b11xxxxxx)
-   Also, we wanted to read the value of a register with the I2C, by using the function HAL_I2C_Master_Receive. By reading the control register we expected to receive its default value. Instead, by writing the value inside tab_sensor the value printed was zero.
-   At once, we've also tried to write and read value by using mem_read and mem_write functions, but nothing.
-   After tried to turn on also only one led, by giving the address of one controller and one led, but not succeeding in it, one last idea.
-   I've decided to try to use HAL_I2C functions differently. I thought that maybe with these functions you first send the device address with which to communicate, and the data argument is the effective data you want to sent. So I've done as it can be seen in the try_led_last_time() function, but one again, nothing.
Future works are to check via a spectrometer if the correct data are sent via the i2c wires.
*/
