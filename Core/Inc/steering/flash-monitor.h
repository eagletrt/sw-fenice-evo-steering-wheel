/*!
 * \file flash-monitor.h
 * \date 2026-10-29
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief Definitions for the detection of other boards being flashed.
 *
 * \details Every board is flashed through OpenBLT over CAN, on a pair of
 *     libcan messages (<Board>FlashingTx / <Board>FlashingRx). While any of
 *     them is on the bus the wheel must stay quiet: the monitor is active from
 *     the first one until none has been seen for FLASH_MONITOR_TIMEOUT_MS (a
 *     libtimebase watchdog), and the FSM follows it (IDLE <-> FLASH). The
 *     wheel's own pair is not part of it: it asks the wheel to reset into its
 *     bootloader.
 */

#ifndef FLASH_MONITOR_H
#define FLASH_MONITOR_H

#include "watchdogs.h"

#include <stdbool.h>
#include <stdint.h>

/*!
 * \brief Time without flashing traffic after which the wheel leaves FLASH, in milliseconds.
 */
#define FLASH_MONITOR_TIMEOUT_MS (500U)

/*!
 * \brief Return codes for flash monitor operations.
 */
enum FlashMonitorReturnCode {
    FLASH_MONITOR_RC_OK,           /*!< Operation completed successfully */
    FLASH_MONITOR_RC_NULL_POINTER, /*!< A required pointer was NULL */
    FLASH_MONITOR_RC_ERROR,        /*!< libtimebase rejected the watchdog */
};

/*!
 * \brief A board that can be flashed over CAN, with the name shown while it is.
 */
struct FlashMonitorTarget {
    uint16_t tx_id;   /*!< <Board>FlashingTx: frames sent by the board's bootloader */
    uint16_t rx_id;   /*!< <Board>FlashingRx: frames sent to it by the flashing tool */
    const char *name; /*!< Name on the FLASH page, upper case */
};

/*!
 * \brief File-static state of the flash monitor.
 */
struct FlashMonitorHandler {
    struct WatchdogHandler *watchdogs;       /*!< Pool the timeout watchdog runs in */
    struct Watchdog watchdog;                /*!< Restarted by every flashing frame, expires after FLASH_MONITOR_TIMEOUT_MS of silence */
    const struct FlashMonitorTarget *target; /*!< Board of the last flashing frame, NULL before the first one */
    bool active;                             /*!< A flashing frame was seen and the watchdog hasn't expired since */
};

#endif // FLASH_MONITOR_H
