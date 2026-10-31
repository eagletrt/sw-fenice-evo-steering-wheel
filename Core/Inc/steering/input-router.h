/*!
 * \file input-router.h
 * \date 2026-10-31
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief Definitions for the input router: who receives each input event,
 *     depending on the FSM state.
 *
 * \details A const table says, per FSM state, whether the driver controls
 *     (knobs and buttons mapped to parameters) are live and which combos are
 *     recognised. Driver controls also go through a filter with one flag per
 *     group (parameters, PTT, TS-on), set by the driving context: in an
 *     autonomous mission they are ignored, and TS-on can be allowed again on
 *     its own.
 *
 *     Button releases always reach the parameters, in every state and
 *     whatever the filter says: a PTT or TS-on pressed before the state or
 *     the filter changed must still be let go. A release whose press was
 *     dropped changes nothing.
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
 * \brief Which groups of driver controls are allowed.
 *
 * \details All true at init (manual driving). TODO(#11): set from the
 *     driving context, everything false in an autonomous mission (D10).
 */
struct InputRouterFilter {
    bool parameters; /*!< Knobs and parameter buttons (telemetry log, launch control) */
    bool ptt;        /*!< Top paddles (push-to-talk) */
    bool ts_on;      /*!< TS-on button, separate so it can be re-allowed in autonomous */
};

/*!
 * \brief Current FSM state.
 */
typedef enum WheelFsmState (*input_router_get_state_callback)(void);

/*!
 * \brief A combo recognised in the current state.
 *
 * \param combo_id The combo.
 */
typedef void (*input_router_combo_callback)(enum InputEventsComboId combo_id);

/*!
 * \brief Hooks of the router.
 */
struct InputRouterConfig {
    input_router_get_state_callback get_state; /*!< Required: the state the routing depends on */
    input_router_combo_callback on_combo;      /*!< Optional: mode changes (menu, wake), combos are dropped when NULL */
};

/*!
 * \brief File-static state of the input router.
 */
struct InputRouterHandler {
    struct InputRouterConfig config; /*!< Copy of the configuration given at init */
    struct InputRouterFilter filter; /*!< Driver controls currently allowed */
};

#endif // INPUT_ROUTER_H
