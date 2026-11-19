/*!
 * \file debug-hv.c
 * \date 2026-11-19
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief HV debug tab (Figma "Debug 1/12 - HV"): the TSAC.
 */

#include "ui/debug/debug-api.h"
#include "ui/debug/debug-tabs.h"

#include "can-primary.h"
#include "eagletrt.h"

#include <inttypes.h>

enum DebugHvSlot : uint8_t {
    DEBUG_HV_CELL_T_MAX,
    DEBUG_HV_CELL_T_MIN,
    DEBUG_HV_CELL_V_MAX,
    DEBUG_HV_CELL_V_MIN,
    DEBUG_HV_IMBALANCE,
    DEBUG_HV_PACK_VOLTAGE,
    DEBUG_HV_DC_LINK,
    DEBUG_HV_CURRENT,
    DEBUG_HV_POWER,
    DEBUG_HV_SOC,
    DEBUG_HV_AIR_PLUS,
    DEBUG_HV_AIR_MINUS,
    DEBUG_HV_PRECHARGE,
    DEBUG_HV_IMPLAUSIBILITY,
    DEBUG_HV_TSAL,
    DEBUG_HV_IMD_AMS,
};

// clang-format off
EAGLETRT_STATIC const struct DebugElement debug_hv_elements[] = {
    DEBUG_GRID_CELL(0, 0, "CELL T MAX", DEBUG_HV_CELL_T_MAX),     DEBUG_GRID_CELL(1, 0, "CELL T MIN", DEBUG_HV_CELL_T_MIN),
    DEBUG_GRID_CELL(2, 0, "CELL V MAX", DEBUG_HV_CELL_V_MAX),     DEBUG_GRID_CELL(3, 0, "CELL V MIN", DEBUG_HV_CELL_V_MIN),
    DEBUG_GRID_CELL(0, 1, "V IMBALANCE", DEBUG_HV_IMBALANCE),     DEBUG_GRID_CELL(1, 1, "PACK VOLTAGE", DEBUG_HV_PACK_VOLTAGE),
    DEBUG_GRID_CELL(2, 1, "DC LINK", DEBUG_HV_DC_LINK),           DEBUG_GRID_CELL(3, 1, "CURRENT", DEBUG_HV_CURRENT),
    DEBUG_GRID_CELL(0, 2, "POWER OUT", DEBUG_HV_POWER),           DEBUG_GRID_CELL(1, 2, "SOC", DEBUG_HV_SOC),
    DEBUG_GRID_CELL(2, 2, "AIR+ EL/MEC", DEBUG_HV_AIR_PLUS),      DEBUG_GRID_CELL(3, 2, "AIR- EL/MEC", DEBUG_HV_AIR_MINUS),
    DEBUG_GRID_CELL(0, 3, "PRECHARGE", DEBUG_HV_PRECHARGE),       DEBUG_GRID_CELL(1, 3, "IMPLAUSIBILITY", DEBUG_HV_IMPLAUSIBILITY),
    DEBUG_GRID_CELL(2, 3, "TSAL", DEBUG_HV_TSAL),                 DEBUG_GRID_CELL(3, 3, "IMD / AMS", DEBUG_HV_IMD_AMS),
};
// clang-format on

/*!
 * \brief An AIR: commanded (EL) and mechanical (MEC) state, red when they disagree.
 *
 * \details TODO(tune): the mechanical feedback is a voltage, "open" at or
 *     above UI_DATA_AIR_CLOSED_V (libcan calls it airpOpenMec / airnOpenMec);
 *     check threshold and polarity on the car.
 */
// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
EAGLETRT_STATIC void prv_debug_hv_air(struct DebugHandler *handler, uint8_t slot, bool stale, uint8_t commanded_closed, float mec_v) {
    if (stale) {
        debug_api_no_value(handler, slot);
        return;
    }
    const bool el = commanded_closed != 0U;
    const bool mec = mec_v < UI_DATA_AIR_CLOSED_V;
    const uint32_t argb = (el != mec) ? DEBUG_COLOR_ERROR : (el ? DEBUG_COLOR_OK : DEBUG_COLOR_VALUE);
    debug_api_text(handler, slot, argb, "%s/%s", el ? "ON" : "OFF", mec ? "ON" : "OFF");
}

