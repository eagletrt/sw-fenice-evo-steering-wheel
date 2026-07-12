/*!
 * \file ktd2052.h
 * \date 2026-07-13
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief This file defines constants and return codes for the KTD2052 4-channel RGB LED driver.
 *
 * \details The steering wheel carries two KTD2052 controllers on the same
 *     I2C bus (one per LED half). Each controller drives up to 4 RGB
 *     modules; this wheel uses 3 modules per controller. The driver is
 *     hardware-agnostic: all bus traffic goes through a user-supplied
 *     register-write callback.
 */

#ifndef KTD2052_H
#define KTD2052_H

#include <stdint.h>

#define KTD2052_REGISTER_CONTROL (0x02U) /*!< Control register address. */

/*!
 * \brief Base address of the first RGB module's red-current register.
 *
 * \details Color registers are laid out per module as red, green, blue
 *     (IRED_n, IGRN_n, IBLU_n), 3 consecutive registers per module starting
 *     at 0x03.
 */
#define KTD2052_REGISTER_COLOR_BASE (0x03U)

/*!
 * \brief Number of RGB modules driven by one controller.
 */
#define KTD2052_MODULE_COUNT (4U)

/*!
 * \brief Control register value used at init.
 *
 * \details Bits [7:6] = 10 (normal mode, 0-24mA range), bit 5 = 0
 *     (BrightExtend disabled), bits [4:3] = 11 (CoolExtend at 90 degrees C),
 *     bits [2:0] = 111 (4s fade time constant).
 */
#define KTD2052_CONTROL_NORMAL_MODE (0b10011111U)

/*!
 * \brief Control register value that shuts the controller down.
 *
 * \details Bits [7:6] = 00 fade every channel to zero and power off.
 */
#define KTD2052_CONTROL_OFF (0x00U)

/*!
 * \brief Return codes for KTD2052 operations.
 */
enum Ktd2052ReturnCode {
    KTD2052_RC_OK,             /*!< Operation successful. */
    KTD2052_RC_NULL_POINTER,   /*!< A null pointer was passed to a function. */
    KTD2052_RC_INVALID_MODULE, /*!< The RGB module index is out of range. */
    KTD2052_RC_BUS_ERROR,      /*!< The register-write callback reported a failure. */
};

/*!
 * \brief Callback invoked for every register write.
 *
 * \details Wraps the vendor I2C call (typically HAL_I2C_Mem_Write on the
 *     STM32H7). The device address is the 8-bit (already shifted) I2C
 *     address passed at init time.
 *
 * \param device_address 8-bit I2C address of the controller.
 * \param register_address Register to write.
 * \param value Value to write into the register.
 *
 * \retval KTD2052_RC_OK on success.
 * \retval KTD2052_RC_BUS_ERROR on a bus failure.
 */
typedef enum Ktd2052ReturnCode (*ktd2052_write_register_callback)(uint8_t device_address, uint8_t register_address, uint8_t value);

/*!
 * \brief Handler for one KTD2052 controller.
 */
struct Ktd2052Handler {
    uint8_t device_address;                         /*!< 8-bit I2C address of the controller */
    ktd2052_write_register_callback write_register; /*!< User-supplied register-write callback */
};

#endif // KTD2052_H
