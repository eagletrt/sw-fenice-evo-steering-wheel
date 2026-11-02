/*!
 * \file input-events-api.c
 * \date 2026-10-31
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief Implementation of the input event layer.
 */

#include "input-events-api.h"

#include "eagletrt-api.h"
#include "timeout-api.h"

#include <stddef.h>
#include <string.h>

EAGLETRT_STATIC struct InputEventsHandler input_events_handler;

/*!
 * \brief Buttons of every combo, indexed by InputEventsComboId.
 */
EAGLETRT_STATIC const uint16_t prv_input_events_api_combos[INPUT_EVENTS_COMBO_COUNT] = {
    [INPUT_EVENTS_COMBO_MENU] = INPUT_EVENTS_COMBO_MENU_BUTTONS,
    [INPUT_EVENTS_COMBO_WAKE] = INPUT_EVENTS_COMBO_WAKE_BUTTONS,
};

/*!
 * \brief Hand an event to the sink.
 *
 * \param event The event.
 *
 * \retval INPUTS_RC_OK if the sink handled it.
 * \retval INPUTS_RC_ERROR if the layer is not initialised or the sink failed.
 */
EAGLETRT_STATIC enum InputsReturnCode prv_input_events_api_emit(const struct InputEvent *event) {
    if (input_events_handler.sink == NULL) {
        return INPUTS_RC_ERROR;
    }
    return (input_events_handler.sink(event) == INPUT_EVENTS_RC_OK) ? INPUTS_RC_OK : INPUTS_RC_ERROR;
}

/*!
 * \brief Emit a button event.
 *
 * \param button_id The button.
 * \param type What happened.
 *
 * \retval INPUTS_RC_OK if the sink handled it.
 * \retval INPUTS_RC_ERROR otherwise.
 */
EAGLETRT_STATIC enum InputsReturnCode prv_input_events_api_emit_button(enum InputsSharedButtonID button_id, enum InputEventType type) {
    const struct InputEvent event = { .source = INPUT_EVENT_SOURCE_BUTTON, .type = type, .button = button_id };
    return prv_input_events_api_emit(&event);
}

/*!
 * \brief Emit a combo event.
 *
 * \param combo_id The combo.
 * \param type What happened.
 *
 * \retval INPUTS_RC_OK if the sink handled it.
 * \retval INPUTS_RC_ERROR otherwise.
 */
EAGLETRT_STATIC enum InputsReturnCode prv_input_events_api_emit_combo(enum InputEventsComboId combo_id, enum InputEventType type) {
    const struct InputEvent event = { .source = INPUT_EVENT_SOURCE_COMBO, .type = type, .combo = combo_id };
    return prv_input_events_api_emit(&event);
}

/*!
 * \brief Combos recognised right now.
 *
 * \return Mask of INPUT_EVENTS_COMBO_BIT.
 */
EAGLETRT_STATIC uint8_t prv_input_events_api_active_combos(void) {
    return (input_events_handler.active_combos != NULL) ? input_events_handler.active_combos() : 0U;
}

/*!
 * \brief Buttons of a combo if it is recognised right now.
 *
 * \param active Mask from prv_input_events_api_active_combos.
 * \param combo_id The combo.
 *
 * \return Its buttons, 0 if it is not active.
 */
EAGLETRT_STATIC uint16_t prv_input_events_api_active_buttons(uint8_t active, enum InputEventsComboId combo_id) {
    return ((active & INPUT_EVENTS_COMBO_BIT(combo_id)) != 0U) ? prv_input_events_api_combos[combo_id] : 0U;
}

/*!
 * \brief Buttons currently in the combo window.
 *
 * \return Mask of INPUT_EVENTS_BUTTON_BIT.
 */
EAGLETRT_STATIC uint16_t prv_input_events_api_pending_mask(void) {
    uint16_t mask = 0U;
    for (uint8_t i = 0U; i < input_events_handler.pending_count; i++) {
        mask |= INPUT_EVENTS_BUTTON_BIT(input_events_handler.pending[i]);
    }
    return mask;
}

/*!
 * \brief Put a button in the combo window.
 *
 * \param button_id The button.
 */
EAGLETRT_STATIC void prv_input_events_api_add_pending(enum InputsSharedButtonID button_id) {
    input_events_handler.buttons[button_id].phase = INPUT_EVENTS_BUTTON_PENDING;
    input_events_handler.pending[input_events_handler.pending_count++] = button_id;
}

/*!
 * \brief Deliver every button of the combo window as a single press, in press order.
 *
 * \details The window is over without a combo.
 *
 * \retval INPUTS_RC_OK on success.
 * \retval INPUTS_RC_ERROR if the sink or a timeout failed.
 */
