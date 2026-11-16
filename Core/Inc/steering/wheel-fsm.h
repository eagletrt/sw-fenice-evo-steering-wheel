/*!
 * \file wheel-fsm.h
 * \date 2026-10-26
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief Definitions for the steering wheel state machine.
 *
 * \details The wheel FSM decides which mode the wheel is in. It is a
 *     libfsm-sw machine run by the scheduler FSM task; any task can request a
 *     transition, which is applied at the end of the next FSM step. Hardware
 *     is reached only through the callbacks in WheelFsmConfig.
 */

#ifndef WHEEL_FSM_H
#define WHEEL_FSM_H

#include "eagletrt-fsm.h"
#include "post.h"
#include "timeout.h"

#include <stdbool.h>

#include <stdint.h>

/*!
 * \brief Return codes for wheel FSM operations.
 */
enum WheelFsmReturnCode {
    WHEEL_FSM_RC_OK,                 /*!< Operation completed successfully */
    WHEEL_FSM_RC_NULL_POINTER,       /*!< A required pointer or callback was NULL */
    WHEEL_FSM_RC_INVALID_TRANSITION, /*!< The requested state can't be reached from the current one */
    WHEEL_FSM_RC_ERROR,              /*!< libfsm rejected the state table or failed at runtime */
};

/*!
 * \brief States of the wheel, in libfsm id order.
 *
 * \details Manual vs autonomous is not a state: it will be a driving context
 *     inside IDLE, taken from the mission the ECU reports.
 */
enum WheelFsmState : uint8_t {
    WHEEL_FSM_STATE_INIT,         /*!< Power-on self test, then the boot logo */
    WHEEL_FSM_STATE_IDLE,         /*!< Normal operation: dashboard, inputs, CAN */
    WHEEL_FSM_STATE_MENU,         /*!< Menu page */
    WHEEL_FSM_STATE_DEBUG,        /*!< Debug pages, driver controls still active */
    WHEEL_FSM_STATE_CALIBRATION,  /*!< Sensor calibrations */
    WHEEL_FSM_STATE_SCREEN_SAVER, /*!< Logo only, every input ignored but the menu combo */
    WHEEL_FSM_STATE_FLASH,        /*!< Another board is being flashed: only the FSM status goes on the bus */
    WHEEL_FSM_STATE_ERROR,        /*!< Unrecoverable error, terminal */
    WHEEL_FSM_STATE_COUNT,
};

#define WHEEL_FSM_BOOT_LOGO_MS (1000U) /*!< Boot logo time after the POST (D13: at least 500 ms) */
#define WHEEL_FSM_TIMEOUT_COUNT (1U)   /*!< Timeouts the FSM takes from the pool (the boot logo) */

/*!
 * \brief Switch the rest of the wheel into or out of a mode (e.g. pause its tasks).
 */
typedef void (*wheel_fsm_mode_callback)(void);

/*!
 * \brief Hooks and init data handed to the wheel FSM.
 */
struct WheelFsmConfig {
    struct PostInitData *post;           /*!< Required: handed to the POST by INIT, must outlive the FSM */
    wheel_fsm_mode_callback enter_flash; /*!< Required: any mode -> FLASH, silence everything but the FSM status */
    wheel_fsm_mode_callback leave_flash; /*!< Required: FLASH -> the interrupted mode, undo enter_flash */
};

/*!
 * \brief File-static state of the wheel FSM.
 */
struct WheelFsmHandler {
    struct FSMHandler fsm;          /*!< libfsm instance */
    struct WheelFsmConfig config;   /*!< Copy of the configuration, passed to every state function */
    enum WheelFsmState interrupted; /*!< Mode FLASH returns to (libfsm has no history) */
    bool post_done;                 /*!< INIT already ran the POST: now it only waits for the logo */
    timeout_id boot_logo;           /*!< Boot logo time, started after the POST */
};

#endif // WHEEL_FSM_H
