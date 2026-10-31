/*!
 * \file input-router-api.c
 * \date 2026-10-31
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief Implementation of the input router.
 */

#include "input-router-api.h"

#include "eagletrt.h"
#include "parameters-api.h"

#include <limits.h>
#include <stddef.h>
#include <string.h>

/*!
 * \brief Bit of a combo in a route's combo mask.
 */
#define INPUT_ROUTER_COMBO_BIT(combo_id) ((uint8_t)(1U << (uint8_t)(combo_id)))

static_assert(INPUT_EVENTS_COMBO_COUNT <= sizeof(uint8_t) * CHAR_BIT, "every combo needs a bit in a route mask");

/*!
 * \brief What one FSM state does with inputs.
 */
struct InputRouterRoute {
    bool controls;  /*!< Driver controls reach the parameters (through the filter) */
    uint8_t combos; /*!< Mask of INPUT_ROUTER_COMBO_BIT recognised in this state */
};

/*!
 * \brief Routing of every FSM state, indexed by WheelFsmState.
 *
 * \details INIT, FLASH and ERROR ignore inputs (FLASH also pauses the inputs
 *     task). TODO(#9): MENU, DEBUG, CALIBRATION and SCREEN_SAVER rows (the
 *     wake combo belongs to SCREEN_SAVER).
 */
EAGLETRT_STATIC const struct InputRouterRoute prv_input_router_api_routes[WHEEL_FSM_STATE_COUNT] = {
    [WHEEL_FSM_STATE_INIT] = { .controls = false, .combos = 0U },
    [WHEEL_FSM_STATE_IDLE] = { .controls = true, .combos = INPUT_ROUTER_COMBO_BIT(INPUT_EVENTS_COMBO_MENU) },
    [WHEEL_FSM_STATE_FLASH] = { .controls = false, .combos = 0U },
    [WHEEL_FSM_STATE_ERROR] = { .controls = false, .combos = 0U },
};

EAGLETRT_STATIC struct InputRouterHandler input_router_handler;

/*!
 * \brief Whether the filter lets a button press through.
 *
 * \param button_id The button.
 *
 * \return true if the button's group is allowed.
 */
EAGLETRT_STATIC bool prv_input_router_api_button_allowed(enum InputsSharedButtonID button_id) {
    switch (button_id) {
        case INPUTS_SHARED_BUTTON_ID_TS_ON:
            return input_router_handler.filter.ts_on;
        case INPUTS_SHARED_BUTTON_ID_PADDLE_TOP_LEFT:
        case INPUTS_SHARED_BUTTON_ID_PADDLE_TOP_RIGHT:
            return input_router_handler.filter.ptt;
        default:
            return input_router_handler.filter.parameters;
    }
}

/*!
 * \brief Map a parameters result to the events sink result.
 */
EAGLETRT_STATIC enum InputEventsReturnCode prv_input_router_api_result(enum InputsReturnCode return_code) {
    return (return_code == INPUTS_RC_OK) ? INPUT_EVENTS_RC_OK : INPUT_EVENTS_RC_ERROR;
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

enum InputEventsReturnCode input_router_api_handle_event(const struct InputEvent *event) {
    if (event == NULL || input_router_handler.config.get_state == NULL) {
        return INPUT_EVENTS_RC_NULL_POINTER;
    }

    // Releases skip both the state and the filter (see input-router.h).
    if (event->type == INPUT_EVENT_TYPE_BUTTON_RELEASE) {
        return prv_input_router_api_result(parameters_api_handle_button_release(event->button));
    }

    const enum WheelFsmState state = input_router_handler.config.get_state();
    if (state >= WHEEL_FSM_STATE_COUNT) {
        return INPUT_EVENTS_RC_OK;
    }
    const struct InputRouterRoute *route = &prv_input_router_api_routes[state];

    switch (event->type) {
        case INPUT_EVENT_TYPE_BUTTON_PRESS:
            if (!route->controls || !prv_input_router_api_button_allowed(event->button)) {
                return INPUT_EVENTS_RC_OK;
            }
            return prv_input_router_api_result(parameters_api_handle_button(event->button));
        case INPUT_EVENT_TYPE_KNOB_ROTATION:
            if (!route->controls || !input_router_handler.filter.parameters) {
                return INPUT_EVENTS_RC_OK;
            }
            return prv_input_router_api_result(parameters_api_handle_knob(event->knob, event->delta));
        case INPUT_EVENT_TYPE_COMBO:
            if (event->combo < INPUT_EVENTS_COMBO_COUNT &&
                (route->combos & INPUT_ROUTER_COMBO_BIT(event->combo)) != 0U &&
                input_router_handler.config.on_combo != NULL) {
                input_router_handler.config.on_combo(event->combo);
            }
            return INPUT_EVENTS_RC_OK;
        case INPUT_EVENT_TYPE_BUTTON_LONG_PRESS:
            // No consumer yet: menu/debug navigation will use it.
        case INPUT_EVENT_TYPE_BUTTON_RELEASE:
        default:
            return INPUT_EVENTS_RC_OK;
    }
}
