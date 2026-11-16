/*!
 * \file screen-api.h
 * \date 2026-03-14
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief Public screen API: lifecycle plus per-field setters for the main
 *     dashboard.
 *
 * \details Lifecycle (init/update/parameter-change handling) lives here; the
 *     actual layout of the dashboard is implemented in dashboard-api.c. The
 *     screen_set_* functions are thin forwarders that target the dashboard
 *     instance owned by the file-static ScreenHandler.
 */

#ifndef SCREEN_API_H
#define SCREEN_API_H

#include "ui-data.h"
#include "screen.h"
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
 * \brief Pull every dashboard value from the UI data snapshot.
 *
 * \details Reads each field of \p ui_data and forwards it to the matching
 *     screen_api_set_* call. Setters are no-ops when the formatted text didn't
 *     change, so calling screen_api_sync_data every tick is cheap.
 *
 * \param ui_data Pointer to the UIData snapshot.
 *
 * \retval SCREEN_RC_OK on success.
 * \retval SCREEN_RC_NULL_POINTER if \p ui_data is NULL.
 * \retval SCREEN_RC_ERROR if any setter failed.
 */
enum ScreenReturnCode screen_api_sync_data(const struct UIData *ui_data);

/*!
 * \brief Update the car state text shown in the center-top box.
 *
 * \param text NULL-terminated text to display (truncated to fit).
 *
 * \retval SCREEN_RC_OK on success.
 * \retval SCREEN_RC_NULL_POINTER if \p text is NULL.
 */
enum ScreenReturnCode screen_api_set_car_state(const char *text);

/*!
 * \brief Update the power level (0..10) in the left strip.
 *
 * \param value Power level to display, from 0 (no power) to 10 (full power).
 *
 * \retval SCREEN_RC_OK on success.
 */
enum ScreenReturnCode screen_api_set_power(uint8_t value);

/*!
 * \brief Update the regen level (0..10) in the left strip.
 *
 * \param value Regen level to display, from 0 (no regen) to 10 (full regen).
 *
 * \retval SCREEN_RC_OK on success.
 */
enum ScreenReturnCode screen_api_set_regen(uint8_t value);

/*!
 * \brief Update the power level (0..10) in the left strip.
 *
 * \param value Torque level to display, from 0 (no torque) to 10 (full torque).
 *
 * \retval SCREEN_RC_OK on success.
 */
enum ScreenReturnCode screen_api_set_torque(uint8_t value);

/*!
 * \brief Update the slip/TC toggle in the left strip.
 *
 * \param slip_on Whether slip/TC is active (on) or not (off).
 *
 * \retval SCREEN_RC_OK on success.
 */
enum ScreenReturnCode screen_api_set_slip(bool slip_on);

/*!
 * \brief Show or clear the push-to-talk indication on the dashboard.
 *
 * \param active true while push-to-talk is keyed.
 *
 * \retval SCREEN_RC_OK on success.
 */
enum ScreenReturnCode screen_api_set_ptt(bool active);

/*!
 * \brief Update the HV state-of-charge percentage (large center value).
 *
 * \param percent SoC percentage to display, from 0 to 100.
 *
 * \retval SCREEN_RC_OK on success.
 */
enum ScreenReturnCode screen_api_set_soc(uint8_t percent);

/*!
 * \brief Update the HV pack temperature shown under the SoC.
 *
 * \param celsius Temperature to display in °C (negative values allowed).
 *
 * \retval SCREEN_RC_OK on success.
 */
enum ScreenReturnCode screen_api_set_hv_temperature(int16_t celsius);

/*!
 * \brief Update the inverter temperature shown at center-bottom.
 *
 * \param celsius Temperature to display in °C (negative values allowed).
 *
 * \retval SCREEN_RC_OK on success.
 */
enum ScreenReturnCode screen_api_set_inv_temperature(int16_t celsius);

/*!
 * \brief Update the current / total lap counter shown at right-top.
 *
 * \param current Current lap number, from 1 to total.
 *
 * \retval SCREEN_RC_OK on success.
 */
enum ScreenReturnCode screen_api_set_lap(uint8_t current, uint8_t total);

/*!
 * \brief Update the lap delta in milliseconds (printed as ±s.mmm).
 *
 * \param delta_ms Lap delta to display, in milliseconds. Positive values are
 *
 * \retval SCREEN_RC_OK on success.
 */
enum ScreenReturnCode screen_api_set_lap_delta_ms(int32_t delta_ms);

/*!
 * \brief Update the four tire temperatures shown under the TRS header.
 *
 * \param front_left  Front-left temperature in °C.
 * \param front_right Front-right temperature in °C.
 * \param rear_left   Rear-left temperature in °C.
 * \param rear_right  Rear-right temperature in °C.
 *
 * \retval SCREEN_RC_OK on success.
 */
enum ScreenReturnCode screen_api_set_tire_temperatures(int16_t front_left, int16_t front_right, int16_t rear_left, int16_t rear_right);

/*!
 * \brief Update the four motor temperatures shown under the MTR header.
 *
 * \param front_left  Front-left temperature in °C.
 * \param front_right Front-right temperature in °C.
 * \param rear_left   Rear-left temperature in °C.
 * \param rear_right  Rear-right temperature in °C.
 *
 * \retval SCREEN_RC_OK on success.
 */
enum ScreenReturnCode screen_api_set_motor_temperatures(int16_t front_left, int16_t front_right, int16_t rear_left, int16_t rear_right);

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
