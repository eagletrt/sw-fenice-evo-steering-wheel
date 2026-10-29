/*!
 * \file flash-monitor-api.h
 * \date 2026-10-29
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief Public API of the flash monitor.
 */

#ifndef FLASH_MONITOR_API_H
#define FLASH_MONITOR_API_H

#include "flash-monitor.h"

#include <stdbool.h>

/*!
 * \brief Set up the timeout watchdog, nothing is being flashed.
 *
 * \param watchdogs The libtimebase pool the watchdog runs in (its routine
 *     calls the timeout from main context).
 *
 * \retval FLASH_MONITOR_RC_OK on success.
 * \retval FLASH_MONITOR_RC_NULL_POINTER if \p watchdogs is NULL.
 * \retval FLASH_MONITOR_RC_ERROR if libtimebase rejected the watchdog.
 */
enum FlashMonitorReturnCode flash_monitor_api_init(struct WatchdogHandler *watchdogs);

/*!
 * \brief Check a received frame for flashing traffic of another board.
 *
 * \details On a match: remembers the board and restarts the timeout. Call
 *     it on every received primary frame, before decoding it. Main context
 *     only.
 *
 * \param id CAN identifier of the frame.
 *
 * \retval true if the frame belongs to another board's flashing (don't decode it).
 * \retval false otherwise.
 */
bool flash_monitor_api_on_frame(uint32_t id);

/*!
 * \brief Whether flashing traffic was seen in the last FLASH_MONITOR_TIMEOUT_MS.
 *
 * \details The FSM polls it: IDLE moves to FLASH while it is true, FLASH
 *     back to IDLE once it is false.
 *
 * \return true from a flashing frame until the timeout watchdog expires.
 */
bool flash_monitor_api_is_active(void);

/*!
 * \brief Name of the board of the last flashing frame.
 *
 * \return Upper-case name, NULL if no flashing frame has been seen yet.
 */
const char *flash_monitor_api_get_board_name(void);

#endif // FLASH_MONITOR_API_H
