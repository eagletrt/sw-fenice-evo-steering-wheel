/*!
 * \file dashboard-api.c
 * \date 2026-06-07
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief Implementation of the dashboard layout and setters.
 *
 * \details The layout table drives initialization: each entry pins one
 *     field to a screen rectangle, a font size and a text color. Setters
 *     each touch exactly one field's value buffer and flag its box as
 *     updated. Adding a new field is a row in the table + (if needed) a
 *     setter; no other module needs to change.
 */

#include "dashboard-api.h"
#include "dashboard.h"
#include "box-api.h"
#include "style.h"
#include "label-api.h"
#include "raster-fonts.h"
#include "eagletrt-api.h"
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

/* ----- layout table -----
 *
 * The 800x480 surface is split into three vertical strips:
 *   left   (  0..220) — scenario presets and toggles
 *   center (220..520) — vehicle state and HV/INV telemetry
 *   right  (520..800) — lap counter and tire/motor temperatures
 */

/*!
 * \brief Static descriptor for one drawable on the dashboard.
 */
struct DashboardFieldLayout {
    struct BoxRectangle rect; /*!< Where the box sits on the 800x480 canvas */
    uint16_t font_size;       /*!< Pixel height of the rendered text (labelled boxes only) */
    uint32_t background_argb; /*!< Fill colour of the box itself */
    uint32_t text_argb;       /*!< Foreground colour of the label (labelled boxes only) */
    const char *initial_text; /*!< Placeholder shown until the first setter call, or NULL for background-only */
};

// clang-format off

