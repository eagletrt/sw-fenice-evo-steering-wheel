/*!
 * \file parameters-api.c
 * \date 2026-04-24
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief Hardware-agnostic API to manage the steering wheel tunable parameters.
 */

#include <string.h>

#include "parameters-api.h"
#include "eagletrt-api.h"

EAGLETRT_STATIC struct ParametersHandler parameters_handler;

/*!
 * \brief Static descriptor for each parameter.
 */
struct ParameterMeta {
    bool is_toggle; /*!< true for ON/OFF parameters, false for 0..NUMERIC_MAX */
    bool is_shared; /*!< true if the parameter is forwarded to the UI */
};

/*!
 * \brief Metadata for each parameter, indexed by enum ID.
 */
EAGLETRT_STATIC const struct ParameterMeta parameters_api_meta[INPUTS_SHARED_PARAMETER_ID_COUNT] = {
    [INPUTS_SHARED_PARAMETER_ID_POWER] = { .is_toggle = false, .is_shared = true },
    [INPUTS_SHARED_PARAMETER_ID_REGEN] = { .is_toggle = false, .is_shared = true },
    [INPUTS_SHARED_PARAMETER_ID_TORQUE_VECTORING] = { .is_toggle = false, .is_shared = true },
    [INPUTS_SHARED_PARAMETER_ID_TELEMETRY_LOG] = { .is_toggle = true, .is_shared = true },
    [INPUTS_SHARED_PARAMETER_ID_LAUNCH_CONTROL] = { .is_toggle = true, .is_shared = true },
    [INPUTS_SHARED_PARAMETER_ID_PTT] = { .is_toggle = true, .is_shared = false },
    [INPUTS_SHARED_PARAMETER_ID_TS_ON] = { .is_toggle = true, .is_shared = false },
};

/*!
 * \brief Clamp a signed value value to the parameter's valid range.
 *
 * \param parameter_id The parameter being set, used to determine the valid range.
 * \param value The candidate value, which may be out of range.
 *
 * \return The clamped value, guaranteed to be within the parameter's valid range.
 */
EAGLETRT_STATIC uint8_t prv_parameters_api_clamp_to_allowed(const enum InputsSharedParameterID parameter_id, int16_t value) {
    int8_t max = parameters_api_meta[parameter_id].is_toggle ? 1 : INPUTS_SHARED_PARAMETER_NUMERIC_MAX;
    return (uint8_t)EAGLETRT_API_CLAMP(value, 0, max);
}

/*!
 * \brief Apply an already-clamped value and notify the caller if it changed.
 *
 * \param parameter_id The parameter being set, used to determine the old value and what effect to apply.
 * \param new_value The new value to apply, which must already be clamped to the parameter's valid range.
 *
 * \retval PARAMETERS_RC_OK if the value is unchanged or the on-change callback succeeded.
 * \retval PARAMETERS_RC_ERROR if the on-change callback returned false.
 */
EAGLETRT_STATIC enum ParametersReturnCode prv_parameters_api_apply_value(const enum InputsSharedParameterID parameter_id, uint8_t new_value) {
    if (parameters_handler.values[parameter_id] == new_value) {
        return PARAMETERS_RC_OK;
    }
    parameters_handler.values[parameter_id] = new_value;
    if (!parameters_handler.on_change(parameter_id, new_value)) {
        return PARAMETERS_RC_ERROR;
    }
    return PARAMETERS_RC_OK;
}

/*!
 * \brief Recompute the PTT toggle from the current paddle hold flags.
 *
 * \details PTT is active while either top paddle is held; it only goes
 *     back to 0 once both are released.
 *
 * \retval PARAMETERS_RC_OK if the value is unchanged or the on-change callback succeeded.
 * \retval PARAMETERS_RC_ERROR if the on-change callback returned false.
 */
EAGLETRT_STATIC enum ParametersReturnCode prv_recompute_ptt(void) {
    uint8_t desired = (parameters_handler.ptt_top_left_held || parameters_handler.ptt_top_right_held) ? 1U : 0U;
    return prv_parameters_api_apply_value(INPUTS_SHARED_PARAMETER_ID_PTT, desired);
}

enum ParametersReturnCode parameters_api_init(parameters_on_change_callback on_change) {
    if (on_change == NULL) {
        return PARAMETERS_RC_ERROR;
    }

    memset(&parameters_handler, 0, sizeof(parameters_handler));
    parameters_handler.on_change = on_change;

    return PARAMETERS_RC_OK;
}

bool parameters_api_is_shared(enum InputsSharedParameterID parameter_id) {
    if (parameter_id >= INPUTS_SHARED_PARAMETER_ID_COUNT) {
        return false;
    }
    return parameters_api_meta[parameter_id].is_shared;
}

