/*!
 * \file dashboard-api.c
 * \date 2026-06-07
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief Implementation of the driver dashboard layout and its sync.
 *
 * \details The layout table drives initialization: each row pins one box to
 *     a rectangle, a font size, a text offset and colours (coordinates 1:1
 *     from the Figma draft). dashboard_api_sync formats the UI data snapshot
 *     into the boxes and flags only what changed.
 */

#include "ui/dashboard/dashboard-api.h"

#include "box-api.h"
#include "eagletrt-api.h"
#include "label-api.h"
#include "raster-fonts.h"

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

/*!
 * \brief Static descriptor of one box.
 */
struct DashboardFieldLayout {
    struct BoxRectangle rect; /*!< Where the box sits on the 800x480 screen */
    uint16_t font_size;       /*!< Pixel height of the text (labelled boxes only) */
    int16_t offset_y;         /*!< Text top, from the top of the box (Figma text y) */
    uint32_t background_argb; /*!< Box fill */
    uint32_t text_argb;       /*!< Text colour (labelled boxes only) */
    const char *initial_text; /*!< Text until the first sync, NULL for a box without a label */
};

#define DASHBOARD_LABEL(x, y, w, h, size, offset, text) { { x, y, w, h }, size, offset, DASHBOARD_COLOR_CELL, DASHBOARD_COLOR_LABEL, text }
#define DASHBOARD_VALUE(x, y, w, h, size, offset) { { x, y, w, h }, size, offset, DASHBOARD_COLOR_CELL, DASHBOARD_COLOR_VALUE, DASHBOARD_NO_VALUE }
#define DASHBOARD_FILL(x, y, w, h, argb) { { x, y, w, h }, 0U, 0, argb, 0U, NULL }
#define DASHBOARD_SECTOR(i) DASHBOARD_FILL(578U + (27U * (i)), 81U, 25U, 8U, DASHBOARD_COLOR_SECTOR)

