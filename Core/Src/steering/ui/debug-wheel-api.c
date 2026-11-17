/*!
 * \file debug-wheel-api.c
 * \date 2026-11-17
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief Implementation of the WHEEL debug page (Figma "Debug 13/13 - WHEEL").
 */

#include "debug-wheel-api.h"

#include "box-api.h"
#include "eagletrt-api.h"
#include "label-api.h"
#include "raster-fonts.h"

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#define DEBUG_WHEEL_TITLE_SPLIT_X (460U) /*!< Title box | counter box */
#define DEBUG_WHEEL_LABEL_SIZE (16U)
#define DEBUG_WHEEL_LABEL_OFFSET (20)
#define DEBUG_WHEEL_VALUE_SIZE (36U)

EAGLETRT_STATIC const char *const prv_debug_wheel_api_captions[DEBUG_WHEEL_CELL_COUNT] = {
    [DEBUG_WHEEL_CELL_CAN_BUS] = "CAN BUS",
    [DEBUG_WHEEL_CELL_TEC_REC] = "TEC / REC",
    [DEBUG_WHEEL_CELL_LAST_ERROR] = "LAST ERROR",
    [DEBUG_WHEEL_CELL_BUS_OFFS] = "BUS-OFFS",
    [DEBUG_WHEEL_CELL_TX_FIFO_FREE] = "TX FIFO FREE",
    [DEBUG_WHEEL_CELL_TX_REFUSED] = "TX REFUSED",
    [DEBUG_WHEEL_CELL_TX_UNJAMS] = "TX UNJAMS",
    [DEBUG_WHEEL_CELL_RESTRICTED] = "RESTRICTED",
    [DEBUG_WHEEL_CELL_FPS] = "FPS",
    [DEBUG_WHEEL_CELL_FRAME_MAX] = "FRAME MAX",
    [DEBUG_WHEEL_CELL_RENDER_CPU] = "RENDER CPU",
    [DEBUG_WHEEL_CELL_DMA2D_ERRORS] = "DMA2D ERRORS",
    [DEBUG_WHEEL_CELL_DROPPED_RUNS] = "DROPPED RUNS",
    [DEBUG_WHEEL_CELL_INPUTS_MAX] = "INPUTS MAX",
    [DEBUG_WHEEL_CELL_UPTIME] = "UPTIME",
    [DEBUG_WHEEL_CELL_LAST_RESET] = "LAST RESET",
};

/*!
 * \brief Set a cell's value text, flagging its box only on a real change.
 */
__attribute__((format(printf, 3, 4))) EAGLETRT_STATIC void prv_debug_wheel_api_format(struct DebugWheelHandler *handler, enum DebugWheelCell cell, const char *fmt, ...) {
    char scratch[DEBUG_WHEEL_TEXT_BUFFER_SIZE];
    va_list args;
    va_start(args, fmt);
    const int written = vsnprintf(scratch, sizeof(scratch), fmt, args);
    va_end(args);
    const uint8_t box = DEBUG_WHEEL_BOX_VALUE(cell);
    if (written < 0 || strncmp(scratch, handler->text[box], DEBUG_WHEEL_TEXT_BUFFER_SIZE) == 0) {
        return;
    }
    memcpy(handler->text[box], scratch, sizeof(scratch));
    handler->boxes[box].updated = true;
}

/*!
 * \brief Set a cell's value colour, flagging its box only on a real change.
 */
// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
EAGLETRT_STATIC void prv_debug_wheel_api_color(struct DebugWheelHandler *handler, enum DebugWheelCell cell, uint32_t argb) {
    const uint8_t box = DEBUG_WHEEL_BOX_VALUE(cell);
    if (handler->labels[box].color.argb == argb) {
        return;
    }
    handler->labels[box].color.argb = argb;
    handler->boxes[box].updated = true;
}

/*!
 * \brief A counter that should stay 0: yellow otherwise.
 */
EAGLETRT_STATIC void prv_debug_wheel_api_counter(struct DebugWheelHandler *handler, enum DebugWheelCell cell, uint32_t count) {
    prv_debug_wheel_api_format(handler, cell, "%" PRIu32, count);
    prv_debug_wheel_api_color(handler, cell, (count > 0U) ? DEBUG_WHEEL_COLOR_WARNING : DEBUG_WHEEL_COLOR_VALUE);
}

