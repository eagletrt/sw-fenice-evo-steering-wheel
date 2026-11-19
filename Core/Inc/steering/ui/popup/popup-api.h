/*!
 * \file popup-api.h
 * \date 2026-04-24
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief API for the parameter-change popup overlay.
 */

#ifndef POPUP_API_H
#define POPUP_API_H

#include "ui/popup/popup.h"

/*!
 * \brief Initialize the popup interface.
 *
 * \details Populates the popup boxes and labels with default values.
 *     The popup starts in the inactive state. Takes POPUP_TIMEOUT_COUNT
 *     timeouts from the pool (timeout_api_init first, call once).
 *
 * \param handler Pointer to the popup handler to initialize.
 *
 * \retval POPUP_RC_OK if initialization was successful.
 * \retval POPUP_RC_ERROR if \p handler is NULL or the timeout pool is full.
 */
enum PopupReturnCode popup_api_init(struct PopupHandler *handler);

/*!
 * \brief Activate (or refresh) the popup with the latest parameter value.
 *
 * \param handler Pointer to the popup handler.
 * \param parameter_id The parameter that was changed.
 * \param value The new value of the parameter.
 *
 * \retval POPUP_RC_OK if the popup was updated successfully.
 * \retval POPUP_RC_ERROR if the arguments are invalid.
 */
enum PopupReturnCode popup_api_show(struct PopupHandler *handler, enum InputsSharedParameterID parameter_id, uint8_t value);

/*!
 * \brief Deactivate the popup regardless of its current state.
 *
 * \param handler Pointer to the popup handler.
 */
void popup_api_dismiss(struct PopupHandler *handler);

/*!
 * \brief Check whether the popup should still be rendered at the given tick.
 *
 * \details Dismisses the popup if more than SCREEN_POPUP_TIMEOUT_MS milliseconds
 *     have elapsed since the last parameter change.
 *
 * \param handler Pointer to the popup handler.
 *
 * \return true if the popup is active, false otherwise.
 */
bool popup_api_is_active(struct PopupHandler *handler);

#endif // POPUP_API_H
