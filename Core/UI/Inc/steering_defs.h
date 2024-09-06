#ifndef STEERING_DEFS
#define STEERING_DEFS

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#define SWOC_STRING_LEN (32U)

#define OLIVEC_COLOR_RED        (0xFFFF0000)
#define OLIVEC_COLOR_BLUE       (0xFF0000FF)
#define OLIVEC_COLOR_LIGHT_BLUE (0xFFADD8E6)
#define OLIVEC_COLOR_YELLOW     (0xFFFFFF00)
#define OLIVEC_COLOR_GREEN      (0xFF00FF00)
#define OLIVEC_COLOR_BLACK      (0xFF000000)
#define OLIVEC_COLOR_WHITE      (0xFFFFFFFF)
#define OLIVEC_COLOR_PURPLE     (0xFFA020F0)

#define clamp(value, min, max) ((value) < (min) ? (min) : ((value) > (max) ? (max) : (value)))

#define GET_LAST_STATE(ntw, msg, NTW, MSG) \
    ntw##_##msg##_converted_t *ntw##_##msg##_last_state = (ntw##_##msg##_converted_t *)&ntw##_messages_last_state[NTW##_##MSG##_INDEX][0]

#define STEER_CAN_UNPACK(ntw, NTW, msg_name, MSG_NAME, arr, update_cond)                                                                        \
    ntw##_##msg_name##_t raw;                                                                                                                   \
    ntw##_##msg_name##_converted_t converted;                                                                                                   \
    ntw##_##msg_name##_unpack(&raw, msg->data, NTW##_##MSG_NAME##_BYTE_SIZE);                                                                   \
    ntw##_##msg_name##_raw_to_conversion_struct(&converted, &raw);                                                                              \
    ntw##_##msg_name##_converted_t *last_state = (ntw##_##msg_name##_converted_t *)&ntw##_messages_last_state[ntw##_index_from_id(msg->id)][0]; \
    if (update_cond) {                                                                                                                          \
        memcpy(last_state, &converted, sizeof(ntw##_##msg_name##_converted_t));                                                                 \
        arr##_new[ntw##_index_from_id(NTW##_##MSG_NAME##_FRAME_ID)]   = true;                                                                   \
        arr##_valid[ntw##_index_from_id(NTW##_##MSG_NAME##_FRAME_ID)] = true;                                                                   \
    }

#define STEER_CAN_PACK(ntw, NTW, msg_name, MSG_NAME)               \
    can_message_t msg        = {0};                                \
    msg.id                   = NTW##_##MSG_NAME##_FRAME_ID;        \
    msg.size                 = NTW##_##MSG_NAME##_BYTE_SIZE;       \
    ntw##_##msg_name##_t raw = {0};                                \
    ntw##_##msg_name##_conversion_to_raw_struct(&raw, &converted); \
    ntw##_##msg_name##_pack(msg.data, &raw, NTW##_##MSG_NAME##_BYTE_SIZE);

#define PERIODIC_SEND(ntw, NTW, msg_name, MSG_NAME)                                           \
    static uint32_t last_sent_##ntw##_##msg_name = 0;                                         \
    void send_##ntw##_##msg_name();                                                           \
    if ((get_current_time_ms() - last_sent_##ntw##_##msg_name) > NTW##_INTERVAL_##MSG_NAME) { \
        last_sent_##ntw##_##msg_name             = get_current_time_ms();                     \
        ntw##_##msg_name##_converted_t converted = *ntw##_##msg_name##_last_state;            \
        STEER_CAN_PACK(ntw, NTW, msg_name, MSG_NAME)                                          \
        can_send(&msg, true);                                                                 \
    }

/***
 * Hardware dependent functions
 */
uint32_t get_current_time_ms(void);
void openblt_reset(void);
void system_reset(void);

#endif  // STEERING_DEFS
