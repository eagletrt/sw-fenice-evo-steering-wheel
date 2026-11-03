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
 * \param reset Resets the wheel into its bootloader (its own flashing frames).
 *
 * \retval FLASH_MONITOR_RC_OK on success.
 * \retval FLASH_MONITOR_RC_NULL_POINTER if \p watchdogs or \p reset is NULL.
 * \retval FLASH_MONITOR_RC_ERROR if libtimebase rejected the watchdog.
 */
enum FlashMonitorReturnCode flash_monitor_api_init(struct WatchdogHandler *watchdogs, flash_monitor_reset_callback reset);

/*!
 * \brief Check a received frame for flashing traffic.
 *
 * \details The wheel's own flashing frames reset it into its bootloader.
 *     Another board's: remembers the board and restarts the timeout. Call it
 *     for every received primary frame the dashboard doesn't use. Main
 *     context only.
 *
 * \param id CAN identifier of the frame.
 *
 * \retval true if the frame is flashing traffic (the wheel's or another board's).
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
