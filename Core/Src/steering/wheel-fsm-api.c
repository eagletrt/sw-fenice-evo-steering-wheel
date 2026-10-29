/*!
 * \file wheel-fsm-api.c
 * \date 2026-10-26
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief Implementation of the steering wheel state machine.
 */

#include "wheel-fsm-api.h"

#include "can-communications-router-api.h"
#include "eagletrt-api.h"
#include "eagletrt-fsm-api.h"
#include "flash-monitor-api.h"
#include "post-api.h"

#include <string.h>

EAGLETRT_STATIC struct WheelFsmHandler wheel_fsm_handler;

/*!
 * \brief INIT: run the POST.
 *
 * \note The logic does:
 *    - if the POST is OK, go to IDLE
 *    - if the POST is not OK, go to ERROR
 *
 * \param data The WheelFsmConfig of the handler.
 */
EAGLETRT_STATIC void prv_wheel_fsm_api_do_init(void *data) {
    struct WheelFsmConfig *config = (struct WheelFsmConfig *)data;
    const enum WheelFsmState next = (post_api_do_init(config->post) == POST_RC_OK) ? WHEEL_FSM_STATE_IDLE : WHEEL_FSM_STATE_ERROR;
    EAGLETRT_API_UNUSED(wheel_fsm_api_request(next));
}

/*!
 * \brief IDLE: handles the bootloader reset request, moves to FLASH while
 *     another board is being flashed.
 *
 * \param data The WheelFsmConfig of the handler.
 */
EAGLETRT_STATIC void prv_wheel_fsm_api_do_idle(void *data) {
    const struct WheelFsmConfig *config = (const struct WheelFsmConfig *)data;
    if (can_communications_router_api_reset_asked()) {
        config->reset();
    }
    if (flash_monitor_api_is_active()) {
        EAGLETRT_API_UNUSED(wheel_fsm_api_request(WHEEL_FSM_STATE_FLASH));
    }
}

/*!
 * \brief FLASH: another board is being flashed; back to IDLE once its
 *     traffic stops. A reset into the bootloader is still honoured.
 *
 * \details Following the monitor's state on every step, instead of reacting
 *     to its edges, means a frame arriving between the timeout and the next
 *     step can't leave the wheel in the wrong state.
 *
 * \param data The WheelFsmConfig of the handler.
 */
EAGLETRT_STATIC void prv_wheel_fsm_api_do_flash(void *data) {
    const struct WheelFsmConfig *config = (const struct WheelFsmConfig *)data;
    if (can_communications_router_api_reset_asked()) {
        config->reset();
    }
    if (!flash_monitor_api_is_active()) {
        EAGLETRT_API_UNUSED(wheel_fsm_api_request(WHEEL_FSM_STATE_IDLE));
    }
}

/*!
 * \brief IDLE -> FLASH: silence the wheel.
 *
 * \param data The WheelFsmConfig of the handler.
 */
EAGLETRT_STATIC void prv_wheel_fsm_api_enter_flash(void *data) {
    const struct WheelFsmConfig *config = (const struct WheelFsmConfig *)data;
    config->enter_flash();
}

/*!
 * \brief FLASH -> IDLE: back to normal operation.
 *
 * \param data The WheelFsmConfig of the handler.
 */
EAGLETRT_STATIC void prv_wheel_fsm_api_leave_flash(void *data) {
    const struct WheelFsmConfig *config = (const struct WheelFsmConfig *)data;
    config->leave_flash();
}

/*!
 * \brief ERROR: terminal, the scheduler keeps reporting it on the bus.
 *
 * \param data The WheelFsmConfig of the handler.
 */
EAGLETRT_STATIC void prv_wheel_fsm_api_do_error(void *data) {
    EAGLETRT_API_UNUSED(data);
}

EAGLETRT_STATIC const struct Transition prv_wheel_fsm_api_init_transitions[] = {
    { WHEEL_FSM_STATE_IDLE, NULL },
    { WHEEL_FSM_STATE_ERROR, NULL },
};

EAGLETRT_STATIC const struct Transition prv_wheel_fsm_api_idle_transitions[] = {
    { WHEEL_FSM_STATE_FLASH, prv_wheel_fsm_api_enter_flash },
    { WHEEL_FSM_STATE_ERROR, NULL },
};

EAGLETRT_STATIC const struct Transition prv_wheel_fsm_api_flash_transitions[] = {
    { WHEEL_FSM_STATE_IDLE, prv_wheel_fsm_api_leave_flash },
    { WHEEL_FSM_STATE_ERROR, NULL },
};

/*!
 * \brief State table, indexed by WheelFsmState (libfsm wants ids in order).
 *
 * \details Every state stays where it is until a transition is requested:
 *     next_default is the state itself.
 */
