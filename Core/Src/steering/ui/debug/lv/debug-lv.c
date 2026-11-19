/*!
 * \file debug-lv.c
 * \date 2026-11-19
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief LV debug tab (Figma "Debug 8/12 - LV"): the LV pack and PIPO summary.
 */

#include "ui/debug/debug-api.h"
#include "ui/debug/debug-tabs.h"

#include "eagletrt.h"

#include <inttypes.h>

enum DebugLvSlot : uint8_t {
    DEBUG_LV_SOC,
    DEBUG_LV_TOTAL,
    DEBUG_LV_CELL_MAX,
    DEBUG_LV_CELL_MIN,
    DEBUG_LV_IMBALANCE,
    DEBUG_LV_POWER,
    DEBUG_LV_CHARGE,
    DEBUG_LV_OUT,
    DEBUG_LV_T_MAX,
    DEBUG_LV_STATE,
    DEBUG_LV_RES,
    DEBUG_LV_ASMS,
    DEBUG_LV_TSMS,
    DEBUG_LV_INPUT,
    DEBUG_LV_FUSES,
};

// clang-format off
EAGLETRT_STATIC const struct DebugElement debug_lv_elements[] = {
    DEBUG_GRID_CELL(0, 0, "SOC", DEBUG_LV_SOC),               DEBUG_GRID_CELL(1, 0, "V TOTAL", DEBUG_LV_TOTAL),
    DEBUG_GRID_CELL(2, 0, "CELL V MAX", DEBUG_LV_CELL_MAX),   DEBUG_GRID_CELL(3, 0, "CELL V MIN", DEBUG_LV_CELL_MIN),
    DEBUG_GRID_CELL(0, 1, "IMBALANCE", DEBUG_LV_IMBALANCE),   DEBUG_GRID_CELL(1, 1, "POWER OUT", DEBUG_LV_POWER),
    DEBUG_GRID_CELL(2, 1, "I CHARGE", DEBUG_LV_CHARGE),       DEBUG_GRID_CELL(3, 1, "I OUT", DEBUG_LV_OUT),
    DEBUG_GRID_CELL(0, 2, "T MAX", DEBUG_LV_T_MAX),
    DEBUG_CELL(DEBUG_GRID_X(1), DEBUG_GRID_Y(2), 398U, DEBUG_GRID_H, "PORCHETTO STATE  /  REASON", DEBUG_LV_STATE, 32U),
    DEBUG_GRID_CELL(3, 2, "RES", DEBUG_LV_RES),
    DEBUG_GRID_CELL(0, 3, "ASMS", DEBUG_LV_ASMS),             DEBUG_GRID_CELL(1, 3, "TSMS", DEBUG_LV_TSMS),
    DEBUG_GRID_CELL(2, 3, "PIPO INPUT", DEBUG_LV_INPUT),      DEBUG_GRID_CELL(3, 3, "PIPO FUSES", DEBUG_LV_FUSES),
};
// clang-format on

EAGLETRT_STATIC void prv_debug_lv_sync(struct DebugHandler *handler, const struct UIData *ui) {
    debug_api_number(handler, DEBUG_LV_SOC, debug_api_stale(ui, UI_DATA_SOURCE_LV_SOC), (float)ui->lv_soc, 0U, "%");
    const bool voltage_stale = debug_api_stale(ui, UI_DATA_SOURCE_LV_VOLTAGE);
    debug_api_number(handler, DEBUG_LV_TOTAL, voltage_stale, ui->lv.total_v, 1U, " V");
    debug_api_number(handler, DEBUG_LV_CELL_MAX, voltage_stale, ui->lv.cell_v_max, 2U, " V");
    debug_api_number(handler, DEBUG_LV_CELL_MIN, voltage_stale, ui->lv.cell_v_min, 2U, " V");
    debug_api_number(handler, DEBUG_LV_IMBALANCE, voltage_stale, (ui->lv.cell_v_max - ui->lv.cell_v_min) * 1000.0F, 0U, " mV");

    const bool supply_stale = debug_api_stale(ui, UI_DATA_SOURCE_PIPO_SUPPLY);
    debug_api_number(handler, DEBUG_LV_POWER, supply_stale, ui->lv.out_w, 0U, " W");
    debug_api_number(handler, DEBUG_LV_CHARGE, debug_api_stale(ui, UI_DATA_SOURCE_PIPO_CHARGER), ui->lv.charge_a, 1U, " A");
    debug_api_number(handler, DEBUG_LV_OUT, supply_stale, ui->lv.out_a, 1U, " A");
    debug_api_number(handler, DEBUG_LV_T_MAX, debug_api_stale(ui, UI_DATA_SOURCE_LV_TEMP), ui->lv.t_max, 0U, "°C");
    debug_api_number(handler, DEBUG_LV_INPUT, supply_stale, ui->lv.input_v, 1U, " V");

    // LvacFsm status names (libcan LvacStatus); the reason is TODO(libcan).
    static const char *const states[] = { "INIT", "IDLE", "ERROR", "FLASH" };
    if (debug_api_stale(ui, UI_DATA_SOURCE_LV_STATUS)) {
        debug_api_no_value(handler, DEBUG_LV_STATE);
    } else {
        const uint8_t status = ui->boards[UI_DATA_BOARD_PORCHETTO].status;
        const char *state = (status < (sizeof(states) / sizeof(states[0]))) ? states[status] : "?";
        const char *reason = debug_api_stale(ui, UI_DATA_SOURCE_LV_REASON) ? DEBUG_NO_VALUE : "OK";
        debug_api_text(handler, DEBUG_LV_STATE, DEBUG_COLOR_VALUE, "%s  /  %s", state, reason);
    }

    const bool shutdown_stale = debug_api_stale(ui, UI_DATA_SOURCE_PIPO_SHUTDOWN);
    debug_api_contact(handler, DEBUG_LV_RES, shutdown_stale, ui->shutdown_v[SHUTDOWN_NODE_RES]);
    debug_api_switch(handler, DEBUG_LV_ASMS, shutdown_stale, ui->shutdown_v[SHUTDOWN_NODE_ASMS]);
    debug_api_switch(handler, DEBUG_LV_TSMS, shutdown_stale, ui->shutdown_v[SHUTDOWN_NODE_TSMS]);

    if (debug_api_stale(ui, UI_DATA_SOURCE_PIPO_FUSES)) {
        debug_api_no_value(handler, DEBUG_LV_FUSES);
        return;
    }
    uint8_t blown = 0U;
    for (uint8_t line = 0U; line < UI_DATA_PIPO_LINE_COUNT; line++) {
        blown += (((uint32_t)ui->lv.fuse_blown >> line) & 1U) ? 1U : 0U;
    }
    if (blown == 0U) {
        debug_api_text(handler, DEBUG_LV_FUSES, DEBUG_COLOR_OK, "%" PRIu8 "/%" PRIu8 " OK", UI_DATA_PIPO_LINE_COUNT, UI_DATA_PIPO_LINE_COUNT);
    } else {
        debug_api_text(handler, DEBUG_LV_FUSES, DEBUG_COLOR_ERROR, "%" PRIu8 " BLOWN", blown);
    }
}

const struct DebugPage debug_lv_page = { "LV", debug_lv_elements, sizeof(debug_lv_elements) / sizeof(debug_lv_elements[0]), prv_debug_lv_sync };