// clang-format off
EAGLETRT_STATIC const struct DashboardFieldLayout prv_dashboard_api_layout[DASHBOARD_FIELD_COUNT] = {
    [DASHBOARD_FIELD_BACKGROUND]       = DASHBOARD_FILL(  0,   0, 800, 480, DASHBOARD_COLOR_SCREEN),

    [DASHBOARD_FIELD_MISSION_LABEL]    = DASHBOARD_LABEL(  1,   1, 230,  38, 20, 10, "MISSION"),
    [DASHBOARD_FIELD_MISSION]          = DASHBOARD_VALUE(  1,  39, 230,  56, 40,  0),
    [DASHBOARD_FIELD_STATE_LABEL]      = DASHBOARD_LABEL(233,   1, 334,  38, 20, 10, "STATE"),
    [DASHBOARD_FIELD_STATE]            = DASHBOARD_VALUE(233,  39, 334,  56, 40,  0),
    [DASHBOARD_FIELD_LAP_LABEL]        = DASHBOARD_LABEL(569,   1, 230,  26, 20,  6, "LAP --"),
    [DASHBOARD_FIELD_LAP_DELTA]        = DASHBOARD_VALUE(569,  27, 230,  53, 40,  0),
    [DASHBOARD_FIELD_LAP_STRIP]        = DASHBOARD_FILL(569,  80, 230,  15, DASHBOARD_COLOR_CELL),
    [DASHBOARD_FIELD_MICROSECTOR_1]    = DASHBOARD_SECTOR(0),
    [DASHBOARD_FIELD_MICROSECTOR_2]    = DASHBOARD_SECTOR(1),
    [DASHBOARD_FIELD_MICROSECTOR_3]    = DASHBOARD_SECTOR(2),
    [DASHBOARD_FIELD_MICROSECTOR_4]    = DASHBOARD_SECTOR(3),
    [DASHBOARD_FIELD_MICROSECTOR_5]    = DASHBOARD_SECTOR(4),
    [DASHBOARD_FIELD_MICROSECTOR_6]    = DASHBOARD_SECTOR(5),
    [DASHBOARD_FIELD_MICROSECTOR_7]    = DASHBOARD_SECTOR(6),
    [DASHBOARD_FIELD_MICROSECTOR_8]    = DASHBOARD_SECTOR(7),

    [DASHBOARD_FIELD_HV_LABEL]         = DASHBOARD_LABEL(  1,  97, 230,  35, 20,  7, "HV"),
    [DASHBOARD_FIELD_HV_SOC]           = DASHBOARD_VALUE(  1, 132, 230,  72, 56,  0),
    [DASHBOARD_FIELD_HV_TEMPERATURE]   = DASHBOARD_VALUE(  1, 204, 230,  33, 22,  0),
    [DASHBOARD_FIELD_LV_LABEL]         = DASHBOARD_LABEL(  1, 239, 230,  39, 20, 15, "LV"),
    [DASHBOARD_FIELD_LV_SOC]           = DASHBOARD_VALUE(  1, 278, 230,  63, 40,  0),

    [DASHBOARD_FIELD_SPEED]            = DASHBOARD_VALUE(233,  97, 334, 138, 112, 2),
    [DASHBOARD_FIELD_SPEED_UNIT]       = DASHBOARD_LABEL(233, 235, 334,  26, 20,  0, "KM/H"),
    [DASHBOARD_FIELD_SHUTDOWN_LABEL]   = DASHBOARD_LABEL(233, 263, 334,  29, 16, 10, "SHUTDOWN"),
    [DASHBOARD_FIELD_SHUTDOWN]         = DASHBOARD_VALUE(233, 292, 334,  49, 32,  0),

    [DASHBOARD_FIELD_MOTOR_LABEL]      = DASHBOARD_LABEL(569,  97, 230,  28, 16,  9, "MOTOR MAX"),
    [DASHBOARD_FIELD_MOTOR]            = DASHBOARD_VALUE(569, 125, 230,  52, 36,  0),
    [DASHBOARD_FIELD_INVERTER_LABEL]   = DASHBOARD_LABEL(569, 179, 230,  28, 16,  9, "INVERTER MAX"),
    [DASHBOARD_FIELD_INVERTER]         = DASHBOARD_VALUE(569, 207, 230,  52, 36,  0),
    [DASHBOARD_FIELD_BRAKE_BIAS_LABEL] = DASHBOARD_LABEL(569, 261, 230,  28, 16,  9, "BRAKE BIAS"),
    [DASHBOARD_FIELD_BRAKE_BIAS]       = DASHBOARD_VALUE(569, 289, 230,  52, 36,  0),

    [DASHBOARD_FIELD_POWER_LABEL]      = DASHBOARD_LABEL(  1, 343, 198,  25, 16,  6, "POWER"),
    [DASHBOARD_FIELD_POWER]            = DASHBOARD_VALUE(  1, 368, 198,  53, 40,  0),
    [DASHBOARD_FIELD_REGEN_LABEL]      = DASHBOARD_LABEL(201, 343, 198,  25, 16,  6, "REGEN"),
    [DASHBOARD_FIELD_REGEN]            = DASHBOARD_VALUE(201, 368, 198,  53, 40,  0),
    [DASHBOARD_FIELD_TORQUE_LABEL]     = DASHBOARD_LABEL(401, 343, 198,  25, 16,  6, "TORQUE"),
    [DASHBOARD_FIELD_TORQUE]           = DASHBOARD_VALUE(401, 368, 198,  53, 40,  0),
    [DASHBOARD_FIELD_SLIP_LABEL]       = DASHBOARD_LABEL(601, 343, 198,  25, 16,  6, "SLIP"),
    [DASHBOARD_FIELD_SLIP]             = DASHBOARD_VALUE(601, 368, 198,  53, 40,  0),

    [DASHBOARD_FIELD_BRAKE_LABEL]      = DASHBOARD_LABEL(  1, 423, 118,  22, 16,  4, "BRAKE"),
    [DASHBOARD_FIELD_BRAKE]            = DASHBOARD_VALUE(  1, 445, 118,  34, 24,  0),
    [DASHBOARD_FIELD_PEDAL_PANEL]      = DASHBOARD_FILL(119, 423, 562,  56, DASHBOARD_COLOR_CELL),
    [DASHBOARD_FIELD_PEDAL_TRACK]      = DASHBOARD_FILL(120, 431, 560,  40, DASHBOARD_COLOR_TRACK),
    [DASHBOARD_FIELD_BRAKE_BAR]        = DASHBOARD_FILL(398, 435,   1,  32, DASHBOARD_COLOR_TRACK),
    [DASHBOARD_FIELD_THROTTLE_BAR]     = DASHBOARD_FILL(403, 435,   1,  32, DASHBOARD_COLOR_TRACK),
    [DASHBOARD_FIELD_PEDAL_CENTER]     = DASHBOARD_FILL(DASHBOARD_PEDAL_CENTER_X, 427, 4, 48, STYLE_WHITE),
    [DASHBOARD_FIELD_THROTTLE_LABEL]   = DASHBOARD_LABEL(681, 423, 118,  22, 16,  4, "THROTTLE"),
    [DASHBOARD_FIELD_THROTTLE]         = DASHBOARD_VALUE(681, 445, 118,  34, 24,  0),
};
// clang-format on

