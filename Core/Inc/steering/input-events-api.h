/*!
 * \file input-events-api.h
 * \date 2026-10-31
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief Public API of the input event layer.
 *
 * \details The three input handlers have the inputs module's callback
 *     signatures: register them with inputs_api_init.
 */

#ifndef INPUT_EVENTS_API_H
#define INPUT_EVENTS_API_H

#include "input-events.h"
#include "inputs.h"

#include <stdbool.h>

/*!
 * \brief Take the layer's timeouts from the pool, no button held.
 *
 * \details Takes INPUT_EVENTS_TIMEOUT_COUNT timeouts (timeout_api_init first).
 *
 * \param sink Receiver of every event.
 * \param active_combos Combos recognised right now (NULL: none, every button is delivered at once).
 *
 * \retval INPUT_EVENTS_RC_OK on success.
 * \retval INPUT_EVENTS_RC_NULL_POINTER if \p sink is NULL.
 * \retval INPUT_EVENTS_RC_ERROR if the timeout pool is full.
 */
enum InputEventsReturnCode input_events_api_init(input_events_sink_callback sink, input_events_active_combos_callback active_combos);

/*!
 * \brief Buttons of a combo.
 *
 * \param combo_id The combo.
 *
 * \return Mask of INPUT_EVENTS_BUTTON_BIT, 0 if \p combo_id is invalid.
 */
uint16_t input_events_api_get_combo_buttons(enum InputEventsComboId combo_id);

/*!
 * \brief Whether a button is physically down.
 *
 * \param button_id The button.
 *
 * \return true if down (whatever its gesture), false if up or \p button_id is invalid.
 */
bool input_events_api_is_held(enum InputsSharedButtonID button_id);

/*!
 * \brief Inputs callback: a button went down.
 *
 * \param button_id The button.
 *
 * \retval INPUTS_RC_OK on success.
 * \retval INPUTS_RC_ERROR if \p button_id is invalid, the sink failed or a timeout could not start.
 */
enum InputsReturnCode input_events_api_on_button_press(enum InputsSharedButtonID button_id);

/*!
 * \brief Inputs callback: a button went up.
 *
 * \param button_id The button.
 *
 * \retval INPUTS_RC_OK on success.
 * \retval INPUTS_RC_ERROR if \p button_id is invalid, the sink failed or a timeout could not stop.
 */
enum InputsReturnCode input_events_api_on_button_release(enum InputsSharedButtonID button_id);

/*!
 * \brief Inputs callback: a knob moved.
 *
 * \param knob_id The knob.
 * \param delta Signed steps since the last report.
 *
 * \retval INPUTS_RC_OK on success.
 * \retval INPUTS_RC_ERROR if \p knob_id is invalid or the sink failed.
 */
enum InputsReturnCode input_events_api_on_knob_rotation(enum InputsSharedKnobID knob_id, int8_t delta);

#endif // INPUT_EVENTS_API_H
