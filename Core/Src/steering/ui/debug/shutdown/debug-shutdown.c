/*!
 * \file debug-shutdown.c
 * \date 2026-11-19
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief SHUTDOWN debug tab (Figma "Debug 7/12 - SHUTDOWN"): every node of
 *     the shutdown circuit, in circuit order.
 *
 * \details States come from the shutdown module (vehicle/shutdown), which
 *     walks the chain in order: CLOSED green, the first open node (the cause)
 *     white OPEN on a red row, the ones after it a grey OPEN, and a node
 *     without a reading a grey UNAVAIL that does not break the chain. Under
 *     each state, the voltage after the node.
 */

#include "ui/debug/debug-api.h"
#include "vehicle/shutdown/shutdown-api.h"
#include "ui/debug/debug-tabs.h"

#include "eagletrt-api.h"
#include "eagletrt.h"

/*! \brief Left column x, right column x, first row y, row pitch. */
#define DEBUG_SHUTDOWN_ROW(INDEX, CAPTION) DEBUG_STATUS_ROW(((INDEX) < 8U) ? 1U : 401U, (uint16_t)(57U + (53U * ((INDEX) % 8U))), CAPTION, (uint8_t)(2U * (INDEX)))

// clang-format off
EAGLETRT_STATIC const struct DebugElement debug_shutdown_elements[SHUTDOWN_NODE_COUNT] = {
    DEBUG_SHUTDOWN_ROW(SHUTDOWN_NODE_AMS, "1  AMS"),
    DEBUG_SHUTDOWN_ROW(SHUTDOWN_NODE_IMD, "2  IMD"),
    DEBUG_SHUTDOWN_ROW(SHUTDOWN_NODE_TSMS, "3  TSMS"),
    DEBUG_SHUTDOWN_ROW(SHUTDOWN_NODE_BOTS, "4  BOTS"),
    DEBUG_SHUTDOWN_ROW(SHUTDOWN_NODE_INERTIA, "5  INERTIA"),
    DEBUG_SHUTDOWN_ROW(SHUTDOWN_NODE_BSPD, "6  BSPD"),
    DEBUG_SHUTDOWN_ROW(SHUTDOWN_NODE_COCKPIT_MUSHROOM, "7  COCKPIT MUSHROOM"),
    DEBUG_SHUTDOWN_ROW(SHUTDOWN_NODE_LEFT_MUSHROOM, "8  LEFT MUSHROOM"),
    DEBUG_SHUTDOWN_ROW(SHUTDOWN_NODE_RIGHT_MUSHROOM, "9  RIGHT MUSHROOM"),
    DEBUG_SHUTDOWN_ROW(SHUTDOWN_NODE_HVD, "10  HVD"),
    DEBUG_SHUTDOWN_ROW(SHUTDOWN_NODE_TSAC_INTERLOCK, "11  TSAC INTERLOCK"),
    DEBUG_SHUTDOWN_ROW(SHUTDOWN_NODE_INVERTER_INTERLOCK, "12  INVERTER INTERLOCK"),
    DEBUG_SHUTDOWN_ROW(SHUTDOWN_NODE_ASMS, "13  ASMS"),
    DEBUG_SHUTDOWN_ROW(SHUTDOWN_NODE_RES, "14  RES"),
    DEBUG_SHUTDOWN_ROW(SHUTDOWN_NODE_EBS, "15  EBS"),
    DEBUG_SHUTDOWN_ROW(SHUTDOWN_NODE_SDC_RELAY, "16  SDC RELAY"),
};

