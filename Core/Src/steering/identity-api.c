/*!
 * \file identity-api.c
 * \date 2026-08-27
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief Implementation of the steering wheel identity broadcast.
 */

#include "identity-api.h"
#include "can-primary-api.h"
#include "can-primary.h"
#include "can-version.h"
#include "can-communications-api.h"
#include "eagletrt-api.h"
#include <time.h>

EAGLETRT_STATIC struct IdentityHandler identity_handler;

/*!
 * \brief Serialize one primary message and queue it for transmission.
 *
 * \param frame_id The primary frame ID to serialize and send.
 * \param message The message union holding the payload.
 * \param length The on-bus length of the message, in bytes.
 */
EAGLETRT_STATIC void prv_identity_api_queue(enum CanPrimaryMessageFrameId frame_id, union CanPrimaryMessages *message, uint8_t length) {
    struct CanCommunicationFrame frame;
    if (can_primary_api_serialize_from_id(frame_id, message, frame.data) == -1) {
        return;
    }
    frame.id = (uint32_t)frame_id;
    frame.length = length;
    EAGLETRT_API_UNUSED(can_communications_api_add_to_tx_buffer(CAN_COMMUNICATION_NETWORK_PRIMARY, &frame));
}

enum IdentityReturnCode identity_api_init(void) {
    struct tm timeinfo = { 0 };
    strptime(__DATE__ " " __TIME__, "%b %d %Y %H:%M:%S", &timeinfo);
    identity_handler = (struct IdentityHandler){
        .build_time = (uint32_t)mktime(&timeinfo),
        .last_tick_ms_status = 0,
        .last_tick_ms_version = 0,
        .last_tick_ms_libcan_version = 0,
    };
    return IDENTITY_RC_OK;
}

enum IdentityReturnCode identity_api_send_state(enum CanPrimarySteeringwheelfsmStatus status) {
    union CanPrimaryMessages message;
    message.steeringwheelfsm = (struct CanPrimarySteeringwheelfsm){
        .status = status,
    };
    prv_identity_api_queue(
        CAN_PRIMARY_MESSAGE_FRAME_ID_STEERINGWHEELFSM,
        &message,
        can_primary_byte_size_steeringwheelfsm);
    return IDENTITY_RC_OK;
}

enum IdentityReturnCode identity_api_periodically_send_state(enum CanPrimarySteeringwheelfsmStatus status, uint32_t tick_ms) {
    if (tick_ms - identity_handler.last_tick_ms_status >= (uint32_t)can_primary_cycle_time_steeringwheelfsm) {
        identity_handler.last_tick_ms_status = tick_ms;

        EAGLETRT_API_UNUSED(identity_api_send_state(status));
    }
    return IDENTITY_RC_OK;
}

enum IdentityReturnCode identity_api_periodically_send_version(uint32_t tick_ms) {
    if (tick_ms - identity_handler.last_tick_ms_version >= (uint32_t)can_primary_cycle_time_steeringwheelversion) {
        identity_handler.last_tick_ms_version = tick_ms;

        union CanPrimaryMessages message;
        message.steeringwheelversion = (struct CanPrimarySteeringwheelversion){
            .major = IDENTITY_VERSION_MAJOR,
            .minor = IDENTITY_VERSION_MINOR,
            .patch = IDENTITY_VERSION_PATCH,
        };
        prv_identity_api_queue(
            CAN_PRIMARY_MESSAGE_FRAME_ID_STEERINGWHEELVERSION,
            &message,
            can_primary_byte_size_steeringwheelversion);

        message.steeringwheelversioninfo = (struct CanPrimarySteeringwheelversioninfo){
            .buildtime = identity_handler.build_time,
            .commithash = IDENTITY_VERSION_INFO_COMMIT_HASH,
            .dirty = IDENTITY_VERSION_DIRTY,
        };
        prv_identity_api_queue(
            CAN_PRIMARY_MESSAGE_FRAME_ID_STEERINGWHEELVERSIONINFO,
            &message,
            can_primary_byte_size_steeringwheelversioninfo);
    }
    return IDENTITY_RC_OK;
}

enum IdentityReturnCode identity_api_periodically_send_libcan_version(uint32_t tick_ms) {
    if (tick_ms - identity_handler.last_tick_ms_libcan_version >= (uint32_t)can_primary_cycle_time_steeringwheellibcanversion) {
        identity_handler.last_tick_ms_libcan_version = tick_ms;

        union CanPrimaryMessages message;
        message.steeringwheellibcanversion = (struct CanPrimarySteeringwheellibcanversion){
            .major = can_version_major,
            .minor = can_version_minor,
            .patch = can_version_patch,
        };
        prv_identity_api_queue(
            CAN_PRIMARY_MESSAGE_FRAME_ID_STEERINGWHEELLIBCANVERSION,
            &message,
            can_primary_byte_size_steeringwheellibcanversion);

        message.steeringwheellibcanversioninfo = (struct CanPrimarySteeringwheellibcanversioninfo){
            .generationtime = (uint32_t)can_generation_time,
            .commithash = 0,
            .dirty = 0,
        };
        prv_identity_api_queue(
            CAN_PRIMARY_MESSAGE_FRAME_ID_STEERINGWHEELLIBCANVERSIONINFO,
            &message,
            can_primary_byte_size_steeringwheellibcanversioninfo);
    }
    return IDENTITY_RC_OK;
}
