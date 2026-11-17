/*!
 * \file mcp23017.h
 * \date 2026-07-13
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief This file defines constants and return codes for the MCP23017 16-bit I2C GPIO expander.
 *
 * \details Two MCP23017 expanders sit on the steering wheel I2C bus and
 *     carry every push button and rotary-switch contact. The driver is
 *     hardware-agnostic: all bus traffic goes through user-supplied
 *     register read/write callbacks. Every pin is used as an input with
 *     the internal pull-up enabled, so a pressed button / closed contact
 *     reads as 0.
 */

#ifndef MCP23017_H
#define MCP23017_H

#include <stdint.h>

#define MCP23017_REGISTER_IODIRA (0x00U)   /*!< I/O direction, port A */
#define MCP23017_REGISTER_IODIRB (0x01U)   /*!< I/O direction, port B */
#define MCP23017_REGISTER_IPOLA (0x02U)    /*!< Input polarity, port A */
#define MCP23017_REGISTER_IPOLB (0x03U)    /*!< Input polarity, port B */
#define MCP23017_REGISTER_GPINTENA (0x04U) /*!< Interrupt-on-change enable, port A */
#define MCP23017_REGISTER_GPINTENB (0x05U) /*!< Interrupt-on-change enable, port B */
#define MCP23017_REGISTER_DEFVALA (0x06U)  /*!< Interrupt default compare, port A */
#define MCP23017_REGISTER_DEFVALB (0x07U)  /*!< Interrupt default compare, port B */
#define MCP23017_REGISTER_INTCONA (0x08U)  /*!< Interrupt control, port A */
#define MCP23017_REGISTER_INTCONB (0x09U)  /*!< Interrupt control, port B */
#define MCP23017_REGISTER_GPPUA (0x0CU)    /*!< Pull-up enable, port A */
#define MCP23017_REGISTER_GPPUB (0x0DU)    /*!< Pull-up enable, port B */
#define MCP23017_REGISTER_GPIOA (0x12U)    /*!< Port A input value */
#define MCP23017_REGISTER_GPIOB (0x13U)    /*!< Port B input value */

/*!
 * \brief Mask enabling the pull-up on every pin of a port.
 */
#define MCP23017_PULL_UP_ALL (0xFFU)

/*!
 * \brief GPIO ports of the expander.
 */
enum Mcp23017Port {
    MCP23017_PORT_A, /*!< Port A (GPA0..GPA7) */
    MCP23017_PORT_B, /*!< Port B (GPB0..GPB7) */
    MCP23017_PORT_COUNT,
};

/*!
 * \brief Return codes for MCP23017 operations.
 */
enum Mcp23017ReturnCode {
    MCP23017_RC_OK,           /*!< Operation successful. */
    MCP23017_RC_NULL_POINTER, /*!< A null pointer was passed to a function. */
    MCP23017_RC_INVALID_PORT, /*!< The port identifier is out of range. */
    MCP23017_RC_BUS_ERROR,    /*!< A register callback reported a failure. */
    MCP23017_RC_VERIFY_ERROR, /*!< A configuration read-back did not match what was written. */
};

/*!
 * \brief Callback invoked for every register read.
 *
 * \param device_address 8-bit (already shifted) I2C address of the expander.
 * \param register_address Register to read.
 * \param value Output slot for the register content.
 *
 * \retval MCP23017_RC_OK on success.
 * \retval MCP23017_RC_BUS_ERROR on a bus failure.
 */
typedef enum Mcp23017ReturnCode (*mcp23017_read_register_callback)(uint8_t device_address, uint8_t register_address, uint8_t *value);

/*!
 * \brief Callback invoked for every register write.
 *
 * \param device_address 8-bit (already shifted) I2C address of the expander.
 * \param register_address Register to write.
 * \param value Value to write into the register.
 *
 * \retval MCP23017_RC_OK on success.
 * \retval MCP23017_RC_BUS_ERROR on a bus failure.
 */
typedef enum Mcp23017ReturnCode (*mcp23017_write_register_callback)(uint8_t device_address, uint8_t register_address, uint8_t value);

/*!
 * \brief Handler for one MCP23017 expander.
 */
struct Mcp23017Handler {
    uint8_t device_address;                          /*!< 8-bit I2C address of the expander */
    mcp23017_read_register_callback read_register;   /*!< User-supplied register-read callback */
    mcp23017_write_register_callback write_register; /*!< User-supplied register-write callback */
};

#endif // MCP23017_H
