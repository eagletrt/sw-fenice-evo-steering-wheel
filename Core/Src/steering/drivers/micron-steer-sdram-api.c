#include <stdio.h>
#include "micron-steer-sdram-api.h"
#include "eagletrt.h"

/*!
 * \brief Enables the clock for the MicronSteer SDRAM
 *
 * \param[in] context The MicronSteer SDRAM context
 * \param[in] send_command_callback The callback function to use for sending commands
 *
 * \return The return code of the operation
 */
EAGLETRT_STATIC enum MicronSteerSdramReturnCode prv_micron_steer_sdram_api_clock_enable(struct MicronSteerSdramContext *context, micron_steer_sdram_api_send_command_callback send_command_callback) {
    if (context == NULL) {
        return MICRON_STEER_SDRAM_RC_ERROR;
    }

    struct MicronSteerSdramCommand command = {
        .command_mode = MICRON_STEER_SDRAM_CLK_ENABLE_CMD,
        .target_bank = context->target_bank,
        .auto_refresh_number = 1,
        .mode_register_definition = 0
    };

    return send_command_callback(&command);
}

/*!
 * \brief Sends a precharge command to the MicronSteer SDRAM
 *
 * \param[in] context The MicronSteer SDRAM context
 * \param[in] send_command_callback The callback function to use for sending commands
 *
 * \return The return code of the operation
 */
EAGLETRT_STATIC enum MicronSteerSdramReturnCode prv_micron_steer_sdram_api_precharge(struct MicronSteerSdramContext *context, micron_steer_sdram_api_send_command_callback send_command_callback) {
    if (context == NULL || send_command_callback == NULL) {
        return MICRON_STEER_SDRAM_RC_ERROR;
    }

    struct MicronSteerSdramCommand command = {
        .command_mode = MICRON_STEER_SDRAM_PALL_CMD,
        .target_bank = context->target_bank,
        .auto_refresh_number = 1,
        .mode_register_definition = 0
    };

    return send_command_callback(&command);
}

/*!
 * \brief Configures the mode register of the MicronSteer SDRAM
 *
 * \param[in] context The MicronSteer SDRAM context
 * \param[in] send_command_callback The callback function to use for sending commands
 *
 * \return The return code of the operation
 */
EAGLETRT_STATIC enum MicronSteerSdramReturnCode prv_micron_steer_sdram_api_mode_reg_config(struct MicronSteerSdramContext *context, micron_steer_sdram_api_send_command_callback send_command_callback) {
    if (context == NULL || send_command_callback == NULL) {
        return MICRON_STEER_SDRAM_RC_ERROR;
    }

    uint32_t mode_register_definition = (uint32_t)context->burst_length | context->burst_type | context->cas_latency | context->operation_mode | context->write_burst_mode;

    struct MicronSteerSdramCommand command = {
        .command_mode = MICRON_STEER_SDRAM_LOAD_MODE_CMD,
        .target_bank = context->target_bank,
        .auto_refresh_number = 1,
        .mode_register_definition = mode_register_definition
    };

    return send_command_callback(&command);
}

/*!
 * \brief Sends a refresh command to the MicronSteer SDRAM
 *
 * \param[in] context The MicronSteer SDRAM context
 * \param[in] send_command_callback The callback function to use for sending commands
 *
 * \return The return code of the operation
 */
EAGLETRT_STATIC enum MicronSteerSdramReturnCode prv_micron_steer_sdram_api_refresh_mode(struct MicronSteerSdramContext *context, micron_steer_sdram_api_send_command_callback send_command_callback) {
    if (context == NULL || send_command_callback == NULL) {
        return MICRON_STEER_SDRAM_RC_ERROR;
    }

    constexpr uint32_t auto_refresh_number = 8; // Number of auto-refresh commands to issue

    struct MicronSteerSdramCommand command = {
        .command_mode = MICRON_STEER_SDRAM_AUTOREFRESH_MODE_CMD,
        .target_bank = context->target_bank,
        .auto_refresh_number = auto_refresh_number,
        .mode_register_definition = 0
    };

    return send_command_callback(&command);
}

