/*!
 * \file debug-pedals.c
 * \date 2026-11-19
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief PEDALS debug tab (Figma "Debug 5/12 - PEDALS")..
 */

#include "ui/debug/debug-api.h"
#include "ui/debug/debug-tabs.h"

#include "can-primary.h"
#include "eagletrt.h"

enum DebugPedalsSlot : uint8_t {
    DEBUG_PEDALS_APPS_V = 0U, /*!< 0..2 */
    DEBUG_PEDALS_BRAKE_SENSOR_V = 3U,
    DEBUG_PEDALS_APPS = 4U, /*!< 4..6 */
    DEBUG_PEDALS_BRAKE_PEDAL = 7U,
    DEBUG_PEDALS_PLAUSIBILITY = 8U,
    DEBUG_PEDALS_BOTS = 9U,
    DEBUG_PEDALS_BIAS = 10U,
    DEBUG_PEDALS_BIAS_SET = 11U,
    DEBUG_PEDALS_PRESSURE = 12U, /*!< 12..15 */
};

// clang-format off
EAGLETRT_STATIC const struct DebugElement debug_pedals_elements[] = {
    DEBUG_GRID_CELL(0, 0, "APPS 1", DEBUG_PEDALS_APPS_V),          DEBUG_GRID_CELL(1, 0, "APPS 2", DEBUG_PEDALS_APPS_V + 1U),
    DEBUG_GRID_CELL(2, 0, "APPS 3", DEBUG_PEDALS_APPS_V + 2U),     DEBUG_GRID_CELL(3, 0, "BRAKE SENSOR", DEBUG_PEDALS_BRAKE_SENSOR_V),
    DEBUG_GRID_CELL(0, 1, "APPS 1", DEBUG_PEDALS_APPS),            DEBUG_GRID_CELL(1, 1, "APPS 2", DEBUG_PEDALS_APPS + 1U),
    DEBUG_GRID_CELL(2, 1, "APPS 3", DEBUG_PEDALS_APPS + 2U),       DEBUG_GRID_CELL(3, 1, "BRAKE PEDAL", DEBUG_PEDALS_BRAKE_PEDAL),
    DEBUG_GRID_CELL(0, 2, "APPS PLAUSIBILITY", DEBUG_PEDALS_PLAUSIBILITY), DEBUG_GRID_CELL(1, 2, "BOTS", DEBUG_PEDALS_BOTS),
    DEBUG_GRID_CELL(2, 2, "BB MEASURED", DEBUG_PEDALS_BIAS),       DEBUG_GRID_CELL(3, 2, "BB COMMANDED", DEBUG_PEDALS_BIAS_SET),
    DEBUG_GRID_CELL(0, 3, "PRESSURE FL", DEBUG_PEDALS_PRESSURE),   DEBUG_GRID_CELL(1, 3, "PRESSURE FR", DEBUG_PEDALS_PRESSURE + 1U),
    DEBUG_GRID_CELL(2, 3, "PRESSURE RL", DEBUG_PEDALS_PRESSURE + 2U), DEBUG_GRID_CELL(3, 3, "PRESSURE RR", DEBUG_PEDALS_PRESSURE + 3U),
};
// clang-format on

EAGLETRT_STATIC void prv_debug_pedals_sync(struct DebugHandler *handler, const struct UIData *ui) {
    const bool voltage_stale = debug_api_stale(ui, UI_DATA_SOURCE_PEDALS_VOLTAGE);
    const bool throttle_stale = debug_api_stale(ui, UI_DATA_SOURCE_THROTTLE);
    for (uint8_t i = 0U; i < 3U; i++) {
        debug_api_number(handler, (uint8_t)(DEBUG_PEDALS_APPS_V + i), voltage_stale, ui->pedals.apps_v[i], 2U, " V");
        debug_api_number(handler, (uint8_t)(DEBUG_PEDALS_APPS + i), throttle_stale, ui->pedals.apps_percent[i], 0U, "%");
    }
    debug_api_number(handler, DEBUG_PEDALS_BRAKE_SENSOR_V, voltage_stale, ui->pedals.brake_sensor_v, 2U, " V");
    debug_api_number(handler, DEBUG_PEDALS_BRAKE_PEDAL, debug_api_stale(ui, UI_DATA_SOURCE_BRAKE), ui->pedals.brake_travel, 0U, "%");

    if (throttle_stale) {
        debug_api_no_value(handler, DEBUG_PEDALS_PLAUSIBILITY);
    } else {
        switch ((enum CanPrimaryPedalsthrottlePlausibility)ui->pedals.plausibility) {
            case CAN_PRIMARY_PEDALSTHROTTLE_PLAUSIBILITY_OK:
                debug_api_text(handler, DEBUG_PEDALS_PLAUSIBILITY, DEBUG_COLOR_OK, "OK");
                break;
            case CAN_PRIMARY_PEDALSTHROTTLE_PLAUSIBILITY_IMPLAUSIBILITY_RECOVERABLE:
                debug_api_text(handler, DEBUG_PEDALS_PLAUSIBILITY, DEBUG_COLOR_WARNING, "RECOVER");
                break;
            default:
                debug_api_text(handler, DEBUG_PEDALS_PLAUSIBILITY, DEBUG_COLOR_ERROR, "ERROR");
                break;
        }
    }
    debug_api_bots(handler, DEBUG_PEDALS_BOTS, ui);
    debug_api_bias(handler, DEBUG_PEDALS_BIAS, DEBUG_PEDALS_BIAS_SET, ui);
    debug_api_corners(handler, DEBUG_PEDALS_PRESSURE, debug_api_stale(ui, UI_DATA_SOURCE_BRAKE), &ui->pedals.pressure_bar, 1U, " bar");
}

const struct DebugPage debug_pedals_page = { "PEDALS", debug_pedals_elements, sizeof(debug_pedals_elements) / sizeof(debug_pedals_elements[0]), prv_debug_pedals_sync };
