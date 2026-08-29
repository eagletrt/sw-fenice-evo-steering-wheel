/*!
 * \file logger-api.c
 * \author Dorijan Di Zepp
 * \date 2026-07-01
 * \brief API execution handling for logging records.
 */

#include "logger-api.h"
#include "eagletrt-api.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#define LOGGER_MAX_LINE_SIZE (64U)

/*!
 * \brief Internal module handler.
 * \details Hidden from external linkage to enforce API-only access.
 */
EAGLETRT_STATIC struct LoggerHandler logger_handler;

enum LoggerReturnCode logger_api_init(struct PalHandler *pal_h, bool logger_state) {
    if (pal_h == NULL) {
        return LOGGER_RC_NULL_POINTER;
    }

    logger_handler.logger_state = logger_state;
    logger_handler.pal_handler = pal_h;

    return LOGGER_RC_OK;
}

void logger_api_set_state(bool logger_state) {
    logger_handler.logger_state = logger_state;
}

enum LoggerReturnCode logger_api_log(enum LoggerLevel level, const char *format, ...) {
    constexpr int16_t length_of_closing_characters = 3; // '\n', '\r', and '\0'

    if (logger_handler.pal_handler == NULL || format == NULL) {
        return LOGGER_RC_NULL_POINTER;
    }

    // Return OK immediately if the logger is currently muted/disabled
    if (!logger_handler.logger_state) {
        return LOGGER_RC_OK;
    }

    char final_buffer[LOGGER_MAX_LINE_SIZE];

    // Lookup table replacing the switch-case matrix
    char *const log_headers[] = {
        [LOGGER_LEVEL_DEBUG] = "[DEBUG]",
        [LOGGER_LEVEL_INFO] = "[INFO]",
        [LOGGER_LEVEL_WARN] = "[WARN]",
        [LOGGER_LEVEL_ERROR] = "[ERROR]"
    };

    // Safely check bounds using LOGGER_LEVEL_COUNT before accessing memory
    // For default [LOG] will be left in case the logger level specified is not valid
    const char *header = (level < LOGGER_LEVEL_COUNT) ? log_headers[level] : "[LOG]";

    int32_t offset = snprintf(final_buffer, sizeof(final_buffer), "%s ", header);

    // Verify no anomalies or truncations occurred during tag placement
    if (offset < 0 || offset >= (int)LOGGER_MAX_LINE_SIZE) {
        return LOGGER_RC_TRANSMISSION_ERROR;
    }

    // Process variable args into the remaining space of the local buffer
    va_list args;
    va_start(args, format);
    int32_t body_len = vsnprintf(final_buffer + offset, (int)LOGGER_MAX_LINE_SIZE - offset - length_of_closing_characters, format, args);
    va_end(args);

    if (body_len < 0) {
        return LOGGER_RC_TRANSMISSION_ERROR; // Format parsing exception
    }

    const int32_t max_wrote =
        (int)(LOGGER_MAX_LINE_SIZE - offset - length_of_closing_characters);

    int32_t wrote = EAGLETRT_API_MIN(body_len, max_wrote);

    // Measure the actual string safely populated inside the buffer boundary
    int32_t actual_len = offset + wrote;

    final_buffer[actual_len++] = '\n';
    final_buffer[actual_len++] = '\r';
    final_buffer[actual_len++] = '\0';

    // Queue the formatted record into PAL
    enum PalReturnCode pal_rc = pal_api_add_to_tx_queue(logger_handler.pal_handler, final_buffer, actual_len);
    if (pal_rc != PAL_RC_OK) {
        if (pal_rc == PAL_RC_QUEUE_FULL) {
            return LOGGER_RC_BUFFER_FULL;
        }
        return LOGGER_RC_TRANSMISSION_ERROR;
    }

    // Process/Flush the queue immediately so diagnostics output synchronously
    pal_rc = pal_api_process_tx(logger_handler.pal_handler);
    if (pal_rc != PAL_RC_OK) {
        if (pal_rc == PAL_RC_QUEUE_FULL) {
            return LOGGER_RC_BUFFER_FULL;
        }
        return LOGGER_RC_TRANSMISSION_ERROR;
    }

    return LOGGER_RC_OK;
}
