/**
 * @file steering_config.h
 * @author Giacomo Mazzucchi (giacomo.mazzucchi@protonmail.com)
 * @brief General configuration file for steering wheel
 * @version 0.1
 * @date 2023-12-12
 *
 * @copyright Copyright (c) 2023
 *
 */
#ifndef STEERING_CONFIG_H
#define STEERING_CONFIG_H

#include "steering_defs.h"
#include "steering_types.h"

/***
 * If you change one to the other, remember to change LTDC and DMA2D config in cubeMX
 */
#define STEERING_WHEEL_LVGL_MODE   (0U)
#define STEERING_WHEEL_OLIVEC_MODE (1U)
#define STEERING_WHEEL_MODE        STEERING_WHEEL_OLIVEC_MODE

#define PERIODIC_SET_ECU_POWER_MAPS

/***
 * Activate the precharge bar when HV accumulator is in precharge or in discharge
 * TODO: make the bar nicer
 */
#define PRECHARGE_BAR_ENABLED (0U)

/***
 * Color resolution in bytes, if you change this please make sure to also change the setting in STM32 CubeMX
 */
#define COLOR_RESOLUTION (4U)

/***
 * Activate the watchdog on CAN bus values: if a value is not update, the watchdog triggers an action
 */
#define WATCHDOG_ENABLED (0U)

/***
 * If WATCHDOG_ENABLED not-updated values are encouraged to use NOT_AVAILABLE_STRING_LABEL
 */
#define NOT_AVAILABLE_STRING_LABEL "NA"

/***
 * Until now MCP23017 is not working with interrupts enables
 */
#define MCP23017_IT_ENABLED (0U)

#define CAN_LOG_ENABLED         (0U)
#define CAN_OVER_SERIAL_ENABLED (0U)

/***
 * LOW LEVEL GRAPHICS SETTING, if you change this be sure to change also STM32 cubeMX settings accordingly
 */
#define SDRAM_BASE_ADDRESS (0xC0000000)
#define FRAMEBUFFER1_ADDR  SDRAM_BASE_ADDRESS
#define FRAMEBUFFER2_ADDR  (0xC0200000)
#define SCREEN_WIDTH       (800u)
#define SCREEN_HEIGHT      (480u)

#define BUTTONS_LONG_PRESS_TIME 500

#define MOTOR_TEMP_ALERT_THRESHOLD    (85.0f)
#define MOTOR_TEMP_CRITICAL_THRESHOLD (100.0f)

#define INVERTER_TEMP_ALERT_THRESHOLD    (65.0f)
#define INVERTER_TEMP_CRITICAL_THRESHOLD (73.0f)

#define SOC_ALERT_THRESHOLD    (0.4f)
#define SOC_CRITICAL_THRESHOLD (0.2f)

#define HV_TEMP_ALERT_THRESHOLD    (46.0f)
#define HV_TEMP_CRITICAL_THRESHOLD (54.0f)

#endif  // STEERING_CONFIG_H
