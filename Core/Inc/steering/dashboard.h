/*!
 * \file dashboard.h
 * \date 2026-06-07
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief Types and storage for the main steering-wheel dashboard interface.
 *
 * \details The dashboard is the always-on view: a strip of telemetry/setup
 *     boxes (scenario presets on the left, HV/INV in the middle, lap +
 *     tire/motor temperatures on the right). Every field on screen is one
 *     Box backed by one Label whose text comes from a small writable buffer
 *     stored in the handler. Setters re-format that buffer and flag the
 *     box as updated so the raster picks the change up on the next render.
 */

#ifndef DASHBOARD_H
#define DASHBOARD_H

#include "box.h"
#include "label.h"
#include "colors.h"
#include "style.h"
#include <stdint.h>

#define DASHBOARD_COLOR_PRIMARY (STYLE_DARK_GRAY)
#define DASHBOARD_COLOR_SECONDARY (STYLE_WHITE)
#define DASHBOARD_COLOR_LINES (STYLE_LIGHT_GRAY)
#define DASHBOARD_COLOR_ALL_OK (STYLE_GREEN)
#define DASHBOARD_COLOR_WARNING (STYLE_YELLOW)
#define DASHBOARD_COLOR_ERROR (STYLE_RED)
#define DASHBOARD_COLOR_FAST_LAP (STYLE_MAGENTA)
#define DASHBOARD_COLOR_COLD_TIRES (STYLE_CYAN)

// TODO: tune these data (and integrate S.P.E.C.)
#define DASHBOARD_THRESHOLD_HV_SOC_PERCENT_WARNING (30U)               /* at or below this SoC is a warning */
#define DASHBOARD_THRESHOLD_HV_SOC_PERCENT_ERROR (15U)                 /* at or below this SoC is an error */
#define DASHBOARD_THRESHOLD_HV_TEMP_CELSIUS_WARNING (45U)              /* at or above this HV pack temp is a warning */
#define DASHBOARD_THRESHOLD_HV_TEMP_CELSIUS_ERROR (50U)                /* at or above this HV pack temp is an error */
#define DASHBOARD_THRESHOLD_INVERTER_TEMPERATURE_CELSIUS_WARNING (60U) /* at or above this inverter temp is a warning */
#define DASHBOARD_THRESHOLD_INVERTER_TEMPERATURE_CELSIUS_ERROR (70U)   /* at or above this inverter temp is an error */
#define DASHBOARD_THRESHOLD_TIRE_TEMPERATURE_CELSIUS_LOW (30U)         /* at or below this tire temp is "cold tires" */
#define DASHBOARD_THRESHOLD_TIRE_TEMPERATURE_CELSIUS_WARNING (80U)     /* at or above this tire temp is a warning */
#define DASHBOARD_THRESHOLD_TIRE_TEMPERATURE_CELSIUS_ERROR (100U)      /* at or above this tire temp is an error */
#define DASHBOARD_THRESHOLD_MOTOR_TEMPERATURE_CELSIUS_WARNING (80U)    /* at or above this motor temp is a warning */
#define DASHBOARD_THRESHOLD_MOTOR_TEMPERATURE_CELSIUS_ERROR (100U)     /* at or above this motor temp is an error */

#define DASHBOARD_FONT_SIZE_HEADER (40U)        /* section headers (SCENARIO, HV, TRS, MTR) */
#define DASHBOARD_FONT_SIZE_FIELD_LABEL (30U)   /* per-row labels (RGN, TQ, POW, SLIP, INV, LAP) */
#define DASHBOARD_FONT_SIZE_VALUE (50U)         /* readable value text shown beside its label */
#define DASHBOARD_FONT_SIZE_MOTORS_PIXELS (35U) /* tire/motor temperature cells */
#define DASHBOARD_FONT_SIZE_STATE (35U)         /* FSM-state text */
#define DASHBOARD_FONT_SIZE_DELTA (45U)         /* lap delta */
#define DASHBOARD_FONT_SIZE_SOC (120U)          /* big SoC value */

/*!
 * \brief Return codes for dashboard operations.
 */
enum DashboardReturnCode {
    DASHBOARD_RC_OK,           /*!< Operation completed successfully */
    DASHBOARD_RC_NULL_POINTER, /*!< A required pointer argument was NULL */
    DASHBOARD_RC_ERROR,        /*!< General error */
};

/*!
 * \brief Identifier of every drawable field on the dashboard.
 *
 * \details Each entry maps one-to-one to a Box and a Label in DashboardHandler.
 *     The ordering only matters for traversal in dashboard_api_init; consumers
 *     never index by raw integer.
 */