/*!
 * \brief A duration in µs shown as "1.4 ms".
 */
EAGLETRT_STATIC void prv_debug_wheel_api_milliseconds(struct DebugWheelHandler *handler, enum DebugWheelCell cell, uint32_t microseconds) {
    prv_debug_wheel_api_format(handler, cell, "%" PRIu32 ".%" PRIu32 " ms", microseconds / 1000U, (microseconds % 1000U) / 100U);
}

EAGLETRT_STATIC void prv_debug_wheel_api_can(struct DebugWheelHandler *handler, const struct WheelHealth *health) {
    static const char *const states[] = { "ACTIVE", "PASSIVE", "BUS-OFF" };
    static const uint32_t state_colors[] = { DEBUG_WHEEL_COLOR_OK, DEBUG_WHEEL_COLOR_WARNING, DEBUG_WHEEL_COLOR_ERROR };
    const uint8_t state = EAGLETRT_API_MIN((uint8_t)health->can_bus_state, (uint8_t)WHEEL_HEALTH_BUS_OFF);
    prv_debug_wheel_api_format(handler, DEBUG_WHEEL_CELL_CAN_BUS, "%s", states[state]);
    prv_debug_wheel_api_color(handler, DEBUG_WHEEL_CELL_CAN_BUS, state_colors[state]);

    prv_debug_wheel_api_format(handler, DEBUG_WHEEL_CELL_TEC_REC, "%" PRIu32 " / %" PRIu32, health->can_transmit_errors, health->can_receive_errors);
    const bool errors = health->can_transmit_errors > 0U || health->can_receive_errors > 0U;
    prv_debug_wheel_api_color(handler, DEBUG_WHEEL_CELL_TEC_REC, errors ? DEBUG_WHEEL_COLOR_WARNING : DEBUG_WHEEL_COLOR_VALUE);

    // 7 = nothing new since the last PSR read (stats every 5 s): no error in the meantime.
    static const char *const codes[] = { "NONE", "STUFF", "FORM", "ACK", "BIT1", "BIT0", "CRC", "NONE" };
    const uint8_t code = health->can_last_error & 7U;
    prv_debug_wheel_api_format(handler, DEBUG_WHEEL_CELL_LAST_ERROR, "%s", codes[code]);
    prv_debug_wheel_api_color(handler, DEBUG_WHEEL_CELL_LAST_ERROR, (code == 0U || code == 7U) ? DEBUG_WHEEL_COLOR_VALUE : DEBUG_WHEEL_COLOR_WARNING);

    prv_debug_wheel_api_counter(handler, DEBUG_WHEEL_CELL_BUS_OFFS, health->can_bus_offs);
    prv_debug_wheel_api_format(handler, DEBUG_WHEEL_CELL_TX_FIFO_FREE, "%" PRIu32 "/%" PRIu32, health->can_tx_fifo_free, health->can_tx_fifo_size);
    prv_debug_wheel_api_color(handler, DEBUG_WHEEL_CELL_TX_FIFO_FREE, (health->can_tx_fifo_free == 0U) ? DEBUG_WHEEL_COLOR_ERROR : DEBUG_WHEEL_COLOR_VALUE);
    prv_debug_wheel_api_counter(handler, DEBUG_WHEEL_CELL_TX_REFUSED, health->can_tx_refused);
    prv_debug_wheel_api_counter(handler, DEBUG_WHEEL_CELL_TX_UNJAMS, health->can_tx_unjams);
    prv_debug_wheel_api_counter(handler, DEBUG_WHEEL_CELL_RESTRICTED, health->can_restricted);
}

