/*!
 * \file bootloader-screen.c
 * \date 2026-10-29
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 *
 * \brief Bootloader screen, see bootloader-screen.h.
 */

#include "bootloader-screen.h"

#include "raster-api.h"
#include "raster-fonts.h"
#include "splash-api.h"

#include <stddef.h>
#include <stdint.h>

/*!
 * \brief LTDC layer 0 framebuffer (ARGB8888, FBStartAdress in the .ioc).
 *
 * \details Overridable so the drawing code can run on a PC.
 */
#ifndef BOOTLOADER_SCREEN_FRAMEBUFFER
#define BOOTLOADER_SCREEN_FRAMEBUFFER ((uint32_t *)0xC0000000U)
#endif

/*!
 * \brief Scratch for the coverage masks: a band of the logo (63 rows) or a whole caption line.
 */
#define BOOTLOADER_SCREEN_MASK_SIZE (32U * 1024U)

#define BOOTLOADER_SCREEN_TITLE "FLASHING"
#define BOOTLOADER_SCREEN_NAME "STEERING WHEEL"

static struct RasterHandler bootloader_screen_raster;
static struct SplashHandler bootloader_screen_splash;
static uint8_t bootloader_screen_mask[BOOTLOADER_SCREEN_MASK_SIZE];
static bool bootloader_screen_flashing;

/*!
 * \brief Blend one colour channel: \p alpha of \p source over \p destination.
 */
static uint32_t bootloader_screen_blend_channel(uint32_t source, uint32_t destination, uint32_t alpha) {
    return ((source * alpha) + (destination * (UINT8_MAX - alpha))) / UINT8_MAX;
}

/*!
 * \brief Write \p color with coverage \p alpha over one pixel.
 */
static void bootloader_screen_blend_pixel(uint32_t *pixel, struct Color color, uint32_t alpha) {
    if (alpha == 0U) {
        return;
    }
    if (alpha == UINT8_MAX) {
        *pixel = color.argb | 0xFF000000U;
        return;
    }
    const uint32_t background = *pixel;
    const uint32_t red = bootloader_screen_blend_channel(color.r, (background >> 16U) & 0xFFU, alpha);
    const uint32_t green = bootloader_screen_blend_channel(color.g, (background >> 8U) & 0xFFU, alpha);
    const uint32_t blue = bootloader_screen_blend_channel(color.b, background & 0xFFU, alpha);
    *pixel = 0xFF000000U | (red << 16U) | (green << 8U) | blue;
}

/*!
 * \brief libraster rectangle callback: fill, or blend when the colour is translucent.
 */
static enum RasterReturnCode bootloader_screen_draw_rectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, struct Color color) {
    if (x >= SPLASH_SCREEN_WIDTH || y >= SPLASH_SCREEN_HEIGHT) {
        return RASTER_RC_OK;
    }
    const uint32_t right = (x + width > SPLASH_SCREEN_WIDTH) ? SPLASH_SCREEN_WIDTH : (uint32_t)x + width;
    const uint32_t bottom = (y + height > SPLASH_SCREEN_HEIGHT) ? SPLASH_SCREEN_HEIGHT : (uint32_t)y + height;
    for (uint32_t row = y; row < bottom; row++) {
        uint32_t *line = &BOOTLOADER_SCREEN_FRAMEBUFFER[row * SPLASH_SCREEN_WIDTH];
        for (uint32_t column = x; column < right; column++) {
            bootloader_screen_blend_pixel(&line[column], color, color.a);
        }
    }
    return RASTER_RC_OK;
}

/*!
 * \brief libraster mask callback: blend \p color through an A8 coverage mask.
 */
static enum RasterReturnCode bootloader_screen_draw_mask(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint8_t *mask, struct Color color) {
    for (uint32_t row = 0U; row < height && (uint32_t)y + row < SPLASH_SCREEN_HEIGHT; row++) {
        uint32_t *line = &BOOTLOADER_SCREEN_FRAMEBUFFER[(y + row) * SPLASH_SCREEN_WIDTH];
        const uint8_t *coverage = &mask[row * width];
        for (uint32_t column = 0U; column < width && (uint32_t)x + column < SPLASH_SCREEN_WIDTH; column++) {
            bootloader_screen_blend_pixel(&line[x + column], color, ((uint32_t)coverage[column] * color.a) / UINT8_MAX);
        }
    }
    return RASTER_RC_OK;
}

bool bootloader_screen_init(void) {
    bootloader_screen_flashing = false;
    if (splash_api_init(&bootloader_screen_splash, &font_inter_boot) != SPLASH_RC_OK ||
        raster_api_init(&bootloader_screen_raster, bootloader_screen_splash.boxes, SPLASH_BOX_COUNT, bootloader_screen_draw_rectangle, NULL) != RASTER_RC_OK ||
        raster_api_set_mask(&bootloader_screen_raster, bootloader_screen_draw_mask, bootloader_screen_mask, sizeof(bootloader_screen_mask)) != RASTER_RC_OK) {
        return false;
    }
    return raster_api_render(&bootloader_screen_raster) == RASTER_RC_OK &&
           splash_api_draw_logo(&bootloader_screen_raster) == SPLASH_RC_OK;
}

void bootloader_screen_update(bool flashing) {
    if (flashing == bootloader_screen_flashing) {
        return;
    }
    bootloader_screen_flashing = flashing;
    // Only the caption boxes change: the logo box isn't redrawn, so the logo stays.
    if (splash_api_set_caption(&bootloader_screen_splash, flashing ? BOOTLOADER_SCREEN_TITLE : NULL, flashing ? BOOTLOADER_SCREEN_NAME : NULL) == SPLASH_RC_OK) {
        (void)raster_api_render(&bootloader_screen_raster);
    }
}