EAGLETRT_STATIC enum InputsReturnCode prv_input_events_api_flush_pending(void) {
    enum InputsReturnCode return_code = (timeout_api_stop(input_events_handler.window) == TIMEOUT_RC_OK) ? INPUTS_RC_OK : INPUTS_RC_ERROR;
    // Emptied before emitting: the sink may cause new presses to be reported.
    const uint8_t count = input_events_handler.pending_count;
    enum InputsSharedButtonID pending[INPUTS_SHARED_BUTTON_ID_COUNT];
    memcpy(pending, input_events_handler.pending, sizeof(pending));
    input_events_handler.pending_count = 0U;

    for (uint8_t i = 0U; i < count; i++) {
        input_events_handler.buttons[pending[i]].phase = INPUT_EVENTS_BUTTON_SINGLE;
        if (prv_input_events_api_emit_button(pending[i], INPUT_EVENT_TYPE_PRESS) != INPUTS_RC_OK) {
            return_code = INPUTS_RC_ERROR;
        }
    }
    return return_code;
}

/*!
 * \brief The buttons of the combo window form a combo: press it.
 *
 * \param combo_id The combo.
 *
 * \retval INPUTS_RC_OK on success.
 * \retval INPUTS_RC_ERROR if the sink or a timeout failed.
 */
EAGLETRT_STATIC enum InputsReturnCode prv_input_events_api_press_combo(enum InputEventsComboId combo_id) {
    enum InputsReturnCode return_code = (timeout_api_stop(input_events_handler.window) == TIMEOUT_RC_OK) ? INPUTS_RC_OK : INPUTS_RC_ERROR;
    // Its buttons have no gesture of their own any more.
    for (uint8_t i = 0U; i < input_events_handler.pending_count; i++) {
        struct InputEventsButton *button = &input_events_handler.buttons[input_events_handler.pending[i]];
        button->phase = INPUT_EVENTS_BUTTON_COMBO;
        if (timeout_api_stop(button->long_press) != TIMEOUT_RC_OK) {
            return_code = INPUTS_RC_ERROR;
        }
    }
    input_events_handler.pending_count = 0U;

    input_events_handler.combo = combo_id;
    if (timeout_api_start(input_events_handler.combo_long_press) != TIMEOUT_RC_OK ||
        prv_input_events_api_emit_combo(combo_id, INPUT_EVENT_TYPE_PRESS) != INPUTS_RC_OK) {
        return_code = INPUTS_RC_ERROR;
    }
    return return_code;
}

/*!
 * \brief Combo window over without a combo: the buttons in it are single presses.
 *
 * \param context Unused.
 */
EAGLETRT_STATIC void prv_input_events_api_on_window_expired(void *context) {
    EAGLETRT_API_UNUSED(context);
    // Called from the timeout routine: there is nobody to report a failure to.
    EAGLETRT_API_UNUSED(prv_input_events_api_flush_pending());
}

/*!
 * \brief A button has been down for INPUT_EVENTS_LONG_PRESS_MS.
 *
 * \details Only a single button gets here: a combo stops its buttons'
 *     timers, and a button still in the combo window can't reach it
 *     (INPUT_EVENTS_LONG_PRESS_MS > INPUT_EVENTS_COMBO_WINDOW_MS).
 *
 * \param context The InputEventsButton.
 */
EAGLETRT_STATIC void prv_input_events_api_on_long_press(void *context) {
    const struct InputEventsButton *button = (const struct InputEventsButton *)context;
    EAGLETRT_API_UNUSED(prv_input_events_api_emit_button(button->id, INPUT_EVENT_TYPE_LONG_PRESS));
}

/*!
 * \brief The pressed combo has been held for INPUT_EVENTS_COMBO_LONG_PRESS_MS.
 *
 * \param context Unused.
 */
EAGLETRT_STATIC void prv_input_events_api_on_combo_long_press(void *context) {
    EAGLETRT_API_UNUSED(context);
    if (input_events_handler.combo < INPUT_EVENTS_COMBO_COUNT) {
        EAGLETRT_API_UNUSED(prv_input_events_api_emit_combo(input_events_handler.combo, INPUT_EVENT_TYPE_LONG_PRESS));
    }
}

/*!
 * \brief A button went down while the combo window is open.
 *
 * \param button_id The button.
 * \param active Combos recognised right now.
 * \param[out] handled true if the button completed a combo or joined the window.
 *
 * \retval INPUTS_RC_OK on success.
 * \retval INPUTS_RC_ERROR if the sink or a timeout failed.
 */
