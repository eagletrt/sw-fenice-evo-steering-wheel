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
#include "watchdogs-api.h"

#include <stddef.h>
#include <string.h>

EAGLETRT_STATIC struct InputEventsHandler input_events_handler;

/*!
 * \brief Every combo, indexed by InputEventsComboId.
 */
EAGLETRT_STATIC const struct InputEventsCombo prv_input_events_api_combos[INPUT_EVENTS_COMBO_COUNT] = {
    [INPUT_EVENTS_COMBO_MENU] = { .buttons = INPUT_EVENTS_COMBO_MENU_BUTTONS, .hold_ms = INPUT_EVENTS_COMBO_HOLD_MS },
    [INPUT_EVENTS_COMBO_WAKE] = { .buttons = INPUT_EVENTS_COMBO_WAKE_BUTTONS, .hold_ms = INPUT_EVENTS_COMBO_HOLD_MS },
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
 * \brief A combo has been held for its hold time: emit it.
 *
 * \details Its watchdog is now timed out, which is what keeps it from firing
 *     again until one of its buttons is released.
 *
 * \param combo_id The combo.
 */
EAGLETRT_STATIC void prv_input_events_api_fire_combo(enum InputEventsComboId combo_id) {
    const struct InputEvent event = { .type = INPUT_EVENT_TYPE_COMBO, .combo = combo_id };
    // Called from the watchdog routine: there is nobody to report a failure to.
    EAGLETRT_API_UNUSED(prv_input_events_api_emit(&event));
}

EAGLETRT_STATIC void prv_input_events_api_on_menu_timeout(void) {
    prv_input_events_api_fire_combo(INPUT_EVENTS_COMBO_MENU);
}

EAGLETRT_STATIC void prv_input_events_api_on_wake_timeout(void) {
    prv_input_events_api_fire_combo(INPUT_EVENTS_COMBO_WAKE);
}

/*!
 * \brief Watchdog callback of every combo: libtimebase passes no argument, so one function each.
 */
EAGLETRT_STATIC const watchdog_timeout_callback prv_input_events_api_combo_timeouts[INPUT_EVENTS_COMBO_COUNT] = {
    [INPUT_EVENTS_COMBO_MENU] = prv_input_events_api_on_menu_timeout,
    [INPUT_EVENTS_COMBO_WAKE] = prv_input_events_api_on_wake_timeout,
};

/*!
 * \brief A button has been held for INPUT_EVENTS_LONG_PRESS_MS: emit the long press.
 *
 * \details Its watchdog is now timed out, which is what keeps it from firing
 *     again until the button is released.
 *
 * \param button_id The button.
 */
EAGLETRT_STATIC void prv_input_events_api_fire_long_press(enum InputsSharedButtonID button_id) {
    const struct InputEvent event = { .type = INPUT_EVENT_TYPE_BUTTON_LONG_PRESS, .button = button_id };
    // Called from the watchdog routine: there is nobody to report a failure to.
    EAGLETRT_API_UNUSED(prv_input_events_api_emit(&event));
}

/*!
 * \brief Define the long-press watchdog callback of one button (libtimebase passes no argument).
 */
#define INPUT_EVENTS_LONG_PRESS_TIMEOUT(button)                                 \
    EAGLETRT_STATIC void prv_input_events_api_long_press_##button(void) {       \
        prv_input_events_api_fire_long_press(INPUTS_SHARED_BUTTON_ID_##button); \
    }

INPUT_EVENTS_LONG_PRESS_TIMEOUT(TS_ON)
INPUT_EVENTS_LONG_PRESS_TIMEOUT(TOP_LEFT)
INPUT_EVENTS_LONG_PRESS_TIMEOUT(BOTTOM_LEFT)
INPUT_EVENTS_LONG_PRESS_TIMEOUT(TOP_RIGHT)
INPUT_EVENTS_LONG_PRESS_TIMEOUT(BOTTOM_RIGHT)
INPUT_EVENTS_LONG_PRESS_TIMEOUT(PADDLE_TOP_LEFT)
INPUT_EVENTS_LONG_PRESS_TIMEOUT(PADDLE_BOTTOM_LEFT)
INPUT_EVENTS_LONG_PRESS_TIMEOUT(PADDLE_TOP_RIGHT)
INPUT_EVENTS_LONG_PRESS_TIMEOUT(PADDLE_BOTTOM_RIGHT)

/*!
 * \brief Long-press watchdog callback of every button, indexed by InputsSharedButtonID.
 *
 * \details A button added to InputsSharedButtonID without an entry here
 *     makes input_events_api_init fail.
 */
EAGLETRT_STATIC const watchdog_timeout_callback prv_input_events_api_long_press_timeouts[INPUTS_SHARED_BUTTON_ID_COUNT] = {
    [INPUTS_SHARED_BUTTON_ID_TS_ON] = prv_input_events_api_long_press_TS_ON,
    [INPUTS_SHARED_BUTTON_ID_TOP_LEFT] = prv_input_events_api_long_press_TOP_LEFT,
    [INPUTS_SHARED_BUTTON_ID_BOTTOM_LEFT] = prv_input_events_api_long_press_BOTTOM_LEFT,
    [INPUTS_SHARED_BUTTON_ID_TOP_RIGHT] = prv_input_events_api_long_press_TOP_RIGHT,
    [INPUTS_SHARED_BUTTON_ID_BOTTOM_RIGHT] = prv_input_events_api_long_press_BOTTOM_RIGHT,
    [INPUTS_SHARED_BUTTON_ID_PADDLE_TOP_LEFT] = prv_input_events_api_long_press_PADDLE_TOP_LEFT,
    [INPUTS_SHARED_BUTTON_ID_PADDLE_BOTTOM_LEFT] = prv_input_events_api_long_press_PADDLE_BOTTOM_LEFT,
    [INPUTS_SHARED_BUTTON_ID_PADDLE_TOP_RIGHT] = prv_input_events_api_long_press_PADDLE_TOP_RIGHT,
    [INPUTS_SHARED_BUTTON_ID_PADDLE_BOTTOM_RIGHT] = prv_input_events_api_long_press_PADDLE_BOTTOM_RIGHT,
};

enum InputEventsReturnCode input_events_api_init(struct WatchdogHandler *watchdogs, input_events_sink_callback sink) {
    if (watchdogs == NULL || sink == NULL) {
        return INPUT_EVENTS_RC_NULL_POINTER;
    }

    memset(&input_events_handler, 0, sizeof(input_events_handler));
    input_events_handler.watchdogs = watchdogs;
    input_events_handler.sink = sink;

    for (enum InputsSharedButtonID i = 0; i < INPUTS_SHARED_BUTTON_ID_COUNT; i++) {
        if (prv_input_events_api_long_press_timeouts[i] == NULL ||
            watchdogs_api_init_watchdog(&input_events_handler.long_press_watchdogs[i], INPUT_EVENTS_LONG_PRESS_MS, prv_input_events_api_long_press_timeouts[i]) != WATCHDOG_RC_OK) {
            return INPUT_EVENTS_RC_ERROR;
        }
    }
    for (uint8_t i = 0U; i < INPUT_EVENTS_COMBO_COUNT; i++) {
        if (watchdogs_api_init_watchdog(&input_events_handler.combo_watchdogs[i], prv_input_events_api_combos[i].hold_ms, prv_input_events_api_combo_timeouts[i]) != WATCHDOG_RC_OK) {
            return INPUT_EVENTS_RC_ERROR;
        }
    }
    return INPUT_EVENTS_RC_OK;
}

const struct InputEventsCombo *input_events_api_get_combo(enum InputEventsComboId combo_id) {
    if (combo_id >= INPUT_EVENTS_COMBO_COUNT) {
        return NULL;
    }
    return &prv_input_events_api_combos[combo_id];
}

bool input_events_api_is_held(enum InputsSharedButtonID button_id) {
    if (button_id >= INPUTS_SHARED_BUTTON_ID_COUNT) {
        return false;
    }
    return (input_events_handler.held & INPUT_EVENTS_BUTTON_BIT(button_id)) != 0U;
}

enum InputsReturnCode input_events_api_on_button_press(enum InputsSharedButtonID button_id) {
    if (button_id >= INPUTS_SHARED_BUTTON_ID_COUNT) {
        return INPUTS_RC_ERROR;
    }

    const bool was_held = input_events_api_is_held(button_id);
    input_events_handler.held |= INPUT_EVENTS_BUTTON_BIT(button_id);
    const struct InputEvent event = { .type = INPUT_EVENT_TYPE_BUTTON_PRESS, .button = button_id };
    enum InputsReturnCode return_code = prv_input_events_api_emit(&event);

    // A repeated press of a button already held keeps its original hold time.
    if (!was_held && watchdogs_api_watchdog_restart(input_events_handler.watchdogs, &input_events_handler.long_press_watchdogs[button_id]) != WATCHDOG_RC_OK) {
        return_code = INPUTS_RC_ERROR;
    }

    // Start the hold time of every combo this press completes. A running
    // watchdog is already counting, a timed-out one has already fired.
    for (uint8_t i = 0U; i < INPUT_EVENTS_COMBO_COUNT; i++) {
        const uint16_t buttons = prv_input_events_api_combos[i].buttons;
        struct Watchdog *watchdog = &input_events_handler.combo_watchdogs[i];
        if ((buttons & INPUT_EVENTS_BUTTON_BIT(button_id)) == 0U ||
            (input_events_handler.held & buttons) != buttons ||
            watchdogs_api_watchdog_is_running(watchdog) ||
            watchdogs_api_watchdog_is_timed_out(watchdog)) {
            continue;
        }
        if (watchdogs_api_watchdog_restart(input_events_handler.watchdogs, watchdog) != WATCHDOG_RC_OK) {
            return_code = INPUTS_RC_ERROR;
        }
    }
    return return_code;
}

enum InputsReturnCode input_events_api_on_button_release(enum InputsSharedButtonID button_id) {
    if (button_id >= INPUTS_SHARED_BUTTON_ID_COUNT) {
        return INPUTS_RC_ERROR;
    }

    enum InputsReturnCode return_code = INPUTS_RC_OK;
    input_events_handler.held &= (uint16_t)~INPUT_EVENTS_BUTTON_BIT(button_id);

    // Cancels a pending long press, or re-arms it if it already fired.
    if (watchdogs_api_watchdog_reset(input_events_handler.watchdogs, &input_events_handler.long_press_watchdogs[button_id]) != WATCHDOG_RC_OK) {
        return_code = INPUTS_RC_ERROR;
    }

    // Releasing any button of a combo cancels its hold time, or re-arms it
    // if it already fired.
    for (uint8_t i = 0U; i < INPUT_EVENTS_COMBO_COUNT; i++) {
        if ((prv_input_events_api_combos[i].buttons & INPUT_EVENTS_BUTTON_BIT(button_id)) == 0U) {
            continue;
        }
        if (watchdogs_api_watchdog_reset(input_events_handler.watchdogs, &input_events_handler.combo_watchdogs[i]) != WATCHDOG_RC_OK) {
            return_code = INPUTS_RC_ERROR;
        }
    }

    const struct InputEvent event = { .type = INPUT_EVENT_TYPE_BUTTON_RELEASE, .button = button_id };
    if (prv_input_events_api_emit(&event) != INPUTS_RC_OK) {
        return_code = INPUTS_RC_ERROR;
    }
    return return_code;
}

enum InputsReturnCode input_events_api_on_knob_rotation(enum InputsSharedKnobID knob_id, int8_t delta) {
    if (knob_id >= INPUTS_SHARED_KNOB_ID_COUNT) {
        return INPUTS_RC_ERROR;
    }
    const struct InputEvent event = { .type = INPUT_EVENT_TYPE_KNOB_ROTATION, .knob = knob_id, .delta = delta };
    return prv_input_events_api_emit(&event);
}