EAGLETRT_STATIC void prv_debug_wheel_api_runtime(struct DebugWheelHandler *handler, const struct WheelHealth *health) {
    if (health->window_ms == 0U) {
        prv_debug_wheel_api_format(handler, DEBUG_WHEEL_CELL_FPS, "%s", DEBUG_WHEEL_NO_VALUE);
    } else {
        prv_debug_wheel_api_format(handler, DEBUG_WHEEL_CELL_FPS, "%" PRIu32, (health->frames * 1000U) / health->window_ms);
    }
    prv_debug_wheel_api_milliseconds(handler, DEBUG_WHEEL_CELL_FRAME_MAX, health->frame_max_us);
    prv_debug_wheel_api_milliseconds(handler, DEBUG_WHEEL_CELL_RENDER_CPU, health->render_max_us);
    prv_debug_wheel_api_counter(handler, DEBUG_WHEEL_CELL_DMA2D_ERRORS, health->dma2d_errors);

    if (health->window_ms < 1000U) { // the first window closes a few ms after boot
        prv_debug_wheel_api_format(handler, DEBUG_WHEEL_CELL_DROPPED_RUNS, "%" PRIu32 " / %" PRIu32 " ms", health->dropped_runs, health->window_ms);
    } else {
        prv_debug_wheel_api_format(handler, DEBUG_WHEEL_CELL_DROPPED_RUNS, "%" PRIu32 " / %" PRIu32 " s", health->dropped_runs, (health->window_ms + 500U) / 1000U);
    }
    prv_debug_wheel_api_color(handler, DEBUG_WHEEL_CELL_DROPPED_RUNS, (health->dropped_runs > 0U) ? DEBUG_WHEEL_COLOR_WARNING : DEBUG_WHEEL_COLOR_VALUE);
    prv_debug_wheel_api_milliseconds(handler, DEBUG_WHEEL_CELL_INPUTS_MAX, health->inputs_max_us);

    const uint32_t uptime = health->uptime_s;
    if (uptime < 60U) {
        prv_debug_wheel_api_format(handler, DEBUG_WHEEL_CELL_UPTIME, "%" PRIu32 " s", uptime);
    } else if (uptime < 3600U) {
        prv_debug_wheel_api_format(handler, DEBUG_WHEEL_CELL_UPTIME, "%" PRIu32 " m %" PRIu32 " s", uptime / 60U, uptime % 60U);
    } else {
        prv_debug_wheel_api_format(handler, DEBUG_WHEEL_CELL_UPTIME, "%" PRIu32 " h %" PRIu32 " m", uptime / 3600U, (uptime / 60U) % 60U);
    }

    static const char *const causes[WHEEL_HEALTH_RESET_COUNT] = { DEBUG_WHEEL_NO_VALUE, "POR", "BOR", "PIN", "SFT", "WDG" };
    const uint8_t cause = ((uint8_t)health->reset_cause < WHEEL_HEALTH_RESET_COUNT) ? (uint8_t)health->reset_cause : 0U;
    prv_debug_wheel_api_format(handler, DEBUG_WHEEL_CELL_LAST_RESET, "%s", causes[cause]);
    const bool unexpected = cause == WHEEL_HEALTH_RESET_BOR || cause == WHEEL_HEALTH_RESET_WDG;
    prv_debug_wheel_api_color(handler, DEBUG_WHEEL_CELL_LAST_RESET, unexpected ? DEBUG_WHEEL_COLOR_WARNING : DEBUG_WHEEL_COLOR_VALUE);
}

/*!
 * \brief Initialize one box and, when \p text is not NULL, its label.
 */
// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
EAGLETRT_STATIC enum DebugWheelReturnCode prv_debug_wheel_api_box(struct DebugWheelHandler *handler, uint8_t box, struct BoxRectangle rect, uint32_t background, const char *text, int16_t x, int16_t y, uint16_t size, enum FontAlignment align, uint32_t color) {
    struct Label *label = NULL;
    if (text != NULL) {
        (void)snprintf(handler->text[box], DEBUG_WHEEL_TEXT_BUFFER_SIZE, "%s", text);
        label = &handler->labels[box];
        if (label_api_init(label, handler->text[box], x, y, &font_inter, size, align, (struct Color){ .argb = color }) != RASTER_RC_OK) {
            return DEBUG_WHEEL_RC_ERROR;
        }
    }
    if (box_api_init(&handler->boxes[box], box, rect, (struct Color){ .argb = background }, label) != RASTER_RC_OK) {
        return DEBUG_WHEEL_RC_ERROR;
    }
    handler->boxes[box].updated = true;
    return DEBUG_WHEEL_RC_OK;
}

enum DebugWheelReturnCode debug_wheel_api_init(struct DebugWheelHandler *handler) {
    if (handler == NULL) {
        return DEBUG_WHEEL_RC_NULL_POINTER;
    }
    memset(handler, 0, sizeof(*handler));

