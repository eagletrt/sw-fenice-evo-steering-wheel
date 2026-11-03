/*!
 * \file splash-api.c
 * \date 2026-10-29
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief Implementation of the splash screen.
 */

#include "splash-api.h"

#include "box-api.h"
#include "eagletrt-api.h"
#include "label-api.h"

#include <stdbool.h>
#include <stddef.h>
#include <string.h>

/*!
 * \brief Largest literal run of the logo RLE stream (control bytes 0..127).
 */
#define SPLASH_RLE_LITERAL_LIMIT (128U)

/*!
 * \brief Repeat count of a repeat run is its control byte minus this value (3..130).
 */
#define SPLASH_RLE_REPEAT_BIAS (125U)

/*!
 * \brief Position inside the logo RLE stream (format in tools/generate_logo.py).
 */
struct SplashRleReader {
    uint32_t position; /*!< Next byte of splash_logo_rle to read */
    uint8_t remaining; /*!< Bytes left in the current run */
    bool literal;      /*!< The current run is a literal one */
    uint8_t value;     /*!< Byte of the current repeat run */
};

/*!
 * \brief Next span of equal coverage of the decoded logo.
 *
 * \param reader Stream position, advanced by the span.
 * \param limit Longest span wanted (e.g. what is left of the row).
 * \param value Coverage of every byte of the span.
 *
 * \return Length of the span (1 inside a literal run), 0 if the data is exhausted or corrupt.
 */
EAGLETRT_STATIC uint32_t prv_splash_api_next_span(struct SplashRleReader *reader, uint32_t limit, uint8_t *value) {
    if (reader->remaining == 0U) {
        if (reader->position >= SPLASH_LOGO_RLE_SIZE) {
            return 0U;
        }
        const uint8_t control = splash_logo_rle[reader->position++];
        reader->literal = (control < SPLASH_RLE_LITERAL_LIMIT);
        if (reader->literal) {
            reader->remaining = (uint8_t)(control + 1U);
        } else {
            if (reader->position >= SPLASH_LOGO_RLE_SIZE) {
                return 0U;
            }
            reader->remaining = (uint8_t)(control - SPLASH_RLE_REPEAT_BIAS);
            reader->value = splash_logo_rle[reader->position++];
        }
    }

    if (reader->literal) {
        if (reader->position >= SPLASH_LOGO_RLE_SIZE) {
            return 0U;
        }
        *value = splash_logo_rle[reader->position++];
        reader->remaining--;
        return 1U;
    }

    const uint32_t span = EAGLETRT_API_MIN((uint32_t)reader->remaining, limit);
    reader->remaining = (uint8_t)(reader->remaining - span);
    *value = reader->value;
    return span;
}

/*!
 * \brief Draw the logo through the mask callback, as many rows at a time as the scratch holds.
 *
 * \param raster Raster with a mask callback and a scratch of at least one logo row.
 *
 * \retval SPLASH_RC_OK on success.
 * \retval SPLASH_RC_ERROR if the callback failed or the data is corrupt.
 */
EAGLETRT_STATIC enum SplashReturnCode prv_splash_api_draw_logo_masked(const struct RasterHandler *raster) {
    const uint32_t band_rows = raster->mask.size / SPLASH_LOGO_WIDTH;
    const struct Color color = { .argb = SPLASH_COLOR_LOGO };
    struct SplashRleReader reader = { 0 };

    for (uint32_t y = 0U; y < SPLASH_LOGO_HEIGHT; y += band_rows) {
        const uint32_t rows = EAGLETRT_API_MIN(band_rows, SPLASH_LOGO_HEIGHT - y);
        const uint32_t size = rows * SPLASH_LOGO_WIDTH;
        uint32_t filled = 0U;
        while (filled < size) {
            uint8_t value = 0U;
            const uint32_t span = prv_splash_api_next_span(&reader, size - filled, &value);
            if (span == 0U) {
                return SPLASH_RC_ERROR;
            }
            memset(&raster->mask.buffer[filled], value, span);
            filled += span;
        }
        if (raster->draw_mask(SPLASH_LOGO_X, (uint16_t)(SPLASH_LOGO_Y + y), SPLASH_LOGO_WIDTH, (uint16_t)rows, raster->mask.buffer, color) != RASTER_RC_OK) {
            return SPLASH_RC_ERROR;
        }
    }
    return SPLASH_RC_OK;
}

/*!
 * \brief Draw the logo with one rectangle per run of equal coverage, row by row.
 *
 * \details Coverage goes into the colour alpha, like libraster's own
 *     rectangle path for text. Fully transparent runs draw nothing.
 *
 * \param raster Raster with a rectangle callback.
 *
 * \retval SPLASH_RC_OK on success.
 * \retval SPLASH_RC_ERROR if the callback failed or the data is corrupt.
 */
