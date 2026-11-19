/*!
 * \file debug-as.c
 * \date 2026-11-19
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief AS debug tab (Figma "Debug 10/12 - AS").
 */

#include "ui/debug/debug-api.h"
#include "ui/debug/debug-tabs.h"

#include "can-primary.h"
#include "eagletrt.h"

enum DebugAsSlot : uint8_t {
    DEBUG_AS_EBS_PRESSURE = 0U, /*!< Tank 1, tank 2, line 1, line 2 */
    DEBUG_AS_STRAIN_1 = 4U,
    DEBUG_AS_STRAIN_2,
    DEBUG_AS_RES,
    DEBUG_AS_PC,
    DEBUG_AS_MISSION,
    DEBUG_AS_ASMS,
    DEBUG_AS_TSMS,
    DEBUG_AS_HV_SOC,
    DEBUG_AS_STATUS,
    DEBUG_AS_HV_TEMP,
    DEBUG_AS_EBS,
};

// clang-format off
EAGLETRT_STATIC const struct DebugElement debug_as_elements[] = {
    DEBUG_QUAD(1U, 57U, 398U, 210U, "EBS PRESSURE", debug_api_ebs_labels, DEBUG_AS_EBS_PRESSURE, 28U, 52U, 89U),
    DEBUG_GRID_CELL(2, 0, "EBS STRAIN 1", DEBUG_AS_STRAIN_1),   DEBUG_GRID_CELL(3, 0, "EBS STRAIN 2", DEBUG_AS_STRAIN_2),
    DEBUG_GRID_CELL(2, 1, "RES", DEBUG_AS_RES),                 DEBUG_GRID_CELL(3, 1, "PC", DEBUG_AS_PC),
    DEBUG_GRID_CELL(0, 2, "MISSION", DEBUG_AS_MISSION),         DEBUG_GRID_CELL(1, 2, "ASMS", DEBUG_AS_ASMS),
    DEBUG_GRID_CELL(2, 2, "TSMS", DEBUG_AS_TSMS),               DEBUG_GRID_CELL(3, 2, "HV SOC", DEBUG_AS_HV_SOC),
    DEBUG_CELL(DEBUG_GRID_X(0), DEBUG_GRID_Y(3), 398U, DEBUG_GRID_H, "AS STATUS", DEBUG_AS_STATUS, 40U),
    DEBUG_GRID_CELL(2, 3, "HV TEMP", DEBUG_AS_HV_TEMP),         DEBUG_GRID_CELL(3, 3, "EBS", DEBUG_AS_EBS),
};
// clang-format on

/*!
 * \brief AS state from EcuFsm, in the ASSI colours (stand-in until the mission message, A24).
 */
EAGLETRT_STATIC void prv_debug_as_status(struct DebugHandler *handler, const struct UIData *ui) {
    if (ui->vehicle_state == UI_DATA_VEHICLE_STATE_COUNT) {
        debug_api_no_value(handler, DEBUG_AS_STATUS);
        return;
    }
    switch ((enum CanPrimaryEcufsmVehiclestatus)ui->ecu_status) {
        case CAN_PRIMARY_ECUFSM_VEHICLESTATUS_AS_READY:
            debug_api_text(handler, DEBUG_AS_STATUS, DEBUG_COLOR_WARNING, "READY");
            break;
        case CAN_PRIMARY_ECUFSM_VEHICLESTATUS_AS_R2D:
            debug_api_text(handler, DEBUG_AS_STATUS, DEBUG_COLOR_WARNING, "R2D");
            break;
        case CAN_PRIMARY_ECUFSM_VEHICLESTATUS_AS_DRIVING:
            debug_api_text(handler, DEBUG_AS_STATUS, DEBUG_COLOR_WARNING, "DRIVING");
            break;
        case CAN_PRIMARY_ECUFSM_VEHICLESTATUS_AS_FINISHED:
            debug_api_text(handler, DEBUG_AS_STATUS, DEBUG_COLOR_AS, "FINISHED");
            break;
        case CAN_PRIMARY_ECUFSM_VEHICLESTATUS_AS_EMERGENCY:
            debug_api_text(handler, DEBUG_AS_STATUS, DEBUG_COLOR_AS, "EMERGENCY");
            break;
        default:
            debug_api_text(handler, DEBUG_AS_STATUS, DEBUG_COLOR_LABEL, "OFF");
            break;
    }
}

EAGLETRT_STATIC void prv_debug_as_sync(struct DebugHandler *handler, const struct UIData *ui) {
    debug_api_ebs(handler, DEBUG_AS_EBS_PRESSURE, ui);
    const bool ebs_stale = debug_api_stale(ui, UI_DATA_SOURCE_EBS);
    debug_api_number(handler, DEBUG_AS_STRAIN_1, ebs_stale, ui->pedals.ebs_load_n[0], 0U, " N");
    debug_api_number(handler, DEBUG_AS_STRAIN_2, ebs_stale, ui->pedals.ebs_load_n[1], 0U, " N");

    const bool shutdown_stale = debug_api_stale(ui, UI_DATA_SOURCE_PIPO_SHUTDOWN);
    debug_api_contact(handler, DEBUG_AS_RES, shutdown_stale, ui->shutdown_v[SHUTDOWN_NODE_RES]);
    debug_api_switch(handler, DEBUG_AS_ASMS, shutdown_stale, ui->shutdown_v[SHUTDOWN_NODE_ASMS]);
    debug_api_switch(handler, DEBUG_AS_TSMS, shutdown_stale, ui->shutdown_v[SHUTDOWN_NODE_TSMS]);

    // TODO(libcan): AS computer status, selected mission (#11), EBS armed state.
    debug_api_number(handler, DEBUG_AS_PC, debug_api_stale(ui, UI_DATA_SOURCE_PC_AS), (float)ui->pc_as_status, 0U, "");
    debug_api_number(handler, DEBUG_AS_MISSION, debug_api_stale(ui, UI_DATA_SOURCE_MISSION), (float)ui->mission, 0U, "");
    if (debug_api_stale(ui, UI_DATA_SOURCE_EBS_STATE)) {
        debug_api_no_value(handler, DEBUG_AS_EBS);
    } else {
        debug_api_text(handler, DEBUG_AS_EBS, (ui->pedals.ebs_armed != 0U) ? DEBUG_COLOR_OK : DEBUG_COLOR_VALUE, "%s", (ui->pedals.ebs_armed != 0U) ? "ARMED" : "DISARMED");
    }

    debug_api_number(handler, DEBUG_AS_HV_SOC, debug_api_stale(ui, UI_DATA_SOURCE_HV_SOC), (float)ui->soc, 0U, "%");
    debug_api_number(handler, DEBUG_AS_HV_TEMP, debug_api_stale(ui, UI_DATA_SOURCE_HV_TEMP), ui->hv.cell_t_max, 0U, "°C");
    prv_debug_as_status(handler, ui);
}

const struct DebugPage debug_as_page = { "AS", debug_as_elements, sizeof(debug_as_elements) / sizeof(debug_as_elements[0]), prv_debug_as_sync };
