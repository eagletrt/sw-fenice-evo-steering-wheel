/*!
 * \file dashboard-api.h
 * \date 2026-06-07
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief Public API to initialize and update the main dashboard.
 *
 * \details Re-format the target field's text
 *     buffer in place and flag the corresponding box as updated. The render
 *     itself is driven by the screen module through libraster.
 */

#ifndef DASHBOARD_API_H
#define DASHBOARD_API_H

#include "dashboard.h"
#include <stdbool.h>
#include <stdint.h>

/*!
 * \brief Build every box, label and value buffer of the dashboard interface.
 *
 * \details Layout and styling come from a static table inside the
 *     implementation file. Each field starts with a sensible placeholder
 *     (e.g. "POW --") so the screen is readable before any real data has
 *     been pushed.
 *
 * \param[out] handler Storage to populate.
 *
 * \retval DASHBOARD_RC_OK on success.
 * \retval DASHBOARD_RC_NULL_POINTER if \p handler is NULL.
 * \retval DASHBOARD_RC_ERROR if libraster reports a
 *     failure while initializing a box or label.
 */
enum DashboardReturnCode dashboard_api_init(struct DashboardHandler *handler);

/*!
 * \brief Update the car state text shown in the center-top box.
 *
 * \param[in,out] handler Dashboard storage.
 * \param[in]     text    NULL-terminated text to display (truncated to fit).
 *
 * \retval DASHBOARD_RC_OK on success.
 * \retval DASHBOARD_RC_NULL_POINTER if \p handler or \p text is NULL.
 */
enum DashboardReturnCode dashboard_api_set_car_state(struct DashboardHandler *handler, const char *text);

/*!
 * \brief Update the power level (0..10) in the left strip.
 *
 * \param[in,out] handler Dashboard storage.
 * \param[in]     value   Level value, expected in the 0..10 range; printed as-is.
 *
 * \retval DASHBOARD_RC_OK on success.
 * \retval DASHBOARD_RC_NULL_POINTER if \p handler is NULL.
 */
enum DashboardReturnCode dashboard_api_set_power(struct DashboardHandler *handler, uint8_t value);

/*!
 * \brief Update the regen level (0..10) in the left strip.
 *
 * \param[in,out] handler Dashboard storage.
 * \param[in]     value   Level value, expected in the 0..10 range; printed as-is.
 *
 * \retval DASHBOARD_RC_OK on success.
 * \retval DASHBOARD_RC_NULL_POINTER if \p handler is NULL.
 */
enum DashboardReturnCode dashboard_api_set_regen(struct DashboardHandler *handler, uint8_t value);

/*!
 * \brief Update the torque level (0..10) in the left strip.
 *
 * \param[in,out] handler Dashboard storage.
 * \param[in]     value   Level value, expected in the 0..10 range; printed as-is.
 *
 * \retval DASHBOARD_RC_OK on success.
 * \retval DASHBOARD_RC_NULL_POINTER if \p handler is NULL.
 */
enum DashboardReturnCode dashboard_api_set_torque(struct DashboardHandler *handler, uint8_t value);

/*!
 * \brief Update the slip toggle in the left strip.
 *
 * \param[in,out] handler  Dashboard storage.
 * \param[in]     slip_on  true → "SLIP ON", false → "SLIP OFF".
 *
 * \retval DASHBOARD_RC_OK on success.
 * \retval DASHBOARD_RC_NULL_POINTER if \p handler is NULL.
 */
enum DashboardReturnCode dashboard_api_set_slip(struct DashboardHandler *handler, bool slip_on);

/*!
 * \brief Tint the SCENARIO header to show push-to-talk is keyed.
 *
 * \details PTT has no value of its own on the dashboard, so it borrows the
 *     header of the block it sits above: blue while the driver holds a top
 *     paddle, back to the normal section colour on release.
 *
 * \param handler Dashboard to update.
 * \param active true while push-to-talk is keyed.
 *
 * \retval DASHBOARD_RC_OK on success.
 * \retval DASHBOARD_RC_NULL_POINTER if \p handler is NULL.
 */
