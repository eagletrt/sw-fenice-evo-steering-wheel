/*!
 * \file timeout.h
 * \date 2026-11-02
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief Definitions for the timeouts of the wheel: one static pool of
 *     one-shot timers, behind ids.
 *
 * \details Every module that needs a timeout (flash traffic silence, long
 *     presses, combo windows, later CAN data staleness and acks) creates
 *     one here at init and gets an id back. The pool is a libtimebase
 *     watchdog pool, but nothing outside timeout-api.c sees libtimebase.
 *     Expiry callbacks run from timeout_api_routine (the scheduler's main
 *     loop), never from an interrupt, and get back the context given at
 *     creation, so one function can serve many timeouts.
 */

#ifndef TIMEOUT_H
#define TIMEOUT_H

#include <stdint.h>

/*!
 * \brief Number of timeouts the pool can hold.
 *
 * \details Overridable with -DTIMEOUT_MAX_COUNT=; libtimebase's
 *     WATCHDOGS_MAX_COUNT must be at least as large.
 */
#ifndef TIMEOUT_MAX_COUNT
#define TIMEOUT_MAX_COUNT (20U)
#endif

/*!
 * \brief Return codes for timeout operations.
 */
enum TimeoutReturnCode {
    TIMEOUT_RC_OK,           /*!< Operation completed successfully */
    TIMEOUT_RC_NULL_POINTER, /*!< A required pointer or callback was NULL */
    TIMEOUT_RC_FULL,         /*!< Every slot of the pool is taken */
    TIMEOUT_RC_INVALID_ID,   /*!< The id was not returned by timeout_api_create */
    TIMEOUT_RC_ERROR,        /*!< libtimebase failed (e.g. time went backwards) */
};

/*!
 * \brief Identifier of a timeout, from timeout_api_create.
 */
typedef uint8_t timeout_id;

/*!
 * \brief Current time in milliseconds.
 */
typedef uint32_t (*timeout_get_tick_callback)(void);

/*!
 * \brief A timeout expired.
 *
 * \param context The pointer given to timeout_api_create.
 */
typedef void (*timeout_expired_callback)(void *context);

#endif // TIMEOUT_H