EAGLETRT_STATIC enum SplashReturnCode prv_splash_api_draw_logo_rectangles(const struct RasterHandler *raster) {
    const struct Color logo = { .argb = SPLASH_COLOR_LOGO };
    struct SplashRleReader reader = { 0 };

    for (uint32_t y = 0U; y < SPLASH_LOGO_HEIGHT; y++) {
        uint32_t x = 0U;
        while (x < SPLASH_LOGO_WIDTH) {
            uint8_t value = 0U;
            const uint32_t span = prv_splash_api_next_span(&reader, SPLASH_LOGO_WIDTH - x, &value);
            if (span == 0U) {
                return SPLASH_RC_ERROR;
            }
            if (value != 0U) {
                struct Color color = logo;
                color.a = (uint8_t)(((uint32_t)value * logo.a) / UINT8_MAX);
                if (raster->draw((uint16_t)(SPLASH_LOGO_X + x), (uint16_t)(SPLASH_LOGO_Y + y), (uint16_t)span, 1U, color) != RASTER_RC_OK) {
                    return SPLASH_RC_ERROR;
                }
            }
            x += span;
        }
    }
    return SPLASH_RC_OK;
}

enum SplashReturnCode splash_api_init(struct SplashHandler *handler, const struct Font *font) {
    if (handler == NULL || font == NULL) {
        return SPLASH_RC_NULL_POINTER;
    }

    memset(handler, 0, sizeof(*handler));
    const struct Color background = { .argb = SPLASH_COLOR_BACKGROUND };
    const int16_t center_x = (int16_t)(SPLASH_SCREEN_WIDTH / 2U);
    const uint16_t title_y = SPLASH_LOGO_BOX_HEIGHT;
    const uint16_t name_y = SPLASH_LOGO_BOX_HEIGHT + SPLASH_TITLE_BOX_HEIGHT;

    if (label_api_init(&handler->title, "", center_x, SPLASH_TITLE_OFFSET_Y, font, SPLASH_TITLE_FONT_SIZE, FONT_ALIGN_CENTER, (struct Color){ .argb = SPLASH_COLOR_TITLE }) != RASTER_RC_OK ||
        label_api_init(&handler->name, "", center_x, SPLASH_NAME_OFFSET_Y, font, SPLASH_NAME_FONT_SIZE, FONT_ALIGN_CENTER, (struct Color){ .argb = SPLASH_COLOR_NAME }) != RASTER_RC_OK) {
        return SPLASH_RC_ERROR;
    }

    if (box_api_init(&handler->boxes[SPLASH_BOX_LOGO], SPLASH_BOX_LOGO, (struct BoxRectangle){ .x = 0U, .y = 0U, .width = SPLASH_SCREEN_WIDTH, .height = SPLASH_LOGO_BOX_HEIGHT }, background, NULL) != RASTER_RC_OK ||
        box_api_init(&handler->boxes[SPLASH_BOX_TITLE], SPLASH_BOX_TITLE, (struct BoxRectangle){ .x = 0U, .y = title_y, .width = SPLASH_SCREEN_WIDTH, .height = SPLASH_TITLE_BOX_HEIGHT }, background, &handler->title) != RASTER_RC_OK ||
        box_api_init(&handler->boxes[SPLASH_BOX_NAME], SPLASH_BOX_NAME, (struct BoxRectangle){ .x = 0U, .y = name_y, .width = SPLASH_SCREEN_WIDTH, .height = (uint16_t)(SPLASH_SCREEN_HEIGHT - name_y) }, background, &handler->name) != RASTER_RC_OK) {
        return SPLASH_RC_ERROR;
    }

    for (uint8_t i = 0U; i < SPLASH_BOX_COUNT; i++) {
        handler->boxes[i].updated = true;
    }
    return SPLASH_RC_OK;
}

/*!
 * \brief Change the text of one caption line, flagging its box only on a real change.
 *
 * \param box Box holding the line.
 * \param text New text, NULL for none.
 *
 * \retval SPLASH_RC_OK on success.
 * \retval SPLASH_RC_ERROR if libraster rejected the text.
 */
EAGLETRT_STATIC enum SplashReturnCode prv_splash_api_set_line(struct Box *box, const char *text) {
    const char *line = (text != NULL) ? text : "";
    if (strcmp(box->label->text, line) == 0) {
        return SPLASH_RC_OK;
    }
    return (box_api_set_label_text(box, line) == RASTER_RC_OK) ? SPLASH_RC_OK : SPLASH_RC_ERROR;
}

enum SplashReturnCode splash_api_set_caption(struct SplashHandler *handler, const char *title, const char *name) {
    if (handler == NULL) {
        return SPLASH_RC_NULL_POINTER;
    }
    if (prv_splash_api_set_line(&handler->boxes[SPLASH_BOX_TITLE], title) != SPLASH_RC_OK) {
        return SPLASH_RC_ERROR;
    }
    return prv_splash_api_set_line(&handler->boxes[SPLASH_BOX_NAME], name);
}

enum SplashReturnCode splash_api_draw_logo(const struct RasterHandler *raster) {
    if (raster == NULL || raster->draw == NULL) {
        return SPLASH_RC_NULL_POINTER;
    }
    if (raster->draw_mask != NULL && raster->mask.buffer != NULL && raster->mask.size >= SPLASH_LOGO_WIDTH) {
        return prv_splash_api_draw_logo_masked(raster);
    }
    return prv_splash_api_draw_logo_rectangles(raster);
}
