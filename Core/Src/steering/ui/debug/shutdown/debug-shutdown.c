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
#include "ui/data/ui-data-api.h"
#include "ui/debug/debug-tabs.h"

#include "eagletrt-api.h"
#include "eagletrt.h"

/*! \brief Nodes 1-9 in the left column, 10-17 in the right one, 47 px pitch. */
#define DEBUG_SHUTDOWN_LEFT_ROWS (9U)
#define DEBUG_SHUTDOWN_ROW(INDEX, CAPTION) \
    DEBUG_STATUS_ROW(((INDEX) < DEBUG_SHUTDOWN_LEFT_ROWS) ? 1U : 401U, (uint16_t)(57U + (47U * ((INDEX) % DEBUG_SHUTDOWN_LEFT_ROWS))), CAPTION, (uint8_t)(2U * (INDEX)))

// clang-format off
EAGLETRT_STATIC const struct DebugElement debug_shutdown_elements[] = {
    DEBUG_SHUTDOWN_ROW(SHUTDOWN_NODE_ECU, "1  ECU"),
    DEBUG_SHUTDOWN_ROW(SHUTDOWN_NODE_COCKPIT_MUSHROOM, "2  COCKPIT MUSHROOM"),
    DEBUG_SHUTDOWN_ROW(SHUTDOWN_NODE_FRONT_RIGHT_A_ARM, "3  FRONT RIGHT A-ARM"),
    DEBUG_SHUTDOWN_ROW(SHUTDOWN_NODE_BOTS, "4  BOTS"),
    DEBUG_SHUTDOWN_ROW(SHUTDOWN_NODE_FRONT_LEFT_A_ARM, "5  FRONT LEFT A-ARM"),
    DEBUG_SHUTDOWN_ROW(SHUTDOWN_NODE_INERTIA, "6  INERTIA"),
    DEBUG_SHUTDOWN_ROW(SHUTDOWN_NODE_INVERTER_INTERLOCK, "7  INVERTER"),
    DEBUG_SHUTDOWN_ROW(SHUTDOWN_NODE_REAR_LEFT_A_ARM, "8  REAR LEFT A-ARM"),
    DEBUG_SHUTDOWN_ROW(SHUTDOWN_NODE_BSPD, "9  BSPD"),
    DEBUG_SHUTDOWN_ROW(SHUTDOWN_NODE_HVD, "10  HVD"),
    DEBUG_SHUTDOWN_ROW(SHUTDOWN_NODE_LEFT_MUSHROOM, "11  LEFT MUSHROOM"),
    DEBUG_SHUTDOWN_ROW(SHUTDOWN_NODE_RIGHT_MUSHROOM, "12  RIGHT MUSHROOM"),
    DEBUG_SHUTDOWN_ROW(SHUTDOWN_NODE_AMS, "13  AMS"),
    DEBUG_SHUTDOWN_ROW(SHUTDOWN_NODE_IMD, "14  IMD"),
    DEBUG_SHUTDOWN_ROW(SHUTDOWN_NODE_REAR_RIGHT_A_ARM, "15  REAR RIGHT A-ARM"),
    DEBUG_SHUTDOWN_ROW(SHUTDOWN_NODE_TSMS, "16  TSMS"),
    DEBUG_SHUTDOWN_ROW(SHUTDOWN_NODE_SD_END, "17  SD END"),
    DEBUG_CELL(401U, 433U, 398U, 45U, "", DEBUG_NO_SLOT, 20U), // fills the slot after the last node
};

// clang-format on

EAGLETRT_STATIC void prv_debug_shutdown_sync(struct DebugHandler *handler, const struct UIData *ui) {
    struct ShutdownChain chain;
    EAGLETRT_API_UNUSED(ui_data_api_shutdown(ui, &chain));

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

const struct DebugPage debug_shutdown_page = { "SHUTDOWN", debug_shutdown_elements, sizeof(debug_shutdown_elements) / sizeof(debug_shutdown_elements[0]), prv_debug_shutdown_sync };
