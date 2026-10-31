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
 * \brief Store the hooks and allow every driver control.
 *
 * \param config Hooks, copied into the router.
 *
 * \retval INPUT_ROUTER_RC_OK on success.
 * \retval INPUT_ROUTER_RC_NULL_POINTER if \p config or its get_state is NULL.
 */
enum InputRouterReturnCode input_router_api_init(const struct InputRouterConfig *config);

/*!
 * \brief Choose which groups of driver controls are allowed.
 *
 * \details Takes effect from the next event. Releases are never filtered.
 *
 * \param filter The new filter.
 */
void input_router_api_set_filter(struct InputRouterFilter filter);

/*!
 * \brief Groups of driver controls currently allowed.
 *
 * \return The filter.
 */
struct InputRouterFilter input_router_api_get_filter(void);

/*!
 * \brief Input events sink: deliver an event to whoever handles it in the current state.
 *
 * \param event The event.
 *
 * \retval INPUT_EVENTS_RC_OK if the event was delivered or deliberately dropped.
 * \retval INPUT_EVENTS_RC_NULL_POINTER if \p event is NULL or the router is not initialised.
 * \retval INPUT_EVENTS_RC_ERROR if the receiver (the parameters) failed.
 */
enum InputEventsReturnCode input_router_api_handle_event(const struct InputEvent *event);

#endif // INPUT_ROUTER_API_H
