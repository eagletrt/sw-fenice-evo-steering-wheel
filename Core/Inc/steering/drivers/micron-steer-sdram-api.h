#ifndef MICRON_STEER_SDRAM_API_H
#define MICRON_STEER_SDRAM_API_H

#include "micron-steer-sdram.h"

/*!
 * \brief Initializes the MicronSteer SDRAM context
 *
 * This function initializes the MicronSteer SDRAM context with the provided delay, send command, and program refresh rate callbacks.
 *
 * \param[in] context The MicronSteer SDRAM context to initialize
 * \param[in] delay_callback The callback function to use for delays
 * \param[in] send_cmd_callback The callback function to use for sending commands
 * \param[in] program_refresh_rate_callback The callback function to use for programming the refresh rate
 *
 * \return The return code of the operation
 */
enum MicronSteerSdramReturnCode micron_steer_sdram_api_init(struct MicronSteerSdramContext *context, micron_steer_sdram_api_delay_callback delay_callback, micron_steer_sdram_api_send_command_callback send_command_callback, micron_steer_sdram_api_program_refresh_rate_callback program_refresh_rate_callback);

/*!
 * \brief Enters the power-down mode of the MicronSteer SDRAM
 *
 * \param[in] context The MicronSteer SDRAM context
 * \param[in] send_command_callback The callback function to use for sending commands
 *
 * \return The return code of the operation
 */
enum MicronSteerSdramReturnCode micron_steer_sdram_api_enter_power_mode(struct MicronSteerSdramContext *context, micron_steer_sdram_api_send_command_callback send_command_callback);

/*!
 * \brief Exits the power-down mode of the MicronSteer SDRAM
 *
 * \param[in] context The MicronSteer SDRAM context
 * \param[in] send_command_callback The callback function to use for sending commands
 *
 * \return The return code of the operation
 */
enum MicronSteerSdramReturnCode micron_steer_sdram_api_exit_power_mode(struct MicronSteerSdramContext *context, micron_steer_sdram_api_send_command_callback send_command_callback);

#endif // MICRON_STEER_SDRAM_API_H
