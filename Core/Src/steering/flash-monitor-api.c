/*!
 * \file flash-monitor-api.c
 * \date 2026-10-29
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief Implementation of the flash monitor.
 */

#include "flash-monitor-api.h"

#include "can-primary.h"
#include "eagletrt.h"
#include "watchdogs-api.h"

#include <stddef.h>
#include <string.h>

/*!
 * \brief Every board flashed over CAN except the wheel, with its name.
 *
 * \details One entry per libcan <Board>FlashingTx/Rx pair. The wheel's own
 *     pair is left out on purpose: the router turns it into a reset into the
 *     bootloader. Names are what the FLASH page shows; edit them here.
 */
EAGLETRT_STATIC const struct FlashMonitorTarget prv_flash_monitor_api_targets[] = {
    { CAN_PRIMARY_MESSAGE_FRAME_ID_ECUFLASHINGTX, CAN_PRIMARY_MESSAGE_FRAME_ID_ECUFLASHINGRX, "ECU" },
    { CAN_PRIMARY_MESSAGE_FRAME_ID_PEDALSFLASHINGTX, CAN_PRIMARY_MESSAGE_FRAME_ID_PEDALSFLASHINGRX, "PEDALS" },
    { CAN_PRIMARY_MESSAGE_FRAME_ID_COOLINGFLASHINGTX, CAN_PRIMARY_MESSAGE_FRAME_ID_COOLINGFLASHINGRX, "COOLING" },
    { CAN_PRIMARY_MESSAGE_FRAME_ID_LVACFLASHINGTX, CAN_PRIMARY_MESSAGE_FRAME_ID_LVACFLASHINGRX, "LVAC" },
    { CAN_PRIMARY_MESSAGE_FRAME_ID_PIPOFLASHINGTX, CAN_PRIMARY_MESSAGE_FRAME_ID_PIPOFLASHINGRX, "PIPO" },
    { CAN_PRIMARY_MESSAGE_FRAME_ID_DASFRONTFLASHINGTX, CAN_PRIMARY_MESSAGE_FRAME_ID_DASFRONTFLASHINGRX, "DAS FRONT" },
    { CAN_PRIMARY_MESSAGE_FRAME_ID_DASREARFLASHINGTX, CAN_PRIMARY_MESSAGE_FRAME_ID_DASREARFLASHINGRX, "DAS REAR" },
    { CAN_PRIMARY_MESSAGE_FRAME_ID_TSACMAINBOARDFLASHINGTX, CAN_PRIMARY_MESSAGE_FRAME_ID_TSACMAINBOARDFLASHINGRX, "TSAC MAINBOARD" },
    { CAN_PRIMARY_MESSAGE_FRAME_ID_TSACCELLBOARD1FLASHINGTX, CAN_PRIMARY_MESSAGE_FRAME_ID_TSACCELLBOARD1FLASHINGRX, "CELLBOARD 1" },
    { CAN_PRIMARY_MESSAGE_FRAME_ID_TSACCELLBOARD2FLASHINGTX, CAN_PRIMARY_MESSAGE_FRAME_ID_TSACCELLBOARD2FLASHINGRX, "CELLBOARD 2" },
    { CAN_PRIMARY_MESSAGE_FRAME_ID_TSACCELLBOARD3FLASHINGTX, CAN_PRIMARY_MESSAGE_FRAME_ID_TSACCELLBOARD3FLASHINGRX, "CELLBOARD 3" },
    { CAN_PRIMARY_MESSAGE_FRAME_ID_TSACCELLBOARD4FLASHINGTX, CAN_PRIMARY_MESSAGE_FRAME_ID_TSACCELLBOARD4FLASHINGRX, "CELLBOARD 4" },
    { CAN_PRIMARY_MESSAGE_FRAME_ID_TSACCELLBOARD5FLASHINGTX, CAN_PRIMARY_MESSAGE_FRAME_ID_TSACCELLBOARD5FLASHINGRX, "CELLBOARD 5" },
    { CAN_PRIMARY_MESSAGE_FRAME_ID_TSACCELLBOARD6FLASHINGTX, CAN_PRIMARY_MESSAGE_FRAME_ID_TSACCELLBOARD6FLASHINGRX, "CELLBOARD 6" },
    { CAN_PRIMARY_MESSAGE_FRAME_ID_STEERINGBOARDFLASHINGTX, CAN_PRIMARY_MESSAGE_FRAME_ID_STEERINGBOARDFLASHINGRX, "STEERING BOARD" },
};

#define FLASH_MONITOR_TARGET_COUNT (sizeof(prv_flash_monitor_api_targets) / sizeof(prv_flash_monitor_api_targets[0]))

EAGLETRT_STATIC struct FlashMonitorHandler flash_monitor_handler;

/*!
 * \brief Board a frame identifier belongs to.
 *
 * \param id CAN identifier.
 *
 * \return The board, NULL if \p id is not another board's flashing frame.
 */
EAGLETRT_STATIC const struct FlashMonitorTarget *prv_flash_monitor_api_find_target(uint32_t id) {
    for (size_t i = 0U; i < FLASH_MONITOR_TARGET_COUNT; i++) {
        const struct FlashMonitorTarget *target = &prv_flash_monitor_api_targets[i];
        if (id == target->tx_id || id == target->rx_id) {
            return target;
        }
    }
    return NULL;
}

/*!
 * \brief No flashing frame for FLASH_MONITOR_TIMEOUT_MS: flashing is over.
 */
EAGLETRT_STATIC void prv_flash_monitor_api_on_timeout(void) {
    flash_monitor_handler.active = false;
}

enum FlashMonitorReturnCode flash_monitor_api_init(struct WatchdogHandler *watchdogs) {
    if (watchdogs == NULL) {
        return FLASH_MONITOR_RC_NULL_POINTER;
    }

    memset(&flash_monitor_handler, 0, sizeof(flash_monitor_handler));
    flash_monitor_handler.watchdogs = watchdogs;
    if (watchdogs_api_init_watchdog(&flash_monitor_handler.watchdog, FLASH_MONITOR_TIMEOUT_MS, prv_flash_monitor_api_on_timeout) != WATCHDOG_RC_OK) {
        return FLASH_MONITOR_RC_ERROR;
    }
    return FLASH_MONITOR_RC_OK;
}

bool flash_monitor_api_on_frame(uint32_t id) {
    const struct FlashMonitorTarget *target = prv_flash_monitor_api_find_target(id);
    if (target == NULL) {
        return false;
    }

    flash_monitor_handler.target = target;
    // Restart works whether the watchdog is idle, running or expired. If it
    // fails the wheel would never leave FLASH: don't enter it at all.
    flash_monitor_handler.active = (watchdogs_api_watchdog_restart(flash_monitor_handler.watchdogs, &flash_monitor_handler.watchdog) == WATCHDOG_RC_OK);
    return true;
}

bool flash_monitor_api_is_active(void) {
    return flash_monitor_handler.active;
}

const char *flash_monitor_api_get_board_name(void) {
    return (flash_monitor_handler.target != NULL) ? flash_monitor_handler.target->name : NULL;
}
