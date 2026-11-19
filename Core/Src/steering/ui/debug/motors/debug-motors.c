/*!
 * \file debug-motors.c
 * \date 2026-11-19
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief MOTORS debug tab (Figma "Debug 2/12 - MOTORS"): per-wheel maxima,
 *     temperatures and tyres.
 */

#include "ui/data/ui-data-api.h"
#include "ui/debug/debug-api.h"
#include "ui/debug/debug-tabs.h"

#include "eagletrt.h"

#define DEBUG_MOTORS_TYRE_COLD_C (35.0F) /*!< TODO(tune): tyres below this are shown cold (cyan) */

enum DebugMotorsSlot : uint8_t {
    DEBUG_MOTORS_TORQUE_MAX = 0U,
    DEBUG_MOTORS_RPM_MAX = 4U,
    DEBUG_MOTORS_MOTOR_TEMP = 8U,
    DEBUG_MOTORS_TYRE_PRESSURE = 12U,
    DEBUG_MOTORS_TYRE_TEMP = 16U,
    DEBUG_MOTORS_DISC_TEMP = 20U,
};

#define DEBUG_MOTORS_PANEL(X, Y, W, CAPTION, SLOT, SIZE, LABEL_Y) DEBUG_QUAD(X, Y, W, 210U, CAPTION, debug_api_corner_labels, SLOT, SIZE, LABEL_Y, 89U)

// clang-format off
EAGLETRT_STATIC const struct DebugElement debug_motors_elements[] = {
    DEBUG_MOTORS_PANEL(  1U,  57U, 265U, "TORQUE MAX 10 S", DEBUG_MOTORS_TORQUE_MAX, 28U, 52U),
    DEBUG_MOTORS_PANEL(268U,  57U, 264U, "RPM MAX 10 S", DEBUG_MOTORS_RPM_MAX, 28U, 52U),
    DEBUG_MOTORS_PANEL(534U,  57U, 265U, "MOTOR TEMP", DEBUG_MOTORS_MOTOR_TEMP, 28U, 52U),
    DEBUG_MOTORS_PANEL(  1U, 269U, 265U, "TYRE PRESSURE", DEBUG_MOTORS_TYRE_PRESSURE, 24U, 55U),
    DEBUG_MOTORS_PANEL(268U, 269U, 264U, "TYRE TEMP", DEBUG_MOTORS_TYRE_TEMP, 28U, 52U),
    DEBUG_MOTORS_PANEL(534U, 269U, 265U, "DISC TEMP", DEBUG_MOTORS_DISC_TEMP, 28U, 52U),
};
// clang-format on

/*!
 * \brief One tyre temperature, cyan while cold.
 */
EAGLETRT_STATIC void prv_debug_motors_tyre(struct DebugHandler *handler, uint8_t slot, bool stale, float celsius) {
    if (stale) {
        debug_api_no_value(handler, slot);
        return;
    }
    debug_api_text(handler, slot, (celsius < DEBUG_MOTORS_TYRE_COLD_C) ? DEBUG_COLOR_COLD : DEBUG_COLOR_VALUE, "%.0f°C", (double)celsius);
}

EAGLETRT_STATIC void prv_debug_motors_sync(struct DebugHandler *handler, const struct UIData *ui) {
    const struct UIDataCorners torque = ui_data_api_window_max(&ui->powertrain.torque_window);
    const struct UIDataCorners rpm = ui_data_api_window_max(&ui->powertrain.rpm_window);
    debug_api_corners(handler, DEBUG_MOTORS_TORQUE_MAX, debug_api_stale(ui, UI_DATA_SOURCE_TORQUE_ACTUAL), &torque, 0U, " Nm");
    debug_api_corners(handler, DEBUG_MOTORS_RPM_MAX, debug_api_stale(ui, UI_DATA_SOURCE_MOTOR_SPEED), &rpm, 0U, "");
    debug_api_corners(handler, DEBUG_MOTORS_MOTOR_TEMP, debug_api_stale(ui, UI_DATA_SOURCE_MOTOR), &ui->powertrain.motor_temp, 0U, "°C");
    debug_api_corners(handler, DEBUG_MOTORS_TYRE_PRESSURE, debug_api_stale(ui, UI_DATA_SOURCE_TYRE_PRESSURE), &ui->chassis.tyre_pressure_bar, 2U, " bar");

    const bool front_stale = debug_api_stale(ui, UI_DATA_SOURCE_TYRE_FRONT);
    const bool rear_stale = debug_api_stale(ui, UI_DATA_SOURCE_TYRE_REAR);
    prv_debug_motors_tyre(handler, DEBUG_MOTORS_TYRE_TEMP, front_stale, ui->chassis.tyre_temp.fl);
    prv_debug_motors_tyre(handler, DEBUG_MOTORS_TYRE_TEMP + 1U, front_stale, ui->chassis.tyre_temp.fr);
    prv_debug_motors_tyre(handler, DEBUG_MOTORS_TYRE_TEMP + 2U, rear_stale, ui->chassis.tyre_temp.rl);
    prv_debug_motors_tyre(handler, DEBUG_MOTORS_TYRE_TEMP + 3U, rear_stale, ui->chassis.tyre_temp.rr);

    debug_api_corners(handler, DEBUG_MOTORS_DISC_TEMP, debug_api_stale(ui, UI_DATA_SOURCE_DISC_TEMP), &ui->chassis.disc_temp, 0U, "°C");
}

const struct DebugPage debug_motors_page = { "MOTORS", debug_motors_elements, sizeof(debug_motors_elements) / sizeof(debug_motors_elements[0]), prv_debug_motors_sync };
