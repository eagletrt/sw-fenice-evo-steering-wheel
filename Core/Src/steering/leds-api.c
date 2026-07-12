/*!
 * \file leds-api.c
 * \date 2026-07-13
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief Implementation of the API for controlling the LED strip on the steering wheel, including functions for setting individual LED colors, filling the strip with a color, clearing the strip, and transmitting data to the LEDs.
 */

#include "leds-api.h"
#include "eagletrt-api.h"
#include <string.h>

#define LEDS_API_BRIGHTNESS_MAX (255U)

EAGLETRT_STATIC struct LedsHandler leds_handler;

/*!
 * \brief Fill an inclusive index range with one color.
 *
 * \param first First LED of the range.
 * \param last  Last LED of the range (inclusive).
 * \param color Color to apply to every LED in the range.
 */
EAGLETRT_STATIC void prv_leds_api_fill_range(enum LedsIndex first, enum LedsIndex last, struct LedColor color) {
    for (size_t i = first; i <= (size_t)last; i++) {
        leds_handler.colors[i] = color;
    }
}

enum LedsReturnCode leds_api_init(leds_transmit_callback transmit) {
    if (transmit == NULL) {
        return LEDS_RC_NULL_POINTER;
    }

    memset(&leds_handler, 0, sizeof(leds_handler));
    leds_handler.brightness = 1.0F;
    leds_handler.transmit_callback = transmit;
    return LEDS_RC_OK;
}

enum LedsReturnCode leds_api_set_led_color(enum LedsIndex index, struct LedColor color) {
    if (index >= LEDS_INDEX_COUNT) {
        return LEDS_RC_INVALID_LED;
    }

    leds_handler.colors[index] = color;
    return LEDS_RC_OK;
}

void leds_api_set_led_color_all(struct LedColor color) {
    prv_leds_api_fill_range(LEDS_INDEX_LEFT_0, LEDS_INDEX_RIGHT_2, color);
}

void leds_api_clear(void) {
    struct LedColor off = { 0, 0, 0 };
    leds_api_set_led_color_all(off);
}

void leds_api_set_brightness(float brightness) {
    leds_handler.brightness = EAGLETRT_API_CLAMP(brightness, 0.0F, 1.0F);
}

enum LedsReturnCode leds_api_show() {
    if (leds_handler.transmit_callback == NULL) {
        return LEDS_RC_NULL_POINTER;
    }

    for (size_t i = 0; i < LEDS_INDEX_COUNT; i++) {
        leds_handler.colors_scaled[i].r = (uint8_t)((float)leds_handler.colors[i].r * leds_handler.brightness);
        leds_handler.colors_scaled[i].g = (uint8_t)((float)leds_handler.colors[i].g * leds_handler.brightness);
        leds_handler.colors_scaled[i].b = (uint8_t)((float)leds_handler.colors[i].b * leds_handler.brightness);
    }

    return leds_handler.transmit_callback(leds_handler.colors_scaled, LEDS_INDEX_COUNT);
}

void leds_api_save_pattern(void) {
    memcpy(leds_handler.colors_backup, leds_handler.colors, sizeof(leds_handler.colors));
}

void leds_api_restore_pattern(void) {
    memcpy(leds_handler.colors, leds_handler.colors_backup, sizeof(leds_handler.colors));
}

void leds_api_set_ptt_pattern(void) {
    struct LedColor blue = { .r = 0, .g = 0, .b = LEDS_API_BRIGHTNESS_MAX };
    prv_leds_api_fill_range(LEDS_INDEX_LEFT_2, LEDS_INDEX_RIGHT_0, blue);
}

void leds_api_set_target_lap_pattern(void) {
    struct LedColor off = { 0, 0, 0 };
    prv_leds_api_fill_range(LEDS_INDEX_LEFT_0, LEDS_INDEX_LEFT_1, off);
    prv_leds_api_fill_range(LEDS_INDEX_RIGHT_1, LEDS_INDEX_RIGHT_2, off);
}

void leds_api_set_fast_lap_pattern(void) {
    struct LedColor green = { .r = 0, .g = LEDS_API_BRIGHTNESS_MAX, .b = 0 };
    prv_leds_api_fill_range(LEDS_INDEX_LEFT_0, LEDS_INDEX_LEFT_1, green);
    prv_leds_api_fill_range(LEDS_INDEX_RIGHT_1, LEDS_INDEX_RIGHT_2, green);
}

void leds_api_set_slow_lap_pattern(void) {
    struct LedColor yellow = { .r = LEDS_API_BRIGHTNESS_MAX, .g = LEDS_API_BRIGHTNESS_MAX, .b = 0 };
    prv_leds_api_fill_range(LEDS_INDEX_LEFT_0, LEDS_INDEX_LEFT_1, yellow);
    prv_leds_api_fill_range(LEDS_INDEX_RIGHT_1, LEDS_INDEX_RIGHT_2, yellow);
}

void leds_api_set_error_pattern(void) {
    struct LedColor red = { .r = LEDS_API_BRIGHTNESS_MAX, .g = 0, .b = 0 };
    prv_leds_api_fill_range(LEDS_INDEX_LEFT_2, LEDS_INDEX_RIGHT_0, red);
}

void leds_api_set_ok_pattern(void) {
    struct LedColor off = { 0, 0, 0 };
    prv_leds_api_fill_range(LEDS_INDEX_LEFT_2, LEDS_INDEX_RIGHT_0, off);
}
