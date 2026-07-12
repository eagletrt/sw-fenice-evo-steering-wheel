/*!
 * \file mcp23017-api.c
 * \date 2026-07-13
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief Implementation of the API for reading one MCP23017 16-bit I2C GPIO expander.
 */

#include "mcp23017-api.h"
#include "eagletrt.h"
#include <stddef.h>

/*!
 * \brief Enable and verify the pull-ups of one port.
 *
 * \param handler Handler of the target expander.
 * \param pull_up_register GPPUA or GPPUB.
 *
 * \retval MCP23017_RC_OK on success.
 * \retval MCP23017_RC_BUS_ERROR if a register access fails.
 * \retval MCP23017_RC_VERIFY_ERROR if the read-back does not match.
 */
EAGLETRT_STATIC enum Mcp23017ReturnCode prv_mcp23017_api_enable_pull_ups(struct Mcp23017Handler *handler, uint8_t pull_up_register) {
    enum Mcp23017ReturnCode return_code = handler->write_register(handler->device_address, pull_up_register, MCP23017_PULL_UP_ALL);
    if (return_code != MCP23017_RC_OK) {
        return return_code;
    }

    uint8_t read_back = 0U;
    return_code = handler->read_register(handler->device_address, pull_up_register, &read_back);
    if (return_code != MCP23017_RC_OK) {
        return return_code;
    }
    if (read_back != MCP23017_PULL_UP_ALL) {
        return MCP23017_RC_VERIFY_ERROR;
    }
    return MCP23017_RC_OK;
}

enum Mcp23017ReturnCode mcp23017_api_init(struct Mcp23017Handler *handler, uint8_t device_address, mcp23017_read_register_callback read_register, mcp23017_write_register_callback write_register) {
    if (handler == NULL || read_register == NULL || write_register == NULL) {
        return MCP23017_RC_NULL_POINTER;
    }

    handler->device_address = device_address;
    handler->read_register = read_register;
    handler->write_register = write_register;

    enum Mcp23017ReturnCode return_code = prv_mcp23017_api_enable_pull_ups(handler, MCP23017_REGISTER_GPPUA);
    if (return_code != MCP23017_RC_OK) {
        return return_code;
    }
    return prv_mcp23017_api_enable_pull_ups(handler, MCP23017_REGISTER_GPPUB);
}

enum Mcp23017ReturnCode mcp23017_api_read_port(struct Mcp23017Handler *handler, enum Mcp23017Port port, uint8_t *value) {
    if (handler == NULL || handler->read_register == NULL || value == NULL) {
        return MCP23017_RC_NULL_POINTER;
    }
    if (port >= MCP23017_PORT_COUNT) {
        return MCP23017_RC_INVALID_PORT;
    }

    const uint8_t gpio_register = (port == MCP23017_PORT_A) ? MCP23017_REGISTER_GPIOA : MCP23017_REGISTER_GPIOB;
    return handler->read_register(handler->device_address, gpio_register, value);
}