/*!
 * \brief Programs the refresh rate of the MicronSteer SDRAM
 *
 * \param[in] context The MicronSteer SDRAM context
 * \param[in] program_refresh_rate_callback The callback function to use for programming the refresh rate
 *
 * \return The return code of the operation
 */
EAGLETRT_STATIC enum MicronSteerSdramReturnCode prv_micron_steer_sdram_api_refresh_rate(struct MicronSteerSdramContext *context, micron_steer_sdram_api_program_refresh_rate_callback program_refresh_rate_callback) {
    if (context == NULL || program_refresh_rate_callback == NULL) {
        return MICRON_STEER_SDRAM_RC_ERROR;
    }

    return program_refresh_rate_callback(context->refresh_rate);
}

enum MicronSteerSdramReturnCode micron_steer_sdram_api_init(struct MicronSteerSdramContext *context, micron_steer_sdram_api_delay_callback delay_callback, micron_steer_sdram_api_send_command_callback send_command_callback, micron_steer_sdram_api_program_refresh_rate_callback program_refresh_rate_callback) {
    if (context == NULL || delay_callback == NULL) {
        return MICRON_STEER_SDRAM_RC_ERROR;
    }

    /* Step 1: Configure a clock configuration enable command */
    if (prv_micron_steer_sdram_api_clock_enable(context, send_command_callback) != MICRON_STEER_SDRAM_RC_OK) {
        return MICRON_STEER_SDRAM_RC_ERROR;
    }

    /* Step 2: Insert 100 us minimum delay */
    /* Inserted delay is equal to 1 ms due to systick time base unit (ms) */
    delay_callback(1);

    /* Step 3: Configure a PALL (precharge all) command */
    if (prv_micron_steer_sdram_api_precharge(context, send_command_callback) != MICRON_STEER_SDRAM_RC_OK) {
        return MICRON_STEER_SDRAM_RC_ERROR;
    }

    /* Step 4: Configure a Refresh command */
    if (prv_micron_steer_sdram_api_refresh_mode(context, send_command_callback) != MICRON_STEER_SDRAM_RC_OK) {
        return MICRON_STEER_SDRAM_RC_ERROR;
    }

    /* Step 5: Program the external memory mode register */
    if (prv_micron_steer_sdram_api_mode_reg_config(context, send_command_callback) != MICRON_STEER_SDRAM_RC_OK) {
        return MICRON_STEER_SDRAM_RC_ERROR;
    }

    /* Step 6: Set the refresh rate counter */
    if (prv_micron_steer_sdram_api_refresh_rate(context, program_refresh_rate_callback) != MICRON_STEER_SDRAM_RC_OK) {
        return MICRON_STEER_SDRAM_RC_ERROR;
    }

    return MICRON_STEER_SDRAM_RC_OK;
}

enum MicronSteerSdramReturnCode micron_steer_sdram_api_enter_power_mode(struct MicronSteerSdramContext *context, micron_steer_sdram_api_send_command_callback send_command_callback) {
    if (context == NULL || send_command_callback == NULL) {
        return MICRON_STEER_SDRAM_RC_ERROR;
    }

    struct MicronSteerSdramCommand command = {
        .command_mode = MICRON_STEER_SDRAM_POWERDOWN_MODE_CMD,
        .target_bank = context->target_bank,
        .auto_refresh_number = 1,
        .mode_register_definition = 0
    };

    return send_command_callback(&command);
}

enum MicronSteerSdramReturnCode micron_steer_sdram_api_exit_power_mode(struct MicronSteerSdramContext *context, micron_steer_sdram_api_send_command_callback send_command_callback) {
    if (context == NULL || send_command_callback == NULL) {
        return MICRON_STEER_SDRAM_RC_ERROR;
    }

    struct MicronSteerSdramCommand command = {
        .command_mode = MICRON_STEER_SDRAM_NORMAL_MODE_CMD,
        .target_bank = context->target_bank,
        .auto_refresh_number = 1,
        .mode_register_definition = 0
    };

    return send_command_callback(&command);
}
