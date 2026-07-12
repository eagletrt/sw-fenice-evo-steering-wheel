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
 * \details Stamps the current tick and refreshes the popup labels. The
 *     raster interface swap is deferred to screen_update so that rendering
 *     stays out of the input-handling path.
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
 * \details Builds the dashboard layout, the popup overlay and mounts the
 *     dashboard on the raster. The popup starts inactive.
 *
 * \param draw_rectangle Raster rendering callback provided by the caller.
 *
 * \retval SCREEN_RC_OK if initialization was successful.
 * \retval SCREEN_RC_ERROR if initialization failed.
 */
enum ScreenReturnCode screen_api_init(raster_draw_rectangle_callback draw_rectangle);

/*!
 * \brief Update the screen state, including popup visibility and raster interface.
 *
 * \details Checks whether the popup should still be active based on the current
 *     tick, and updates the raster interface accordingly.
 *
 * \param tick Current tick count in milliseconds, used for popup timeout checks.
 *
 * \retval SCREEN_RC_OK if the screen was updated successfully.
 * \retval SCREEN_RC_ERROR if an error occurred during update.
 */
enum ScreenReturnCode screen_api_update(uint32_t tick);

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

#endif // SCREEN_API_H