EAGLETRT_STATIC const struct DashboardFieldLayout prv_dashboard_layout[DASHBOARD_FIELD_COUNT] = {
    /*
     * Z-order: every entry is drawn in array order, so the dashboard-wide white
     * background paints first; the dark section backgrounds paint on top of it
     * (leaving 2px of white visible as inter-section dividers); the labels and
     * values paint last on top of their section background.
     *
     * `initial_text == NULL` marks a background-only box: no glyphs, no Label
     * slot consumed — the renderer just fills the rectangle with the box's
     * background colour.
     */
    [DASHBOARD_FIELD_DASHBOARD_BACKGROUND] = { {   0,   0, 800, 480 }, 0,                                 STYLE_LIGHT_GRAY, 0,                         NULL       },

    /* left strip: small label box on the left, big value box on the right of each row. */
    [DASHBOARD_FIELD_SCENARIO_HEADER]      = { {   0,   0, 262,  92 }, DASHBOARD_FONT_SIZE_FIELD_LABEL,   DASHBOARD_COLOR_PRIMARY,   DASHBOARD_COLOR_SECONDARY, "SCENARIO" },
    [DASHBOARD_FIELD_REGEN_BACKGROUND]     = { {   0,  94, 262,  95 }, 0,                                 DASHBOARD_COLOR_PRIMARY,   0,                         NULL       },
    [DASHBOARD_FIELD_REGEN_LABEL]          = { {  14, 117, 118,  52 }, DASHBOARD_FONT_SIZE_FIELD_LABEL,   DASHBOARD_COLOR_PRIMARY,   DASHBOARD_COLOR_SECONDARY, "RGN"      },
    [DASHBOARD_FIELD_REGEN]                = { { 132, 117, 118,  52 }, DASHBOARD_FONT_SIZE_VALUE,         DASHBOARD_COLOR_PRIMARY,   DASHBOARD_COLOR_SECONDARY, "--"       },
    [DASHBOARD_FIELD_TORQUE_BACKGROUND]    = { {   0, 191, 262,  94 }, 0,                                 DASHBOARD_COLOR_PRIMARY,   0,                         NULL       },
    [DASHBOARD_FIELD_TORQUE_LABEL]         = { {  14, 212, 118,  52 }, DASHBOARD_FONT_SIZE_FIELD_LABEL,   DASHBOARD_COLOR_PRIMARY,   DASHBOARD_COLOR_SECONDARY, "TQ"       },
    [DASHBOARD_FIELD_TORQUE]               = { { 132, 212, 118,  52 }, DASHBOARD_FONT_SIZE_VALUE,         DASHBOARD_COLOR_PRIMARY,   DASHBOARD_COLOR_SECONDARY, "--"       },
    [DASHBOARD_FIELD_POWER_BACKGROUND]     = { {   0, 287, 262,  94 }, 0,                                 DASHBOARD_COLOR_PRIMARY,   0,                         NULL       },
    [DASHBOARD_FIELD_POWER_LABEL]          = { {  14, 308, 118,  52 }, DASHBOARD_FONT_SIZE_FIELD_LABEL,   DASHBOARD_COLOR_PRIMARY,   DASHBOARD_COLOR_SECONDARY, "POW"      },
    [DASHBOARD_FIELD_POWER]                = { { 132, 308, 118,  52 }, DASHBOARD_FONT_SIZE_VALUE,         DASHBOARD_COLOR_PRIMARY,   DASHBOARD_COLOR_SECONDARY, "--"       },
    [DASHBOARD_FIELD_SLIP_BACKGROUND]      = { {   0, 383, 262,  97 }, 0,                                 DASHBOARD_COLOR_PRIMARY,   0,                         NULL       },
    [DASHBOARD_FIELD_SLIP_LABEL]           = { {  14, 405, 118,  52 }, DASHBOARD_FONT_SIZE_FIELD_LABEL,   DASHBOARD_COLOR_PRIMARY,   DASHBOARD_COLOR_SECONDARY, "SLIP"     },
    [DASHBOARD_FIELD_SLIP]                 = { { 132, 405, 118,  52 }, DASHBOARD_FONT_SIZE_VALUE,         DASHBOARD_COLOR_PRIMARY,   DASHBOARD_COLOR_SECONDARY, "--"       },

    /* center strip */
    [DASHBOARD_FIELD_CAR_STATE]            = { { 264,   0, 272,  92 }, DASHBOARD_FONT_SIZE_STATE,         DASHBOARD_COLOR_PRIMARY,   DASHBOARD_COLOR_SECONDARY, "----"     },
    [DASHBOARD_FIELD_HV_BACKGROUND]        = { { 264,  94, 272, 252 }, 0,                                 DASHBOARD_COLOR_PRIMARY,   0,                         NULL       },
    [DASHBOARD_FIELD_HV_HEADER]            = { { 351, 110,  98,  35 }, DASHBOARD_FONT_SIZE_HEADER,        DASHBOARD_COLOR_PRIMARY,   DASHBOARD_COLOR_SECONDARY, "HV"       },
    [DASHBOARD_FIELD_HV_SOC]               = { { 284, 158, 231, 118 }, DASHBOARD_FONT_SIZE_SOC,           DASHBOARD_COLOR_PRIMARY,   DASHBOARD_COLOR_SECONDARY, "--%"      },
    [DASHBOARD_FIELD_HV_TEMPERATURE]       = { { 329, 286, 141,  48 }, DASHBOARD_FONT_SIZE_HEADER,        DASHBOARD_COLOR_PRIMARY,   DASHBOARD_COLOR_WARNING,   "--°C"},
    [DASHBOARD_FIELD_INVERTER_BACKGROUND]  = { { 264, 348, 272, 132 }, 0,                                 DASHBOARD_COLOR_PRIMARY,   0,                         NULL       },
    [DASHBOARD_FIELD_INVERTER_LABEL]       = { { 368, 362,  64,  38 }, DASHBOARD_FONT_SIZE_FIELD_LABEL,   DASHBOARD_COLOR_PRIMARY,   DASHBOARD_COLOR_SECONDARY, "INV"      },
    [DASHBOARD_FIELD_INVERTER]             = { { 339, 406, 121,  61 }, DASHBOARD_FONT_SIZE_VALUE,         DASHBOARD_COLOR_PRIMARY,   DASHBOARD_COLOR_SECONDARY, "--°C"},

    /* right strip */
    [DASHBOARD_FIELD_LAP_BACKGROUND]       = { { 538,   0, 264, 131 }, 0,                                 DASHBOARD_COLOR_PRIMARY,   0,                         NULL       },
    [DASHBOARD_FIELD_LAP_LABEL]            = { { 574,  21,  70,  35 }, DASHBOARD_FONT_SIZE_FIELD_LABEL,   DASHBOARD_COLOR_PRIMARY,   DASHBOARD_COLOR_SECONDARY, "LAP"      },
    [DASHBOARD_FIELD_LAP]                  = { { 667,  11, 120,  45 }, DASHBOARD_FONT_SIZE_VALUE,         DASHBOARD_COLOR_PRIMARY,   DASHBOARD_COLOR_SECONDARY, "-/-"      },
    [DASHBOARD_FIELD_LAP_DELTA]            = { { 565,  73, 203,  42 }, DASHBOARD_FONT_SIZE_DELTA,         DASHBOARD_COLOR_PRIMARY,   DASHBOARD_COLOR_SECONDARY, "0.000"    },
    [DASHBOARD_FIELD_TIRES_BACKGROUND]     = { { 538, 133, 264, 175 }, 0,                                 DASHBOARD_COLOR_PRIMARY,   0,                         NULL       },
    [DASHBOARD_FIELD_TIRES_HEADER]         = { { 609, 139, 118,  25 }, DASHBOARD_FONT_SIZE_FIELD_LABEL,   DASHBOARD_COLOR_PRIMARY,   DASHBOARD_COLOR_SECONDARY, "TRS"      },
    [DASHBOARD_FIELD_TIRES_FRONT_LEFT]     = { { 565, 176,  88,  42 }, DASHBOARD_FONT_SIZE_MOTORS_PIXELS, DASHBOARD_COLOR_PRIMARY,   DASHBOARD_COLOR_SECONDARY, "--°C"     },
    [DASHBOARD_FIELD_TIRES_FRONT_RIGHT]    = { { 683, 176,  88,  42 }, DASHBOARD_FONT_SIZE_MOTORS_PIXELS, DASHBOARD_COLOR_PRIMARY,   DASHBOARD_COLOR_SECONDARY, "--°C"     },
    [DASHBOARD_FIELD_TIRES_REAR_LEFT]      = { { 565, 249,  88,  42 }, DASHBOARD_FONT_SIZE_MOTORS_PIXELS, DASHBOARD_COLOR_PRIMARY,   DASHBOARD_COLOR_SECONDARY, "--°C"     },
    [DASHBOARD_FIELD_TIRES_REAR_RIGHT]     = { { 682, 249,  88,  42 }, DASHBOARD_FONT_SIZE_MOTORS_PIXELS, DASHBOARD_COLOR_PRIMARY,   DASHBOARD_COLOR_SECONDARY, "--°C"     },
    [DASHBOARD_FIELD_MOTORS_BACKGROUND]    = { { 538, 310, 264, 170 }, 0,                                 DASHBOARD_COLOR_PRIMARY,   0,                         NULL       },
    [DASHBOARD_FIELD_MOTORS_HEADER]        = { { 609, 312, 118,  25 }, DASHBOARD_FONT_SIZE_FIELD_LABEL,   DASHBOARD_COLOR_PRIMARY,   DASHBOARD_COLOR_SECONDARY, "MTR"      },
    [DASHBOARD_FIELD_MOTORS_FRONT_LEFT]    = { { 565, 348,  88,  42 }, DASHBOARD_FONT_SIZE_MOTORS_PIXELS, DASHBOARD_COLOR_PRIMARY,   DASHBOARD_COLOR_SECONDARY, "--°C"     },
    [DASHBOARD_FIELD_MOTORS_FRONT_RIGHT]   = { { 683, 348,  88,  42 }, DASHBOARD_FONT_SIZE_MOTORS_PIXELS, DASHBOARD_COLOR_PRIMARY,   DASHBOARD_COLOR_SECONDARY, "--°C"     },
    [DASHBOARD_FIELD_MOTORS_REAR_LEFT]     = { { 565, 422,  88,  42 }, DASHBOARD_FONT_SIZE_MOTORS_PIXELS, DASHBOARD_COLOR_PRIMARY,   DASHBOARD_COLOR_SECONDARY, "--°C"     },
    [DASHBOARD_FIELD_MOTORS_REAR_RIGHT]    = { { 682, 422,  88,  42 }, DASHBOARD_FONT_SIZE_MOTORS_PIXELS, DASHBOARD_COLOR_PRIMARY,   DASHBOARD_COLOR_SECONDARY, "--°C"     },
};
// clang-format on

