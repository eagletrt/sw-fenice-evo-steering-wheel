/*!
 * \file wheel-fsm-api.c
 * \date 2026-10-26
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief Implementation of the steering wheel state machine.
 */

#include "wheel-fsm-api.h"

#include "eagletrt-api.h"
#include "eagletrt-fsm-api.h"
#include "flash-monitor-api.h"
#include "post-api.h"
#include "timeout-api.h"

#include <string.h>

EAGLETRT_STATIC struct WheelFsmHandler wheel_fsm_handler;

/*!
 * \brief Leave for FLASH if another board is being flashed.
 *
 * \details Level-triggered (A9): every mode polls the monitor at each step.
 *
 * \retval true if FLASH was requested.
 */
EAGLETRT_STATIC bool prv_wheel_fsm_api_follow_flash(void) {
    if (!flash_monitor_api_is_active()) {
        return false;
    }
    EAGLETRT_API_UNUSED(wheel_fsm_api_request(WHEEL_FSM_STATE_FLASH));
    return true;
}

/*!
 * \brief INIT: run the POST, then show the boot logo.
 *
 * \note The logic does:
 *    - first step: POST, ERROR if it fails, else start the boot logo time
 *    - then: FLASH if another board is being flashed
 *    - IDLE once the boot logo time is over (TODO(#11): and the ECU reported a mission, D13/D20)
 *
 * \param data The WheelFsmConfig of the handler.
 */
EAGLETRT_STATIC void prv_wheel_fsm_api_do_init(void *data) {
    struct WheelFsmConfig *config = (struct WheelFsmConfig *)data;
    if (!wheel_fsm_handler.post_done) {
        wheel_fsm_handler.post_done = true;
        if (post_api_do_init(config->post) != POST_RC_OK || timeout_api_start(wheel_fsm_handler.boot_logo) != TIMEOUT_RC_OK) {
            EAGLETRT_API_UNUSED(wheel_fsm_api_request(WHEEL_FSM_STATE_ERROR));
        }
        return;
    }
    if (prv_wheel_fsm_api_follow_flash()) {
        return;
    }
    if (timeout_api_has_expired(wheel_fsm_handler.boot_logo)) {
        EAGLETRT_API_UNUSED(wheel_fsm_api_request(WHEEL_FSM_STATE_IDLE));
    }
}

/*!
 * \brief IDLE, MENU, DEBUG, CALIBRATION, SCREEN_SAVER: only follow the flash monitor.
 *
 * \details Moving between these modes is requested from outside (inputs).
 *
 * \param data The WheelFsmConfig of the handler (unused).
 */
EAGLETRT_STATIC void prv_wheel_fsm_api_do_mode(void *data) {
    EAGLETRT_API_UNUSED(data);
    EAGLETRT_API_UNUSED(prv_wheel_fsm_api_follow_flash());
}

/*!
 * \brief FLASH: back to the interrupted mode once the monitor stops.
 *
 * \param data The WheelFsmConfig of the handler (unused).
 */
EAGLETRT_STATIC void prv_wheel_fsm_api_do_flash(void *data) {
    EAGLETRT_API_UNUSED(data);
    if (!flash_monitor_api_is_active()) {
        EAGLETRT_API_UNUSED(wheel_fsm_api_request(wheel_fsm_handler.interrupted));
    }
}

/*!
 * \brief Any mode -> FLASH: remember the mode, silence the wheel.
 *
 * \details libfsm runs it before switching, so the current state is still the mode left.
 *
 * \param data The WheelFsmConfig of the handler.
 */
EAGLETRT_STATIC void prv_wheel_fsm_api_enter_flash(void *data) {
    const struct WheelFsmConfig *config = (const struct WheelFsmConfig *)data;
    wheel_fsm_handler.interrupted = wheel_fsm_api_get_state();
    config->enter_flash();
}

/*!
 * \brief FLASH -> the interrupted mode: back to normal operation.
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

#define WHEEL_FSM_TO_FLASH { WHEEL_FSM_STATE_FLASH, prv_wheel_fsm_api_enter_flash }
#define WHEEL_FSM_TO_ERROR { WHEEL_FSM_STATE_ERROR, NULL }

EAGLETRT_STATIC const struct Transition prv_wheel_fsm_api_init_transitions[] = {
    { WHEEL_FSM_STATE_IDLE, NULL },
    WHEEL_FSM_TO_FLASH,
    WHEEL_FSM_TO_ERROR,
};

EAGLETRT_STATIC const struct Transition prv_wheel_fsm_api_idle_transitions[] = {
    { WHEEL_FSM_STATE_MENU, NULL },
    WHEEL_FSM_TO_FLASH,
    WHEEL_FSM_TO_ERROR,
};

EAGLETRT_STATIC const struct Transition prv_wheel_fsm_api_menu_transitions[] = {
    { WHEEL_FSM_STATE_IDLE, NULL },
    { WHEEL_FSM_STATE_DEBUG, NULL },
    { WHEEL_FSM_STATE_CALIBRATION, NULL },
    { WHEEL_FSM_STATE_SCREEN_SAVER, NULL },
    WHEEL_FSM_TO_FLASH,
    WHEEL_FSM_TO_ERROR,
};

/*!
 * \brief DEBUG, CALIBRATION and SCREEN_SAVER all go back through the menu.
 */
EAGLETRT_STATIC const struct Transition prv_wheel_fsm_api_page_transitions[] = {
    { WHEEL_FSM_STATE_MENU, NULL },
    WHEEL_FSM_TO_FLASH,
    WHEEL_FSM_TO_ERROR,
};

