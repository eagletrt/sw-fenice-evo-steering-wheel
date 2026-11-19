/*!
 * \file debug-wheel-api.h
 * \date 2026-11-17
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief Public API of the WHEEL debug page.
 */

#ifndef DEBUG_WHEEL_API_H
#define DEBUG_WHEEL_API_H

#include "ui/debug/wheel/debug-wheel.h"

/*!
 * \brief Build every box of the page, values "--" until the first sync.
 *
 * \param[out] handler Storage to populate.
 *
 * \retval DEBUG_WHEEL_RC_OK on success.
 * \retval DEBUG_WHEEL_RC_NULL_POINTER if \p handler is NULL.
 * \retval DEBUG_WHEEL_RC_ERROR if libraster rejected a box or a label.
 */
enum DebugWheelReturnCode debug_wheel_api_init(struct DebugWheelHandler *handler);

/*!
 * \brief Show a health snapshot.
 *
 * \details Colours: bus ACTIVE green, PASSIVE yellow, BUS-OFF red; any error
 *     counter above 0, a CAN error code and a BOR/WDG reset yellow; a full Tx
 *     FIFO (0 free) red. Only boxes whose text or colour changes are flagged.
 *
 * \param[in,out] handler Page storage.
 * \param[in] health The snapshot.
 *
 * \retval DEBUG_WHEEL_RC_OK on success.
 * \retval DEBUG_WHEEL_RC_NULL_POINTER if an argument is NULL.
 */
enum DebugWheelReturnCode debug_wheel_api_sync(struct DebugWheelHandler *handler, const struct WheelHealth *health);

#endif // DEBUG_WHEEL_API_H
