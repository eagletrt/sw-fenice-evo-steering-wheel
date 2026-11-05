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
    [INPUTS_SHARED_PARAMETER_ID_DRIVER_OK] = { .is_toggle = true, .is_shared = false },
    [INPUTS_SHARED_PARAMETER_ID_DRIVER_REPEAT] = { .is_toggle = true, .is_shared = false },
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

enum ParametersReturnCode parameters_api_toggle(enum InputsSharedParameterID parameter_id) {
    if (parameter_id >= INPUTS_SHARED_PARAMETER_ID_COUNT) {
        return PARAMETERS_RC_ERROR;
    }
    return prv_parameters_api_apply_value(parameter_id, (parameters_handler.values[parameter_id] != 0U) ? 0U : 1U);
}

enum ParametersReturnCode parameters_api_step(enum InputsSharedParameterID parameter_id, int8_t delta) {
    if (parameter_id >= INPUTS_SHARED_PARAMETER_ID_COUNT) {
        return PARAMETERS_RC_ERROR;
    }
    const int16_t candidate = (int16_t)((int16_t)parameters_handler.values[parameter_id] + delta);
    return prv_parameters_api_apply_value(parameter_id, prv_parameters_api_clamp_to_allowed(parameter_id, candidate));
}

enum ParametersReturnCode parameters_api_hold(enum InputsSharedParameterID parameter_id, uint8_t source, bool held) {
    if (parameter_id >= INPUTS_SHARED_PARAMETER_ID_COUNT || source >= PARAMETERS_HOLD_SOURCE_COUNT) {
        return PARAMETERS_RC_ERROR;
    }
    const uint32_t bit = 1UL << source;
    if (held) {
        parameters_handler.holders[parameter_id] |= bit;
    } else {
        parameters_handler.holders[parameter_id] &= ~bit;
    }
    return prv_parameters_api_apply_value(parameter_id, (parameters_handler.holders[parameter_id] != 0U) ? 1U : 0U);
}

enum ParametersReturnCode parameters_api_release_source(uint8_t source) {
    if (source >= PARAMETERS_HOLD_SOURCE_COUNT) {
        return PARAMETERS_RC_ERROR;
    }
    enum ParametersReturnCode return_code = PARAMETERS_RC_OK;
    for (enum InputsSharedParameterID i = 0; i < INPUTS_SHARED_PARAMETER_ID_COUNT; i++) {
        if ((parameters_handler.holders[i] & (1UL << source)) != 0U &&
            parameters_api_hold(i, source, false) != PARAMETERS_RC_OK) {
            return_code = PARAMETERS_RC_ERROR;
        }
    }
    return return_code;
}
