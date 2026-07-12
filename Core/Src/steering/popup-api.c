/*!
 * \file popup-api.c
 * \date 2026-04-24
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief API for the parameter-change popup overlay.
 */

#include "popup-api.h"
#include "font.h"
#include "label-api.h"
#include "box-api.h"
#include "popup.h"
#include "screen.h"
#include "eagletrt-api.h"
#include "raster-fonts.h"
#include <stddef.h>
#include <stdio.h>

#define POPUP_VALUE_BOX_HEIGHT ((SCREEN_HEIGHT / 10U) * 7U)            /* 70% of the screen height */
#define POPUP_NAME_BOX_HEIGHT (SCREEN_HEIGHT - POPUP_VALUE_BOX_HEIGHT) /* remaining 30% */

/*!
 * \brief Human-readable names for each parameter, indexed by InputsSharedParameterID.
 */
EAGLETRT_STATIC const char *prv_parameter_names[INPUTS_SHARED_PARAMETER_ID_COUNT] = {
    [INPUTS_SHARED_PARAMETER_ID_POWER] = "POWER",
    [INPUTS_SHARED_PARAMETER_ID_REGEN] = "REGEN",
    [INPUTS_SHARED_PARAMETER_ID_TORQUE_VECTORING] = "TORQUE",
    [INPUTS_SHARED_PARAMETER_ID_TELEMETRY_LOG] = "LOG",
    [INPUTS_SHARED_PARAMETER_ID_LAUNCH_CONTROL] = "SLIP",
};

/*!
 * \brief Returns true if the parameter uses an ON/OFF toggle semantic.
 *
 * \param parameter_id The parameter to check.
 *
 * \retval true if the parameter is a toggle
 * \retval false if the parameter is a numeric value
 */
EAGLETRT_STATIC bool prv_popup_is_toggle(enum InputsSharedParameterID parameter_id) {
    return parameter_id == INPUTS_SHARED_PARAMETER_ID_TELEMETRY_LOG ||
           parameter_id == INPUTS_SHARED_PARAMETER_ID_LAUNCH_CONTROL;
}

/*!
 * \brief Format a parameter value into the popup buffer.
 *
 * \param handler Pointer to the popup handler owning the buffer.
 * \param parameter_id The parameter being displayed.
 * \param value The new parameter value.
 */
EAGLETRT_STATIC void prv_popup_format_value(struct PopupHandler *handler, const enum InputsSharedParameterID parameter_id, uint8_t value) {
    if (prv_popup_is_toggle(parameter_id)) {
        EAGLETRT_API_UNUSED(snprintf(handler->value_buffer, POPUP_VALUE_BUFFER_SIZE, "%s", value ? "ON" : "OFF"));
    } else {
        EAGLETRT_API_UNUSED(snprintf(handler->value_buffer, POPUP_VALUE_BUFFER_SIZE, "%u", (unsigned)value));
    }
}

enum PopupReturnCode popup_api_init(struct PopupHandler *handler) {
    if (handler == NULL) {
        return POPUP_RC_ERROR;
    }

    handler->active = false;
    handler->last_event_tick = 0U;
    handler->current_parameter = INPUTS_SHARED_PARAMETER_ID_POWER;
    handler->value_buffer[0] = '\0';

    /* labels[0] is the value (top, big), labels[1] is the parameter name (bottom, small). */
    const int16_t value_offset_y = (int16_t)((POPUP_VALUE_BOX_HEIGHT - POPUP_VALUE_FONT_SIZE) / 2U);
    const int16_t name_offset_y = (int16_t)((POPUP_NAME_BOX_HEIGHT - POPUP_NAME_FONT_SIZE) / 2U);
    const int16_t center_offset_x = (int16_t)(SCREEN_WIDTH / 2U);
    const struct Color foreground = { .argb = POPUP_COLOR_FOREGROUND };
    const struct Color background = { .argb = POPUP_COLOR_BACKGROUND };

    if (label_api_init(&handler->labels[0], handler->value_buffer, center_offset_x, value_offset_y, &font_inter, POPUP_VALUE_FONT_SIZE, FONT_ALIGN_CENTER, foreground) != RASTER_RC_OK) {
        return POPUP_RC_ERROR;
    }
    if (label_api_init(&handler->labels[1], prv_parameter_names[INPUTS_SHARED_PARAMETER_ID_POWER], center_offset_x, name_offset_y, &font_inter, POPUP_NAME_FONT_SIZE, FONT_ALIGN_CENTER, foreground) != RASTER_RC_OK) {
        return POPUP_RC_ERROR;
    }
    if (box_api_init(&handler->boxes[0], 0, (struct BoxRectangle){ .x = 0, .y = 0, .width = SCREEN_WIDTH, .height = POPUP_VALUE_BOX_HEIGHT }, background, &handler->labels[0]) != RASTER_RC_OK) {
        return POPUP_RC_ERROR;
    }
    if (box_api_init(&handler->boxes[1], 1, (struct BoxRectangle){ .x = 0, .y = POPUP_VALUE_BOX_HEIGHT, .width = SCREEN_WIDTH, .height = POPUP_NAME_BOX_HEIGHT }, background, &handler->labels[1]) != RASTER_RC_OK) {
        return POPUP_RC_ERROR;
    }
    return POPUP_RC_OK;
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
enum PopupReturnCode popup_api_show(struct PopupHandler *handler, enum InputsSharedParameterID parameter_id, uint8_t value, uint32_t tick) {
    if (handler == NULL || parameter_id >= INPUTS_SHARED_PARAMETER_ID_COUNT) {
        return POPUP_RC_ERROR;
    }

    handler->active = true;
    handler->last_event_tick = tick;
    handler->current_parameter = parameter_id;

    handler->labels[1].text = (char *)prv_parameter_names[parameter_id];

    prv_popup_format_value(handler, parameter_id, value);

    handler->boxes[0].updated = true;
    handler->boxes[1].updated = true;

    return POPUP_RC_OK;
}

void popup_api_dismiss(struct PopupHandler *handler) {
    if (handler == NULL) {
        return;
    }
    handler->active = false;
}

bool popup_api_is_active(struct PopupHandler *handler, uint32_t tick) {
    if (handler == NULL || !handler->active) {
        return false;
    }
    if (tick - handler->last_event_tick >= SCREEN_POPUP_TIMEOUT_MS) {
        handler->active = false;
        return false;
    }
    return true;
}
