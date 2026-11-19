/*!
 * \file debug-sw.c
 * \date 2026-11-19
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief SW debug tab (Figma "Debug 12/12 - SW"): libcan version, commit and
 *     state of every board.
 *
 * \details OFFLINE (grey) while the board's FSM message is stale; CANLIB
 *     MISMATCH (red) when its libcan version differs from the wheel's.
 */

#include "ui/debug/debug-api.h"
#include "ui/debug/debug-tabs.h"

#include "can-version.h"
#include "eagletrt.h"

#include <inttypes.h>
#include <string.h>

/*! \brief Slots of a board row: libcan, commit, state. */
#define DEBUG_SW_SLOT(BOARD) ((uint8_t)(3U * (uint8_t)(BOARD)))

EAGLETRT_STATIC const char *const debug_sw_header[4] = { "BOARD", "CANLIB", "COMMIT", "STATE" };

/*! \brief Row of a board: header 57-92, rows 33 px tall every 35 px from y 95. */
#define DEBUG_SW_ROW(BOARD, CAPTION) DEBUG_TABLE_ROW((uint16_t)(95U + (35U * (uint8_t)(BOARD))), CAPTION, DEBUG_SW_SLOT(BOARD))

// clang-format off
EAGLETRT_STATIC const struct DebugElement debug_sw_elements[] = {
    { .kind = DEBUG_ELEMENT_TABLE_HEADER, .rect = { 1U, 57U, 798U, 36U }, .labels = debug_sw_header },
    DEBUG_SW_ROW(UI_DATA_BOARD_ECU, "ECU"),
    DEBUG_SW_ROW(UI_DATA_BOARD_PEDALS, "PEDALS"),
    DEBUG_SW_ROW(UI_DATA_BOARD_PIPO, "PIPO"),
    DEBUG_SW_ROW(UI_DATA_BOARD_PORCHETTO, "PORCHETTO"),
    DEBUG_SW_ROW(UI_DATA_BOARD_DAS_FRONT, "DAS FRONT"),
    DEBUG_SW_ROW(UI_DATA_BOARD_DAS_REAR, "DAS REAR"),
    DEBUG_SW_ROW(UI_DATA_BOARD_TSAC, "TSAC"),
    DEBUG_SW_ROW(UI_DATA_BOARD_TELEMETRY, "TELEMETRY"),
    DEBUG_SW_ROW(UI_DATA_BOARD_PC_AS, "PC AS"),
    DEBUG_SW_ROW(UI_DATA_BOARD_STEERING, "STEERING"),
    DEBUG_SW_ROW(UI_DATA_BOARD_STEERING_WHEEL, "STEERING WHEEL"),
};

/*! \brief The message telling each board's state (its staleness = OFFLINE). */
EAGLETRT_STATIC const enum UIDataSource debug_sw_sources[UI_DATA_BOARD_COUNT] = {
    [UI_DATA_BOARD_ECU] = UI_DATA_SOURCE_COUNT, // EcuFsm: the router's ECU timeout (vehicle_state unknown)
    [UI_DATA_BOARD_PEDALS] = UI_DATA_SOURCE_PEDALS_STATUS,
    [UI_DATA_BOARD_PIPO] = UI_DATA_SOURCE_PIPO_STATUS,
    [UI_DATA_BOARD_PORCHETTO] = UI_DATA_SOURCE_LV_STATUS,
    [UI_DATA_BOARD_DAS_FRONT] = UI_DATA_SOURCE_DAS_FRONT_STATUS,
    [UI_DATA_BOARD_DAS_REAR] = UI_DATA_SOURCE_DAS_REAR_STATUS,
    [UI_DATA_BOARD_TSAC] = UI_DATA_SOURCE_HV_STATUS,
    [UI_DATA_BOARD_TELEMETRY] = UI_DATA_SOURCE_TELEMETRY_STATUS,
    [UI_DATA_BOARD_PC_AS] = UI_DATA_SOURCE_PC_AS,
    [UI_DATA_BOARD_STEERING] = UI_DATA_SOURCE_STEERING_STATUS,
    [UI_DATA_BOARD_STEERING_WHEEL] = UI_DATA_SOURCE_COUNT, // this board: never offline
};
// clang-format on