enum DashboardFieldId : uint8_t {
    DASHBOARD_FIELD_DASHBOARD_BACKGROUND, /*!< Background box covering the whole dashboard */
    DASHBOARD_FIELD_SCENARIO_HEADER,      /*!< "SCENARIO" header, left strip top */
    DASHBOARD_FIELD_REGEN_BACKGROUND,     /*!< Background box for the regen slot */
    DASHBOARD_FIELD_REGEN_LABEL,          /*!< "RGN" inline label */
    DASHBOARD_FIELD_REGEN,                /*!< Regen level value, e.g. "10" */
    DASHBOARD_FIELD_TORQUE_BACKGROUND,    /*!< Background box for the torque slot */
    DASHBOARD_FIELD_TORQUE_LABEL,         /*!< "TQ" inline label */
    DASHBOARD_FIELD_TORQUE,               /*!< Torque level value, e.g. "6" */
    DASHBOARD_FIELD_POWER_BACKGROUND,     /*!< Background box for the power slot */
    DASHBOARD_FIELD_POWER_LABEL,          /*!< "POW" inline label */
    DASHBOARD_FIELD_POWER,                /*!< Power level value, e.g. "5" */
    DASHBOARD_FIELD_SLIP_BACKGROUND,      /*!< Background box for the slip slot */
    DASHBOARD_FIELD_SLIP_LABEL,           /*!< "SLIP" inline label */
    DASHBOARD_FIELD_SLIP,                 /*!< Slip toggle value, "ON" or "OFF" */

    DASHBOARD_FIELD_CAR_STATE,           /*!< Vehicle state, center top */
    DASHBOARD_FIELD_HV_BACKGROUND,       /*!< Background box for the HV block */
    DASHBOARD_FIELD_HV_HEADER,           /*!< "HV" header */
    DASHBOARD_FIELD_HV_SOC,              /*!< "69%" large state-of-charge value */
    DASHBOARD_FIELD_HV_TEMPERATURE,      /*!< "104°C" pack temperature */
    DASHBOARD_FIELD_INVERTER_BACKGROUND, /*!< Background box for the inverter block */
    DASHBOARD_FIELD_INVERTER_LABEL,      /*!< "INV" inline label */
    DASHBOARD_FIELD_INVERTER,            /*!< Inverter temperature value, e.g. "22C" */

    DASHBOARD_FIELD_LAP_BACKGROUND,     /*!< Background box for the lap counter and delta */
    DASHBOARD_FIELD_LAP_LABEL,          /*!< "LAP" inline label */
    DASHBOARD_FIELD_LAP,                /*!< Lap counter value, e.g. "9/11" */
    DASHBOARD_FIELD_LAP_DELTA,          /*!< "-0.420" lap delta in seconds */
    DASHBOARD_FIELD_TIRES_BACKGROUND,   /*!< Background box for the tire/motor temp blocks */
    DASHBOARD_FIELD_TIRES_HEADER,       /*!< "TRS" tire-temps header */
    DASHBOARD_FIELD_TIRES_FRONT_LEFT,   /*!< Front-left tire temperature */
    DASHBOARD_FIELD_TIRES_FRONT_RIGHT,  /*!< Front-right tire temperature */
    DASHBOARD_FIELD_TIRES_REAR_LEFT,    /*!< Rear-left tire temperature */
    DASHBOARD_FIELD_TIRES_REAR_RIGHT,   /*!< Rear-right tire temperature */
    DASHBOARD_FIELD_MOTORS_BACKGROUND,  /*!< Background box for the motor temp block */
    DASHBOARD_FIELD_MOTORS_HEADER,      /*!< "MTR" motor-temps header */
    DASHBOARD_FIELD_MOTORS_FRONT_LEFT,  /*!< Front-left motor temperature */
    DASHBOARD_FIELD_MOTORS_FRONT_RIGHT, /*!< Front-right motor temperature */
    DASHBOARD_FIELD_MOTORS_REAR_LEFT,   /*!< Rear-left motor temperature */
    DASHBOARD_FIELD_MOTORS_REAR_RIGHT,  /*!< Rear-right motor temperature */

    DASHBOARD_FIELD_COUNT, /*!< Total number of fields, also the box count */
};

/*!
 * \brief Inclusive size (including the terminator) of each per-field text buffer.
 */
#define DASHBOARD_TEXT_BUFFER_SIZE (16U)

/*!
 * \brief Total number of boxes composing the dashboard.
 *
 * \details Mirrors DASHBOARD_FIELD_COUNT. Callers that need a specific
 *     integer width (e.g. raster_api_init's uint16_t) cast at the call site
 *     rather than baking the cast into the macro.
 */
#define DASHBOARD_BOX_COUNT (DASHBOARD_FIELD_COUNT)

/*!
 * \brief Self-contained storage for the dashboard interface.
 *
 * \details Owns the value buffers, labels and boxes. Labels point at the
 *     buffers inside this handler, so the whole layout can be embedded in
 *     a parent struct (e.g. ScreenHandler) and moved/copied without breaking
 *     pointer relationships as long as the handler is not duplicated.
 */
struct DashboardHandler {
    char text[DASHBOARD_FIELD_COUNT][DASHBOARD_TEXT_BUFFER_SIZE]; /*!< Mutable backing strings */
    struct Label labels[DASHBOARD_FIELD_COUNT];                   /*!< One label per field */
    struct Box boxes[DASHBOARD_FIELD_COUNT];                      /*!< Boxes mounted on the raster */
};

#endif // DASHBOARD_H
