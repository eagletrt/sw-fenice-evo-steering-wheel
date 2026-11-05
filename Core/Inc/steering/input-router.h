/*!
 * \file input-router.h
 * \date 2026-10-31
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief Definitions for the input router: what each input event does, per
 *     FSM state.
 *
 * \details Every FSM state has its own binding table (input-router-api.c):
 *     button events, combo events and knob rotations are bound to actions
 *     (step, toggle or hold a parameter, or a request such as opening the
 *     menu). Anything without a binding does nothing in that state, and the
 *     combos a state binds are the only ones recognised in it (see
 *     input-events.h).
 *
 *     Every action belongs to a group that the driving-context filter can
 *     turn off (parameters, PTT, TS-on; system actions are never filtered):
 *     in an autonomous mission the driver controls are ignored, and TS-on can
 *     be allowed again on its own.
 *
 *     Releases always let go of what the released button (or combo) holds,
 *     in every state and whatever the filter says: a PTT or TS-on held while
 *     the state or the filter changed must not stay on.
 */

#ifndef INPUT_ROUTER_H
#define INPUT_ROUTER_H

#include "input-events.h"
#include "wheel-fsm.h"

#include <stdbool.h>
#include <stdint.h>

/*!
 * \brief Return codes for input router operations.
 */
enum InputRouterReturnCode {
    INPUT_ROUTER_RC_OK,           /*!< Operation completed successfully */
    INPUT_ROUTER_RC_NULL_POINTER, /*!< A required pointer or callback was NULL */
};

/*!
 * \brief What an action does.
 */
enum InputActionType : uint8_t {
    INPUT_ACTION_STEP,    /*!< Move a parameter by amount (knobs: by amount x the rotation) */
    INPUT_ACTION_TOGGLE,  /*!< Flip a parameter */
    INPUT_ACTION_HOLD,    /*!< Hold a parameter at 1 until the input that did it is released */
    INPUT_ACTION_REQUEST, /*!< Ask the rest of the wheel for something (InputRequest) */
};

/*!
 * \brief Filter group of an action (InputRouterFilter).
 */
enum InputActionGroup : uint8_t {
    INPUT_ACTION_GROUP_PARAMETERS, /*!< Driver controls: knobs, toggles, driver actions */
    INPUT_ACTION_GROUP_PTT,        /*!< Push-to-talk */
    INPUT_ACTION_GROUP_TS_ON,      /*!< TS-on, separate so it can be re-allowed in autonomous */
    INPUT_ACTION_GROUP_SYSTEM,     /*!< Mode changes: never filtered */
};

/*!
 * \brief Requests the router hands to the rest of the wheel.
 */
enum InputRequest : uint8_t {
    INPUT_REQUEST_MENU, /*!< Open the menu */
    INPUT_REQUEST_WAKE, /*!< Leave the screen saver */
};

/*!
 * \brief An action and its filter group.
 */
struct InputAction {
    enum InputActionType type;   /*!< What it does */
    enum InputActionGroup group; /*!< Which filter flag gates it */
    union {
        enum InputsSharedParameterID parameter; /*!< STEP, TOGGLE, HOLD */
        enum InputRequest request;              /*!< REQUEST */
    };
    int8_t amount; /*!< STEP only: steps per event (knobs: per detent) */
};

/*!
 * \brief A button event bound to an action.
 */
struct InputButtonBinding {
    enum InputsSharedButtonID button; /*!< The button */
    enum InputEventType event;        /*!< PRESS, RELEASE, TAP or LONG_PRESS */
    struct InputAction action;        /*!< What it does */
};

/*!
 * \brief A combo event bound to an action.
 */
struct InputComboBinding {
    enum InputEventsComboId combo; /*!< The combo (recognised in the state only because it is bound) */
    enum InputEventType event;     /*!< PRESS, RELEASE, TAP or LONG_PRESS */
    struct InputAction action;     /*!< What it does */
};

/*!
 * \brief A knob bound to an action (every rotation).
 */
struct InputKnobBinding {
    enum InputsSharedKnobID knob; /*!< The knob */
    struct InputAction action;    /*!< What it does, usually a STEP */
};

/*!
 * \brief Everything inputs do in one FSM state.
 */
struct InputStateBindings {
    const struct InputButtonBinding *buttons; /*!< Button bindings */
    const struct InputComboBinding *combos;   /*!< Combo bindings */
    const struct InputKnobBinding *knobs;     /*!< Knob bindings */
    uint8_t button_count;                     /*!< Entries of buttons */
    uint8_t combo_count;                      /*!< Entries of combos */
    uint8_t knob_count;                       /*!< Entries of knobs */
};

/*!
 * \brief Hold source of a combo (buttons use their own id).
 */
#define INPUT_ROUTER_COMBO_SOURCE(combo_id) ((uint8_t)((uint8_t)INPUTS_SHARED_BUTTON_ID_COUNT + (uint8_t)(combo_id)))

/*!
 * \brief Which groups of actions are allowed.
 *
 * \details All true at init (manual driving). TODO(#11): set from the
 *     driving context, parameters/PTT/TS-on false in an autonomous mission
 *     (D10).
 */
struct InputRouterFilter {
    bool parameters; /*!< INPUT_ACTION_GROUP_PARAMETERS */
    bool ptt;        /*!< INPUT_ACTION_GROUP_PTT */
    bool ts_on;      /*!< INPUT_ACTION_GROUP_TS_ON */
};

/*!
 * \brief Current FSM state.
 */
typedef enum WheelFsmState (*input_router_get_state_callback)(void);

/*!
 * \brief A request action fired.
 *
 * \param request What is asked.
 */
typedef void (*input_router_request_callback)(enum InputRequest request);

/*!
 * \brief Hooks of the router.
 */
struct InputRouterConfig {
    input_router_get_state_callback get_state; /*!< Required: the state whose bindings apply */
    input_router_request_callback on_request;  /*!< Optional: requests are dropped when NULL */
};

/*!
 * \brief File-static state of the input router.
 */
struct InputRouterHandler {
    struct InputRouterConfig config; /*!< Copy of the configuration given at init */
    struct InputRouterFilter filter; /*!< Action groups currently allowed */
};

#endif // INPUT_ROUTER_H
