/*!
 * \file parameters.h
 * \date 2026-04-24
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief Hardware-agnostic definitions for the steering wheel tunable parameters.
 */

#ifndef PARAMETERS_H
#define PARAMETERS_H

#include "inputs-shared.h"
#include <stdbool.h>
#include <stdint.h>

/*!
 * \brief Return codes for parameters operations.
 */
enum ParametersReturnCode {
    PARAMETERS_RC_OK,    /*!< Operation completed successfully */
    PARAMETERS_RC_ERROR, /*!< Invalid input or the on-change handler reported failure */
};

/*!
 * \brief Callback fired whenever a parameter's value actually changes.
 *
 * \details Registered through parameters_api_init and invoked by both the
 *     input-driven handlers and the external setter. The caller uses it
 *     to broadcast the change to the rest of the system (typically the
 *     UI popup, and in the future a CAN frame).
 *
 * \param parameter_id Parameter whose value just changed.
 * \param value New clamped value of the parameter.
 *
 * \retval true if the change was dispatched successfully.
 * \retval false on failure; the parameter value is still updated locally.
 */
typedef bool (*parameters_on_change_callback)(enum InputsSharedParameterID parameter_id, uint8_t value);

/*!
 * \brief Number of distinct inputs that can hold a parameter (bits of ParametersHandler::holders).
 */
#define PARAMETERS_HOLD_SOURCE_COUNT (32U)

/*!
 * \brief Parameters handler structure.
 */
struct ParametersHandler {
    parameters_on_change_callback on_change;            /*!< Called on every value transition */
    uint8_t values[INPUTS_SHARED_PARAMETER_ID_COUNT];   /*!< Current value of each parameter */
    uint32_t holders[INPUTS_SHARED_PARAMETER_ID_COUNT]; /*!< Inputs currently holding each parameter at 1, one bit per source */
};

#endif // PARAMETERS_H