/*! \brief The message each node's voltage comes in. */
EAGLETRT_STATIC const enum UIDataSource debug_shutdown_sources[SHUTDOWN_NODE_COUNT] = {
    [SHUTDOWN_NODE_AMS] = UI_DATA_SOURCE_HV_SHUTDOWN,
    [SHUTDOWN_NODE_IMD] = UI_DATA_SOURCE_HV_SHUTDOWN,
    [SHUTDOWN_NODE_TSMS] = UI_DATA_SOURCE_PIPO_SHUTDOWN,
    [SHUTDOWN_NODE_BOTS] = UI_DATA_SOURCE_BRAKE,
    [SHUTDOWN_NODE_INERTIA] = UI_DATA_SOURCE_DAS_SHUTDOWN,
    [SHUTDOWN_NODE_BSPD] = UI_DATA_SOURCE_PIPO_SHUTDOWN,
    [SHUTDOWN_NODE_COCKPIT_MUSHROOM] = UI_DATA_SOURCE_DAS_SHUTDOWN,
    [SHUTDOWN_NODE_LEFT_MUSHROOM] = UI_DATA_SOURCE_DAS_SHUTDOWN,
    [SHUTDOWN_NODE_RIGHT_MUSHROOM] = UI_DATA_SOURCE_DAS_SHUTDOWN,
    [SHUTDOWN_NODE_HVD] = UI_DATA_SOURCE_PIPO_SHUTDOWN,
    [SHUTDOWN_NODE_TSAC_INTERLOCK] = UI_DATA_SOURCE_HV_SHUTDOWN,
    [SHUTDOWN_NODE_INVERTER_INTERLOCK] = UI_DATA_SOURCE_SHUTDOWN_EXTRA,
    [SHUTDOWN_NODE_ASMS] = UI_DATA_SOURCE_PIPO_SHUTDOWN,
    [SHUTDOWN_NODE_RES] = UI_DATA_SOURCE_PIPO_SHUTDOWN,
    [SHUTDOWN_NODE_EBS] = UI_DATA_SOURCE_EBS,
    [SHUTDOWN_NODE_SDC_RELAY] = UI_DATA_SOURCE_SHUTDOWN_EXTRA,
};
// clang-format on

EAGLETRT_STATIC void prv_debug_shutdown_sync(struct DebugHandler *handler, const struct UIData *ui) {
    uint32_t available = 0U;
    for (uint8_t node = 0U; node < SHUTDOWN_NODE_COUNT; node++) {
        available |= debug_api_stale(ui, debug_shutdown_sources[node]) ? 0U : SHUTDOWN_NODE_BIT(node);
    }
    struct ShutdownChain chain;
    EAGLETRT_API_UNUSED(shutdown_api_evaluate(ui->shutdown_v, available, &chain));

    for (uint8_t node = 0U; node < SHUTDOWN_NODE_COUNT; node++) {
        const uint8_t slot = (uint8_t)(2U * node);
        switch (chain.state[node]) {
            case SHUTDOWN_STATE_CLOSED:
                debug_api_text(handler, slot, DEBUG_COLOR_OK, "CLOSED");
                break;
            case SHUTDOWN_STATE_OPEN:
                debug_api_text(handler, slot, DEBUG_COLOR_VALUE, "OPEN");
                break;
            case SHUTDOWN_STATE_DOWNSTREAM:
                debug_api_text(handler, slot, DEBUG_COLOR_LABEL, "OPEN");
                break;
            case SHUTDOWN_STATE_UNAVAILABLE:
            default:
                debug_api_text(handler, slot, DEBUG_COLOR_LABEL, "UNAVAIL");
                break;
        }
        debug_api_row_background(handler, slot, (chain.state[node] == SHUTDOWN_STATE_OPEN) ? DEBUG_COLOR_ERROR : DEBUG_COLOR_CELL);
        if (chain.state[node] == SHUTDOWN_STATE_UNAVAILABLE) {
            debug_api_text(handler, (uint8_t)(slot + 1U), DEBUG_COLOR_LABEL, "%s", DEBUG_NO_VALUE);
        } else {
            debug_api_text(handler, (uint8_t)(slot + 1U), (chain.state[node] == SHUTDOWN_STATE_OPEN) ? DEBUG_COLOR_VALUE : DEBUG_COLOR_LABEL, "%.1f V", (double)ui->shutdown_v[node]);
        }
    }
}

const struct DebugPage debug_shutdown_page = { "SHUTDOWN", debug_shutdown_elements, SHUTDOWN_NODE_COUNT, prv_debug_shutdown_sync };
