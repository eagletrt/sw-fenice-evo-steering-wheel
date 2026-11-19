/*!
 * \file debug-pipo.c
 * \date 2026-11-19
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief PIPO debug tab (Figma "Debug 9/12 - PIPO"): inputs and every output
 *     line (libcan LineCurrent* names, spares left out), three per row.
 */

#include "ui/debug/debug-api.h"
#include "ui/debug/debug-tabs.h"

#include "eagletrt.h"

enum DebugPipoSlot : uint8_t {
    DEBUG_PIPO_INPUT,
    DEBUG_PIPO_CHARGER,
    DEBUG_PIPO_LINES, /*!< Status, current for each line, then the total */
    DEBUG_PIPO_TOTAL = DEBUG_PIPO_LINES + (2U * UI_DATA_PIPO_LINE_COUNT),
};

/*! \brief Line \p I of the 3 x 4 grid (column widths 265 / 264 / 265). */
#define DEBUG_PIPO_LINE(I, CAPTION) DEBUG_LINE((uint16_t)(1U + (267U * ((I) % 3U)) - (((I) % 3U) == 2U ? 1U : 0U)), (uint16_t)(153U + (82U * ((I) / 3U))), (((I) % 3U) == 1U) ? 264U : 265U, CAPTION, (uint8_t)(DEBUG_PIPO_LINES + (2U * (I))))

// clang-format off
EAGLETRT_STATIC const struct DebugElement debug_pipo_elements[] = {
    DEBUG_CELL(  1U, 57U, 398U, 94U, "INPUT LV BATTERY", DEBUG_PIPO_INPUT, 32U),
    DEBUG_CELL(401U, 57U, 398U, 94U, "INPUT CHARGER", DEBUG_PIPO_CHARGER, 32U),
    DEBUG_PIPO_LINE(UI_DATA_PIPO_LINE_TELEMETRY, "TELEMETRY"),
    DEBUG_PIPO_LINE(UI_DATA_PIPO_LINE_POWERTRAIN, "POWERTRAIN"),
    DEBUG_PIPO_LINE(UI_DATA_PIPO_LINE_LIGHTS, "LIGHTS"),
    DEBUG_PIPO_LINE(UI_DATA_PIPO_LINE_DRIVER, "DRIVER"),
    DEBUG_PIPO_LINE(UI_DATA_PIPO_LINE_SHUTDOWN, "SHUTDOWN"),
    DEBUG_PIPO_LINE(UI_DATA_PIPO_LINE_COOLING_LEFT, "COOLING L"),
    DEBUG_PIPO_LINE(UI_DATA_PIPO_LINE_COOLING_RIGHT, "COOLING R"),
    DEBUG_PIPO_LINE(UI_DATA_PIPO_LINE_VISION, "VISION"),
    DEBUG_PIPO_LINE(UI_DATA_PIPO_LINE_PC_AS, "PC AS"),
    DEBUG_PIPO_LINE(UI_DATA_PIPO_LINE_STEERING, "STEERING"),
    DEBUG_PIPO_LINE(UI_DATA_PIPO_LINE_EBS, "EBS"),
    DEBUG_PIPO_LINE(UI_DATA_PIPO_LINE_COUNT, "TOTAL OUT"),
};

/*! \brief The message each line's current comes in. */
EAGLETRT_STATIC const enum UIDataSource debug_pipo_sources[UI_DATA_PIPO_LINE_COUNT] = {
    [UI_DATA_PIPO_LINE_TELEMETRY] = UI_DATA_SOURCE_PIPO_LINES_LOW,
    [UI_DATA_PIPO_LINE_POWERTRAIN] = UI_DATA_SOURCE_PIPO_LINES_LOW,
    [UI_DATA_PIPO_LINE_LIGHTS] = UI_DATA_SOURCE_PIPO_LINES_LOW,
    [UI_DATA_PIPO_LINE_DRIVER] = UI_DATA_SOURCE_PIPO_LINES_LOW,
    [UI_DATA_PIPO_LINE_SHUTDOWN] = UI_DATA_SOURCE_PIPO_LINES_LOW,
    [UI_DATA_PIPO_LINE_COOLING_LEFT] = UI_DATA_SOURCE_PIPO_LINES_HIGH,
    [UI_DATA_PIPO_LINE_COOLING_RIGHT] = UI_DATA_SOURCE_PIPO_LINES_HIGH,
    [UI_DATA_PIPO_LINE_VISION] = UI_DATA_SOURCE_PIPO_LINES_AS,
    [UI_DATA_PIPO_LINE_PC_AS] = UI_DATA_SOURCE_PIPO_LINES_AS,
    [UI_DATA_PIPO_LINE_STEERING] = UI_DATA_SOURCE_PIPO_LINES_AS,
    [UI_DATA_PIPO_LINE_EBS] = UI_DATA_SOURCE_PIPO_LINES_AS,
};
// clang-format on

EAGLETRT_STATIC void prv_debug_pipo_sync(struct DebugHandler *handler, const struct UIData *ui) {
    const bool supply_stale = debug_api_stale(ui, UI_DATA_SOURCE_PIPO_SUPPLY);
    debug_api_number(handler, DEBUG_PIPO_INPUT, supply_stale, ui->lv.input_v, 1U, " V");
    debug_api_number(handler, DEBUG_PIPO_CHARGER, debug_api_stale(ui, UI_DATA_SOURCE_PIPO_CHARGER), ui->lv.charger_v, 1U, " V");

    const bool fuses_stale = debug_api_stale(ui, UI_DATA_SOURCE_PIPO_FUSES);
    for (uint8_t line = 0U; line < UI_DATA_PIPO_LINE_COUNT; line++) {
        const uint8_t status = (uint8_t)(DEBUG_PIPO_LINES + (2U * line));
        const bool blown = !fuses_stale && (((uint32_t)ui->lv.fuse_blown >> line) & 1U) != 0U;
        if (fuses_stale) {
            debug_api_text(handler, status, DEBUG_COLOR_LABEL, "FUSE %s", DEBUG_NO_VALUE);
        } else {
            debug_api_text(handler, status, blown ? DEBUG_COLOR_ERROR : DEBUG_COLOR_OK, "%s", blown ? "FUSE BLOWN" : "FUSE OK");
        }
        if (debug_api_stale(ui, debug_pipo_sources[line])) {
            debug_api_no_value(handler, (uint8_t)(status + 1U));
        } else {
            debug_api_text(handler, (uint8_t)(status + 1U), blown ? DEBUG_COLOR_ERROR : DEBUG_COLOR_VALUE, "%.1f A", (double)ui->lv.line_a[line]);
        }
    }

    if (supply_stale) {
        debug_api_text(handler, DEBUG_PIPO_TOTAL, DEBUG_COLOR_LABEL, "%s", DEBUG_NO_VALUE);
    } else {
        debug_api_text(handler, DEBUG_PIPO_TOTAL, DEBUG_COLOR_LABEL, "%.0f W", (double)ui->lv.out_w);
    }
    debug_api_number(handler, DEBUG_PIPO_TOTAL + 1U, supply_stale, ui->lv.out_a, 1U, " A");
}

const struct DebugPage debug_pipo_page = { "PIPO", debug_pipo_elements, sizeof(debug_pipo_elements) / sizeof(debug_pipo_elements[0]), prv_debug_pipo_sync };