enum DashboardReturnCode dashboard_api_set_ptt(struct DashboardHandler *handler, bool active);

/*!
 * \brief Update the HV state-of-charge percentage (large center value).
 *
 * \param[in,out] handler Dashboard storage.
 * \param[in]     percent SoC in 0..100, clamped on display.
 *
 * \retval DASHBOARD_RC_OK on success.
 * \retval DASHBOARD_RC_NULL_POINTER if \p handler is NULL.
 */
enum DashboardReturnCode dashboard_api_set_soc(struct DashboardHandler *handler, uint8_t percent);

/*!
 * \brief Update the HV pack temperature shown under the SoC.
 *
 * \param[in,out] handler Dashboard storage.
 * \param[in]     celsius Temperature in °C; negative values are rendered with a sign.
 *
 * \retval DASHBOARD_RC_OK on success.
 * \retval DASHBOARD_RC_NULL_POINTER if \p handler is NULL.
 */
enum DashboardReturnCode dashboard_api_set_hv_temperature(struct DashboardHandler *handler, int16_t celsius);

/*!
 * \brief Update the inverter temperature shown at center-bottom.
 *
 * \param[in,out] handler Dashboard storage.
 * \param[in]     celsius Temperature in °C; negative values are rendered with a sign.
 *
 * \retval DASHBOARD_RC_OK on success.
 * \retval DASHBOARD_RC_NULL_POINTER if \p handler is NULL.
 */
enum DashboardReturnCode dashboard_api_set_inverter_temperature(struct DashboardHandler *handler, int16_t celsius);

/*!
 * \brief Update the current / total lap counter shown at right-top.
 *
 * \param[in,out] handler Dashboard storage.
 * \param[in]     current Current lap number.
 * \param[in]     total   Total number of laps for the session.
 *
 * \retval DASHBOARD_RC_OK on success.
 * \retval DASHBOARD_RC_NULL_POINTER if \p handler is NULL.
 */
enum DashboardReturnCode dashboard_api_set_lap(struct DashboardHandler *handler, uint8_t current, uint8_t total);

/*!
 * \brief Update the lap delta in milliseconds (printed as ±s.mmm).
 *
 * \param[in,out] handler Dashboard storage.
 * \param[in]     delta_ms Signed delta relative to the reference lap, in ms.
 *
 * \retval DASHBOARD_RC_OK on success.
 * \retval DASHBOARD_RC_NULL_POINTER if \p handler is NULL.
 */
enum DashboardReturnCode dashboard_api_set_lap_delta_ms(struct DashboardHandler *handler, int32_t delta_ms);

/*!
 * \brief Update the four tire temperatures shown under the TRS header.
 *
 * \param[in,out] handler      Dashboard storage.
 * \param[in]     front_left   Front-left in °C.
 * \param[in]     front_right  Front-right in °C.
 * \param[in]     rear_left    Rear-left in °C.
 * \param[in]     rear_right   Rear-right in °C.
 *
 * \retval DASHBOARD_RC_OK on success.
 * \retval DASHBOARD_RC_NULL_POINTER if \p handler is NULL.
 */
enum DashboardReturnCode dashboard_api_set_tire_temperatures(struct DashboardHandler *handler, int16_t front_left, int16_t front_right, int16_t rear_left, int16_t rear_right);

/*!
 * \brief Update the four motor temperatures shown under the MTR header.
 *
 * \param[in,out] handler      Dashboard storage.
 * \param[in]     front_left   Front-left in °C.
 * \param[in]     front_right  Front-right in °C.
 * \param[in]     rear_left    Rear-left in °C.
 * \param[in]     rear_right   Rear-right in °C.
 *
 * \retval DASHBOARD_RC_OK on success.
 * \retval DASHBOARD_RC_NULL_POINTER if \p handler is NULL.
 */
enum DashboardReturnCode dashboard_api_set_motor_temperatures(struct DashboardHandler *handler, int16_t front_left, int16_t front_right, int16_t rear_left, int16_t rear_right);

#endif // DASHBOARD_API_H
