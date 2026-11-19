/*!
 * \file debug-wheel.c
 * \date 2026-11-17
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief WHEEL debug tab (Figma "Debug 13/13 - WHEEL"): the steering wheel's
 *     own health, one snapshot per scheduler statistics window (UIData.wheel).
 *
 * \details Colours: bus ACTIVE green, PASSIVE yellow, BUS-OFF red; any error
 *     counter above 0, a CAN error code and a BOR/WDG reset yellow; a full Tx
 *     FIFO (0 free) red.
 */

#include "ui/debug/debug-api.h"
#include "ui/debug/debug-tabs.h"

#include "eagletrt-api.h"
#include "eagletrt.h"

#include <inttypes.h>

enum DebugWheelSlot : uint8_t {
    DEBUG_WHEEL_CAN_BUS,
    DEBUG_WHEEL_TEC_REC,
    DEBUG_WHEEL_LAST_ERROR,
    DEBUG_WHEEL_BUS_OFFS,
    DEBUG_WHEEL_TX_FIFO_FREE,
    DEBUG_WHEEL_TX_REFUSED,
    DEBUG_WHEEL_TX_UNJAMS,
    DEBUG_WHEEL_RESTRICTED,
    DEBUG_WHEEL_FPS,
    DEBUG_WHEEL_FRAME_MAX,
    DEBUG_WHEEL_RENDER_CPU,
    DEBUG_WHEEL_DMA2D_ERRORS,
    DEBUG_WHEEL_DROPPED_RUNS,
    DEBUG_WHEEL_INPUTS_MAX,
    DEBUG_WHEEL_UPTIME,
    DEBUG_WHEEL_LAST_RESET,
};

// clang-format off
EAGLETRT_STATIC const struct DebugElement debug_wheel_elements[] = {
    DEBUG_GRID_CELL(0, 0, "CAN BUS", DEBUG_WHEEL_CAN_BUS),           DEBUG_GRID_CELL(1, 0, "TEC / REC", DEBUG_WHEEL_TEC_REC),
    DEBUG_GRID_CELL(2, 0, "LAST ERROR", DEBUG_WHEEL_LAST_ERROR),     DEBUG_GRID_CELL(3, 0, "BUS-OFFS", DEBUG_WHEEL_BUS_OFFS),
    DEBUG_GRID_CELL(0, 1, "TX FIFO FREE", DEBUG_WHEEL_TX_FIFO_FREE), DEBUG_GRID_CELL(1, 1, "TX REFUSED", DEBUG_WHEEL_TX_REFUSED),
    DEBUG_GRID_CELL(2, 1, "TX UNJAMS", DEBUG_WHEEL_TX_UNJAMS),       DEBUG_GRID_CELL(3, 1, "RESTRICTED", DEBUG_WHEEL_RESTRICTED),
    DEBUG_GRID_CELL(0, 2, "FPS", DEBUG_WHEEL_FPS),                   DEBUG_GRID_CELL(1, 2, "FRAME MAX", DEBUG_WHEEL_FRAME_MAX),
    DEBUG_GRID_CELL(2, 2, "RENDER CPU", DEBUG_WHEEL_RENDER_CPU),     DEBUG_GRID_CELL(3, 2, "DMA2D ERRORS", DEBUG_WHEEL_DMA2D_ERRORS),
    DEBUG_GRID_CELL(0, 3, "DROPPED RUNS", DEBUG_WHEEL_DROPPED_RUNS), DEBUG_GRID_CELL(1, 3, "INPUTS MAX", DEBUG_WHEEL_INPUTS_MAX),
    DEBUG_GRID_CELL(2, 3, "UPTIME", DEBUG_WHEEL_UPTIME),             DEBUG_GRID_CELL(3, 3, "LAST RESET", DEBUG_WHEEL_LAST_RESET),
};
// clang-format on

/*!
 * \brief A counter that should stay 0: yellow otherwise.
 */
EAGLETRT_STATIC void prv_debug_wheel_counter(struct DebugHandler *handler, uint8_t slot, uint32_t count) {
    debug_api_text(handler, slot, (count > 0U) ? DEBUG_COLOR_WARNING : DEBUG_COLOR_VALUE, "%" PRIu32, count);
}

/*!
 * \brief A duration in µs shown as "1.4 ms".
 */
EAGLETRT_STATIC void prv_debug_wheel_milliseconds(struct DebugHandler *handler, uint8_t slot, uint32_t microseconds) {
    debug_api_text(handler, slot, DEBUG_COLOR_VALUE, "%" PRIu32 ".%" PRIu32 " ms", microseconds / 1000U, (microseconds % 1000U) / 100U);
}

