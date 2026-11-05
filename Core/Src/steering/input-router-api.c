/*!
 * \file input-router-api.c
 * \date 2026-10-31
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief Implementation of the input router, with the binding table of every FSM state.
 */

#include "input-router-api.h"

#include "eagletrt-api.h"
#include "parameters-api.h"

#include <stddef.h>
#include <string.h>

static_assert(INPUT_ROUTER_COMBO_SOURCE(INPUT_EVENTS_COMBO_COUNT) <= PARAMETERS_HOLD_SOURCE_COUNT, "every button and combo needs its own hold source");

/*!
 * \brief A STEP of a parameter in the parameters group.
 */
#define INPUT_ROUTER_STEP(parameter_id, steps) { .type = INPUT_ACTION_STEP, .group = INPUT_ACTION_GROUP_PARAMETERS, .parameter = (parameter_id), .amount = (steps) }

/*!
 * \brief A TOGGLE of a parameter in the parameters group.
 */
#define INPUT_ROUTER_TOGGLE(parameter_id) { .type = INPUT_ACTION_TOGGLE, .group = INPUT_ACTION_GROUP_PARAMETERS, .parameter = (parameter_id), .amount = 0 }

/*!
 * \brief A HOLD of a parameter in a filter group.
 */
#define INPUT_ROUTER_HOLD(parameter_id, action_group) { .type = INPUT_ACTION_HOLD, .group = (action_group), .parameter = (parameter_id), .amount = 0 }

/*!
 * \brief A REQUEST, never filtered.
 */
#define INPUT_ROUTER_REQUEST(input_request) { .type = INPUT_ACTION_REQUEST, .group = INPUT_ACTION_GROUP_SYSTEM, .request = (input_request), .amount = 0 }

/*!
 * \brief Number of entries of a binding array.
 */
#define INPUT_ROUTER_COUNT(array) ((uint8_t)(sizeof(array) / sizeof((array)[0])))

/*!
 * \brief IDLE (driving): what every button does.
 *
 * \details The top buttons are the driver actions, 1 while held; they also
 *     form the menu combo, so their press arrives INPUT_EVENTS_COMBO_WINDOW_MS
 *     late and none is sent when both go down together.
 */
EAGLETRT_STATIC const struct InputButtonBinding prv_input_router_api_idle_buttons[] = {
    { INPUTS_SHARED_BUTTON_ID_BOTTOM_LEFT, INPUT_EVENT_TYPE_PRESS, INPUT_ROUTER_TOGGLE(INPUTS_SHARED_PARAMETER_ID_TELEMETRY_LOG) },
    { INPUTS_SHARED_BUTTON_ID_BOTTOM_RIGHT, INPUT_EVENT_TYPE_PRESS, INPUT_ROUTER_TOGGLE(INPUTS_SHARED_PARAMETER_ID_LAUNCH_CONTROL) },
    { INPUTS_SHARED_BUTTON_ID_PADDLE_TOP_LEFT, INPUT_EVENT_TYPE_PRESS, INPUT_ROUTER_HOLD(INPUTS_SHARED_PARAMETER_ID_PTT, INPUT_ACTION_GROUP_PTT) },
    { INPUTS_SHARED_BUTTON_ID_PADDLE_TOP_RIGHT, INPUT_EVENT_TYPE_PRESS, INPUT_ROUTER_HOLD(INPUTS_SHARED_PARAMETER_ID_PTT, INPUT_ACTION_GROUP_PTT) },
    { INPUTS_SHARED_BUTTON_ID_TS_ON, INPUT_EVENT_TYPE_PRESS, INPUT_ROUTER_HOLD(INPUTS_SHARED_PARAMETER_ID_TS_ON, INPUT_ACTION_GROUP_TS_ON) },
    { INPUTS_SHARED_BUTTON_ID_TOP_RIGHT, INPUT_EVENT_TYPE_PRESS, INPUT_ROUTER_HOLD(INPUTS_SHARED_PARAMETER_ID_DRIVER_OK, INPUT_ACTION_GROUP_PARAMETERS) },
    { INPUTS_SHARED_BUTTON_ID_TOP_LEFT, INPUT_EVENT_TYPE_PRESS, INPUT_ROUTER_HOLD(INPUTS_SHARED_PARAMETER_ID_DRIVER_REPEAT, INPUT_ACTION_GROUP_PARAMETERS) },
};