/*!
 * \brief FLASH returns to whichever mode it interrupted (A5).
 */
EAGLETRT_STATIC const struct Transition prv_wheel_fsm_api_flash_transitions[] = {
    { WHEEL_FSM_STATE_INIT, prv_wheel_fsm_api_leave_flash },
    { WHEEL_FSM_STATE_IDLE, prv_wheel_fsm_api_leave_flash },
    { WHEEL_FSM_STATE_MENU, prv_wheel_fsm_api_leave_flash },
    { WHEEL_FSM_STATE_DEBUG, prv_wheel_fsm_api_leave_flash },
    { WHEEL_FSM_STATE_CALIBRATION, prv_wheel_fsm_api_leave_flash },
    { WHEEL_FSM_STATE_SCREEN_SAVER, prv_wheel_fsm_api_leave_flash },
    WHEEL_FSM_TO_ERROR,
};

#define WHEEL_FSM_STATE(id, function, transitions) [id] = { id, function, id, transitions, (uint8_t)EAGLETRT_FSM_TRANSITION_LEN(transitions) }

/*!
 * \brief State table, indexed by WheelFsmState (libfsm wants ids in order).
 *
 * \details Every state stays where it is until a transition is requested:
 *     next_default is the state itself.
 */
EAGLETRT_STATIC const struct State prv_wheel_fsm_api_states[WHEEL_FSM_STATE_COUNT] = {
    WHEEL_FSM_STATE(WHEEL_FSM_STATE_INIT, prv_wheel_fsm_api_do_init, prv_wheel_fsm_api_init_transitions),
    WHEEL_FSM_STATE(WHEEL_FSM_STATE_IDLE, prv_wheel_fsm_api_do_mode, prv_wheel_fsm_api_idle_transitions),
    WHEEL_FSM_STATE(WHEEL_FSM_STATE_MENU, prv_wheel_fsm_api_do_mode, prv_wheel_fsm_api_menu_transitions),
    WHEEL_FSM_STATE(WHEEL_FSM_STATE_DEBUG, prv_wheel_fsm_api_do_mode, prv_wheel_fsm_api_page_transitions),
    WHEEL_FSM_STATE(WHEEL_FSM_STATE_CALIBRATION, prv_wheel_fsm_api_do_mode, prv_wheel_fsm_api_page_transitions),
    WHEEL_FSM_STATE(WHEEL_FSM_STATE_SCREEN_SAVER, prv_wheel_fsm_api_do_mode, prv_wheel_fsm_api_page_transitions),
    WHEEL_FSM_STATE(WHEEL_FSM_STATE_FLASH, prv_wheel_fsm_api_do_flash, prv_wheel_fsm_api_flash_transitions),
    [WHEEL_FSM_STATE_ERROR] = { WHEEL_FSM_STATE_ERROR, prv_wheel_fsm_api_do_error, WHEEL_FSM_STATE_ERROR, NULL, 0U },
};

EAGLETRT_STATIC const char *const prv_wheel_fsm_api_state_names[WHEEL_FSM_STATE_COUNT] = {
    [WHEEL_FSM_STATE_INIT] = "INIT",
    [WHEEL_FSM_STATE_IDLE] = "IDLE",
    [WHEEL_FSM_STATE_MENU] = "MENU",
    [WHEEL_FSM_STATE_DEBUG] = "DEBUG",
    [WHEEL_FSM_STATE_CALIBRATION] = "CALIBRATION",
    [WHEEL_FSM_STATE_SCREEN_SAVER] = "SCREEN_SAVER",
    [WHEEL_FSM_STATE_FLASH] = "FLASH",
    [WHEEL_FSM_STATE_ERROR] = "ERROR",
};

enum WheelFsmReturnCode wheel_fsm_api_init(const struct WheelFsmConfig *config) {
    if (config == NULL || config->post == NULL || config->enter_flash == NULL || config->leave_flash == NULL) {
        return WHEEL_FSM_RC_NULL_POINTER;
    }

    memset(&wheel_fsm_handler, 0, sizeof(wheel_fsm_handler));
    wheel_fsm_handler.config = *config;

    if (timeout_api_create(WHEEL_FSM_BOOT_LOGO_MS, NULL, NULL, &wheel_fsm_handler.boot_logo) != TIMEOUT_RC_OK ||
        fsm_api_init(&wheel_fsm_handler.fsm, prv_wheel_fsm_api_states, WHEEL_FSM_STATE_COUNT, WHEEL_FSM_STATE_INIT) != FSM_RC_OK) {
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
    // TODO(libcan): the menu, debug, calibration and screen-saver modes report IDLE until libcan has them (D16).
    switch (state) {
        case WHEEL_FSM_STATE_INIT:
            return CAN_PRIMARY_STEERINGWHEELFSM_STATUS_INIT;
        case WHEEL_FSM_STATE_FLASH:
            return CAN_PRIMARY_STEERINGWHEELFSM_STATUS_FLASH;
        case WHEEL_FSM_STATE_ERROR:
            return CAN_PRIMARY_STEERINGWHEELFSM_STATUS_ERROR;
        case WHEEL_FSM_STATE_IDLE:
        case WHEEL_FSM_STATE_MENU:
        case WHEEL_FSM_STATE_DEBUG:
        case WHEEL_FSM_STATE_CALIBRATION:
        case WHEEL_FSM_STATE_SCREEN_SAVER:
        case WHEEL_FSM_STATE_COUNT:
        default:
            return CAN_PRIMARY_STEERINGWHEELFSM_STATUS_IDLE;
    }
}
