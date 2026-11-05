/*!
 * \file inputs-api.c
 * \date 2025-12-24
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief Hardware-agnostic input handling implementation for the steering wheel.
 */

#include <string.h>

#include "inputs-api.h"
#include "eagletrt.h"

EAGLETRT_STATIC struct InputsHandler inputs_handler;

/*!
 * \brief Invoke a button callback if one is registered.
 *
 * \param callback Callback pointer (may be NULL).
 * \param button_id Identifier to pass through.
 *
 * \retval INPUTS_RC_OK if the callback is NULL or returned OK.
 * \retval INPUTS_RC_ERROR if the callback returned an error.
 */
EAGLETRT_STATIC enum InputsReturnCode prv_fire_button_callback(inputs_button_event_callback callback, enum InputsSharedButtonID button_id) {
    if (callback == NULL) {
        return INPUTS_RC_OK;
    }
    return callback(button_id);
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
enum InputsReturnCode inputs_api_init(inputs_button_event_callback on_button_press, inputs_button_event_callback on_button_release, inputs_knob_rotation_callback on_knob_rotation) {
    memset(&inputs_handler, 0, sizeof(inputs_handler));

    inputs_handler.on_button_press = on_button_press;
    inputs_handler.on_button_release = on_button_release;
    inputs_handler.on_knob_rotation = on_knob_rotation;

    for (enum InputsSharedButtonID i = 0; i < INPUTS_SHARED_BUTTON_ID_COUNT; i++) {
        inputs_handler.buttons[i].enabled = true;
        inputs_handler.buttons[i].pressed = false;
    }

    for (enum InputsSharedKnobID i = 0; i < INPUTS_SHARED_KNOB_ID_COUNT; i++) {
        inputs_handler.knobs[i].enabled = true;
        inputs_handler.knobs[i].last_position = 0;
    }

    return INPUTS_RC_OK;
}

enum InputsReturnCode inputs_api_update_button(const enum InputsSharedButtonID button_id, bool pressed) {
    if (button_id >= INPUTS_SHARED_BUTTON_ID_COUNT) {
        return INPUTS_RC_ERROR;
    }

    struct InputsButtonHandler *btn = &inputs_handler.buttons[button_id];
    if (!btn->enabled) {
        return INPUTS_RC_OK;
    }

    if (pressed == btn->pressed) {
        return INPUTS_RC_OK;
    }
    btn->pressed = pressed;
    return prv_fire_button_callback(pressed ? inputs_handler.on_button_press : inputs_handler.on_button_release, button_id);
}

enum InputsReturnCode inputs_api_update_knob(const enum InputsSharedKnobID knob_id, int16_t current_position) {
    if (knob_id >= INPUTS_SHARED_KNOB_ID_COUNT) {
        return INPUTS_RC_ERROR;
    }

    struct InputsKnobHandler *knob = &inputs_handler.knobs[knob_id];
    if (!knob->enabled) {
        return INPUTS_RC_OK;
    }

    int16_t delta = current_position - knob->last_position;
    knob->last_position = current_position;

    if (delta == 0 || inputs_handler.on_knob_rotation == NULL) {
        return INPUTS_RC_OK;
    }

    return inputs_handler.on_knob_rotation(knob_id, (int8_t)delta);
}
