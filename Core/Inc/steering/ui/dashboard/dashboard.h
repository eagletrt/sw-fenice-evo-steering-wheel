/*!
 * \file dashboard.h
 * \date 2026-06-07
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief Definitions for the driver dashboard.
 *
 * \details Layout from the Figma draft "Pilot - driving + maps" (page
 *     "3 Pilot"): DARK_GRAY cells on a LIGHT_GRAY screen, the 2 px gaps are
 *     the grid lines. Every cell is split into a label box and a value box (a
 *     libraster box holds one label), so a value change redraws only its value.
 */

#ifndef DASHBOARD_H
#define DASHBOARD_H

#include "box.h"
#include "colors.h"
#include "label.h"
#include "ui/style.h"

#include <stdint.h>

#define DASHBOARD_COLOR_SCREEN (STYLE_LIGHT_GRAY) /*!< Grid lines (the screen behind the cells) */
#define DASHBOARD_COLOR_CELL (STYLE_DARK_GRAY)    /*!< Cell background */
#define DASHBOARD_COLOR_LABEL (STYLE_LIGHT_GRAY)  /*!< Cell captions */
#define DASHBOARD_COLOR_VALUE (STYLE_WHITE)       /*!< Plain values */
#define DASHBOARD_COLOR_ALL_OK (STYLE_GREEN)
#define DASHBOARD_COLOR_WARNING (STYLE_YELLOW)
#define DASHBOARD_COLOR_ERROR (STYLE_RED)
#define DASHBOARD_COLOR_PTT (STYLE_BLUE)          /*!< MISSION cell while push-to-talk is held */
#define DASHBOARD_COLOR_TRACK (STYLE_BLACK)       /*!< Pedal bar track */
#define DASHBOARD_COLOR_SECTOR (STYLE_LIGHT_GRAY) /*!< Microsector not driven yet */

// TODO: tune these data (and integrate S.P.E.C.)
#define DASHBOARD_THRESHOLD_HV_SOC_PERCENT_WARNING (30U)               /* at or below this SoC is a warning */
#define DASHBOARD_THRESHOLD_HV_SOC_PERCENT_ERROR (15U)                 /* at or below this SoC is an error */
#define DASHBOARD_THRESHOLD_HV_TEMP_CELSIUS_WARNING (45U)              /* at or above this HV pack temp is a warning */
#define DASHBOARD_THRESHOLD_HV_TEMP_CELSIUS_ERROR (50U)                /* at or above this HV pack temp is an error */
#define DASHBOARD_THRESHOLD_INVERTER_TEMPERATURE_CELSIUS_WARNING (60U) /* at or above this inverter temp is a warning */
#define DASHBOARD_THRESHOLD_INVERTER_TEMPERATURE_CELSIUS_ERROR (70U)   /* at or above this inverter temp is an error */
#define DASHBOARD_THRESHOLD_MOTOR_TEMPERATURE_CELSIUS_WARNING (80U)    /* at or above this motor temp is a warning */
#define DASHBOARD_THRESHOLD_MOTOR_TEMPERATURE_CELSIUS_ERROR (100U)     /* at or above this motor temp is an error */

#define DASHBOARD_PEDAL_CENTER_X (399U)   /*!< Left edge of the white centre marker */
#define DASHBOARD_PEDAL_BAR_X (403U)      /*!< Left edge of the throttle bar (right of the marker) */
#define DASHBOARD_PEDAL_BAR_MAX (277U)    /*!< Bar length at 100 %, both sides */
#define DASHBOARD_NO_VALUE "--"           /*!< Shown for a stale value */
#define DASHBOARD_MISSION_MANUAL "MANUAL" /*!< TODO(#11): from the mission message */

/*!
 * \brief Return codes for dashboard operations.
 */
enum DashboardReturnCode {
    DASHBOARD_RC_OK,           /*!< Operation completed successfully */
    DASHBOARD_RC_NULL_POINTER, /*!< A required pointer argument was NULL */
    DASHBOARD_RC_ERROR,        /*!< libraster rejected a box or a label */
};

/*!
 * \brief Every box of the dashboard, in drawing order.
 */
enum DashboardFieldId : uint8_t {
    DASHBOARD_FIELD_BACKGROUND, /*!< Whole screen, LIGHT_GRAY: the grid lines */

