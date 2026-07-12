/*!
 * \file inputs-shared.h
 * \date 2025-12-21
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief Shared vocabulary for the steering wheel inputs and parameters.
 *
 * \details This header carries the symbols shared by the input-facing and
 *     UI-facing modules:
 *     - the taxonomy of physical inputs (knob and button IDs), consumed by
 *       the inputs module;
 *     - the full list of parameters managed by the parameters module. Some
 *       are forwarded to the UI popup, others (like PTT) only drive
 *       hardware side effects. parameters_api_is_shared is the source of
 *       truth on which is which.
 *
 *     Raw button/knob events never reach the UI: they are mapped to
 *     parameter changes by the parameters module and only the resulting
 *     {parameter_id, value} pair is forwarded to the popup for parameters
 *     that opt into it.
 */

#ifndef INPUTS_SHARED_H
#define INPUTS_SHARED_H

#include <stdint.h>

/*!
 * \brief Enumeration of knob identifiers
 */
enum InputsSharedKnobID {
    INPUTS_SHARED_KNOB_ID_FRONT_LEFT,
    INPUTS_SHARED_KNOB_ID_FRONT_RIGHT,
    INPUTS_SHARED_KNOB_ID_SIDE_LEFT,
    INPUTS_SHARED_KNOB_ID_SIDE_RIGHT,
    INPUTS_SHARED_KNOB_ID_COUNT,
};

/*!
 * \brief Enumeration of button identifiers
 */
enum InputsSharedButtonID {
    INPUTS_SHARED_BUTTON_ID_TS_ON,
    INPUTS_SHARED_BUTTON_ID_TOP_LEFT_1,
    INPUTS_SHARED_BUTTON_ID_TOP_LEFT_2,
    INPUTS_SHARED_BUTTON_ID_BOTTOM_LEFT,
    INPUTS_SHARED_BUTTON_ID_TOP_RIGHT_1,
    INPUTS_SHARED_BUTTON_ID_TOP_RIGHT_2,
    INPUTS_SHARED_BUTTON_ID_BOTTOM_RIGHT,
    INPUTS_SHARED_BUTTON_ID_PADDLE_TOP_LEFT,
    INPUTS_SHARED_BUTTON_ID_PADDLE_BOTTOM_LEFT,
    INPUTS_SHARED_BUTTON_ID_PADDLE_TOP_RIGHT,
    INPUTS_SHARED_BUTTON_ID_PADDLE_BOTTOM_RIGHT,
    INPUTS_SHARED_BUTTON_ID_KNOB_PUSH_FRONT_LEFT,
    INPUTS_SHARED_BUTTON_ID_KNOB_PUSH_FRONT_RIGHT,
    INPUTS_SHARED_BUTTON_ID_KNOB_PUSH_SIDE_LEFT,
    INPUTS_SHARED_BUTTON_ID_KNOB_PUSH_SIDE_RIGHT,
    INPUTS_SHARED_BUTTON_ID_COUNT,
};

/*!
 * \brief Enumeration of user-facing parameters driven by inputs.
 *
 * \details Parameters with a numeric range use a uint8_t value from 0 to
 *     INPUTS_SHARED_PARAMETER_NUMERIC_MAX. Toggle parameters use 0 for OFF
 *     and 1 for ON.
 */
enum InputsSharedParameterID {
    INPUTS_SHARED_PARAMETER_ID_POWER,            /*!< Power level (0..10) */
    INPUTS_SHARED_PARAMETER_ID_REGEN,            /*!< Regenerative braking level (0..10) */
    INPUTS_SHARED_PARAMETER_ID_TORQUE_VECTORING, /*!< Torque vectoring level (0..10) */
    INPUTS_SHARED_PARAMETER_ID_TELEMETRY_LOG,    /*!< Telemetry log toggle */
    INPUTS_SHARED_PARAMETER_ID_LAUNCH_CONTROL,   /*!< Launch control toggle */
    INPUTS_SHARED_PARAMETER_ID_PTT,              /*!< Push-To-Talk toggle */
    INPUTS_SHARED_PARAMETER_ID_COUNT,
};

/*!
 * \brief Inclusive upper bound for numeric parameter values.
 */
#define INPUTS_SHARED_PARAMETER_NUMERIC_MAX (10U)

#endif // INPUTS_SHARED_H