/*!
 * \brief IDLE (driving): combos. TODO(#9): the request opens the menu, today it is only logged.
 */
EAGLETRT_STATIC const struct InputComboBinding prv_input_router_api_idle_combos[] = {
    { INPUT_EVENTS_COMBO_MENU, INPUT_EVENT_TYPE_LONG_PRESS, INPUT_ROUTER_REQUEST(INPUT_REQUEST_MENU) },
};

/*!
 * \brief IDLE (driving): the manettini.
 */
EAGLETRT_STATIC const struct InputKnobBinding prv_input_router_api_idle_knobs[] = {
    { INPUTS_SHARED_KNOB_ID_FRONT_LEFT, INPUT_ROUTER_STEP(INPUTS_SHARED_PARAMETER_ID_POWER, 1) },
    { INPUTS_SHARED_KNOB_ID_FRONT_RIGHT, INPUT_ROUTER_STEP(INPUTS_SHARED_PARAMETER_ID_REGEN, 1) },
    { INPUTS_SHARED_KNOB_ID_FRONT_CENTER, INPUT_ROUTER_STEP(INPUTS_SHARED_PARAMETER_ID_TORQUE_VECTORING, 1) },
};

/*!
 * \brief Bindings of every FSM state, indexed by WheelFsmState.
 *
 * \details INIT, FLASH and ERROR bind nothing: inputs are ignored there
 *     (FLASH also pauses the inputs task); releases still let go of holds.
 *     TODO(#9): MENU, DEBUG, CALIBRATION and SCREEN_SAVER (the wake combo).
 */
EAGLETRT_STATIC const struct InputStateBindings prv_input_router_api_bindings[WHEEL_FSM_STATE_COUNT] = {
    [WHEEL_FSM_STATE_INIT] = { 0 },
    [WHEEL_FSM_STATE_IDLE] = {
        .buttons = prv_input_router_api_idle_buttons,
        .button_count = INPUT_ROUTER_COUNT(prv_input_router_api_idle_buttons),
        .combos = prv_input_router_api_idle_combos,
        .combo_count = INPUT_ROUTER_COUNT(prv_input_router_api_idle_combos),
        .knobs = prv_input_router_api_idle_knobs,
        .knob_count = INPUT_ROUTER_COUNT(prv_input_router_api_idle_knobs),
    },
    [WHEEL_FSM_STATE_FLASH] = { 0 },
    [WHEEL_FSM_STATE_ERROR] = { 0 },
};

EAGLETRT_STATIC struct InputRouterHandler input_router_handler;

/*!
 * \brief Whether the filter lets an action group through.
 *
 * \param group The group.
 *
 * \return true if allowed.
 */
EAGLETRT_STATIC bool prv_input_router_api_allowed(enum InputActionGroup group) {
    switch (group) {
        case INPUT_ACTION_GROUP_PARAMETERS:
            return input_router_handler.filter.parameters;
        case INPUT_ACTION_GROUP_PTT:
            return input_router_handler.filter.ptt;
        case INPUT_ACTION_GROUP_TS_ON:
            return input_router_handler.filter.ts_on;
        case INPUT_ACTION_GROUP_SYSTEM:
        default:
            return true;
    }
}

/*!
 * \brief Run one action.
 *
 * \param action The action.
 * \param source Hold source of the input (button id or combo source).
 * \param multiplier Knob rotation (steps), 1 for buttons and combos.
 *
 * \retval true on success (or if the filter dropped it).
 * \retval false if the parameter update failed.
 */
EAGLETRT_STATIC bool prv_input_router_api_run(const struct InputAction *action, uint8_t source, int8_t multiplier) {
    if (!prv_input_router_api_allowed(action->group)) {
        return true;
    }
    switch (action->type) {
        case INPUT_ACTION_STEP: {
            const int32_t steps = (int32_t)action->amount * (int32_t)multiplier;
            return parameters_api_step(action->parameter, (int8_t)EAGLETRT_API_CLAMP(steps, INT8_MIN, INT8_MAX)) == PARAMETERS_RC_OK;
        }
        case INPUT_ACTION_TOGGLE:
            return parameters_api_toggle(action->parameter) == PARAMETERS_RC_OK;
        case INPUT_ACTION_HOLD:
            return parameters_api_hold(action->parameter, source, true) == PARAMETERS_RC_OK;
        case INPUT_ACTION_REQUEST:
            if (input_router_handler.config.on_request != NULL) {
                input_router_handler.config.on_request(action->request);
            }
            return true;
        default:
            return true;
    }
}

