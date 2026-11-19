/*!
 * \file debug-cooling.c
 * \date 2026-11-19
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief COOLING debug tab (Figma "Debug 3/12 - COOLING").
 *
 * \details TODO(later): pump and fan set-points editable from this tab only
 *     (libcan already has CoolingSteeringWheelSet).
 */

#include "ui/debug/debug-api.h"
#include "ui/debug/debug-tabs.h"

#include "eagletrt.h"

enum DebugCoolingSlot : uint8_t {
    DEBUG_COOLING_PUMP_LEFT,
    DEBUG_COOLING_PUMP_RIGHT,
    DEBUG_COOLING_FAN_LEFT,
    DEBUG_COOLING_FAN_RIGHT,
    DEBUG_COOLING_MOTOR_NTC,
    DEBUG_COOLING_MOTOR_COOLANT = DEBUG_COOLING_MOTOR_NTC + 4U,
    DEBUG_COOLING_INVERTER = DEBUG_COOLING_MOTOR_COOLANT + 4U,
    DEBUG_COOLING_HV_NTC = DEBUG_COOLING_INVERTER + 4U,
    DEBUG_COOLING_TANK_TEMP = DEBUG_COOLING_HV_NTC + UI_DATA_HV_COOLING_COUNT,
    DEBUG_COOLING_TANK_LEVEL,
    DEBUG_COOLING_RADIATOR,
    DEBUG_COOLING_HV_T,
};

#define DEBUG_COOLING_PANEL(X, W, CAPTION, SLOT) DEBUG_QUAD(X, 153U, W, 174U, CAPTION, debug_api_corner_labels, SLOT, 24U, 46U, 71U)

// clang-format off
EAGLETRT_STATIC const struct DebugElement debug_cooling_elements[] = {
    DEBUG_CELL(DEBUG_GRID_X(0), 57U, DEBUG_GRID_W, 94U, "PUMP L", DEBUG_COOLING_PUMP_LEFT, 32U),
    DEBUG_CELL(DEBUG_GRID_X(1), 57U, DEBUG_GRID_W, 94U, "PUMP R", DEBUG_COOLING_PUMP_RIGHT, 32U),
    DEBUG_CELL(DEBUG_GRID_X(2), 57U, DEBUG_GRID_W, 94U, "FAN L", DEBUG_COOLING_FAN_LEFT, 32U),
    DEBUG_CELL(DEBUG_GRID_X(3), 57U, DEBUG_GRID_W, 94U, "FAN R", DEBUG_COOLING_FAN_RIGHT, 32U),
    DEBUG_COOLING_PANEL(  1U, 265U, "MOTOR NTC", DEBUG_COOLING_MOTOR_NTC),
    DEBUG_COOLING_PANEL(268U, 264U, "MOTOR COOLING", DEBUG_COOLING_MOTOR_COOLANT),
    DEBUG_COOLING_PANEL(534U, 265U, "INVERTER MOSFET", DEBUG_COOLING_INVERTER),
    { .kind = DEBUG_ELEMENT_STRIP, .rect = { 1U, 329U, 798U, 74U }, .caption = "HV NTC", .slot = DEBUG_COOLING_HV_NTC, .value_size = 24U, .count = UI_DATA_HV_COOLING_COUNT },
    DEBUG_CELL(DEBUG_GRID_X(0), 405U, DEBUG_GRID_W, 74U, "TANK TEMP L/R", DEBUG_COOLING_TANK_TEMP, 24U),
    DEBUG_CELL(DEBUG_GRID_X(1), 405U, DEBUG_GRID_W, 74U, "TANK LEVEL L/R", DEBUG_COOLING_TANK_LEVEL, 24U),
    DEBUG_CELL(DEBUG_GRID_X(2), 405U, DEBUG_GRID_W, 74U, "RADIATOR L/R", DEBUG_COOLING_RADIATOR, 24U),
    DEBUG_CELL(DEBUG_GRID_X(3), 405U, DEBUG_GRID_W, 74U, "HV T MAX/MIN", DEBUG_COOLING_HV_T, 24U),
};
// clang-format on