EAGLETRT_STATIC void prv_debug_wheel_can(struct DebugHandler *handler, const struct WheelHealth *health) {
    static const char *const states[] = { "ACTIVE", "PASSIVE", "BUS-OFF" };
    static const uint32_t state_colors[] = { DEBUG_COLOR_OK, DEBUG_COLOR_WARNING, DEBUG_COLOR_ERROR };
    const uint8_t state = EAGLETRT_API_MIN((uint8_t)health->can_bus_state, (uint8_t)WHEEL_HEALTH_BUS_OFF);
    debug_api_text(handler, DEBUG_WHEEL_CAN_BUS, state_colors[state], "%s", states[state]);

    const bool errors = health->can_transmit_errors > 0U || health->can_receive_errors > 0U;
    debug_api_text(handler, DEBUG_WHEEL_TEC_REC, errors ? DEBUG_COLOR_WARNING : DEBUG_COLOR_VALUE, "%" PRIu32 " / %" PRIu32, health->can_transmit_errors, health->can_receive_errors);

    // 7 = nothing new since the last PSR read (stats every 5 s): no error in the meantime.
    static const char *const codes[] = { "NONE", "STUFF", "FORM", "ACK", "BIT1", "BIT0", "CRC", "NONE" };
    const uint8_t code = health->can_last_error & 7U;
    debug_api_text(handler, DEBUG_WHEEL_LAST_ERROR, (code == 0U || code == 7U) ? DEBUG_COLOR_VALUE : DEBUG_COLOR_WARNING, "%s", codes[code]);

    prv_debug_wheel_counter(handler, DEBUG_WHEEL_BUS_OFFS, health->can_bus_offs);
    debug_api_text(handler, DEBUG_WHEEL_TX_FIFO_FREE, (health->can_tx_fifo_free == 0U) ? DEBUG_COLOR_ERROR : DEBUG_COLOR_VALUE, "%" PRIu32 "/%" PRIu32, health->can_tx_fifo_free, health->can_tx_fifo_size);
    prv_debug_wheel_counter(handler, DEBUG_WHEEL_TX_REFUSED, health->can_tx_refused);
    prv_debug_wheel_counter(handler, DEBUG_WHEEL_TX_UNJAMS, health->can_tx_unjams);
    prv_debug_wheel_counter(handler, DEBUG_WHEEL_RESTRICTED, health->can_restricted);
}

EAGLETRT_STATIC void prv_debug_wheel_runtime(struct DebugHandler *handler, const struct WheelHealth *health) {
    if (health->window_ms == 0U) {
        debug_api_no_value(handler, DEBUG_WHEEL_FPS);
    } else {
        debug_api_text(handler, DEBUG_WHEEL_FPS, DEBUG_COLOR_VALUE, "%" PRIu32, (health->frames * 1000U) / health->window_ms);
    }
    prv_debug_wheel_milliseconds(handler, DEBUG_WHEEL_FRAME_MAX, health->frame_max_us);
    prv_debug_wheel_milliseconds(handler, DEBUG_WHEEL_RENDER_CPU, health->render_max_us);
    prv_debug_wheel_counter(handler, DEBUG_WHEEL_DMA2D_ERRORS, health->dma2d_errors);

    const uint32_t dropped_argb = (health->dropped_runs > 0U) ? DEBUG_COLOR_WARNING : DEBUG_COLOR_VALUE;
    if (health->window_ms < 1000U) { // the first window closes a few ms after boot
        debug_api_text(handler, DEBUG_WHEEL_DROPPED_RUNS, dropped_argb, "%" PRIu32 " / %" PRIu32 " ms", health->dropped_runs, health->window_ms);
    } else {
        debug_api_text(handler, DEBUG_WHEEL_DROPPED_RUNS, dropped_argb, "%" PRIu32 " / %" PRIu32 " s", health->dropped_runs, (health->window_ms + 500U) / 1000U);
    }
    prv_debug_wheel_milliseconds(handler, DEBUG_WHEEL_INPUTS_MAX, health->inputs_max_us);

    const uint32_t uptime = health->uptime_s;
    if (uptime < 60U) {
        debug_api_text(handler, DEBUG_WHEEL_UPTIME, DEBUG_COLOR_VALUE, "%" PRIu32 " s", uptime);
    } else if (uptime < 3600U) {
        debug_api_text(handler, DEBUG_WHEEL_UPTIME, DEBUG_COLOR_VALUE, "%" PRIu32 " m %" PRIu32 " s", uptime / 60U, uptime % 60U);
    } else {
        debug_api_text(handler, DEBUG_WHEEL_UPTIME, DEBUG_COLOR_VALUE, "%" PRIu32 " h %" PRIu32 " m", uptime / 3600U, (uptime / 60U) % 60U);
    }

    static const char *const causes[WHEEL_HEALTH_RESET_COUNT] = { DEBUG_NO_VALUE, "POR", "BOR", "PIN", "SFT", "WDG" };
    const uint8_t cause = ((uint8_t)health->reset_cause < WHEEL_HEALTH_RESET_COUNT) ? (uint8_t)health->reset_cause : 0U;
    const bool unexpected = cause == WHEEL_HEALTH_RESET_BOR || cause == WHEEL_HEALTH_RESET_WDG;
    debug_api_text(handler, DEBUG_WHEEL_LAST_RESET, unexpected ? DEBUG_COLOR_WARNING : DEBUG_COLOR_VALUE, "%s", causes[cause]);
}

EAGLETRT_STATIC void prv_debug_wheel_sync(struct DebugHandler *handler, const struct UIData *ui) {
    prv_debug_wheel_can(handler, &ui->wheel);
    prv_debug_wheel_runtime(handler, &ui->wheel);
}

const struct DebugPage debug_wheel_page = { "WHEEL", debug_wheel_elements, sizeof(debug_wheel_elements) / sizeof(debug_wheel_elements[0]), prv_debug_wheel_sync };