EAGLETRT_STATIC enum InputsReturnCode prv_input_events_api_press_in_window(enum InputsSharedButtonID button_id, uint8_t active, bool *handled) {
    const uint16_t candidate = prv_input_events_api_pending_mask() | INPUT_EVENTS_BUTTON_BIT(button_id);
    bool partial = false;
    for (enum InputEventsComboId i = 0; i < INPUT_EVENTS_COMBO_COUNT; i++) {
        const uint16_t combo = prv_input_events_api_active_buttons(active, i);
        if (combo == 0U || (candidate & combo) != candidate) {
            continue;
        }
        if (candidate == combo) {
            *handled = true;
            prv_input_events_api_add_pending(button_id);
            return prv_input_events_api_press_combo(i);
        }
        partial = true;
    }
    if (partial) {
        // Part of a bigger combo: wait for the rest, the window keeps its start.
        *handled = true;
        prv_input_events_api_add_pending(button_id);
        return INPUTS_RC_OK;
    }
    // Belongs to no combo with the others: they were single presses after all.
    *handled = false;
    return prv_input_events_api_flush_pending();
}

enum InputEventsReturnCode input_events_api_init(input_events_sink_callback sink, input_events_active_combos_callback active_combos) {
    if (sink == NULL) {
        return INPUT_EVENTS_RC_NULL_POINTER;
    }

    memset(&input_events_handler, 0, sizeof(input_events_handler));
    input_events_handler.sink = sink;
    input_events_handler.active_combos = active_combos;
    input_events_handler.combo = INPUT_EVENTS_NO_COMBO;

    for (enum InputsSharedButtonID i = 0; i < INPUTS_SHARED_BUTTON_ID_COUNT; i++) {
        struct InputEventsButton *button = &input_events_handler.buttons[i];
        button->id = i;
        button->phase = INPUT_EVENTS_BUTTON_RELEASED;
        if (timeout_api_create(INPUT_EVENTS_LONG_PRESS_MS, prv_input_events_api_on_long_press, button, &button->long_press) != TIMEOUT_RC_OK) {
            return INPUT_EVENTS_RC_ERROR;
        }
    }
    if (timeout_api_create(INPUT_EVENTS_COMBO_WINDOW_MS, prv_input_events_api_on_window_expired, NULL, &input_events_handler.window) != TIMEOUT_RC_OK ||
        timeout_api_create(INPUT_EVENTS_COMBO_LONG_PRESS_MS, prv_input_events_api_on_combo_long_press, NULL, &input_events_handler.combo_long_press) != TIMEOUT_RC_OK) {
        return INPUT_EVENTS_RC_ERROR;
    }
    return INPUT_EVENTS_RC_OK;
}

uint16_t input_events_api_get_combo_buttons(enum InputEventsComboId combo_id) {
    return (combo_id < INPUT_EVENTS_COMBO_COUNT) ? prv_input_events_api_combos[combo_id] : 0U;
}

bool input_events_api_is_held(enum InputsSharedButtonID button_id) {
    return (button_id < INPUTS_SHARED_BUTTON_ID_COUNT) && (input_events_handler.buttons[button_id].phase != INPUT_EVENTS_BUTTON_RELEASED);
}

enum InputsReturnCode input_events_api_on_button_press(enum InputsSharedButtonID button_id) {
    if (button_id >= INPUTS_SHARED_BUTTON_ID_COUNT) {
        return INPUTS_RC_ERROR;
    }
    struct InputEventsButton *button = &input_events_handler.buttons[button_id];
    if (button->phase != INPUT_EVENTS_BUTTON_RELEASED) {
        return INPUTS_RC_OK;
    }

    enum InputsReturnCode return_code = (timeout_api_start(button->long_press) == TIMEOUT_RC_OK) ? INPUTS_RC_OK : INPUTS_RC_ERROR;
    const uint8_t active = prv_input_events_api_active_combos();

    if (input_events_handler.pending_count > 0U) {
        bool handled = false;
        if (prv_input_events_api_press_in_window(button_id, active, &handled) != INPUTS_RC_OK) {
            return_code = INPUTS_RC_ERROR;
        }
        if (handled) {
            return return_code;
        }
    }

