/*!
 * \file screen-api.h
 * \date 2026-03-14
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief Public screen API: lifecycle, views and the UI data sync.
 *
 * \details The layouts live in dashboard-api.c, menu-api.c, popup-api.c and
 *     splash-api.c; this module mounts the view the scheduler selects and
 *     forwards the UI data snapshot to the dashboard.
 */

#ifndef SCREEN_API_H
#define SCREEN_API_H

#include "ui/data/ui-data.h"
#include "ui/screen/screen.h"
#include <stdbool.h>
#include <stdint.h>

/*!
 * \brief Handle a parameter change event by showing the popup overlay.
 *
 * \param parameter_id The parameter that was changed.
 * \param value The new value of the parameter.
 *
 * \retval SCREEN_RC_OK on success.
 * \retval SCREEN_RC_ERROR on failure.
 */
enum ScreenReturnCode screen_api_on_parameter_change(enum InputsSharedParameterID parameter_id, uint8_t value);

/*!
 * \brief Initialize the screen interface.
 *
 * \details Builds every view and mounts the driver view on the raster.
 *     The popup starts inactive.
 *
 * \param draw_rectangle Raster rendering callback provided by the caller.
 *
 * \retval SCREEN_RC_OK if initialization was successful.
 * \retval SCREEN_RC_ERROR if initialization failed.
 */
enum ScreenReturnCode screen_api_init(raster_draw_rectangle_callback draw_rectangle);

/*!
 * \brief Enable the optional hardware accelerators (call after screen_api_init).
 *
 * \param accelerators Mask blend and frame cache callbacks; any field may be
 *     NULL to keep the corresponding fallback (see ScreenAccelerators).
 *
 * \retval SCREEN_RC_OK on success.
 * \retval SCREEN_RC_NULL_POINTER if \p accelerators is NULL.
 * \retval SCREEN_RC_ERROR if libraster rejected the mask (callback without buffer).
 */
enum ScreenReturnCode screen_api_set_accelerators(const struct ScreenAccelerators *accelerators);

/*!
 * \brief Update the screen state, including popup visibility and raster interface.
 *
 * \details Checks whether the popup should still be active and updates the raster interface accordingly.
 *
 * \retval SCREEN_RC_OK if the screen was updated but nothing had to be redrawn.
 * \retval SCREEN_RC_RENDERED if the screen was updated and the raster redrew at least one box.
 * \retval SCREEN_RC_ERROR if an error occurred during update.
 */
enum ScreenReturnCode screen_api_update(void);

/*!
 * \brief Select the view the screen shows.
 *
 * \details Mounted (fully redrawn) by the next screen_api_update when it
 *     differs from the current one; selecting the current view again only
 *     refreshes its caption, so calling this every frame is cheap. The popup
 *     still covers a view that allows it (the driver view). The caption is
 *     referenced, not copied.
 *
 * \param view The view to show (not SCREEN_VIEW_POPUP: it is an overlay).
 * \param caption Board name for SCREEN_VIEW_FLASH, page name for
 *     SCREEN_VIEW_PLACEHOLDER, NULL for none; ignored by the other views.
 *
 * \retval SCREEN_RC_OK on success.
 * \retval SCREEN_RC_ERROR if \p view can't be selected or the caption could not be set.
 */
enum ScreenReturnCode screen_api_set_view(enum ScreenViewId view, const char *caption);

/*!
 * \brief Show a UI data snapshot: dashboard values and menu locking.
 *
 * \details Only boxes whose look changes are flagged, so calling it every
 *     frame is cheap.
 *
 * \param ui_data Pointer to the UIData snapshot.
 *
 * \retval SCREEN_RC_OK on success.
 * \retval SCREEN_RC_NULL_POINTER if \p ui_data is NULL.
 * \retval SCREEN_RC_ERROR if the dashboard rejected it.
 */
enum ScreenReturnCode screen_api_sync_data(const struct UIData *ui_data);

/*!
 * \brief Show a health snapshot on the WHEEL debug page.
 *
 * \param health The snapshot.
 *
 * \retval SCREEN_RC_OK on success.
 * \retval SCREEN_RC_NULL_POINTER if \p health is NULL.
 */
enum ScreenReturnCode screen_api_sync_health(const struct WheelHealth *health);

/*!
 * \brief Check if any of the given boxes needs to be redrawn.
 *
 * \param boxes Pointer to an array of Box structures to check.
 * \param count Number of boxes in the array.
 *
 * \return true if any box needs to be redrawn, false otherwise.
 */
bool screen_api_should_redraw(struct Box *boxes, uint16_t count);

/*!
 * \brief Back to the menu's item list, cursor on the first selectable item.
 */
void screen_api_menu_open(void);

/*!
 * \brief Handle one menu input (menu_api_handle on the screen's menu).
 *
 * \param input The input.
 * \param amount MOVE only: rows to move, positive = down.
 *
 * \return What the wheel should do.
 */
enum MenuAction screen_api_menu_handle(enum MenuInput input, int8_t amount);

/*!
 * \brief Whether the menu shows the mission page instead of the item list.
 *
 * \return true while the mission page is open.
 */
bool screen_api_menu_is_mission_open(void);

#endif // SCREEN_API_H
