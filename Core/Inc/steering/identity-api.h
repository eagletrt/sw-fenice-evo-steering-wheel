/*!
 * \file identity-api.h
 * \date 2026-08-27
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief Public API for the steering wheel identity broadcast.
 *
 * \details Each periodic sender is a no-op until the matching libcan cycle
 *     time has elapsed, so they are cheap to call on every FSM tick. All
 *     frames are queued on the primary network through
 *     can_communications_api_add_to_tx_buffer and flushed by the usual
 *     process_tx drain.
 */

#ifndef IDENTITY_API_H
#define IDENTITY_API_H

#include "identity.h"
#include "can-primary.h"
#include <stdint.h>

/*!
 * \brief Initialize the identity API.
 *
 * \retval IDENTITY_RC_OK if the identity API was initialized successfully.
 * \retval IDENTITY_RC_ERROR if there was an error initializing the identity API.
 */
enum IdentityReturnCode identity_api_init(void);

/*!
 * \brief Send state identity informations.
 *
 * \param status The current steering wheel FSM status.
 *
 * \retval IDENTITY_RC_OK if the identity information was sent successfully.
 * \retval IDENTITY_RC_ERROR if there was an error sending the identity information.
 */
enum IdentityReturnCode identity_api_send_state(enum CanPrimarySteeringwheelfsmStatus status);

/*!
 * \brief Periodically send state identity informations.
 *
 * \param status The current steering wheel FSM status.
 * \param tick_ms The current tick, in milliseconds.
 *
 * \retval IDENTITY_RC_OK if the identity information was sent successfully.
 * \retval IDENTITY_RC_ERROR if there was an error sending the identity information.
 */
enum IdentityReturnCode identity_api_periodically_send_state(enum CanPrimarySteeringwheelfsmStatus status, uint32_t tick_ms);

/*!
 * \brief Periodically send version identity informations.
 *
 * \param tick_ms The current tick, in milliseconds.
 *
 * \retval IDENTITY_RC_OK if the identity information was sent successfully.
 * \retval IDENTITY_RC_ERROR if there was an error sending the identity information.
 */
enum IdentityReturnCode identity_api_periodically_send_version(uint32_t tick_ms);

/*!
 * \brief Periodically send libcan version identity informations.
 *
 * \param tick_ms The current tick, in milliseconds.
 *
 * \retval IDENTITY_RC_OK if the identity information was sent successfully.
 * \retval IDENTITY_RC_ERROR if there was an error sending the identity information.
 */
enum IdentityReturnCode identity_api_periodically_send_libcan_version(uint32_t tick_ms);

#endif // IDENTITY_API_H
