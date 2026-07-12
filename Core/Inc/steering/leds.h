/*!
 * \file leds.h
 * \date 2026-07-13
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief This file defines structures and enumerations to manage the 6 RGB leds on the steering wheel.
 *
 * \details The module owns the color state and brightness of the strip and
 *     hands fully-scaled colors to a user-supplied transmit callback. The
 *     callback wraps the actual hardware driver (two KTD2052 controllers on
 *     this wheel), so the module itself stays hardware-agnostic.
 */

#ifndef LEDS_H
#define LEDS_H

#include <stdint.h>
#include <stddef.h>

/*!
 * \brief Return codes for LED operations.
 */
enum LedsReturnCode {
    LEDS_RC_OK,                 /*!< Operation successful. */
    LEDS_RC_INVALID_LED,        /*!< The specified LED index is out of range. */
    LEDS_RC_NULL_POINTER,       /*!< A null pointer was passed */
    LEDS_RC_TRANSMISSION_ERROR, /*!< An error ocurred during data transmission */
    LEDS_RC_BUSY,               /*!< The handler is currently busy transmitting data. */
};

/*!
 * \brief Structure representing the color of a LED.
 */
struct LedColor {
    uint8_t r; /*!< Red component (0-255). */
    uint8_t g; /*!< Green component (0-255). */
    uint8_t b; /*!< Blue component (0-255). */
};

/*!
 * \brief Enumeration for LED indices in the strip.
 *
 * \details The old steering wheel carries 6 RGB LEDs in a single top row:
 *     three on the left half (driven by the first KTD2052) and three on the
 *     right half (driven by the second KTD2052). Index 0 is the outermost
 *     left LED and index 5 the outermost right one.
 */
enum LedsIndex {
    LEDS_INDEX_LEFT_0 = 0,  /*!< Outermost left LED */
    LEDS_INDEX_LEFT_1 = 1,  /*!< Middle left LED */
    LEDS_INDEX_LEFT_2 = 2,  /*!< Innermost left LED */
    LEDS_INDEX_RIGHT_0 = 3, /*!< Innermost right LED */
    LEDS_INDEX_RIGHT_1 = 4, /*!< Middle right LED */
    LEDS_INDEX_RIGHT_2 = 5, /*!< Outermost right LED */
    LEDS_INDEX_COUNT = 6    /*!< Total number of LEDs in the strip. */
};

/*!
 * \brief Callback function type for transmitting LED data.
 *
 * This function should be implemented by the user to handle the actual transmission of the LED colors to the hardware. Colors are already scaled by the module brightness.
 *
 * \param colors Pointer to the array of colors to be applied, one entry per LED.
 * \param count The number of entries in \p colors (always LEDS_INDEX_COUNT).
 *
 * \retval LEDS_RC_OK Transmission successful.
 * \retval LEDS_RC_TRANSMISSION_ERROR An error occurred during data transmission.
 * \retval LEDS_RC_NULL_POINTER A null pointer was passed for the colors array.
 * \retval LEDS_RC_BUSY The handler is currently busy transmitting data.
 */
typedef enum LedsReturnCode (*leds_transmit_callback)(const struct LedColor *colors, uint16_t count);

/*!
 * \brief Handler structure for managing the LED system.
 */
struct LedsHandler {
    struct LedColor colors[LEDS_INDEX_COUNT];        /*!< Array of LedColor structures representing the colors of each LED. */
    struct LedColor colors_backup[LEDS_INDEX_COUNT]; /*!< Snapshot used by leds_api_save_pattern / leds_api_restore_pattern. */
    struct LedColor colors_scaled[LEDS_INDEX_COUNT]; /*!< Brightness-scaled copy handed to the transmit callback. */
    float brightness;                                /*!< Brightness level for the LEDs (0-1), where 0 is off and 1 is full brightness. (default: 1) */
    leds_transmit_callback transmit_callback;        /*!< Callback function for transmitting the LED colors to the hardware. */
};

#endif // LEDS_H