/*!
 * \brief Set a field's text, flagging its box only on a real change.
 */
__attribute__((format(printf, 3, 4))) EAGLETRT_STATIC void prv_dashboard_api_format_field(struct DashboardHandler *handler, enum DashboardFieldId field_id, const char *fmt, ...) {
    char scratch[DASHBOARD_TEXT_BUFFER_SIZE];
    va_list args;
    va_start(args, fmt);
    const int written = vsnprintf(scratch, sizeof(scratch), fmt, args);
    va_end(args);
    if (written < 0 || strncmp(scratch, handler->text[field_id], DASHBOARD_TEXT_BUFFER_SIZE) == 0) {
        return;
    }
    memcpy(handler->text[field_id], scratch, sizeof(scratch));
    handler->boxes[field_id].updated = true;
}

/*!
 * \brief Set a field's background and text colours, flagging its box only on a real change.
 */
EAGLETRT_STATIC void prv_dashboard_api_set_colors(struct DashboardHandler *handler, enum DashboardFieldId field_id, uint32_t background_argb, uint32_t text_argb) {
    struct Box *box = &handler->boxes[field_id];
    if (box->color.argb == background_argb && handler->labels[field_id].color.argb == text_argb) {
        return;
    }
    box->color.argb = background_argb;
    handler->labels[field_id].color.argb = text_argb;
    box->updated = true;
}

/*!
 * \brief Text colour of a value against a "higher is worse" warning/error pair.
 */
// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
EAGLETRT_STATIC uint32_t prv_dashboard_api_high_color(int32_t value, int32_t warning, int32_t error) {
    if (value >= error) {
        return DASHBOARD_COLOR_ERROR;
    }
    return (value >= warning) ? DASHBOARD_COLOR_WARNING : DASHBOARD_COLOR_VALUE;
}

/*!
 * \brief Show a temperature, "--" when stale.
 */
EAGLETRT_STATIC void prv_dashboard_api_temperature(struct DashboardHandler *handler, enum DashboardFieldId field_id, bool stale, int16_t celsius, int32_t warning, int32_t error) {
    if (stale) {
        prv_dashboard_api_format_field(handler, field_id, "%s", DASHBOARD_NO_VALUE);
        prv_dashboard_api_set_colors(handler, field_id, DASHBOARD_COLOR_CELL, DASHBOARD_COLOR_VALUE);
        return;
    }
    prv_dashboard_api_format_field(handler, field_id, "%" PRId16 "°C", celsius);
    prv_dashboard_api_set_colors(handler, field_id, DASHBOARD_COLOR_CELL, prv_dashboard_api_high_color(celsius, warning, error));
}

