/*!
 * \file debug-brakes.c
 * \date 2026-11-19
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief BRAKES debug tab (Figma "Debug 6/12 - BRAKES").
 */

#include "ui/debug/debug-api.h"
#include "ui/debug/debug-tabs.h"

#include "eagletrt.h"

enum DebugBrakesSlot : uint8_t {
    DEBUG_BRAKES_DISC = 0U,
    DEBUG_BRAKES_PRESSURE = 4U,
    DEBUG_BRAKES_EBS = 8U, /*!< Tank 1, tank 2, line 1, line 2 */
    DEBUG_BRAKES_BIAS = 12U,
    DEBUG_BRAKES_BIAS_SET,
    DEBUG_BRAKES_STRAIN_1,
    DEBUG_BRAKES_STRAIN_2,
    DEBUG_BRAKES_BOTS,
    DEBUG_BRAKES_PEDAL,
};

// clang-format off
EAGLETRT_STATIC const struct DebugElement debug_brakes_elements[] = {
    DEBUG_QUAD(  1U, 57U, 265U, 210U, "DISC TEMP", debug_api_corner_labels, DEBUG_BRAKES_DISC, 28U, 52U, 89U),
    DEBUG_QUAD(268U, 57U, 264U, 210U, "PRESSURE", debug_api_corner_labels, DEBUG_BRAKES_PRESSURE, 24U, 55U, 89U),
    DEBUG_QUAD(534U, 57U, 265U, 210U, "EBS PRESSURE", debug_api_ebs_labels, DEBUG_BRAKES_EBS, 24U, 55U, 89U),
    DEBUG_CELL(  1U, 269U, 265U, 104U, "BB MEASURED", DEBUG_BRAKES_BIAS, 32U),
    DEBUG_CELL(  1U, 375U, 265U, 104U, "BB COMMANDED", DEBUG_BRAKES_BIAS_SET, 32U),
    DEBUG_CELL(268U, 269U, 264U, 104U, "EBS STRAIN 1", DEBUG_BRAKES_STRAIN_1, 32U),
    DEBUG_CELL(268U, 375U, 264U, 104U, "EBS STRAIN 2", DEBUG_BRAKES_STRAIN_2, 32U),
    DEBUG_CELL(534U, 269U, 265U, 104U, "BOTS", DEBUG_BRAKES_BOTS, 32U),
    DEBUG_CELL(534U, 375U, 265U, 104U, "BRAKE PEDAL", DEBUG_BRAKES_PEDAL, 32U),
};
// clang-format on

EAGLETRT_STATIC void prv_debug_brakes_sync(struct DebugHandler *handler, const struct UIData *ui) {
    debug_api_corners(handler, DEBUG_BRAKES_DISC, debug_api_stale(ui, UI_DATA_SOURCE_DISC_TEMP), &ui->chassis.disc_temp, 0U, "°C");
    const bool brake_stale = debug_api_stale(ui, UI_DATA_SOURCE_BRAKE);
    debug_api_corners(handler, DEBUG_BRAKES_PRESSURE, brake_stale, &ui->pedals.pressure_bar, 1U, " bar");
    debug_api_ebs(handler, DEBUG_BRAKES_EBS, ui);
    debug_api_bias(handler, DEBUG_BRAKES_BIAS, DEBUG_BRAKES_BIAS_SET, ui);
    const bool ebs_stale = debug_api_stale(ui, UI_DATA_SOURCE_EBS);
    debug_api_number(handler, DEBUG_BRAKES_STRAIN_1, ebs_stale, ui->pedals.ebs_load_n[0], 0U, " N");
    debug_api_number(handler, DEBUG_BRAKES_STRAIN_2, ebs_stale, ui->pedals.ebs_load_n[1], 0U, " N");
    debug_api_bots(handler, DEBUG_BRAKES_BOTS, ui);
    debug_api_number(handler, DEBUG_BRAKES_PEDAL, brake_stale, ui->pedals.brake_travel, 0U, "%");
}

const struct DebugPage debug_brakes_page = { "BRAKES", debug_brakes_elements, sizeof(debug_brakes_elements) / sizeof(debug_brakes_elements[0]), prv_debug_brakes_sync };
