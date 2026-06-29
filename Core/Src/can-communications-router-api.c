#include "can-communications-router-api.h"

#include "can-primary-api.h"
#include "can-primary.h"
#include "wheelstate.h"

enum CanCommunicationReturnCode can_communications_router_api_receive_primary(const struct CanCommunicationFrame *frame) {
    if (frame == NULL) {
        return CAN_COMMUNICATION_RC_NULL_POINTER;
    }

    union CanPrimaryMessages message = {0};
    if (can_primary_api_deserialize_from_id(frame->id, (uint8_t *)frame->data, &message) != -1) {
        return CAN_COMMUNICATION_RC_ERROR;
    }

    switch (frame->id) {
        case CAN_PRIMARY_MESSAGE_FRAME_ID_ECU_STATUS: {
            switch (message.ecu_status.name) {
                case CAN_PRIMARY_ECU_STATUS_NAME_WAIT_DRIVER: {
                    
                }
            }
        }
    }

    return CAN_COMMUNICATION_RC_OK;
}

enum CanCommunicationReturnCode can_communications_router_api_receive_secondary(const struct CanCommunicationFrame *frame) {
    if (frame == NULL) {
        return CAN_COMMUNICATION_RC_NULL_POINTER;
    }

    // TODO: add libcan deserialization and dispatch logic here

    return CAN_COMMUNICATION_RC_OK;
}