/*!
 * \brief Pixel offset from a box's top-left to its label anchor.
 *
 * \details With FONT_ALIGN_CENTER the X anchor is the horizontal middle of
 *     the box; the Y anchor is the top of the rendered glyphs, so we center
 *     the line vertically against the box height.
 */
EAGLETRT_STATIC int16_t prv_dashboard_api_label_offset_y(uint16_t box_height, uint16_t font_size) {
    if (box_height <= font_size) {
        return 0;
    }
    return (int16_t)((box_height - font_size) / 2U);
}

/*!
 * \brief Refresh one field's text buffer and mark its box dirty if it changed.
 *
 * \details Formats into a scratch buffer first and compares to the current
 *     content: if identical we leave the box's \c updated flag alone so the
 *     raster stays in partial mode when the consumer calls every setter each
 *     tick. The label keeps the per-field buffer pointer set at init, so
 *     updating it in place is enough to drive the next render.
 */
EAGLETRT_STATIC void prv_dashboard_api_format_field(struct DashboardHandler *handler, enum DashboardFieldId field_id, const char *fmt, ...) {
    char scratch[DASHBOARD_TEXT_BUFFER_SIZE];
    va_list args;
    va_start(args, fmt);
    if (vsnprintf(scratch, sizeof(scratch), fmt, args) < 0) {
        /* Formatting error, leave the field unchanged. */
        va_end(args);
        return;
    }
    va_end(args);

    if (strncmp(scratch, handler->text[field_id], DASHBOARD_TEXT_BUFFER_SIZE) == 0) {
        return;
    }
    memcpy(handler->text[field_id], scratch, sizeof(scratch));
    handler->boxes[field_id].updated = true;
}

