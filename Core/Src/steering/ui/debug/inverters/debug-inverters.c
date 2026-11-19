/*!
 * \file debug-inverters.c
 * \date 2026-11-19
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief INVERTERS debug tab (Figma "Debug 4/12 - INVERTERS").
 *
 * \details TORQUE CMD is MotorTorque, what the ECU sets; TORQUE MAX is the
 *     torque the motors deliver (not on CAN yet). Status, control enable (the enable input as each inverter reads
 *     it, normally equal to sdEnd) and DC link come from the inverter
 *     messages of the next libcan update (TODO(libcan)).
 */

#include "ui/data/ui-data-api.h"
#include "ui/debug/debug-api.h"
#include "ui/debug/debug-tabs.h"

#include "eagletrt.h"

enum DebugInvertersSlot : uint8_t {
    DEBUG_INVERTERS_MOSFET = 0U,
    DEBUG_INVERTERS_TORQUE = 4U,
    DEBUG_INVERTERS_TORQUE_MAX = 8U,
    DEBUG_INVERTERS_STATUS = 12U,
    DEBUG_INVERTERS_ENABLE = 16U,
    DEBUG_INVERTERS_DC_LINK = 20U,
};

#define DEBUG_INVERTERS_PANEL(X, Y, W, CAPTION, SLOT, SIZE, LABEL_Y) DEBUG_QUAD(X, Y, W, 210U, CAPTION, debug_api_corner_labels, SLOT, SIZE, LABEL_Y, 89U)

// clang-format off
EAGLETRT_STATIC const struct DebugElement debug_inverters_elements[] = {
    DEBUG_INVERTERS_PANEL(  1U,  57U, 265U, "MOSFET TEMP", DEBUG_INVERTERS_MOSFET, 28U, 52U),
    DEBUG_INVERTERS_PANEL(268U,  57U, 264U, "TORQUE CMD (ECU)", DEBUG_INVERTERS_TORQUE, 28U, 52U),
    DEBUG_INVERTERS_PANEL(534U,  57U, 265U, "TORQUE MAX 10 S", DEBUG_INVERTERS_TORQUE_MAX, 28U, 52U),
    DEBUG_INVERTERS_PANEL(  1U, 269U, 265U, "STATUS", DEBUG_INVERTERS_STATUS, 24U, 55U),
    DEBUG_INVERTERS_PANEL(268U, 269U, 264U, "CONTROL ENABLE", DEBUG_INVERTERS_ENABLE, 28U, 52U),
    DEBUG_CELL(534U, 269U, 265U, 210U, "DC LINK", DEBUG_INVERTERS_DC_LINK, 48U),
};
// clang-format on

EAGLETRT_STATIC void prv_debug_inverters_sync(struct DebugHandler *handler, const struct UIData *ui) {
    debug_api_corners(handler, DEBUG_INVERTERS_MOSFET, debug_api_stale(ui, UI_DATA_SOURCE_INVERTER), &ui->powertrain.inverter_temp, 0U, "°C");
    const bool torque_stale = debug_api_stale(ui, UI_DATA_SOURCE_TORQUE);
    debug_api_corners(handler, DEBUG_INVERTERS_TORQUE, torque_stale, &ui->powertrain.torque, 0U, " Nm");
    const struct UIDataCorners torque_max = ui_data_api_window_max(&ui->powertrain.torque_window);
    debug_api_corners(handler, DEBUG_INVERTERS_TORQUE_MAX, debug_api_stale(ui, UI_DATA_SOURCE_TORQUE_ACTUAL), &torque_max, 0U, " Nm");

    static const char *const names[UI_DATA_INVERTER_STATUS_COUNT] = { "OFF", "RUN", "DERATE", "FAULT" };
    static const uint32_t colors[UI_DATA_INVERTER_STATUS_COUNT] = { DEBUG_COLOR_LABEL, DEBUG_COLOR_OK, DEBUG_COLOR_WARNING, DEBUG_COLOR_ERROR };
    const bool stale = debug_api_stale(ui, UI_DATA_SOURCE_INVERTER_STATUS);
    for (uint8_t i = 0U; i < 4U; i++) {
        const uint8_t status = (ui->powertrain.inverter_status[i] < UI_DATA_INVERTER_STATUS_COUNT) ? ui->powertrain.inverter_status[i] : UI_DATA_INVERTER_FAULT;
        const bool enabled = ui->powertrain.control_enable[i] != 0U;
        if (stale) {
            debug_api_no_value(handler, (uint8_t)(DEBUG_INVERTERS_STATUS + i));
            debug_api_no_value(handler, (uint8_t)(DEBUG_INVERTERS_ENABLE + i));
            continue;
        }
        debug_api_text(handler, (uint8_t)(DEBUG_INVERTERS_STATUS + i), colors[status], "%s", names[status]);
        debug_api_text(handler, (uint8_t)(DEBUG_INVERTERS_ENABLE + i), enabled ? DEBUG_COLOR_OK : DEBUG_COLOR_LABEL, "%s", enabled ? "ON" : "OFF");
    }
    debug_api_number(handler, DEBUG_INVERTERS_DC_LINK, stale, ui->powertrain.dc_link_v, 0U, " V");
}

const struct DebugPage debug_inverters_page = { "INVERTERS", debug_inverters_elements, sizeof(debug_inverters_elements) / sizeof(debug_inverters_elements[0]), prv_debug_inverters_sync };
