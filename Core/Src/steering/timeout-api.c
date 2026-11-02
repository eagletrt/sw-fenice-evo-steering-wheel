/*!
 * \file timeout-api.c
 * \date 2026-11-02
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief Implementation of the timeout pool, on libtimebase watchdogs.
 */

#include "timeout-api.h"

#include "eagletrt-api.h"
#include "watchdogs-api.h"

#include <stddef.h>
#include <string.h>

static_assert(TIMEOUT_MAX_COUNT <= WATCHDOGS_MAX_COUNT, "libtimebase's pool is smaller than TIMEOUT_MAX_COUNT: raise WATCHDOGS_MAX_COUNT");
static_assert(TIMEOUT_MAX_COUNT <= UINT8_MAX, "timeout_id is 8 bits wide");

/*!
 * \brief One timeout: a libtimebase watchdog and who to tell when it expires.
 */
struct TimeoutSlot {
    struct Watchdog watchdog;            /*!< Counts the time */
    timeout_expired_callback on_expired; /*!< Called once per expiry */
    void *context;                       /*!< Handed back to on_expired */
    bool notified;                       /*!< on_expired already called for the current expiry */
};

/*!
 * \brief File-static state of the timeout pool.
 *
 * \details Kept out of timeout.h so libtimebase stays private to this file.
 */
struct TimeoutHandler {
    struct WatchdogHandler pool;                 /*!< The only libtimebase watchdog pool of the wheel */
    struct TimeoutSlot slots[TIMEOUT_MAX_COUNT]; /*!< Created timeouts, in creation order */
    uint8_t count;                               /*!< Slots taken */
    bool expired;                                /*!< A watchdog expired during the current routine */
};

EAGLETRT_STATIC struct TimeoutHandler timeout_handler;

/*!
 * \brief Shared libtimebase callback: which watchdog expired is found by the routine.
 *
 * \details libtimebase passes no argument, so every watchdog of the pool
 *     shares this one and timeout_api_routine looks for the expired slots.
 */
EAGLETRT_STATIC void prv_timeout_api_on_watchdog_expired(void) {
    timeout_handler.expired = true;
}

/*!
 * \brief Slot of an id.
 *
 * \param id The timeout.
 *
 * \return The slot, NULL if \p id was not handed out.
 */
EAGLETRT_STATIC struct TimeoutSlot *prv_timeout_api_slot(timeout_id id) {
    return (id < timeout_handler.count) ? &timeout_handler.slots[id] : NULL;
}

enum TimeoutReturnCode timeout_api_init(timeout_get_tick_callback get_tick) {
    if (get_tick == NULL) {
        return TIMEOUT_RC_NULL_POINTER;
    }
    memset(&timeout_handler, 0, sizeof(timeout_handler));
    return (watchdogs_api_init_pool(&timeout_handler.pool, get_tick) == WATCHDOG_RC_OK) ? TIMEOUT_RC_OK : TIMEOUT_RC_ERROR;
}

enum TimeoutReturnCode timeout_api_create(uint32_t duration_ms, timeout_expired_callback on_expired, void *context, timeout_id *id) {
    if (on_expired == NULL || id == NULL) {
        return TIMEOUT_RC_NULL_POINTER;
    }
    if (timeout_handler.count >= TIMEOUT_MAX_COUNT) {
        return TIMEOUT_RC_FULL;
    }

    struct TimeoutSlot *slot = &timeout_handler.slots[timeout_handler.count];
    if (watchdogs_api_init_watchdog(&slot->watchdog, duration_ms, prv_timeout_api_on_watchdog_expired) != WATCHDOG_RC_OK) {
        return TIMEOUT_RC_ERROR;
    }
    slot->on_expired = on_expired;
    slot->context = context;
    slot->notified = false;
    *id = timeout_handler.count++;
    return TIMEOUT_RC_OK;
}

enum TimeoutReturnCode timeout_api_start(timeout_id id) {
    struct TimeoutSlot *slot = prv_timeout_api_slot(id);
    if (slot == NULL) {
        return TIMEOUT_RC_INVALID_ID;
    }
    slot->notified = false;
    // Restart re-arms from any state: stopped, running or expired.
    return (watchdogs_api_watchdog_restart(&timeout_handler.pool, &slot->watchdog) == WATCHDOG_RC_OK) ? TIMEOUT_RC_OK : TIMEOUT_RC_ERROR;
}

enum TimeoutReturnCode timeout_api_stop(timeout_id id) {
    struct TimeoutSlot *slot = prv_timeout_api_slot(id);
    if (slot == NULL) {
        return TIMEOUT_RC_INVALID_ID;
    }
    slot->notified = false;
    // Reset (not stop): libtimebase's stop refuses an expired watchdog.
    return (watchdogs_api_watchdog_reset(&timeout_handler.pool, &slot->watchdog) == WATCHDOG_RC_OK) ? TIMEOUT_RC_OK : TIMEOUT_RC_ERROR;
}

bool timeout_api_is_running(timeout_id id) {
    struct TimeoutSlot *slot = prv_timeout_api_slot(id);
    return (slot != NULL) && watchdogs_api_watchdog_is_running(&slot->watchdog);
}

bool timeout_api_has_expired(timeout_id id) {
    struct TimeoutSlot *slot = prv_timeout_api_slot(id);
    return (slot != NULL) && watchdogs_api_watchdog_is_timed_out(&slot->watchdog);
}

enum TimeoutReturnCode timeout_api_routine(void) {
    if (watchdogs_api_routine(&timeout_handler.pool) != WATCHDOG_RC_OK) {
        return TIMEOUT_RC_ERROR;
    }
    if (!timeout_handler.expired) {
        return TIMEOUT_RC_OK;
    }
    timeout_handler.expired = false;

    // A callback may start or stop any timeout: a restarted one is running
    // again and is skipped, a stopped one is no longer expired.
    for (uint8_t i = 0U; i < timeout_handler.count; i++) {
        struct TimeoutSlot *slot = &timeout_handler.slots[i];
        if (slot->notified || !watchdogs_api_watchdog_is_timed_out(&slot->watchdog)) {
            continue;
        }
        slot->notified = true;
        slot->on_expired(slot->context);
    }
    return TIMEOUT_RC_OK;
}