/*!
 * \brief Run every action a state binds to a button or combo event.
 *
 * \param bindings The state's bindings.
 * \param event The event.
 *
 * \retval true on success.
 * \retval false if an action failed.
 */
EAGLETRT_STATIC bool prv_input_router_api_dispatch(const struct InputStateBindings *bindings, const struct InputEvent *event) {
    bool result = true;
    switch (event->source) {
        case INPUT_EVENT_SOURCE_BUTTON:
            for (uint8_t i = 0U; i < bindings->button_count; i++) {
                const struct InputButtonBinding *binding = &bindings->buttons[i];
                if (binding->button == event->button && binding->event == event->type) {
                    result = prv_input_router_api_run(&binding->action, (uint8_t)event->button, 1) && result;
                }
            }
            break;
        case INPUT_EVENT_SOURCE_COMBO:
            for (uint8_t i = 0U; i < bindings->combo_count; i++) {
                const struct InputComboBinding *binding = &bindings->combos[i];
                if (binding->combo == event->combo && binding->event == event->type) {
                    result = prv_input_router_api_run(&binding->action, INPUT_ROUTER_COMBO_SOURCE(event->combo), 1) && result;
                }
            }
            break;
        case INPUT_EVENT_SOURCE_KNOB:
            for (uint8_t i = 0U; i < bindings->knob_count; i++) {
                const struct InputKnobBinding *binding = &bindings->knobs[i];
                if (binding->knob == event->knob && event->type == INPUT_EVENT_TYPE_ROTATION) {
                    result = prv_input_router_api_run(&binding->action, 0U, event->delta) && result;
                }
            }
            break;
        default:
            break;
    }
    return result;
}

enum InputRouterReturnCode input_router_api_init(const struct InputRouterConfig *config) {
    if (config == NULL || config->get_state == NULL) {
        return INPUT_ROUTER_RC_NULL_POINTER;
    }

    memset(&input_router_handler, 0, sizeof(input_router_handler));
    input_router_handler.config = *config;
    input_router_handler.filter = (struct InputRouterFilter){ .parameters = true, .ptt = true, .ts_on = true };
    return INPUT_ROUTER_RC_OK;
}

void input_router_api_set_filter(struct InputRouterFilter filter) {
    input_router_handler.filter = filter;
}

struct InputRouterFilter input_router_api_get_filter(void) {
    return input_router_handler.filter;
}

const struct InputStateBindings *input_router_api_get_bindings(enum WheelFsmState state) {
    return (state < WHEEL_FSM_STATE_COUNT) ? &prv_input_router_api_bindings[state] : NULL;
}

uint8_t input_router_api_active_combos(void) {
    if (input_router_handler.config.get_state == NULL) {
        return 0U;
    }
    const struct InputStateBindings *bindings = input_router_api_get_bindings(input_router_handler.config.get_state());
    if (bindings == NULL) {
        return 0U;
    }
    uint8_t active = 0U;
    for (uint8_t i = 0U; i < bindings->combo_count; i++) {
        active |= INPUT_EVENTS_COMBO_BIT(bindings->combos[i].combo);
    }
    return active;
}

enum InputEventsReturnCode input_router_api_handle_event(const struct InputEvent *event) {
    if (event == NULL || input_router_handler.config.get_state == NULL) {
        return INPUT_EVENTS_RC_NULL_POINTER;
    }

    bool result = true;
    // Releases first, in every state and through any filter (input-router.h).
    if (event->type == INPUT_EVENT_TYPE_RELEASE) {
        if (event->source == INPUT_EVENT_SOURCE_BUTTON) {
            result = parameters_api_release_source((uint8_t)event->button) == PARAMETERS_RC_OK;
        } else if (event->source == INPUT_EVENT_SOURCE_COMBO) {
            result = parameters_api_release_source(INPUT_ROUTER_COMBO_SOURCE(event->combo)) == PARAMETERS_RC_OK;
        }
    }

    const struct InputStateBindings *bindings = input_router_api_get_bindings(input_router_handler.config.get_state());
    if (bindings != NULL) {
        result = prv_input_router_api_dispatch(bindings, event) && result;
    }
    return result ? INPUT_EVENTS_RC_OK : INPUT_EVENTS_RC_ERROR;
}
