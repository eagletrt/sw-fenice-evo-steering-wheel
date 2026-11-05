/*!
 * \file timeout-api.h
 * \date 2026-11-02
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief Public API of the timeout pool.
 */

#ifndef TIMEOUT_API_H
#define TIMEOUT_API_H

#include "timeout.h"

#include <stdbool.h>

/*!
 * \brief Empty the pool and set its clock.
 *
 * \details Call once before any timeout_api_create (the scheduler does).
 *     Every id handed out before is invalid afterwards.
 *
 * \param get_tick Millisecond clock.
 *
 * \retval TIMEOUT_RC_OK on success.
 * \retval TIMEOUT_RC_NULL_POINTER if \p get_tick is NULL.
 * \retval TIMEOUT_RC_ERROR if libtimebase rejected the pool.
 */
enum TimeoutReturnCode timeout_api_init(timeout_get_tick_callback get_tick);

/*!
 * \brief Take a slot of the pool, stopped.
 *
 * \param duration_ms Time from timeout_api_start to expiry.
 * \param on_expired Called once per start when the time is up.
 * \param context Handed back to \p on_expired (may be NULL).
 * \param[out] id Identifier of the new timeout.
 *
 * \retval TIMEOUT_RC_OK on success.
 * \retval TIMEOUT_RC_NULL_POINTER if \p on_expired or \p id is NULL.
 * \retval TIMEOUT_RC_FULL if TIMEOUT_MAX_COUNT timeouts already exist.
 * \retval TIMEOUT_RC_ERROR if libtimebase rejected the timeout.
 */
enum TimeoutReturnCode timeout_api_create(uint32_t duration_ms, timeout_expired_callback on_expired, void *context, timeout_id *id);

/*!
 * \brief (Re)start a timeout: it expires duration_ms from now.
 *
 * \details Works whether it is stopped, running (the time starts over) or
 *     expired (it can expire again).
 *
 * \param id The timeout.
 *
 * \retval TIMEOUT_RC_OK on success.
 * \retval TIMEOUT_RC_INVALID_ID if \p id is unknown.
 * \retval TIMEOUT_RC_ERROR if libtimebase failed.
 */
enum TimeoutReturnCode timeout_api_start(timeout_id id);

/*!
 * \brief Stop a timeout: neither running nor expired afterwards.
 *
 * \details Stopping a stopped or expired timeout is fine.
 *
 * \param id The timeout.
 *
 * \retval TIMEOUT_RC_OK on success.
 * \retval TIMEOUT_RC_INVALID_ID if \p id is unknown.
 * \retval TIMEOUT_RC_ERROR if libtimebase failed.
 */
enum TimeoutReturnCode timeout_api_stop(timeout_id id);

/*!
 * \brief Whether a timeout is counting.
 *
 * \param id The timeout.
 *
 * \return true between timeout_api_start and its expiry (or stop), false otherwise or for an unknown id.
 */
bool timeout_api_is_running(timeout_id id);

/*!
 * \brief Whether a timeout has expired since its last start.
 *
 * \param id The timeout.
 *
 * \return true from its expiry until the next start or stop, false otherwise or for an unknown id.
 */
bool timeout_api_has_expired(timeout_id id);

/*!
 * \brief Expire every timeout that is due and call its callback.
 *
 * \details Call it from the main loop as often as possible (the scheduler
 *     routine does). Callbacks may start or stop any timeout, their own
 *     included.
 *
 * \retval TIMEOUT_RC_OK on success.
 * \retval TIMEOUT_RC_ERROR if libtimebase failed (e.g. the tick went backwards).
 */
enum TimeoutReturnCode timeout_api_routine(void);

#endif // TIMEOUT_API_H
