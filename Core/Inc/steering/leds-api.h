/*!
 * \file leds-api.h
 * \date 2026-07-13
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief This file defines the API for controlling the LED strip on the steering wheel, including functions for setting individual LED colors, filling the strip with a color, clearing the strip, and transmitting data to the LEDs.
 */

#ifndef LEDS_API_H
#define LEDS_API_H

#include "leds.h"

/*!
 * \brief Initializes the LED system, clearing all LEDs and preparing the driver for operation.
 *
 * \param transmit A callback function that will be called to transmit the data to the LEDs.
 *
 * \retval LEDS_RC_OK Initialization successful.
 * \retval LEDS_RC_NULL_POINTER A null pointer was passed for the transmit callback.
 */
enum LedsReturnCode leds_api_init(leds_transmit_callback transmit);

/*!
 * \brief Sets the color of a specific LED in the strip.
 *
 * \param index Index of the LED to be set (0-based).
 * \param color LedColor structure representing the desired color for the LED.
 *
 * \retval LEDS_RC_OK Color set successfully.
 * \retval LEDS_RC_INVALID_LED The specified LED index is out of range.
 */
enum LedsReturnCode leds_api_set_led_color(enum LedsIndex index, struct LedColor color);

/*!
 * \brief Fills the entire LED strip with a specific color.
 *
 * \param color LedColor structure representing the desired color for all LEDs.
 */
void leds_api_set_led_color_all(struct LedColor color);

/*!
 * \brief Clears the LED strip by setting all LEDs to off (black).
 */
void leds_api_clear(void);

/*!
 * \brief Sets the brightness level for the LEDs.
 *
 * \param brightness Brightness level (0-1), where 0 is off and 1 is full brightness.
 */
void leds_api_set_brightness(float brightness);

/*!
 * \brief Transmits the current LED color data to the LED strip, updating the displayed colors.
 *
 * \retval LEDS_RC_OK Data transmitted successfully.
 * \retval LEDS_RC_NULL_POINTER The transmit callback function is not set.
 * \retval LEDS_RC_TRANSMISSION_ERROR An error occurred during data transmission.
 * \retval LEDS_RC_BUSY The handler is currently busy transmitting data.
 */
enum LedsReturnCode leds_api_show();

/*!
 * \brief Snapshot the current LED colors into the backup slot.
 *
 * \details Used by transient overlays (such as PTT) that need to restore
 *     whatever pattern was on the strip before they were applied. Calling
 *     it twice in a row simply overwrites the previous snapshot.
 */
void leds_api_save_pattern(void);

/*!
 * \brief Restore the LED colors saved by the last leds_api_save_pattern call.
 *
 * \details If save was never called the backup slot is zeroed, so this
 *     effectively clears the strip.
 */
void leds_api_restore_pattern(void);

/*!
 * \brief Sets the LEDs to indicate the PTT (Push-To-Talk) status.
 *
 * This function can be used to visually indicate when the PTT is active.
 *
 * \details The 2 innermost LEDs will be set to blue
 */
void leds_api_set_ptt_pattern();

/*!
 * \brief Sets the LEDs to indicate the current lap status.
 *
 * This function can be used to visually indicate the current lap number.
 *
 * \details The 4 outermost LEDs (2 per side) will be set to off (TODO: ask if green is better)
 */
void leds_api_set_target_lap_pattern(void);

/*!
 * \brief Sets the LEDs to indicate the current lap status.
 *
 * This function can be used to visually indicate the lap delta is improving.
 *
 * \details The 4 outermost LEDs (2 per side) will be set to green (TODO: ask if green or purple)
 */
void leds_api_set_fast_lap_pattern(void);

/*!
 * \brief Sets the LEDs to indicate the current lap status.
 *
 * This function can be used to visually indicate the lap delta is worsening.
 *
 * \details The 4 outermost LEDs (2 per side) will be set to yellow
 */
void leds_api_set_slow_lap_pattern(void);

/*!
 * \brief Sets the LEDs to indicate an error state.
 *
 * This function can be used to visually indicate that an error has occurred.
 *
 * \details The 2 innermost LEDs will be set to red
 */
void leds_api_set_error_pattern(void);

/*!
 * \brief Sets the LEDs to indicate an OK state.
 *
 * This function can be used to visually indicate that everything is functioning correctly.
 *
 * \details The 2 innermost LEDs will be set to off (TODO: ask if green is better)
 */
void leds_api_set_ok_pattern(void);

#endif // LEDS_API_H