enum DashboardReturnCode dashboard_api_init(struct DashboardHandler *handler) {
    if (handler == NULL) {
        return DASHBOARD_RC_NULL_POINTER;
    }

    memset(handler, 0, sizeof(*handler));

    for (uint16_t i = 0U; i < DASHBOARD_FIELD_COUNT; i++) {
        const struct DashboardFieldLayout *field_layout = &prv_dashboard_layout[i];
        const struct Color background = { .argb = field_layout->background_argb };

        // used by background-only boxes
        if (field_layout->initial_text == NULL) {
            if (box_api_init(&handler->boxes[i], i, field_layout->rect, background, NULL) != RASTER_RC_OK) {
                return DASHBOARD_RC_ERROR;
            }
            continue;
        }

        (void)snprintf(handler->text[i], DASHBOARD_TEXT_BUFFER_SIZE, "%s", field_layout->initial_text);

        const int16_t offset_x = (int16_t)(field_layout->rect.width / 2U);
        const int16_t offset_y = prv_dashboard_api_label_offset_y(field_layout->rect.height, field_layout->font_size);
        const struct Color text_color = { .argb = field_layout->text_argb };

        if (label_api_init(&handler->labels[i], handler->text[i], offset_x, offset_y, &font_inter, field_layout->font_size, FONT_ALIGN_CENTER, text_color) != RASTER_RC_OK) {
            return DASHBOARD_RC_ERROR;
        }
        if (box_api_init(&handler->boxes[i], i, field_layout->rect, background, &handler->labels[i]) != RASTER_RC_OK) {
            return DASHBOARD_RC_ERROR;
        }
    }

    return DASHBOARD_RC_OK;
}

enum DashboardReturnCode dashboard_api_set_car_state(struct DashboardHandler *handler, const char *text) {
    if (handler == NULL || text == NULL) {
        return DASHBOARD_RC_NULL_POINTER;
    }
    prv_dashboard_api_format_field(handler, DASHBOARD_FIELD_CAR_STATE, "%s", text);
    return DASHBOARD_RC_OK;
}

