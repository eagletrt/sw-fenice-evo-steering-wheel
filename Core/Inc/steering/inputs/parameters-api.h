/*!
 * \file parameters-api.h
 * \date 2026-04-24
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief Hardware-agnostic API to manage the steering wheel tunable parameters.
 *
 * \details This module is the single source of truth for the user-facing
 *     parameters (power, regen, torque vectoring, telemetry log, launch
 *     control, PTT, TS-on, driver actions). It knows nothing about buttons
 *     or knobs: the input router maps them to the actions below (set,
 *     toggle, step, hold), per FSM state. Every transition triggers the
 *     on-change callback registered at init time, which is the single hook
 *     the integrator uses to broadcast the change to the rest of the system
 *     (UI popup, CAN TX, ...).
 */

#ifndef PARAMETERS_API_H
#define PARAMETERS_API_H

#include "parameters.h"

/*!
 * \brief Initialize the parameters handler.
 *
 * \details All parameter values are reset to 0. The on-change callback is
 *     invoked on every transition (both input-driven and external).
 *
 * \param on_change Callback fired on every value transition.
 *
 * \retval PARAMETERS_RC_OK if initialization was successful.
 * \retval PARAMETERS_RC_ERROR if \p on_change is NULL.
 */
enum ParametersReturnCode parameters_api_init(parameters_on_change_callback on_change);

/*!
 * \brief Read the current value of a parameter.
 *
 * \param parameter_id The parameter to read.
 *
 * \return Current value of the parameter, 0 if \p parameter_id is invalid.
 */
uint8_t parameters_api_get(enum InputsSharedParameterID parameter_id);

/*!
 * \brief Set a parameter to an explicit value.
 *
 * \details The value is clamped to the parameter's valid range (0..1 for
 *     toggle parameters, 0..INPUTS_SHARED_PARAMETER_NUMERIC_MAX for numeric
 *     ones). The on-change callback is fired only when the clamped value
 *     differs from the current one.
 *
 * \param parameter_id The parameter to update.
 * \param value The desired value (will be clamped).
 *
 * \retval PARAMETERS_RC_OK if the set was applied (whether or not the value
 *     actually changed).
 * \retval PARAMETERS_RC_ERROR if \p parameter_id is invalid or the on-change
 *     callback reported failure.
 */
enum ParametersReturnCode parameters_api_set(enum InputsSharedParameterID parameter_id, uint8_t value);

/*!
 * \brief Whether a parameter's transitions should reach the UI.
 *
 * \details Shared parameters are forwarded to the popup by the on-change
 *     handler wired in main; non-shared parameters (PTT) drive only
 *     hardware side effects and the CAN broadcast.
 *
 * \param parameter_id The parameter to query.
 *
 * \return true if the parameter is forwarded to the UI, false otherwise
 *     (also false for invalid IDs).
 */
bool parameters_api_is_shared(enum InputsSharedParameterID parameter_id);

/*!
 * \brief Flip a parameter between 0 and 1 (numeric ones: 0 <-> 1 too).
 *
 * \param parameter_id The parameter.
 *
 * \retval PARAMETERS_RC_OK on success.
 * \retval PARAMETERS_RC_ERROR if \p parameter_id is invalid or the on-change callback failed.
 */
enum ParametersReturnCode parameters_api_toggle(enum InputsSharedParameterID parameter_id);

/*!
 * \brief Move a parameter by \p delta steps, clamped to its range.
 *
 * \param parameter_id The parameter.
 * \param delta Signed number of steps (e.g. a knob rotation).
 *
 * \retval PARAMETERS_RC_OK on success (also when clamping leaves it unchanged).
 * \retval PARAMETERS_RC_ERROR if \p parameter_id is invalid or the on-change callback failed.
 */
enum ParametersReturnCode parameters_api_step(enum InputsSharedParameterID parameter_id, int8_t delta);

/*!
 * \brief Hold a parameter at 1, or let go of it, on behalf of one input.
 *
 * \details The parameter is 1 while at least one source holds it (PTT is
 *     held by either top paddle) and back to 0 once every source has let
 *     go.
 *
 * \param parameter_id The parameter.
 * \param source Input holding it, below PARAMETERS_HOLD_SOURCE_COUNT.
 * \param held true to hold, false to let go.
 *
 * \retval PARAMETERS_RC_OK on success.
 * \retval PARAMETERS_RC_ERROR if an argument is invalid or the on-change callback failed.
 */
enum ParametersReturnCode parameters_api_hold(enum InputsSharedParameterID parameter_id, uint8_t source, bool held);

/*!
 * \brief Let go of every parameter a source holds.
 *
 * \details Called on every release of an input, whatever the FSM state:
 *     a hold must never outlive the press that made it.
 *
 * \param source Input released, below PARAMETERS_HOLD_SOURCE_COUNT.
 *
 * \retval PARAMETERS_RC_OK on success (also if it held nothing).
 * \retval PARAMETERS_RC_ERROR if \p source is invalid or an on-change callback failed.
 */
enum ParametersReturnCode parameters_api_release_source(uint8_t source);

#endif // PARAMETERS_API_H
