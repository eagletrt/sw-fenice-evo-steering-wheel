/*!
 * \file inputs.h
 * \date 2025-12-24
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief Hardware-agnostic input handling definitions for the steering wheel.
 *
 * \details The inputs module turns sampled levels into edges (press,
 *     release) and encoder positions into deltas. It reports what the user
 *     did to the physical controls through a set of callbacks; timing
 *     (long presses, combos) and mapping those events to application-level
 *     effects (such as parameter updates) is the responsibility of the
 *     caller (the input events layer).
 */

#ifndef INPUTS_H
#define INPUTS_H

#include "inputs-shared.h"
#include <stdint.h>
#include <stdbool.h>

/*!
 * \brief Return codes for inputs operations.
 */
enum InputsReturnCode {
    INPUTS_RC_OK,    /*!< Operation successful */
    INPUTS_RC_ERROR, /*!< General error (e.g. a callback reported failure) */
};

/*!
 * \brief Button tracking structure
 */
struct InputsButtonHandler {
    bool pressed; /*!< Last reported level */
    bool enabled; /*!< Whether this button is enabled for tracking */
};

/*!
 * \brief Knob (encoder) tracking structure
 */
struct InputsKnobHandler {
    int16_t last_position; /*!< Last known position of the encoder */
    bool enabled;          /*!< Whether this encoder is enabled for tracking */
};

/*!
 * \brief Callback type for button press/release events.
 *
 * \param button_id Identifier of the button that produced the event.
 *
 * \retval INPUTS_RC_OK if the event was handled successfully.
 * \retval INPUTS_RC_ERROR if there was an error handling the event.
 */
typedef enum InputsReturnCode (*inputs_button_event_callback)(
    enum InputsSharedButtonID button_id);

/*!
 * \brief Callback type for knob rotation events.
 *
 * \param knob_id Identifier of the knob that moved.
 * \param delta Signed rotation delta since the last report.
 *
 * \retval INPUTS_RC_OK if the event was handled successfully.
 * \retval INPUTS_RC_ERROR if there was an error handling the event.
 */
typedef enum InputsReturnCode (*inputs_knob_rotation_callback)(
    enum InputsSharedKnobID knob_id,
    int8_t delta);

/*!
 * \brief Main input handler structure.
 *
 * \details Each callback may be NULL: in that case the corresponding event
 *     type is silently dropped.
 */
struct InputsHandler {
    inputs_button_event_callback on_button_press;   /*!< Fired on the rising edge of a button */
    inputs_button_event_callback on_button_release; /*!< Fired on the falling edge of a button */
    inputs_knob_rotation_callback on_knob_rotation; /*!< Fired on every non-zero encoder delta */

    struct InputsButtonHandler buttons[INPUTS_SHARED_BUTTON_ID_COUNT]; /*!< Tracking state for each button */
    struct InputsKnobHandler knobs[INPUTS_SHARED_KNOB_ID_COUNT];       /*!< Tracking state for each encoder */
};

#endif // INPUTS_H