EAGLETRT_STATIC void prv_debug_hv_sync(struct DebugHandler *handler, const struct UIData *ui) {
    const bool temp_stale = debug_api_stale(ui, UI_DATA_SOURCE_HV_TEMP);
    debug_api_number(handler, DEBUG_HV_CELL_T_MAX, temp_stale, ui->hv.cell_t_max, 0U, "°C");
    debug_api_number(handler, DEBUG_HV_CELL_T_MIN, temp_stale, ui->hv.cell_t_min, 0U, "°C");

    const bool voltage_stale = debug_api_stale(ui, UI_DATA_SOURCE_HV_VOLTAGE);
    debug_api_number(handler, DEBUG_HV_CELL_V_MAX, voltage_stale, ui->hv.cell_v_max, 2U, " V");
    debug_api_number(handler, DEBUG_HV_CELL_V_MIN, voltage_stale, ui->hv.cell_v_min, 2U, " V");
    debug_api_number(handler, DEBUG_HV_IMBALANCE, voltage_stale, (ui->hv.cell_v_max - ui->hv.cell_v_min) * 1000.0F, 0U, " mV");
    debug_api_number(handler, DEBUG_HV_PACK_VOLTAGE, voltage_stale, ui->hv.pack_v, 0U, " V");
    debug_api_number(handler, DEBUG_HV_DC_LINK, voltage_stale, ui->hv.ts_v, 0U, " V");

    const bool current_stale = debug_api_stale(ui, UI_DATA_SOURCE_HV_CURRENT);
    debug_api_number(handler, DEBUG_HV_CURRENT, current_stale, ui->hv.current_a, 1U, " A");
    debug_api_number(handler, DEBUG_HV_POWER, current_stale, ui->hv.power_kw, 1U, " kW");
    debug_api_number(handler, DEBUG_HV_SOC, debug_api_stale(ui, UI_DATA_SOURCE_HV_SOC), (float)ui->soc, 0U, "%");

    const bool feedback_stale = debug_api_stale(ui, UI_DATA_SOURCE_HV_FEEDBACK);
    prv_debug_hv_air(handler, DEBUG_HV_AIR_PLUS, feedback_stale, ui->hv.airp_closed, ui->hv.airp_mec_v);
    prv_debug_hv_air(handler, DEBUG_HV_AIR_MINUS, feedback_stale, ui->hv.airn_closed, ui->hv.airn_mec_v);

    if (debug_api_stale(ui, UI_DATA_SOURCE_HV_STATUS)) {
        debug_api_no_value(handler, DEBUG_HV_PRECHARGE);
    } else {
        const uint8_t status = ui->boards[UI_DATA_BOARD_BMS_HV].status;
        if (status == CAN_PRIMARY_TSACSTATUS_MAINBOARDSTATUS_PRECHARGE) {
            debug_api_text(handler, DEBUG_HV_PRECHARGE, DEBUG_COLOR_WARNING, "ACTIVE");
        } else if (status == CAN_PRIMARY_TSACSTATUS_MAINBOARDSTATUS_AIRP_CHECK || status == CAN_PRIMARY_TSACSTATUS_MAINBOARDSTATUS_TS_ON) {
            debug_api_text(handler, DEBUG_HV_PRECHARGE, DEBUG_COLOR_OK, "DONE");
        } else {
            debug_api_text(handler, DEBUG_HV_PRECHARGE, DEBUG_COLOR_VALUE, "OFF");
        }
    }

    if (feedback_stale) {
        debug_api_no_value(handler, DEBUG_HV_IMPLAUSIBILITY);
        debug_api_no_value(handler, DEBUG_HV_TSAL);
        debug_api_no_value(handler, DEBUG_HV_IMD_AMS);
        return;
    }
    const bool plausible = ui->hv.plausible != 0U;
    debug_api_text(handler, DEBUG_HV_IMPLAUSIBILITY, plausible ? DEBUG_COLOR_OK : DEBUG_COLOR_ERROR, "%s", plausible ? "NONE" : "FAULT");
    // TODO(tune): green lamp on at or above UI_DATA_TSAL_GREEN_V.
    const bool green = ui->hv.tsal_green_v >= UI_DATA_TSAL_GREEN_V;
    debug_api_text(handler, DEBUG_HV_TSAL, green ? DEBUG_COLOR_OK : DEBUG_COLOR_ERROR, "%s", green ? "GREEN" : "RED");
    const bool faults = ui->hv.imd_fault != 0U || ui->hv.ams_fault != 0U;
    debug_api_text(handler, DEBUG_HV_IMD_AMS, faults ? DEBUG_COLOR_ERROR : DEBUG_COLOR_OK, "%s/%s", (ui->hv.imd_fault != 0U) ? "FAULT" : "OK", (ui->hv.ams_fault != 0U) ? "FAULT" : "OK");
}

const struct DebugPage debug_hv_page = { "HV", debug_hv_elements, sizeof(debug_hv_elements) / sizeof(debug_hv_elements[0]), prv_debug_hv_sync };