    enum DebugWheelReturnCode rc = prv_debug_wheel_api_box(handler, DEBUG_WHEEL_BOX_BACKGROUND, (struct BoxRectangle){ 0U, DEBUG_WHEEL_BAR_HEIGHT, STYLE_SCREEN_WIDTH, STYLE_SCREEN_HEIGHT - DEBUG_WHEEL_BAR_HEIGHT }, DEBUG_WHEEL_COLOR_SCREEN, NULL, 0, 0, 0U, FONT_ALIGN_CENTER, 0U);
    // Figma: "WHEEL" 36 px centred on the screen, the counter 16 px at x 475.
    if (rc == DEBUG_WHEEL_RC_OK) {
        rc = prv_debug_wheel_api_box(handler, DEBUG_WHEEL_BOX_TITLE, (struct BoxRectangle){ 0U, 0U, DEBUG_WHEEL_TITLE_SPLIT_X, DEBUG_WHEEL_BAR_HEIGHT }, DEBUG_WHEEL_COLOR_BAR, DEBUG_WHEEL_TITLE, (int16_t)(STYLE_SCREEN_WIDTH / 2U), 6, 36U, FONT_ALIGN_CENTER, DEBUG_WHEEL_COLOR_VALUE);
    }
    if (rc == DEBUG_WHEEL_RC_OK) {
        rc = prv_debug_wheel_api_box(handler, DEBUG_WHEEL_BOX_COUNTER, (struct BoxRectangle){ DEBUG_WHEEL_TITLE_SPLIT_X, 0U, STYLE_SCREEN_WIDTH - DEBUG_WHEEL_TITLE_SPLIT_X, DEBUG_WHEEL_BAR_HEIGHT }, DEBUG_WHEEL_COLOR_BAR, DEBUG_WHEEL_COUNTER, 15, 20, 16U, FONT_ALIGN_LEFT, DEBUG_WHEEL_COLOR_LABEL);
    }
    for (uint8_t cell = 0U; cell < DEBUG_WHEEL_CELL_COUNT && rc == DEBUG_WHEEL_RC_OK; cell++) {
        const uint16_t x = (uint16_t)(DEBUG_WHEEL_CELL_X + ((cell % DEBUG_WHEEL_COLUMNS) * DEBUG_WHEEL_CELL_PITCH_X));
        const uint16_t y = (uint16_t)(DEBUG_WHEEL_CELL_Y + ((cell / DEBUG_WHEEL_COLUMNS) * DEBUG_WHEEL_CELL_PITCH_Y));
        const int16_t center = (int16_t)(DEBUG_WHEEL_CELL_WIDTH / 2U);
        rc = prv_debug_wheel_api_box(handler, DEBUG_WHEEL_BOX_LABEL(cell), (struct BoxRectangle){ x, y, DEBUG_WHEEL_CELL_WIDTH, DEBUG_WHEEL_LABEL_HEIGHT }, DEBUG_WHEEL_COLOR_CELL, prv_debug_wheel_api_captions[cell], center, DEBUG_WHEEL_LABEL_OFFSET, DEBUG_WHEEL_LABEL_SIZE, FONT_ALIGN_CENTER, DEBUG_WHEEL_COLOR_LABEL);
        if (rc == DEBUG_WHEEL_RC_OK) {
            rc = prv_debug_wheel_api_box(handler, DEBUG_WHEEL_BOX_VALUE(cell), (struct BoxRectangle){ x, (uint16_t)(y + DEBUG_WHEEL_LABEL_HEIGHT), DEBUG_WHEEL_CELL_WIDTH, DEBUG_WHEEL_CELL_HEIGHT - DEBUG_WHEEL_LABEL_HEIGHT }, DEBUG_WHEEL_COLOR_CELL, DEBUG_WHEEL_NO_VALUE, center, 0, DEBUG_WHEEL_VALUE_SIZE, FONT_ALIGN_CENTER, DEBUG_WHEEL_COLOR_VALUE);
        }
    }
    return rc;
}

enum DebugWheelReturnCode debug_wheel_api_sync(struct DebugWheelHandler *handler, const struct WheelHealth *health) {
    if (handler == NULL || health == NULL) {
        return DEBUG_WHEEL_RC_NULL_POINTER;
    }
    prv_debug_wheel_api_can(handler, health);
    prv_debug_wheel_api_runtime(handler, health);
    return DEBUG_WHEEL_RC_OK;
}
