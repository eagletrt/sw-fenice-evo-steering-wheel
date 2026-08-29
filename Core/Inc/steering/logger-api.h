/*!
 * \file logger-api.h
 * \author Dorijan Di Zepp
 * \date 2026-06-14
 * \brief API interface for initializing and writing to the system logger.
 */

#ifndef LOGGER_API_H
#define LOGGER_API_H

#include "logger.h"

/*!
 * \brief Initializes the logger storage and binds it to an active PAL context instance.
 * \note The struct \ref PalHandler passed to this module should be dedicated solely to 
 * the physical interface used for logging (e.g. UART). 
 * If your system interfaces with separate physical protocols (such as a CAN bus), 
 * a distinct, separate \ref PalHandler instance must be initialized for that peripheral 
 * to prevent transmission queue collisions and conflicts.
 * \param[in] pal_h Pointer to the initialized PAL configuration tracking block.
 * \param[in] logger_state Runtime flag: true to enable log output immediately, false to initialize muted.
 * \retval LOGGER_RC_OK if setup succeeds.
 * \retval LOGGER_RC_NULL_POINTER if the input pointer \p pal_h is \c NULL.
 */
enum LoggerReturnCode logger_api_init(struct PalHandler *pal_h, bool logger_state);

/*!
 * \brief Modifies the runtime state of the logger dynamically.
 * \details Can be used to change printing behavior at runtime, such as enabling 
 * full logging during testing or limiting it during production loops.
 * \param[in] logger_state Target state to enable (true) or disable (false) logging.
 */
void logger_api_set_state(bool logger_state);

/*!
 * \brief Dispatches a formatted log message over the registered PAL UART interface.
 * \details Leverages C variable arguments to provide printf-style formatting.
 * \param[in] level Severity level of the outgoing record.
 * \param[in] format Standard printf-style format configuration string.
 * \param[in] ... Variable argument listing.
 * \retval LOGGER_RC_OK Operation completed successfully and data was processed by the PAL, or the logger is disabled.
 * \retval LOGGER_RC_NULL_POINTER An invalid NULL pointer was passed for the format string or the internal module handler is uninitialized.
 * \retval LOGGER_RC_TRANSMISSION_ERROR String generation failed or the downstream PAL physical hardware transmission encountered a failure.
 * \retval LOGGER_RC_BUFFER_FULL The underlying PAL transmission queue or logging buffer has become saturated and cannot accept new records.
 */
enum LoggerReturnCode logger_api_log(enum LoggerLevel level, const char *format, ...);

#endif