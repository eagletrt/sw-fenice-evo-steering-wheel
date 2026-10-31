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
 * \brief Set up the hold-time watchdogs, no button held.
 *
 * \details Takes INPUT_EVENTS_WATCHDOG_COUNT watchdogs from the pool.
 *
 * \param watchdogs The libtimebase pool running the hold times (its routine
 *     fires long presses and combos, from main context).
 * \param sink Receiver of every event.
 *
 * \retval INPUT_EVENTS_RC_OK on success.
 * \retval INPUT_EVENTS_RC_NULL_POINTER if \p watchdogs or \p sink is NULL.
 * \retval INPUT_EVENTS_RC_ERROR if libtimebase rejected a watchdog.
 */
enum InputEventsReturnCode input_events_api_init(struct WatchdogHandler *watchdogs, input_events_sink_callback sink);

/*!
 * \brief Buttons and hold time of a combo.
 *
 * \param combo_id The combo.
 *
 * \return The combo, NULL if \p combo_id is invalid.
 */
const struct InputEventsCombo *input_events_api_get_combo(enum InputEventsComboId combo_id);

/*!
 * \brief Whether a button is currently held.
 *
 * \param button_id The button.
 *
 * \return true if held, false if released or \p button_id is invalid.
 */
bool input_events_api_is_held(enum InputsSharedButtonID button_id);

/*!
 * \brief Inputs callback: a button went down.
 *
 * \details Forwards the press, then starts its long-press hold time and the
 *     hold time of every combo the press completes.
 *
 * \param button_id The button.
 *
 * \retval INPUTS_RC_OK on success.
 * \retval INPUTS_RC_ERROR if \p button_id is invalid, the sink failed or a watchdog could not start.
 */
enum InputsReturnCode input_events_api_on_button_press(enum InputsSharedButtonID button_id);

/*!
 * \brief Inputs callback: a button went up.
 *
 * \details Cancels (or re-arms, if it already fired) its long press and
 *     every combo the button belongs to, then forwards the release.
 *
 * \param button_id The button.
 *
 * \retval INPUTS_RC_OK on success.
 * \retval INPUTS_RC_ERROR if \p button_id is invalid, the sink failed or a watchdog could not be reset.
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