/*!
 * \brief Show a 0..100 percentage, "--" when stale.
 */
EAGLETRT_STATIC void prv_dashboard_api_percent(struct DashboardHandler *handler, enum DashboardFieldId field_id, bool stale, uint8_t percent) {
    if (stale) {
        prv_dashboard_api_format_field(handler, field_id, "%s", DASHBOARD_NO_VALUE);
        return;
    }
    prv_dashboard_api_format_field(handler, field_id, "%" PRIu8 "%%", (uint8_t)EAGLETRT_API_MIN(percent, 100U));
}

/*!
 * \brief Name and colour of a vehicle state (Figma: DRIVE green, ERROR red).
 */
EAGLETRT_STATIC const char *prv_dashboard_api_state(uint8_t state, uint32_t *argb) {
    *argb = DASHBOARD_COLOR_VALUE;
    switch ((enum UIDataVehicleState)state) {
        case UI_DATA_VEHICLE_STATE_IDLE:
            return "IDLE";
        case UI_DATA_VEHICLE_STATE_PRECHARGE:
            *argb = DASHBOARD_COLOR_WARNING;
            return "PRECHARGE";
        case UI_DATA_VEHICLE_STATE_DISCHARGE:
            *argb = DASHBOARD_COLOR_WARNING;
            return "DISCHARGE";
        case UI_DATA_VEHICLE_STATE_TSON:
            *argb = DASHBOARD_COLOR_WARNING;
            return "TSON";
        case UI_DATA_VEHICLE_STATE_DRIVE:
            *argb = DASHBOARD_COLOR_ALL_OK;
            return "DRIVE";
        case UI_DATA_VEHICLE_STATE_AUTONOMOUS:
            return "AUTONOMOUS";
        case UI_DATA_VEHICLE_STATE_FLASH:
            return "FLASH";
        case UI_DATA_VEHICLE_STATE_ERROR:
            *argb = DASHBOARD_COLOR_ERROR;
            return "ERROR";
        case UI_DATA_VEHICLE_STATE_COUNT:
        default:
            return "----";
    }
}

/*!
 * \brief Lap number and delta to the best lap: green faster, red slower.
 */
EAGLETRT_STATIC void prv_dashboard_api_lap(struct DashboardHandler *handler, const struct UIData *ui_data, bool stale) {
    if (stale) {
        prv_dashboard_api_format_field(handler, DASHBOARD_FIELD_LAP_LABEL, "%s", "LAP " DASHBOARD_NO_VALUE);
        prv_dashboard_api_format_field(handler, DASHBOARD_FIELD_LAP_DELTA, "%s", DASHBOARD_NO_VALUE);
        prv_dashboard_api_set_colors(handler, DASHBOARD_FIELD_LAP_DELTA, DASHBOARD_COLOR_CELL, DASHBOARD_COLOR_VALUE);
        return;
    }
    constexpr int32_t ms_per_second = 1000;
    const int32_t delta = ui_data->lap_delta_ms;
    const int32_t magnitude = (delta < 0) ? -delta : delta;
    const char *sign = (delta < 0) ? "-" : ((delta > 0) ? "+" : "");
    uint32_t argb = DASHBOARD_COLOR_VALUE;
    if (delta != 0) {
        argb = (delta < 0) ? DASHBOARD_COLOR_ALL_OK : DASHBOARD_COLOR_ERROR;
    }
    prv_dashboard_api_format_field(handler, DASHBOARD_FIELD_LAP_LABEL, "LAP %" PRIu8, ui_data->lap_current);
    prv_dashboard_api_format_field(handler, DASHBOARD_FIELD_LAP_DELTA, "%s%" PRId32 ".%03" PRId32, sign, magnitude / ms_per_second, magnitude % ms_per_second);
    prv_dashboard_api_set_colors(handler, DASHBOARD_FIELD_LAP_DELTA, DASHBOARD_COLOR_CELL, argb);
}

/*!
 * \brief Shutdown cell: green CLOSED, whole cell red when OPEN, "--" when stale.
 */
