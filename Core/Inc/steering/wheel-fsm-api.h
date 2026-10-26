/*!
 * \file wheel-fsm-api.h
 * \date 2026-10-26
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief Public API of the steering wheel state machine.
 */

#ifndef WHEEL_FSM_API_H
#define WHEEL_FSM_API_H

#include "can-primary.h"
#include "wheel-fsm.h"

/*!
 * \brief Validate the state table and start in INIT.
 *
 * \details Nothing runs yet: the POST happens on the first
 *     wheel_fsm_api_run_state call.
 *
 * \param config Hooks and init data, copied into the FSM.
 *
 * \retval WHEEL_FSM_RC_OK on success.
 * \retval WHEEL_FSM_RC_NULL_POINTER if \p config or a required field is NULL.
 * \retval WHEEL_FSM_RC_ERROR if libfsm rejected the state table.
 */
enum WheelFsmReturnCode wheel_fsm_api_init(const struct WheelFsmConfig *config);

/*!
 * \brief Run the current state, then apply the pending transition, if any.
 *
 * \retval WHEEL_FSM_RC_OK on success.
 * \retval WHEEL_FSM_RC_ERROR if libfsm failed (e.g. called before init).
 */
enum WheelFsmReturnCode wheel_fsm_api_run_state(void);

/*!
 * \brief Request a transition, applied at the end of the next FSM step.
 *
 * \details Main context only (tasks), never from an interrupt. The last valid
 *     request wins, except ERROR: once requested, nothing can replace it.
 *     Requesting the state already pending does nothing.
 *
 * \param state The state to move to.
 *
 * \retval WHEEL_FSM_RC_OK if the transition is pending.
 * \retval WHEEL_FSM_RC_INVALID_TRANSITION if \p state can't be reached from
 *     the current state, or ERROR is already pending.
 */
enum WheelFsmReturnCode wheel_fsm_api_request(enum WheelFsmState state);

/*!
 * \brief Current state of the wheel.
 *
 * \return The current state.
 */
enum WheelFsmState wheel_fsm_api_get_state(void);

/*!
 * \brief Human-readable name of a state.
 *
 * \param state The state.
 *
 * \return A static string, "?" for an invalid state.
 */
const char *wheel_fsm_api_get_state_name(enum WheelFsmState state);

/*!
 * \brief Map a wheel state onto the status signal of SteeringWheelFsm.
 *
 * \details libcan has no value for most wheel modes yet: everything that is
 *     not INIT, FLASH or ERROR is reported as IDLE.
 *
 * \param state The wheel state.
 *
 * \return The matching CanPrimarySteeringwheelfsmStatus value.
 */
enum CanPrimarySteeringwheelfsmStatus wheel_fsm_api_get_can_status(enum WheelFsmState state);

#endif // WHEEL_FSM_API_H
