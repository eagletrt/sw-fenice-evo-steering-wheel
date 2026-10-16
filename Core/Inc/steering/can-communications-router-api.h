/*!
 * \file can-communications-router-api.h
 * \date 2026-08-27
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief Public API for the steering-wheel CAN routing layer.
 *
 * \details This module is the only place that knows about libcan. It sits
 *     on both ends of the can-communications queues:
 *     - inbound, the receive callbacks deserialize each frame and fold the
 *       values the dashboard renders into the UIData snapshot;
 *     - outbound, the wheel's own messages (button status, control maps,
 *       driver action) are built here, both on a parameter transition and
 *       on their libcan cycle time.
 *
 *     The identity broadcasts (FSM status and versions) live in their own
 *     module, see identity-api.h.
 */

#ifndef CAN_COMMUNICATIONS_ROUTER_API_H
#define CAN_COMMUNICATIONS_ROUTER_API_H

#include "can-communications.h"
#include "inputs-shared.h"

/*!
 * \brief Dispatch one frame received on the primary network.
 *
 * \details Deserializes the frame with libcan and, for the messages the
 *     dashboard cares about, writes the decoded values into the UIData
 *     snapshot. Unknown IDs are silently ignored.
 *
 * \param[in] frame The frame popped off the primary RX queue.
 *
 * \retval CAN_COMMUNICATION_RC_OK on success, or for an unhandled ID.
 * \retval CAN_COMMUNICATION_RC_NULL_POINTER if \p frame is NULL.
 * \retval CAN_COMMUNICATION_RC_RECEIVE_HANDLER_ERROR if libcan failed to
 *     deserialize a frame whose ID we do handle.
 */
enum CanCommunicationReturnCode can_communications_router_api_receive_primary(const struct CanCommunicationFrame *frame);

/*!
 * \brief Dispatch one frame received on the secondary network.
 *
 * \param[in] frame The frame popped off the secondary RX queue.
 *
 * \retval CAN_COMMUNICATION_RC_OK on success, or for an unhandled ID.
 * \retval CAN_COMMUNICATION_RC_NULL_POINTER if \p frame is NULL.
 */
enum CanCommunicationReturnCode can_communications_router_api_receive_secondary(const struct CanCommunicationFrame *frame);

/*!
 * \brief Queue the messages affected by a parameter transition.
 *
 * \details Called from the parameters on-change handler so a change reaches
 *     the bus immediately instead of waiting for the next cycle time. The
 *     scheduler keeps re-sending the same values afterwards.
 *
 * \param parameter_id The parameter that changed.
 * \param value The new value of the parameter.
 *
 * \retval CAN_COMMUNICATION_RC_OK on success.
 * \retval CAN_COMMUNICATION_RC_QUEUE_FULL if a TX queue is saturated.
 * \retval CAN_COMMUNICATION_RC_ERROR if libcan failed to serialize.
 */
enum CanCommunicationReturnCode can_communications_router_api_on_parameter_change(enum InputsSharedParameterID parameter_id, uint8_t value);

/*!
 * \brief Build and queue SteeringWheelButtonStatus from the current parameters.
 *
 * \details Carries the two momentary requests the ECU acts on: TS-on and
 *     push-to-talk. Sent on parameter change and by the scheduler at the
 *     libcan cycle time.
 *
 * \retval CAN_COMMUNICATION_RC_OK on success.
 * \retval CAN_COMMUNICATION_RC_QUEUE_FULL if the TX queue is saturated.
 * \retval CAN_COMMUNICATION_RC_ERROR if libcan failed to serialize.
 */
enum CanCommunicationReturnCode can_communications_router_api_send_button_status(void);

/*!
 * \brief Build and queue ControlMapsSet from the current parameters.
 *
 * \details The three maps are 0..INPUTS_SHARED_PARAMETER_NUMERIC_MAX on the
 *     wheel but normalized 0..1 floats in libcan, which re-quantizes them
 *     to the same ten steps on the wire. Sent on parameter change and by the
 *     scheduler at the libcan cycle time.
 *
 * \retval CAN_COMMUNICATION_RC_OK on success.
 * \retval CAN_COMMUNICATION_RC_QUEUE_FULL if the TX queue is saturated.
 * \retval CAN_COMMUNICATION_RC_ERROR if libcan failed to serialize.
 */
enum CanCommunicationReturnCode can_communications_router_api_send_control_maps(void);

/*!
 * \brief Check if a reset has been requested via CAN (same ID as the OpenBLT RX).
 *
 * \retval true if a reset has been requested.
 * \retval false if no reset has been requested.
 */
bool can_communications_router_api_reset_asked(void);

#endif // CAN_COMMUNICATIONS_ROUTER_API_H