uint8_t parameters_api_get(enum InputsSharedParameterID parameter_id) {
    if (parameter_id >= INPUTS_SHARED_PARAMETER_ID_COUNT) {
        return 0U;
    }
    return parameters_handler.values[parameter_id];
}

enum ParametersReturnCode parameters_api_set(const enum InputsSharedParameterID parameter_id, uint8_t value) {
    if (parameter_id >= INPUTS_SHARED_PARAMETER_ID_COUNT) {
        return PARAMETERS_RC_ERROR;
    }
    return prv_parameters_api_apply_value(parameter_id, prv_parameters_api_clamp_to_allowed(parameter_id, (int16_t)value));
}

enum InputsReturnCode parameters_api_handle_button(enum InputsSharedButtonID button_id) {
    switch (button_id) {
        case INPUTS_SHARED_BUTTON_ID_BOTTOM_LEFT:
        case INPUTS_SHARED_BUTTON_ID_BOTTOM_RIGHT: {
            enum InputsSharedParameterID param =
                (button_id == INPUTS_SHARED_BUTTON_ID_BOTTOM_LEFT)
                    ? INPUTS_SHARED_PARAMETER_ID_TELEMETRY_LOG
                    : INPUTS_SHARED_PARAMETER_ID_LAUNCH_CONTROL;

            uint8_t next = parameters_handler.values[param] ? 0U : 1U;

            return prv_parameters_api_apply_value(param, next) == PARAMETERS_RC_OK
                       ? INPUTS_RC_OK
                       : INPUTS_RC_ERROR;
        }
        case INPUTS_SHARED_BUTTON_ID_PADDLE_TOP_LEFT:
        case INPUTS_SHARED_BUTTON_ID_PADDLE_TOP_RIGHT: {
            bool *held =
                (button_id == INPUTS_SHARED_BUTTON_ID_PADDLE_TOP_LEFT)
                    ? &parameters_handler.ptt_top_left_held
                    : &parameters_handler.ptt_top_right_held;

            *held = true;

            return prv_recompute_ptt() == PARAMETERS_RC_OK
                       ? INPUTS_RC_OK
                       : INPUTS_RC_ERROR;
        }
        case INPUTS_SHARED_BUTTON_ID_TS_ON: {
            // Momentary: the ECU wants the request asserted for as long as
            // the driver holds the button, so press/release drive it
            // directly instead of toggling.
            return prv_parameters_api_apply_value(INPUTS_SHARED_PARAMETER_ID_TS_ON, 1U) == PARAMETERS_RC_OK
                       ? INPUTS_RC_OK
                       : INPUTS_RC_ERROR;
        }
        default:
            return INPUTS_RC_OK;
    }
}

enum InputsReturnCode parameters_api_handle_button_release(enum InputsSharedButtonID button_id) {
    switch (button_id) {
        case INPUTS_SHARED_BUTTON_ID_PADDLE_TOP_LEFT:
        case INPUTS_SHARED_BUTTON_ID_PADDLE_TOP_RIGHT: {
            bool *released =
                (button_id == INPUTS_SHARED_BUTTON_ID_PADDLE_TOP_LEFT)
                    ? &parameters_handler.ptt_top_left_held
                    : &parameters_handler.ptt_top_right_held;

            *released = false;

            return prv_recompute_ptt() == PARAMETERS_RC_OK
                       ? INPUTS_RC_OK
                       : INPUTS_RC_ERROR;
        }
        case INPUTS_SHARED_BUTTON_ID_TS_ON: {
            return prv_parameters_api_apply_value(INPUTS_SHARED_PARAMETER_ID_TS_ON, 0U) == PARAMETERS_RC_OK
                       ? INPUTS_RC_OK
                       : INPUTS_RC_ERROR;
        }
        default:
            return INPUTS_RC_OK;
    }
}

enum InputsReturnCode parameters_api_handle_knob(const enum InputsSharedKnobID knob_id, int8_t delta) {
    enum InputsSharedParameterID parameter_id;
    switch (knob_id) {
        case INPUTS_SHARED_KNOB_ID_FRONT_LEFT:
            parameter_id = INPUTS_SHARED_PARAMETER_ID_POWER;
            break;
        case INPUTS_SHARED_KNOB_ID_FRONT_RIGHT:
            parameter_id = INPUTS_SHARED_PARAMETER_ID_REGEN;
            break;
        case INPUTS_SHARED_KNOB_ID_FRONT_CENTER:
            parameter_id = INPUTS_SHARED_PARAMETER_ID_TORQUE_VECTORING;
            break;
        default:
            return INPUTS_RC_OK;
    }
    int16_t candidate = parameters_handler.values[parameter_id] + delta;
    if (prv_parameters_api_apply_value(parameter_id, prv_parameters_api_clamp_to_allowed(parameter_id, candidate)) != PARAMETERS_RC_OK) {
        return INPUTS_RC_ERROR;
    }
    return INPUTS_RC_OK;
}