    DASHBOARD_FIELD_MISSION_LABEL, /*!< "MISSION" (turns PTT blue with the value) */
    DASHBOARD_FIELD_MISSION,       /*!< Mission name */
    DASHBOARD_FIELD_STATE_LABEL,   /*!< "STATE" */
    DASHBOARD_FIELD_STATE,         /*!< Vehicle state */
    DASHBOARD_FIELD_LAP_LABEL,     /*!< "LAP 7" */
    DASHBOARD_FIELD_LAP_DELTA,     /*!< Delta to the best lap, "-0.420" */
    DASHBOARD_FIELD_LAP_STRIP,     /*!< Background of the microsectors */
    DASHBOARD_FIELD_MICROSECTOR_1, /*!< TODO(libcan): no microsector data yet, the 8 stay LIGHT_GRAY, also microsector number might be dynamic */
    DASHBOARD_FIELD_MICROSECTOR_2,
    DASHBOARD_FIELD_MICROSECTOR_3,
    DASHBOARD_FIELD_MICROSECTOR_4,
    DASHBOARD_FIELD_MICROSECTOR_5,
    DASHBOARD_FIELD_MICROSECTOR_6,
    DASHBOARD_FIELD_MICROSECTOR_7,
    DASHBOARD_FIELD_MICROSECTOR_8,

    DASHBOARD_FIELD_HV_LABEL,       /*!< "HV" */
    DASHBOARD_FIELD_HV_SOC,         /*!< HV state of charge */
    DASHBOARD_FIELD_HV_TEMPERATURE, /*!< HV pack temperature */
    DASHBOARD_FIELD_LV_LABEL,       /*!< "LV" */
    DASHBOARD_FIELD_LV_SOC,         /*!< LV state of charge */

    DASHBOARD_FIELD_SPEED,          /*!< Speed, big */
    DASHBOARD_FIELD_SPEED_UNIT,     /*!< "KM/H" */
    DASHBOARD_FIELD_SHUTDOWN_LABEL, /*!< "SHUTDOWN" (red cell when open) */
    DASHBOARD_FIELD_SHUTDOWN,       /*!< "CLOSED" / "OPEN" */

    DASHBOARD_FIELD_MOTOR_LABEL,      /*!< "MOTOR MAX" */
    DASHBOARD_FIELD_MOTOR,            /*!< Hottest motor */
    DASHBOARD_FIELD_INVERTER_LABEL,   /*!< "INVERTER MAX" */
    DASHBOARD_FIELD_INVERTER,         /*!< Hottest inverter */
    DASHBOARD_FIELD_BRAKE_BIAS_LABEL, /*!< "BRAKE BIAS" */
    DASHBOARD_FIELD_BRAKE_BIAS,       /*!< TODO(libcan): no brake bias on the bus yet, "--" */

    DASHBOARD_FIELD_POWER_LABEL, /*!< Maps row (TODO(#10): values from the ECU) */
    DASHBOARD_FIELD_POWER,
    DASHBOARD_FIELD_REGEN_LABEL,
    DASHBOARD_FIELD_REGEN,
    DASHBOARD_FIELD_TORQUE_LABEL,
    DASHBOARD_FIELD_TORQUE,
    DASHBOARD_FIELD_SLIP_LABEL,
    DASHBOARD_FIELD_SLIP,

    DASHBOARD_FIELD_BRAKE_LABEL,    /*!< "BRAKE" */
    DASHBOARD_FIELD_BRAKE,          /*!< Brake % */
    DASHBOARD_FIELD_PEDAL_PANEL,    /*!< Background around the track */
    DASHBOARD_FIELD_PEDAL_TRACK,    /*!< Black track of both bars */
    DASHBOARD_FIELD_BRAKE_BAR,      /*!< Red, grows left from the centre */
    DASHBOARD_FIELD_THROTTLE_BAR,   /*!< Green, grows right from the centre */
    DASHBOARD_FIELD_PEDAL_CENTER,   /*!< White centre marker */
    DASHBOARD_FIELD_THROTTLE_LABEL, /*!< "THROTTLE" */
    DASHBOARD_FIELD_THROTTLE,       /*!< Throttle % */

    DASHBOARD_FIELD_COUNT,
};

#define DASHBOARD_TEXT_BUFFER_SIZE (16U)            /*!< Per-field text, terminator included */
#define DASHBOARD_BOX_COUNT (DASHBOARD_FIELD_COUNT) /*!< Boxes mounted on the raster */

/*!
 * \brief Storage of the dashboard: text buffers, labels and boxes.
 */
struct DashboardHandler {
    char text[DASHBOARD_FIELD_COUNT][DASHBOARD_TEXT_BUFFER_SIZE]; /*!< Mutable backing strings */
    struct Label labels[DASHBOARD_FIELD_COUNT];                   /*!< One label per labelled box */
    struct Box boxes[DASHBOARD_FIELD_COUNT];                      /*!< Boxes mounted on the raster */
};

#endif // DASHBOARD_H
