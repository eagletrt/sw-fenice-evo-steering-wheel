/*!
 * \file inputs-api.h
 * \date 2025-12-24
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief Hardware-agnostic input handling function definitions for the steering wheel.
 */

#ifndef INPUTS_API_H
#define INPUTS_API_H

#include "inputs.h"

/*!
 * \brief Initialize the input handler.
 *
 * \details Any callback may be NULL: in that case the corresponding event
 *     type is silently dropped without being reported as an error.
 *
 * \param on_button_press Callback fired on the rising edge of a button (may be NULL).
 * \param on_button_long_press Callback fired once the long-press threshold is reached (may be NULL).
 * \param on_button_release Callback fired on the falling edge of a button (may be NULL).
 * \param on_knob_rotation Callback fired on every non-zero encoder delta (may be NULL).
 *
 * \retval INPUTS_RC_OK on success.
 */
enum InputsReturnCode inputs_api_init(inputs_button_event_callback on_button_press, inputs_button_event_callback on_button_long_press, inputs_button_event_callback on_button_release, inputs_knob_rotation_callback on_knob_rotation);

/*!
 * \brief Feed a new raw button state into the handler.
 *
 * \param button_id Identifier of the button that changed state.
 * \param pressed True if the button is now pressed, false if released.
 * \param current_tick_ms Current tick count in milliseconds (used for long press tracking).
 *
 * \retval INPUTS_RC_OK if the button state was updated successfully.
 * \retval INPUTS_RC_ERROR if the button ID is out of range or a callback reported failure.
 */
enum InputsReturnCode inputs_api_update_button(enum InputsSharedButtonID button_id, bool pressed, uint32_t current_tick_ms);

/*!
 * \brief Feed a new raw encoder position into the handler.
 *
 * \param knob_id Identifier of the knob that changed state.
 * \param current_position Current position of the encoder.
 *
 * \retval INPUTS_RC_OK if the knob state was updated successfully.
 * \retval INPUTS_RC_ERROR if the knob ID is out of range or a callback reported failure.
 */
enum InputsReturnCode inputs_api_update_knob(enum InputsSharedKnobID knob_id, int16_t current_position);

/*!
 * \brief Periodic tick used to emit long-press events.
 *
 * \param current_tick_ms Current tick count in milliseconds.
 *
 * \retval INPUTS_RC_OK if the update was successful.
 * \retval INPUTS_RC_ERROR if a callback reported failure.
 */
enum InputsReturnCode inputs_api_poll_for_long_press(uint32_t current_tick_ms);

#endif // INPUTS_API_H