EAGLETRT_STATIC void prv_dashboard_api_shutdown(struct DashboardHandler *handler, bool stale, bool closed) {
    const bool open = !stale && !closed;
    const uint32_t background = open ? DASHBOARD_COLOR_ERROR : DASHBOARD_COLOR_CELL;
    uint32_t value_argb = DASHBOARD_COLOR_VALUE;
    if (!stale && closed) {
        value_argb = DASHBOARD_COLOR_ALL_OK;
    }
    // TODO(design): the first open node instead of "OPEN" once the node order is defined.
    prv_dashboard_api_format_field(handler, DASHBOARD_FIELD_SHUTDOWN, "%s", stale ? DASHBOARD_NO_VALUE : (closed ? "CLOSED" : "OPEN"));
    prv_dashboard_api_set_colors(handler, DASHBOARD_FIELD_SHUTDOWN, background, value_argb);
    prv_dashboard_api_set_colors(handler, DASHBOARD_FIELD_SHUTDOWN_LABEL, background, open ? DASHBOARD_COLOR_VALUE : DASHBOARD_COLOR_LABEL);
}

/*!
 * \brief Place one pedal bar; a 0 % bar is a 1 px box in the track colour.
 *
 * \retval true if its geometry or colour changed.
 */
EAGLETRT_STATIC bool prv_dashboard_api_place_bar(struct Box *bar, uint16_t x, uint16_t width, uint32_t argb) {
    if (bar->rect.x == x && bar->rect.width == width && bar->color.argb == argb) {
        return false;
    }
    bar->rect.x = x;
    bar->rect.width = width;
    bar->color.argb = argb;
    return true;
}

/*!
 * \brief Pedal values and bars: brake grows left, throttle right of the centre marker.
 *
 * \details A bar that shrinks leaves its old pixels on the track, so any bar
 *     change redraws the track, both bars and the marker (in that order).
 */
EAGLETRT_STATIC void prv_dashboard_api_pedals(struct DashboardHandler *handler, const struct UIData *ui_data, bool brake_stale, bool throttle_stale) {
    prv_dashboard_api_percent(handler, DASHBOARD_FIELD_BRAKE, brake_stale, ui_data->brake_percent);
    prv_dashboard_api_percent(handler, DASHBOARD_FIELD_THROTTLE, throttle_stale, ui_data->throttle_percent);
    prv_dashboard_api_set_colors(handler, DASHBOARD_FIELD_BRAKE, DASHBOARD_COLOR_CELL, brake_stale ? DASHBOARD_COLOR_VALUE : DASHBOARD_COLOR_ERROR);
    prv_dashboard_api_set_colors(handler, DASHBOARD_FIELD_THROTTLE, DASHBOARD_COLOR_CELL, throttle_stale ? DASHBOARD_COLOR_VALUE : DASHBOARD_COLOR_ALL_OK);

    const uint16_t brake = brake_stale ? 0U : (uint16_t)((EAGLETRT_API_MIN(ui_data->brake_percent, 100U) * DASHBOARD_PEDAL_BAR_MAX) / 100U);
    const uint16_t throttle = throttle_stale ? 0U : (uint16_t)((EAGLETRT_API_MIN(ui_data->throttle_percent, 100U) * DASHBOARD_PEDAL_BAR_MAX) / 100U);
    bool changed = prv_dashboard_api_place_bar(&handler->boxes[DASHBOARD_FIELD_BRAKE_BAR],
                                               (uint16_t)(DASHBOARD_PEDAL_CENTER_X - EAGLETRT_API_MAX(brake, 1U)),
                                               (uint16_t)EAGLETRT_API_MAX(brake, 1U),
                                               (brake > 0U) ? DASHBOARD_COLOR_ERROR : DASHBOARD_COLOR_TRACK);
    changed = prv_dashboard_api_place_bar(&handler->boxes[DASHBOARD_FIELD_THROTTLE_BAR],
                                          DASHBOARD_PEDAL_BAR_X,
                                          (uint16_t)EAGLETRT_API_MAX(throttle, 1U),
                                          (throttle > 0U) ? DASHBOARD_COLOR_ALL_OK : DASHBOARD_COLOR_TRACK) ||
              changed;
    if (changed) {
        handler->boxes[DASHBOARD_FIELD_PEDAL_TRACK].updated = true;
        handler->boxes[DASHBOARD_FIELD_BRAKE_BAR].updated = true;
        handler->boxes[DASHBOARD_FIELD_THROTTLE_BAR].updated = true;
        handler->boxes[DASHBOARD_FIELD_PEDAL_CENTER].updated = true;
    }
}