/*!
 * \brief Two values in one cell, "34/35°C".
 */
// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
EAGLETRT_STATIC void prv_debug_cooling_pair(struct DebugHandler *handler, uint8_t slot, bool stale, float first, float second, const char *unit) {
    if (stale) {
        debug_api_no_value(handler, slot);
        return;
    }
    debug_api_text(handler, slot, DEBUG_COLOR_VALUE, "%.0f/%.0f%s", (double)first, (double)second, unit);
}

EAGLETRT_STATIC void prv_debug_cooling_sync(struct DebugHandler *handler, const struct UIData *ui) {
    const bool out_stale = debug_api_stale(ui, UI_DATA_SOURCE_COOLING_OUT);
    debug_api_number(handler, DEBUG_COOLING_PUMP_LEFT, out_stale, ui->cooling.pump_percent[0], 0U, "%");
    debug_api_number(handler, DEBUG_COOLING_PUMP_RIGHT, out_stale, ui->cooling.pump_percent[1], 0U, "%");
    debug_api_number(handler, DEBUG_COOLING_FAN_LEFT, out_stale, ui->cooling.fan_percent[0], 0U, "%");
    debug_api_number(handler, DEBUG_COOLING_FAN_RIGHT, out_stale, ui->cooling.fan_percent[1], 0U, "%");

    debug_api_corners(handler, DEBUG_COOLING_MOTOR_NTC, debug_api_stale(ui, UI_DATA_SOURCE_MOTOR), &ui->powertrain.motor_temp, 0U, "°C");
    debug_api_corners(handler, DEBUG_COOLING_MOTOR_COOLANT, debug_api_stale(ui, UI_DATA_SOURCE_COOLING_MOTOR), &ui->cooling.motor, 0U, "°C");
    debug_api_corners(handler, DEBUG_COOLING_INVERTER, debug_api_stale(ui, UI_DATA_SOURCE_INVERTER), &ui->powertrain.inverter_temp, 0U, "°C");

    // Inlet and outlets 1-3 come in TsacMainboardCoolingTemperature1, outlets 4-6 in ...2.
    for (uint8_t i = 0U; i < UI_DATA_HV_COOLING_COUNT; i++) {
        const bool stale = debug_api_stale(ui, (i < 4U) ? UI_DATA_SOURCE_HV_COOLING_1 : UI_DATA_SOURCE_HV_COOLING_2);
        debug_api_number(handler, (uint8_t)(DEBUG_COOLING_HV_NTC + i), stale, ui->hv.cooling[i], 0U, "°C");
    }

    const bool tank_stale = debug_api_stale(ui, UI_DATA_SOURCE_COOLING_TANK);
    prv_debug_cooling_pair(handler, DEBUG_COOLING_TANK_TEMP, tank_stale, ui->cooling.tank_temp[0], ui->cooling.tank_temp[1], "°C");
    prv_debug_cooling_pair(handler, DEBUG_COOLING_TANK_LEVEL, tank_stale, ui->cooling.tank_level_percent[0], ui->cooling.tank_level_percent[1], "%");
    prv_debug_cooling_pair(handler, DEBUG_COOLING_RADIATOR, debug_api_stale(ui, UI_DATA_SOURCE_COOLING_CIRCUIT), ui->cooling.radiator[0], ui->cooling.radiator[1], "°C");
    prv_debug_cooling_pair(handler, DEBUG_COOLING_HV_T, debug_api_stale(ui, UI_DATA_SOURCE_HV_TEMP), ui->hv.cell_t_max, ui->hv.cell_t_min, "°C");
}

const struct DebugPage debug_cooling_page = { "COOLING", debug_cooling_elements, sizeof(debug_cooling_elements) / sizeof(debug_cooling_elements[0]), prv_debug_cooling_sync };
