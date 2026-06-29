#include "can-communications-router-api.h"

#include "can-primary-api.h"
#include "can-primary.h"
#include "wheelstate.h"

enum CanCommunicationReturnCode can_communications_router_api_receive_primary(const struct CanCommunicationFrame *frame) {
    if (frame == NULL) {
        return CAN_COMMUNICATION_RC_NULL_POINTER;
    }

    if (!can_primary_api_id_is_valid(frame->id)) {
        return CAN_COMMUNICATION_RC_ERROR;
    }

    union CanPrimaryMessages message = {0};
    if (can_primary_api_deserialize_from_id(frame->id, (uint8_t *)frame->data, &message) != -1) {
        return CAN_COMMUNICATION_RC_ERROR;
    }

    switch (frame->id) {
        case CAN_PRIMARY_MESSAGE_FRAME_ID_ECU_STATUS: {
            switch (message.ecu_status.name) {
                case CAN_PRIMARY_ECU_STATUS_NAME_INIT:
                case CAN_PRIMARY_ECU_STATUS_NAME_ENABLE_INV_UPDATES:
                case CAN_PRIMARY_ECU_STATUS_NAME_CHECK_INV_SETTINGS:
                case CAN_PRIMARY_ECU_STATUS_NAME_FATAL_ERROR:
                case CAN_PRIMARY_ECU_STATUS_NAME_DRIVE: {
                    wheel_state_set_tson(CAN_PRIMARY_STEERING_WHEEL_SET_ECU_STATUS_TARGETSTATUS_IDLE);
                    break;
                }
                case CAN_PRIMARY_ECU_STATUS_NAME_IDLE: {
                    wheel_state_set_tson(CAN_PRIMARY_STEERING_WHEEL_SET_ECU_STATUS_TARGETSTATUS_READY);
                    break;
                }
                case CAN_PRIMARY_ECU_STATUS_NAME_WAIT_DRIVER: {
                    wheel_state_set_tson(CAN_PRIMARY_STEERING_WHEEL_SET_ECU_STATUS_TARGETSTATUS_DRIVE);
                    break;
                }
                case CAN_PRIMARY_ECU_STATUS_NAME_ENABLE_INV_DRIVE:
                case CAN_PRIMARY_ECU_STATUS_NAME_DISABLE_INV_DRIVE:
                case CAN_PRIMARY_ECU_STATUS_NAME_START_TS_DISCHARGE:
                case CAN_PRIMARY_ECU_STATUS_NAME_RE_ENABLE_INV_DRIVE:
                case CAN_PRIMARY_ECU_STATUS_NAME_WAIT_TS_DISCHARGE:
                case CAN_PRIMARY_ECU_STATUS_NAME_START_TS_PRECHARGE:
                case CAN_PRIMARY_ECU_STATUS_NAME_WAIT_TS_PRECHARGE: {
                    break;
                }
            }
            break;
        }
        case CAN_PRIMARY_MESSAGE_FRAME_ID_STEERING_WHEEL_FLASH: {
            wheel_state_set_request_reset();
            break;
        }
        default: {
            break;
        }
    }

    return CAN_COMMUNICATION_RC_OK;
}

enum CanCommunicationReturnCode can_communications_router_api_receive_secondary(const struct CanCommunicationFrame *frame) {
    if (frame == NULL) {
        return CAN_COMMUNICATION_RC_NULL_POINTER;
    }

    return CAN_COMMUNICATION_RC_OK;
}