/*! \brief EcuFsm vehicle status names (libcan VehicleStatus order, no '_' in the font). */
EAGLETRT_STATIC const char *const debug_sw_ecu_states[] = { "INIT", "IDLE", "PRECHARGE", "TSON", "DISCHARGE", "ASOFF", "AS READY", "AS DRIVING", "ERROR", "R2D", "AS R2D", "AS FINISHED", "AS EMERGENCY" };
/*! \brief TsacStatus mainboard names (libcan TsacMainboardStatus order). */
EAGLETRT_STATIC const char *const debug_sw_tsac_states[] = { "INIT", "IDLE", "ERROR", "FLASH", "BALANCING", "AIRN CHECK", "PRECHARGE", "AIRP CHECK", "TS ON" };
/*! \brief TelemetryFsm names (libcan TelemetryStatus order). */
EAGLETRT_STATIC const char *const debug_sw_telemetry_states[] = { "INIT", "IDLE", "SETUP RUN", "RUN", "ERROR" };
/*! \brief Every other board's FSM (INIT, IDLE, ERROR, FLASH). */
EAGLETRT_STATIC const char *const debug_sw_states[] = { "INIT", "IDLE", "ERROR", "FLASH" };

/*!
 * \brief A board's state name and colour (ERROR red, FLASH yellow, else green).
 */
// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
EAGLETRT_STATIC void prv_debug_sw_state(struct DebugHandler *handler, uint8_t slot, enum UIDataBoard board, uint8_t status) {
    const char *const *names = debug_sw_states;
    size_t count = sizeof(debug_sw_states) / sizeof(debug_sw_states[0]);
    if (board == UI_DATA_BOARD_ECU) {
        names = debug_sw_ecu_states;
        count = sizeof(debug_sw_ecu_states) / sizeof(debug_sw_ecu_states[0]);
    } else if (board == UI_DATA_BOARD_TELEMETRY) {
        names = debug_sw_telemetry_states;
        count = sizeof(debug_sw_telemetry_states) / sizeof(debug_sw_telemetry_states[0]);
    } else if (board == UI_DATA_BOARD_TSAC) {
        names = debug_sw_tsac_states;
        count = sizeof(debug_sw_tsac_states) / sizeof(debug_sw_tsac_states[0]);
    }
    if (status >= count) {
        debug_api_text(handler, slot, DEBUG_COLOR_WARNING, "%" PRIu8, status);
        return;
    }
    const char *name = names[status];
    uint32_t argb = DEBUG_COLOR_OK;
    if (strcmp(name, "ERROR") == 0 || strcmp(name, "AS EMERGENCY") == 0) {
        argb = DEBUG_COLOR_ERROR;
    } else if (strcmp(name, "FLASH") == 0) {
        argb = DEBUG_COLOR_WARNING;
    }
    debug_api_text(handler, slot, argb, "%s", name);
}

EAGLETRT_STATIC void prv_debug_sw_sync(struct DebugHandler *handler, const struct UIData *ui) {
    for (uint8_t board = 0U; board < UI_DATA_BOARD_COUNT; board++) {
        const struct UIDataBoardInfo *info = &ui->boards[board];
        const uint8_t slot = DEBUG_SW_SLOT(board);
        const bool mismatch = (info->known & UI_DATA_BOARD_KNOWN_LIBCAN) != 0U &&
                              (info->libcan[0] != can_version_major || info->libcan[1] != can_version_minor || info->libcan[2] != can_version_patch);
        if ((info->known & UI_DATA_BOARD_KNOWN_LIBCAN) != 0U) {
            debug_api_text(handler, slot, mismatch ? DEBUG_COLOR_ERROR : DEBUG_COLOR_VALUE, "v%" PRIu16 ".%" PRIu16 ".%" PRIu16, info->libcan[0], info->libcan[1], info->libcan[2]);
        } else {
            debug_api_text(handler, slot, DEBUG_COLOR_VALUE, "-");
        }
        if ((info->known & UI_DATA_BOARD_KNOWN_COMMIT) != 0U) {
            debug_api_text(handler, (uint8_t)(slot + 1U), DEBUG_COLOR_VALUE, "%07" PRIx32, info->commit);
        } else {
            debug_api_text(handler, (uint8_t)(slot + 1U), DEBUG_COLOR_VALUE, "-");
        }

        bool offline = false;
        if (board == UI_DATA_BOARD_ECU) {
            offline = ui->vehicle_state == UI_DATA_VEHICLE_STATE_COUNT;
        } else if (board != UI_DATA_BOARD_STEERING_WHEEL) {
            offline = debug_api_stale(ui, debug_sw_sources[board]);
        }
        if (offline) {
            debug_api_text(handler, (uint8_t)(slot + 2U), DEBUG_COLOR_LABEL, "OFFLINE");
        } else if (mismatch) {
            debug_api_text(handler, (uint8_t)(slot + 2U), DEBUG_COLOR_ERROR, "CANLIB MISMATCH");
        } else {
            prv_debug_sw_state(handler, (uint8_t)(slot + 2U), (enum UIDataBoard)board, info->status);
        }
    }
}

const struct DebugPage debug_sw_page = { "SW", debug_sw_elements, sizeof(debug_sw_elements) / sizeof(debug_sw_elements[0]), prv_debug_sw_sync };
