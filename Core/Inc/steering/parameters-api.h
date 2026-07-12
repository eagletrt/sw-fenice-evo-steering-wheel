/*!
 * \file parameters-api.h
 * \date 2026-04-24
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief Hardware-agnostic API to manage the steering wheel tunable parameters.
 *
 * \details This module is the single source of truth for the user-facing
 *     parameters (power, regen, torque vectoring, traction/launch control).
 *     It can be driven either from the inputs module (knob/button callbacks
 *     wired to parameters_api_handle_*) or from any other source through
 *     parameters_api_set (for instance a CAN RX handler). Every transition
 *     triggers the on-change callback registered at init time, which is
 *     the single hook the integrator uses to broadcast the change to the
 *     rest of the system (UI popup, CAN TX, ...).
 */

#ifndef PARAMETERS_API_H
#define PARAMETERS_API_H

#include "inputs.h"
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
 * \brief Map a button press to a parameter transition.
 *
 * \details Intended to be registered as the button-press callback of the
 *     inputs module. Buttons that do not map to any parameter are silently
 *     ignored. Either top paddle press activates PTT.
 *
 * \param button_id The button that was pressed.
 *
 * \retval INPUTS_RC_OK on success (including "no mapping").
 * \retval INPUTS_RC_ERROR if the on-change callback reported failure.
 */
enum InputsReturnCode parameters_api_handle_button(enum InputsSharedButtonID button_id);

/*!
 * \brief Map a button release to a parameter transition.
 *
 * \details Intended to be registered as the button-release callback of the
 *     inputs module. Only paddle releases are meaningful today: PTT goes
 *     inactive once both top paddles are released. Every other button
 *     release is silently ignored.
 *
 * \param button_id The button that was released.
 *
 * \retval INPUTS_RC_OK on success (including "no mapping").
 * \retval INPUTS_RC_ERROR if the on-change callback reported failure.
 */
enum InputsReturnCode parameters_api_handle_button_release(enum InputsSharedButtonID button_id);

/*!
 * \brief Map a knob rotation to a parameter transition.
 *
 * \details Intended to be registered as the knob-rotation callback of the
 *     inputs module. Knobs that do not map to any parameter are silently
 *     ignored.
 *
 * \param knob_id The knob that moved.
 * \param delta Signed rotation delta.
 *
 * \retval INPUTS_RC_OK on success (including "no mapping").
 * \retval INPUTS_RC_ERROR if the on-change callback reported failure.
 */
enum InputsReturnCode parameters_api_handle_knob(enum InputsSharedKnobID knob_id, int8_t delta);

#endif // PARAMETERS_API_H