EAGLETRT_STATIC const struct State prv_wheel_fsm_api_states[WHEEL_FSM_STATE_COUNT] = {
    [WHEEL_FSM_STATE_INIT] = {
        WHEEL_FSM_STATE_INIT,
        prv_wheel_fsm_api_do_init,
        WHEEL_FSM_STATE_INIT,
        prv_wheel_fsm_api_init_transitions,
        (uint8_t)EAGLETRT_FSM_TRANSITION_LEN(prv_wheel_fsm_api_init_transitions),
    },
    [WHEEL_FSM_STATE_IDLE] = {
        WHEEL_FSM_STATE_IDLE,
        prv_wheel_fsm_api_do_idle,
        WHEEL_FSM_STATE_IDLE,
        prv_wheel_fsm_api_idle_transitions,
        (uint8_t)EAGLETRT_FSM_TRANSITION_LEN(prv_wheel_fsm_api_idle_transitions),
    },
    [WHEEL_FSM_STATE_FLASH] = {
        WHEEL_FSM_STATE_FLASH,
        prv_wheel_fsm_api_do_flash,
        WHEEL_FSM_STATE_FLASH,
        prv_wheel_fsm_api_flash_transitions,
        (uint8_t)EAGLETRT_FSM_TRANSITION_LEN(prv_wheel_fsm_api_flash_transitions),
    },
    [WHEEL_FSM_STATE_ERROR] = {
        WHEEL_FSM_STATE_ERROR,
        prv_wheel_fsm_api_do_error,
        WHEEL_FSM_STATE_ERROR,
        NULL,
        0U,
    },
};

EAGLETRT_STATIC const char *const prv_wheel_fsm_api_state_names[WHEEL_FSM_STATE_COUNT] = {
    [WHEEL_FSM_STATE_INIT] = "INIT",
    [WHEEL_FSM_STATE_IDLE] = "IDLE",
    [WHEEL_FSM_STATE_FLASH] = "FLASH",
    [WHEEL_FSM_STATE_ERROR] = "ERROR",
};

enum WheelFsmReturnCode wheel_fsm_api_init(const struct WheelFsmConfig *config) {
    if (config == NULL || config->post == NULL || config->reset == NULL || config->enter_flash == NULL || config->leave_flash == NULL) {
        return WHEEL_FSM_RC_NULL_POINTER;
    }

    memset(&wheel_fsm_handler, 0, sizeof(wheel_fsm_handler));
    wheel_fsm_handler.config = *config;

    if (fsm_api_init(&wheel_fsm_handler.fsm, prv_wheel_fsm_api_states, WHEEL_FSM_STATE_COUNT, WHEEL_FSM_STATE_INIT) != FSM_RC_OK) {
        return WHEEL_FSM_RC_ERROR;
    }
    return WHEEL_FSM_RC_OK;
}

enum WheelFsmReturnCode wheel_fsm_api_run_state(void) {
    return (fsm_api_run_state(&wheel_fsm_handler.fsm, &wheel_fsm_handler.config) == FSM_RC_OK) ? WHEEL_FSM_RC_OK : WHEEL_FSM_RC_ERROR;
}

enum WheelFsmReturnCode wheel_fsm_api_request(enum WheelFsmState state) {
    // A fatal error must not be overwritten by a later request in the same
    // step (e.g. a flash ID seen right after the screen failed).
    if (wheel_fsm_handler.fsm.requested_state == WHEEL_FSM_STATE_ERROR && state != WHEEL_FSM_STATE_ERROR) {
        return WHEEL_FSM_RC_INVALID_TRANSITION;
    }
    return (fsm_api_transition_to(&wheel_fsm_handler.fsm, state) == FSM_RC_OK) ? WHEEL_FSM_RC_OK : WHEEL_FSM_RC_INVALID_TRANSITION;
}

enum WheelFsmState wheel_fsm_api_get_state(void) {
    return (enum WheelFsmState)fsm_api_get_state(&wheel_fsm_handler.fsm);
}

const char *wheel_fsm_api_get_state_name(enum WheelFsmState state) {
    return (state < WHEEL_FSM_STATE_COUNT) ? prv_wheel_fsm_api_state_names[state] : "?";
}

enum CanPrimarySteeringwheelfsmStatus wheel_fsm_api_get_can_status(enum WheelFsmState state) {
    switch (state) {
        case WHEEL_FSM_STATE_IDLE:
            return CAN_PRIMARY_STEERINGWHEELFSM_STATUS_IDLE;
        case WHEEL_FSM_STATE_FLASH:
            return CAN_PRIMARY_STEERINGWHEELFSM_STATUS_FLASH;
        case WHEEL_FSM_STATE_ERROR:
            return CAN_PRIMARY_STEERINGWHEELFSM_STATUS_ERROR;
        case WHEEL_FSM_STATE_INIT:
        default:
            return CAN_PRIMARY_STEERINGWHEELFSM_STATUS_INIT;
    }
}