enum DashboardReturnCode dashboard_api_set_power(struct DashboardHandler *handler, uint8_t value) {
    if (handler == NULL) {
        return DASHBOARD_RC_NULL_POINTER;
    }
    prv_dashboard_api_format_field(handler, DASHBOARD_FIELD_POWER, "%" PRIu8, value);
    return DASHBOARD_RC_OK;
}

enum DashboardReturnCode dashboard_api_set_regen(struct DashboardHandler *handler, uint8_t value) {
    if (handler == NULL) {
        return DASHBOARD_RC_NULL_POINTER;
    }
    prv_dashboard_api_format_field(handler, DASHBOARD_FIELD_REGEN, "%" PRIu8, value);
    return DASHBOARD_RC_OK;
}

enum DashboardReturnCode dashboard_api_set_torque(struct DashboardHandler *handler, uint8_t value) {
    if (handler == NULL) {
        return DASHBOARD_RC_NULL_POINTER;
    }
    prv_dashboard_api_format_field(handler, DASHBOARD_FIELD_TORQUE, "%" PRIu8, value);
    return DASHBOARD_RC_OK;
}

enum DashboardReturnCode dashboard_api_set_slip(struct DashboardHandler *handler, bool slip_on) {
    if (handler == NULL) {
        return DASHBOARD_RC_NULL_POINTER;
    }
    prv_dashboard_api_format_field(handler, DASHBOARD_FIELD_SLIP, "%s", slip_on ? "ON" : "OFF");
    return DASHBOARD_RC_OK;
}

enum DashboardReturnCode dashboard_api_set_soc(struct DashboardHandler *handler, uint8_t percent) {
    if (handler == NULL) {
        return DASHBOARD_RC_NULL_POINTER;
    }
    percent = EAGLETRT_API_CLAMP(percent, 0U, 100U);
    prv_dashboard_api_format_field(handler, DASHBOARD_FIELD_HV_SOC, "%" PRIu8 "%%", percent);

    if (percent <= DASHBOARD_THRESHOLD_HV_SOC_PERCENT_ERROR) {
        handler->labels[DASHBOARD_FIELD_HV_SOC].color.argb = DASHBOARD_COLOR_ERROR;
    } else if (percent <= DASHBOARD_THRESHOLD_HV_SOC_PERCENT_WARNING) {
        handler->labels[DASHBOARD_FIELD_HV_SOC].color.argb = DASHBOARD_COLOR_WARNING;
    } else {
        handler->labels[DASHBOARD_FIELD_HV_SOC].color.argb = DASHBOARD_COLOR_SECONDARY;
    }

    return DASHBOARD_RC_OK;
}

enum DashboardReturnCode dashboard_api_set_hv_temperature(struct DashboardHandler *handler, int16_t celsius) {
    if (handler == NULL) {
        return DASHBOARD_RC_NULL_POINTER;
    }
    prv_dashboard_api_format_field(handler, DASHBOARD_FIELD_HV_TEMPERATURE, "%" PRId16 "°C", celsius);

    if (celsius >= (int16_t)DASHBOARD_THRESHOLD_HV_TEMP_CELSIUS_ERROR) {
        handler->labels[DASHBOARD_FIELD_HV_TEMPERATURE].color.argb = DASHBOARD_COLOR_ERROR;
    } else if (celsius >= (int16_t)DASHBOARD_THRESHOLD_HV_TEMP_CELSIUS_WARNING) {
        handler->labels[DASHBOARD_FIELD_HV_TEMPERATURE].color.argb = DASHBOARD_COLOR_WARNING;
    } else {
        handler->labels[DASHBOARD_FIELD_HV_TEMPERATURE].color.argb = DASHBOARD_COLOR_SECONDARY;
    }

    return DASHBOARD_RC_OK;
}

enum DashboardReturnCode dashboard_api_set_inverter_temperature(struct DashboardHandler *handler, int16_t celsius) {
    if (handler == NULL) {
        return DASHBOARD_RC_NULL_POINTER;
    }
    prv_dashboard_api_format_field(handler, DASHBOARD_FIELD_INVERTER, "%" PRId16 "°C", celsius);

