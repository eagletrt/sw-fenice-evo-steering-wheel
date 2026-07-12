/*!
 * \file ktd2052-api.c
 * \date 2026-07-13
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief Implementation of the API for driving one KTD2052 4-channel RGB LED controller.
 */

#include "ktd2052-api.h"
#include <stddef.h>

enum Ktd2052ReturnCode ktd2052_api_init(struct Ktd2052Handler *handler, uint8_t device_address, ktd2052_write_register_callback write_register) {
    if (handler == NULL || write_register == NULL) {
        return KTD2052_RC_NULL_POINTER;
    }

    handler->device_address = device_address;
    handler->write_register = write_register;

    return handler->write_register(handler->device_address, KTD2052_REGISTER_CONTROL, KTD2052_CONTROL_NORMAL_MODE);
}

enum Ktd2052ReturnCode ktd2052_api_set_color(struct Ktd2052Handler *handler, uint8_t module, uint8_t red, uint8_t green, uint8_t blue) {
    if (handler == NULL || handler->write_register == NULL) {
        return KTD2052_RC_NULL_POINTER;
    }
    if (module >= KTD2052_MODULE_COUNT) {
        return KTD2052_RC_INVALID_MODULE;
    }

    const uint8_t base = (uint8_t)(KTD2052_REGISTER_COLOR_BASE + (module * 3U));

    enum Ktd2052ReturnCode return_code = handler->write_register(handler->device_address, base, red);
    if (return_code != KTD2052_RC_OK) {
        return return_code;
    }
    return_code = handler->write_register(handler->device_address, (uint8_t)(base + 1U), green);
    if (return_code != KTD2052_RC_OK) {
        return return_code;
    }
    return handler->write_register(handler->device_address, (uint8_t)(base + 2U), blue);
}

enum Ktd2052ReturnCode ktd2052_api_shutdown(struct Ktd2052Handler *handler) {
    if (handler == NULL || handler->write_register == NULL) {
        return KTD2052_RC_NULL_POINTER;
    }
    return handler->write_register(handler->device_address, KTD2052_REGISTER_CONTROL, KTD2052_CONTROL_OFF);
}