    // A fresh press: open the window if it may start a combo, deliver it otherwise.
    for (enum InputEventsComboId i = 0; i < INPUT_EVENTS_COMBO_COUNT; i++) {
        if ((prv_input_events_api_active_buttons(active, i) & INPUT_EVENTS_BUTTON_BIT(button_id)) != 0U) {
            prv_input_events_api_add_pending(button_id);
            if (timeout_api_start(input_events_handler.window) != TIMEOUT_RC_OK) {
                return_code = INPUTS_RC_ERROR;
            }
            return return_code;
        }
    }
    button->phase = INPUT_EVENTS_BUTTON_SINGLE;
    if (prv_input_events_api_emit_button(button_id, INPUT_EVENT_TYPE_PRESS) != INPUTS_RC_OK) {
        return_code = INPUTS_RC_ERROR;
    }
    return return_code;
}

/*!
 * \brief A single button went up: release, and tap if it wasn't a long press.
 *
 * \param button The button.
 *
 * \retval INPUTS_RC_OK on success.
 * \retval INPUTS_RC_ERROR if the sink or a timeout failed.
 */
EAGLETRT_STATIC enum InputsReturnCode prv_input_events_api_release_single(struct InputEventsButton *button) {
    const bool long_pressed = timeout_api_has_expired(button->long_press);
    button->phase = INPUT_EVENTS_BUTTON_RELEASED;
    if (timeout_api_stop(button->long_press) != TIMEOUT_RC_OK ||
        prv_input_events_api_emit_button(button->id, INPUT_EVENT_TYPE_RELEASE) != INPUTS_RC_OK ||
        (!long_pressed && prv_input_events_api_emit_button(button->id, INPUT_EVENT_TYPE_TAP) != INPUTS_RC_OK)) {
        return INPUTS_RC_ERROR;
    }
    return INPUTS_RC_OK;
}

/*!
 * \brief A button of the pressed combo went up.
 *
 * \details The first one releases the combo (and taps it if it wasn't a
 *     long press); the others go up silently.
 *
 * \param button The button.
 *
 * \retval INPUTS_RC_OK on success.
 * \retval INPUTS_RC_ERROR if the sink or a timeout failed.
 */
EAGLETRT_STATIC enum InputsReturnCode prv_input_events_api_release_combo(struct InputEventsButton *button) {
    button->phase = INPUT_EVENTS_BUTTON_RELEASED;
    const enum InputEventsComboId combo_id = input_events_handler.combo;
    if (combo_id >= INPUT_EVENTS_COMBO_COUNT) {
        return INPUTS_RC_OK;
    }
    const bool long_pressed = timeout_api_has_expired(input_events_handler.combo_long_press);
    input_events_handler.combo = INPUT_EVENTS_NO_COMBO;
    if (timeout_api_stop(input_events_handler.combo_long_press) != TIMEOUT_RC_OK ||
        prv_input_events_api_emit_combo(combo_id, INPUT_EVENT_TYPE_RELEASE) != INPUTS_RC_OK ||
        (!long_pressed && prv_input_events_api_emit_combo(combo_id, INPUT_EVENT_TYPE_TAP) != INPUTS_RC_OK)) {
        return INPUTS_RC_ERROR;
    }
    return INPUTS_RC_OK;
}

enum InputsReturnCode input_events_api_on_button_release(enum InputsSharedButtonID button_id) {
    if (button_id >= INPUTS_SHARED_BUTTON_ID_COUNT) {
        return INPUTS_RC_ERROR;
    }
    struct InputEventsButton *button = &input_events_handler.buttons[button_id];

    switch (button->phase) {
        case INPUT_EVENTS_BUTTON_PENDING: {
            // Released inside the window: no combo, every pending press now.
            const enum InputsReturnCode flushed = prv_input_events_api_flush_pending();
            const enum InputsReturnCode released = prv_input_events_api_release_single(button);
            return (flushed == INPUTS_RC_OK && released == INPUTS_RC_OK) ? INPUTS_RC_OK : INPUTS_RC_ERROR;
        }
        case INPUT_EVENTS_BUTTON_SINGLE:
            return prv_input_events_api_release_single(button);
        case INPUT_EVENTS_BUTTON_COMBO:
            return prv_input_events_api_release_combo(button);
        case INPUT_EVENTS_BUTTON_RELEASED:
        default:
            return INPUTS_RC_OK;
    }
}

enum InputsReturnCode input_events_api_on_knob_rotation(enum InputsSharedKnobID knob_id, int8_t delta) {
    if (knob_id >= INPUTS_SHARED_KNOB_ID_COUNT) {
        return INPUTS_RC_ERROR;
    }
    const struct InputEvent event = { .source = INPUT_EVENT_SOURCE_KNOB, .type = INPUT_EVENT_TYPE_ROTATION, .knob = knob_id, .delta = delta };
    return prv_input_events_api_emit(&event);
}
