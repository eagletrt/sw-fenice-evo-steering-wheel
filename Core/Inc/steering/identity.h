/*!
 * \file identity.h
 * \date 2026-08-27
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief Definitions for the steering wheel identity broadcast.
 *
 * \details Every ECU on the car periodically announces who it is: its FSM
 *     status, its firmware version and the version of the CAN library it
 *     was built against. This module owns the cycle-time bookkeeping for
 *     those three broadcasts.
 */

#ifndef IDENTITY_H
#define IDENTITY_H

#include <stdint.h>

#define IDENTITY_VERSION_MAJOR (0U)
#define IDENTITY_VERSION_MINOR (1U)
#define IDENTITY_VERSION_PATCH (0U)

#define IDENTITY_VERSION_DIRTY (0U)

#define IDENTITY_VERSION_INFO_COMMIT_HASH (0x0676767)

/*!
 * \brief Return codes for identity operations.
 */
enum IdentityReturnCode {
    IDENTITY_RC_OK,    /*!< Operation completed successfully. */
    IDENTITY_RC_ERROR, /*!< An error occurred during the operation. */
};

/*!
 * \brief File-static state of the identity module.
 */
struct IdentityHandler {
    uint32_t build_time;                  /*!< Firmware build time, as a UNIX timestamp. */
    uint32_t last_tick_ms_status;         /*!< Tick of the last FSM status broadcast. */
    uint32_t last_tick_ms_version;        /*!< Tick of the last firmware version broadcast. */
    uint32_t last_tick_ms_libcan_version; /*!< Tick of the last libcan version broadcast. */
};

#endif // IDENTITY_H