    if (celsius >= (int16_t)DASHBOARD_THRESHOLD_INVERTER_TEMPERATURE_CELSIUS_ERROR) {
        handler->labels[DASHBOARD_FIELD_INVERTER].color.argb = DASHBOARD_COLOR_ERROR;
    } else if (celsius >= (int16_t)DASHBOARD_THRESHOLD_INVERTER_TEMPERATURE_CELSIUS_WARNING) {
        handler->labels[DASHBOARD_FIELD_INVERTER].color.argb = DASHBOARD_COLOR_WARNING;
    } else {
        handler->labels[DASHBOARD_FIELD_INVERTER].color.argb = DASHBOARD_COLOR_SECONDARY;
    }

    return DASHBOARD_RC_OK;
}

enum DashboardReturnCode dashboard_api_set_lap(struct DashboardHandler *handler, uint8_t current, uint8_t total) {
    if (handler == NULL) {
        return DASHBOARD_RC_NULL_POINTER;
    }
    prv_dashboard_api_format_field(handler, DASHBOARD_FIELD_LAP, "%" PRIu8 "/%" PRIu8, current, total);
    return DASHBOARD_RC_OK;
}

enum DashboardReturnCode dashboard_api_set_lap_delta_ms(struct DashboardHandler *handler, int32_t delta_ms) {
    if (handler == NULL) {
        return DASHBOARD_RC_NULL_POINTER;
    }
    constexpr int32_t divider = 1000;
    const char sign = (delta_ms < 0) ? '-' : '+';
    int32_t magnitude = (delta_ms < 0) ? -delta_ms : delta_ms;
    int32_t integer = magnitude / divider;
    int32_t decimal = magnitude % divider;
    prv_dashboard_api_format_field(handler, DASHBOARD_FIELD_LAP_DELTA, "%c%" PRId32 ".%03" PRId32, sign, integer, decimal);
    return DASHBOARD_RC_OK;
}

/*!
 * \brief Shared helper for the TRS / MTR 4-temperature blocks.
 *
 * \param[in,out] handler         Dashboard storage.
 * \param[in]     front_left_id   Field ID for the front-left temperature.
 * \param[in]     front_right_id  Field ID for the front-right temperature.
 * \param[in]     rear_left_id    Field ID for the rear-left temperature.
 * \param[in]     rear_right_id   Field ID for the rear-right temperature.
 * \param[in]     front_left      Front-left temperature in °C.
 * \param[in]     front_right     Front-right temperature in °C.
 * \param[in]     rear_left       Rear-left temperature in °C.
 * \param[in]     rear_right      Rear-right temperature in °C.
 */
EAGLETRT_STATIC void prv_dashboard_api_set_temperature_quad(
    struct DashboardHandler *handler,
    enum DashboardFieldId front_left_id,
    enum DashboardFieldId front_right_id,
    enum DashboardFieldId rear_left_id,
    enum DashboardFieldId rear_right_id,
    int16_t front_left,
    int16_t front_right,
    int16_t rear_left,
    int16_t rear_right) {
    prv_dashboard_api_format_field(handler, front_left_id, "%" PRId16 "°C", front_left);
    prv_dashboard_api_format_field(handler, front_right_id, "%" PRId16 "°C", front_right);
    prv_dashboard_api_format_field(handler, rear_left_id, "%" PRId16 "°C", rear_left);
    prv_dashboard_api_format_field(handler, rear_right_id, "%" PRId16 "°C", rear_right);
}

/*!
 * \brief Helper to determine the color for a tire temperature value based on thresholds.
 *
 * \param temp Tire temperature in °C.
 *
 * \return ARGB color code corresponding to the temperature thresholds:
 */
