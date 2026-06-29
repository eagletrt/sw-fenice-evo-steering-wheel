#ifndef CAN_COMMUNICATIONS_ROUTER_H
#define CAN_COMMUNICATIONS_ROUTER_H

#include "can-communications.h"

/*!
 * \brief Router function for incoming CAN frames on primary network.
 *
 * \param[in] frame The frame just popped off the RX queue.
 *
 * \retval CAN_COMMUNICATION_RC_OK on success.
 * \retval CAN_COMMUNICATION_RC_RECEIVE_HANDLER_ERROR if dispatch fails.
 */
enum CanCommunicationReturnCode can_communications_router_api_receive_primary(const struct CanCommunicationFrame *frame);

/*!
 * \brief Router function for incoming CAN frames on the secondary network.
 *
 * \param[in] frame The frame just popped off the RX queue.
 *
 * \retval CAN_COMMUNICATION_RC_OK on success.
 * \retval CAN_COMMUNICATION_RC_RECEIVE_HANDLER_ERROR if dispatch fails.
 */
enum CanCommunicationReturnCode can_communications_router_api_receive_secondary(const struct CanCommunicationFrame *frame);

#endif // CAN_COMMUNICATIONS_ROUTER_H
