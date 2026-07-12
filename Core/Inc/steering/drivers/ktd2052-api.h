/*!
 * \file ktd2052-api.h
 * \date 2026-07-13
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief This file defines the API for driving one KTD2052 4-channel RGB LED controller.
 */

#ifndef KTD2052_API_H
#define KTD2052_API_H

#include "ktd2052.h"

/*!
 * \brief Initialize a KTD2052 handler and put the controller in normal mode.
 *
 * \param handler Handler to initialize.
 * \param device_address 8-bit (already shifted) I2C address of the controller.
 * \param write_register Callback wrapping the bus register write.
 *
 * \retval KTD2052_RC_OK on success.
 * \retval KTD2052_RC_NULL_POINTER if \p handler or \p write_register is NULL.
 * \retval KTD2052_RC_BUS_ERROR if the control register write fails.
 */
enum Ktd2052ReturnCode ktd2052_api_init(struct Ktd2052Handler *handler, uint8_t device_address, ktd2052_write_register_callback write_register);

/*!
 * \brief Set the color of one RGB module.
 *
 * \param handler Handler of the target controller.
 * \param module RGB module index, 0-based (0..KTD2052_MODULE_COUNT-1).
 * \param red Red current setting (0-255).
 * \param green Green current setting (0-255).
 * \param blue Blue current setting (0-255).
 *
 * \retval KTD2052_RC_OK on success.
 * \retval KTD2052_RC_NULL_POINTER if \p handler is NULL or not initialized.
 * \retval KTD2052_RC_INVALID_MODULE if \p module is out of range.
 * \retval KTD2052_RC_BUS_ERROR if any register write fails.
 */
enum Ktd2052ReturnCode ktd2052_api_set_color(struct Ktd2052Handler *handler, uint8_t module, uint8_t red, uint8_t green, uint8_t blue);

/*!
 * \brief Shut the controller down (fade every channel to zero and power off).
 *
 * \param handler Handler of the target controller.
 *
 * \retval KTD2052_RC_OK on success.
 * \retval KTD2052_RC_NULL_POINTER if \p handler is NULL or not initialized.
 * \retval KTD2052_RC_BUS_ERROR if the control register write fails.
 */
enum Ktd2052ReturnCode ktd2052_api_shutdown(struct Ktd2052Handler *handler);

#endif // KTD2052_API_H