EAGLETRT_STATIC uint32_t prv_dashboard_api_tire_temp_color(int16_t celsius) {
    if (celsius <= (int16_t)DASHBOARD_THRESHOLD_TIRE_TEMPERATURE_CELSIUS_LOW) {
        return DASHBOARD_COLOR_COLD_TIRES;
    }
    if (celsius >= (int16_t)DASHBOARD_THRESHOLD_TIRE_TEMPERATURE_CELSIUS_ERROR) {
        return DASHBOARD_COLOR_ERROR;
    }
    if (celsius >= (int16_t)DASHBOARD_THRESHOLD_TIRE_TEMPERATURE_CELSIUS_WARNING) {
        return DASHBOARD_COLOR_WARNING;
    }
    return DASHBOARD_COLOR_SECONDARY;
}

/*!
 * \brief Helper to determine the color for a motor temperature value based on thresholds.
 *
 * \param temp Motor temperature in °C.
 *
 * \return ARGB color code corresponding to the temperature thresholds:
 */
EAGLETRT_STATIC uint32_t prv_dashboard_api_motor_temp_color(int16_t celsius) {
    if (celsius >= (int16_t)DASHBOARD_THRESHOLD_MOTOR_TEMPERATURE_CELSIUS_ERROR) {
        return DASHBOARD_COLOR_ERROR;
    }
    if (celsius >= (int16_t)DASHBOARD_THRESHOLD_MOTOR_TEMPERATURE_CELSIUS_WARNING) {
        return DASHBOARD_COLOR_WARNING;
    }
    return DASHBOARD_COLOR_SECONDARY;
}

enum DashboardReturnCode dashboard_api_set_tire_temperatures(struct DashboardHandler *handler, int16_t front_left, int16_t front_right, int16_t rear_left, int16_t rear_right) {
    if (handler == NULL) {
        return DASHBOARD_RC_NULL_POINTER;
    }
    prv_dashboard_api_set_temperature_quad(handler, DASHBOARD_FIELD_TIRES_FRONT_LEFT, DASHBOARD_FIELD_TIRES_FRONT_RIGHT, DASHBOARD_FIELD_TIRES_REAR_LEFT, DASHBOARD_FIELD_TIRES_REAR_RIGHT, front_left, front_right, rear_left, rear_right);

    handler->labels[DASHBOARD_FIELD_TIRES_FRONT_LEFT].color.argb = prv_dashboard_api_tire_temp_color(front_left);
    handler->labels[DASHBOARD_FIELD_TIRES_FRONT_RIGHT].color.argb = prv_dashboard_api_tire_temp_color(front_right);
    handler->labels[DASHBOARD_FIELD_TIRES_REAR_LEFT].color.argb = prv_dashboard_api_tire_temp_color(rear_left);
    handler->labels[DASHBOARD_FIELD_TIRES_REAR_RIGHT].color.argb = prv_dashboard_api_tire_temp_color(rear_right);

    return DASHBOARD_RC_OK;
}

enum DashboardReturnCode dashboard_api_set_motor_temperatures(struct DashboardHandler *handler, int16_t front_left, int16_t front_right, int16_t rear_left, int16_t rear_right) {
    if (handler == NULL) {
        return DASHBOARD_RC_NULL_POINTER;
    }
    prv_dashboard_api_set_temperature_quad(handler, DASHBOARD_FIELD_MOTORS_FRONT_LEFT, DASHBOARD_FIELD_MOTORS_FRONT_RIGHT, DASHBOARD_FIELD_MOTORS_REAR_LEFT, DASHBOARD_FIELD_MOTORS_REAR_RIGHT, front_left, front_right, rear_left, rear_right);

    handler->labels[DASHBOARD_FIELD_MOTORS_FRONT_LEFT].color.argb = prv_dashboard_api_motor_temp_color(front_left);
    handler->labels[DASHBOARD_FIELD_MOTORS_FRONT_RIGHT].color.argb = prv_dashboard_api_motor_temp_color(front_right);
    handler->labels[DASHBOARD_FIELD_MOTORS_REAR_LEFT].color.argb = prv_dashboard_api_motor_temp_color(rear_left);
    handler->labels[DASHBOARD_FIELD_MOTORS_REAR_RIGHT].color.argb = prv_dashboard_api_motor_temp_color(rear_right);

    return DASHBOARD_RC_OK;
}
