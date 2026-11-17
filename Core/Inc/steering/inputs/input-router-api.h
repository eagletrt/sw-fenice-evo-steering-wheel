/*!
 * \file input-router-api.h
 * \date 2026-10-31
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief Public API of the input router.
 */

#ifndef INPUT_ROUTER_API_H
#define INPUT_ROUTER_API_H

#include "input-router.h"

/*!
 * \brief Store the hooks and allow every action group.
 *
 * \param config Hooks, copied into the router.
 *
 * \retval INPUT_ROUTER_RC_OK on success.
 * \retval INPUT_ROUTER_RC_NULL_POINTER if \p config or its get_state is NULL.
 */
enum InputRouterReturnCode input_router_api_init(const struct InputRouterConfig *config);

/*!
 * \brief Choose which action groups are allowed.
 *
 * \details Takes effect from the next event. Releases are never filtered.
 *
 * \param filter The new filter.
 */
void input_router_api_set_filter(struct InputRouterFilter filter);

/*!
 * \brief Action groups currently allowed.
 *
 * \return The filter.
 */
struct InputRouterFilter input_router_api_get_filter(void);

/*!
 * \brief Bindings of a state.
 *
 * \param state The FSM state.
 *
 * \return The bindings, NULL if \p state is invalid.
 */
const struct InputStateBindings *input_router_api_get_bindings(enum WheelFsmState state);

/*!
 * \brief Input events callback: combos the current state binds (the only ones recognised).
 *
 * \return Mask of INPUT_EVENTS_COMBO_BIT, 0 before init.
 */
uint8_t input_router_api_active_combos(void);

/*!
 * \brief Input events sink: run the actions the current state binds to an event.
 *
 * \param event The event.
 *
 * \retval INPUT_EVENTS_RC_OK if the event was handled or has no binding.
 * \retval INPUT_EVENTS_RC_NULL_POINTER if \p event is NULL or the router is not initialised.
 * \retval INPUT_EVENTS_RC_ERROR if a parameter update failed.
 */
enum InputEventsReturnCode input_router_api_handle_event(const struct InputEvent *event);

#endif // INPUT_ROUTER_API_H
