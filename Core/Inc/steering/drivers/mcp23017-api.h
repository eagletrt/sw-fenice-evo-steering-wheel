/*!
 * \file mcp23017-api.h
 * \date 2026-07-13
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief This file defines the API for reading one MCP23017 16-bit I2C GPIO expander.
 */

#ifndef MCP23017_API_H
#define MCP23017_API_H

#include "mcp23017.h"

/*!
 * \brief Initialize an MCP23017 handler and configure every pin as a pulled-up input.
 *
 * \details The power-on default of the expander already has every pin as an
 *     input, so init only has to enable the internal pull-ups. Each pull-up
 *     register is read back after writing so a dead or mis-addressed device
 *     is detected at startup instead of producing ghost inputs later.
 *
 * \param handler Handler to initialize.
 * \param device_address 8-bit (already shifted) I2C address of the expander.
 * \param read_register Callback wrapping the bus register read.
 * \param write_register Callback wrapping the bus register write.
 *
 * \retval MCP23017_RC_OK on success.
 * \retval MCP23017_RC_NULL_POINTER if \p handler or any callback is NULL.
 * \retval MCP23017_RC_BUS_ERROR if a register access fails.
 * \retval MCP23017_RC_VERIFY_ERROR if a pull-up read-back does not match.
 */
enum Mcp23017ReturnCode mcp23017_api_init(struct Mcp23017Handler *handler, uint8_t device_address, mcp23017_read_register_callback read_register, mcp23017_write_register_callback write_register);

/*!
 * \brief Read the current input value of one port.
 *
 * \param handler Handler of the target expander.
 * \param port Port to read.
 * \param value Output slot for the 8 input bits (1 = released / open, 0 = pressed / closed).
 *
 * \retval MCP23017_RC_OK on success.
 * \retval MCP23017_RC_NULL_POINTER if \p handler or \p value is NULL or the handler is not initialized.
 * \retval MCP23017_RC_INVALID_PORT if \p port is out of range.
 * \retval MCP23017_RC_BUS_ERROR if the register read fails.
 */
enum Mcp23017ReturnCode mcp23017_api_read_port(struct Mcp23017Handler *handler, enum Mcp23017Port port, uint8_t *value);

#endif // MCP23017_API_H