enum DashboardReturnCode dashboard_api_init(struct DashboardHandler *handler) {
    if (handler == NULL) {
        return DASHBOARD_RC_NULL_POINTER;
    }
    memset(handler, 0, sizeof(*handler));

    for (uint8_t i = 0U; i < DASHBOARD_FIELD_COUNT; i++) {
        const struct DashboardFieldLayout *layout = &prv_dashboard_api_layout[i];
        struct Label *label = NULL;
        if (layout->initial_text != NULL) {
            (void)snprintf(handler->text[i], DASHBOARD_TEXT_BUFFER_SIZE, "%s", layout->initial_text);
            label = &handler->labels[i];
            if (label_api_init(label, handler->text[i], (int16_t)(layout->rect.width / 2U), layout->offset_y, &font_inter, layout->font_size, FONT_ALIGN_CENTER, (struct Color){ .argb = layout->text_argb }) != RASTER_RC_OK) {
                return DASHBOARD_RC_ERROR;
            }
        }
        if (box_api_init(&handler->boxes[i], i, layout->rect, (struct Color){ .argb = layout->background_argb }, label) != RASTER_RC_OK) {
            return DASHBOARD_RC_ERROR;
        }
        handler->boxes[i].updated = true;
    }
    return DASHBOARD_RC_OK;
}

enum DashboardReturnCode dashboard_api_sync(struct DashboardHandler *handler, const struct UIData *ui_data) {
    if (handler == NULL || ui_data == NULL) {
        return DASHBOARD_RC_NULL_POINTER;
    }
    const uint16_t stale = ui_data->stale;
#define DASHBOARD_STALE(source) ((stale & UI_DATA_STALE_BIT(source)) != 0U)

    const uint32_t mission_background = (ui_data->ptt != 0U) ? DASHBOARD_COLOR_PTT : DASHBOARD_COLOR_CELL;
    prv_dashboard_api_format_field(handler, DASHBOARD_FIELD_MISSION, "%s", DASHBOARD_MISSION_MANUAL);
    prv_dashboard_api_set_colors(handler, DASHBOARD_FIELD_MISSION, mission_background, DASHBOARD_COLOR_VALUE);
    prv_dashboard_api_set_colors(handler, DASHBOARD_FIELD_MISSION_LABEL, mission_background, DASHBOARD_COLOR_LABEL);

    uint32_t state_argb = DASHBOARD_COLOR_VALUE;
    prv_dashboard_api_format_field(handler, DASHBOARD_FIELD_STATE, "%s", prv_dashboard_api_state(ui_data->vehicle_state, &state_argb));
    prv_dashboard_api_set_colors(handler, DASHBOARD_FIELD_STATE, DASHBOARD_COLOR_CELL, state_argb);

    prv_dashboard_api_lap(handler, ui_data, DASHBOARD_STALE(UI_DATA_SOURCE_LAP));

    prv_dashboard_api_percent(handler, DASHBOARD_FIELD_HV_SOC, DASHBOARD_STALE(UI_DATA_SOURCE_HV_SOC), ui_data->soc);
    uint32_t soc_argb = DASHBOARD_COLOR_VALUE;
    if (!DASHBOARD_STALE(UI_DATA_SOURCE_HV_SOC) && ui_data->soc <= DASHBOARD_THRESHOLD_HV_SOC_PERCENT_ERROR) {
        soc_argb = DASHBOARD_COLOR_ERROR;
    } else if (!DASHBOARD_STALE(UI_DATA_SOURCE_HV_SOC) && ui_data->soc <= DASHBOARD_THRESHOLD_HV_SOC_PERCENT_WARNING) {
        soc_argb = DASHBOARD_COLOR_WARNING;
    }
    prv_dashboard_api_set_colors(handler, DASHBOARD_FIELD_HV_SOC, DASHBOARD_COLOR_CELL, soc_argb);
    prv_dashboard_api_temperature(handler, DASHBOARD_FIELD_HV_TEMPERATURE, DASHBOARD_STALE(UI_DATA_SOURCE_HV_TEMP), ui_data->hv_temp, DASHBOARD_THRESHOLD_HV_TEMP_CELSIUS_WARNING, DASHBOARD_THRESHOLD_HV_TEMP_CELSIUS_ERROR);
    prv_dashboard_api_percent(handler, DASHBOARD_FIELD_LV_SOC, DASHBOARD_STALE(UI_DATA_SOURCE_LV_SOC), ui_data->lv_soc);

    if (DASHBOARD_STALE(UI_DATA_SOURCE_SPEED)) {
        prv_dashboard_api_format_field(handler, DASHBOARD_FIELD_SPEED, "%s", DASHBOARD_NO_VALUE);
    } else {
        prv_dashboard_api_format_field(handler, DASHBOARD_FIELD_SPEED, "%" PRIu16, ui_data->speed_kmh);
    }
    prv_dashboard_api_shutdown(handler, DASHBOARD_STALE(UI_DATA_SOURCE_SHUTDOWN), ui_data->shutdown_closed != 0U);

    const int16_t motor = EAGLETRT_API_MAX(EAGLETRT_API_MAX(ui_data->motor_fl_temp, ui_data->motor_fr_temp), EAGLETRT_API_MAX(ui_data->motor_rl_temp, ui_data->motor_rr_temp));
    prv_dashboard_api_temperature(handler, DASHBOARD_FIELD_MOTOR, DASHBOARD_STALE(UI_DATA_SOURCE_MOTOR), motor, DASHBOARD_THRESHOLD_MOTOR_TEMPERATURE_CELSIUS_WARNING, DASHBOARD_THRESHOLD_MOTOR_TEMPERATURE_CELSIUS_ERROR);
    prv_dashboard_api_temperature(handler, DASHBOARD_FIELD_INVERTER, DASHBOARD_STALE(UI_DATA_SOURCE_INVERTER), ui_data->inverter_temp, DASHBOARD_THRESHOLD_INVERTER_TEMPERATURE_CELSIUS_WARNING, DASHBOARD_THRESHOLD_INVERTER_TEMPERATURE_CELSIUS_ERROR);

    // TODO(#10, D58): the maps the ECU reports, not the wheel's own parameters.
    prv_dashboard_api_format_field(handler, DASHBOARD_FIELD_POWER, "%" PRIu8, ui_data->power);
    prv_dashboard_api_format_field(handler, DASHBOARD_FIELD_REGEN, "%" PRIu8, ui_data->regen);
    prv_dashboard_api_format_field(handler, DASHBOARD_FIELD_TORQUE, "%" PRIu8, ui_data->torque);
    prv_dashboard_api_format_field(handler, DASHBOARD_FIELD_SLIP, "%s", (ui_data->slip_on != 0U) ? "ON" : "OFF");
    prv_dashboard_api_set_colors(handler, DASHBOARD_FIELD_SLIP, DASHBOARD_COLOR_CELL, (ui_data->slip_on != 0U) ? DASHBOARD_COLOR_ALL_OK : DASHBOARD_COLOR_VALUE);

    prv_dashboard_api_pedals(handler, ui_data, DASHBOARD_STALE(UI_DATA_SOURCE_BRAKE), DASHBOARD_STALE(UI_DATA_SOURCE_THROTTLE));
#undef DASHBOARD_STALE
    return DASHBOARD_RC_OK;
}
