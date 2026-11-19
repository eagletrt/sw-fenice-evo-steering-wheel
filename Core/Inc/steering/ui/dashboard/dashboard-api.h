/*!
 * \file dashboard-api.h
 * \date 2026-06-07
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief Public API to initialize and update the driver dashboard.
 */

#ifndef DASHBOARD_API_H
#define DASHBOARD_API_H

#include "ui/dashboard/dashboard.h"
#include "ui/data/ui-data.h"

/*!
 * \brief Build every box, label and text buffer of the dashboard.
 *
 * \details Values start as "--" until the first dashboard_api_sync.
 *
 * \param[out] handler Storage to populate.
 *
 * \retval DASHBOARD_RC_OK on success.
 * \retval DASHBOARD_RC_NULL_POINTER if \p handler is NULL.
 * \retval DASHBOARD_RC_ERROR if libraster rejected a box or a label.
 */
enum DashboardReturnCode dashboard_api_init(struct DashboardHandler *handler);

/*!
 * \brief Show a UI data snapshot.
 *
 * \details Formats every value, applies the colour rules and "--" for stale
 *     sources. Only boxes whose text, colour or geometry changes are flagged,
 *     so calling it every frame is cheap.
 *
 * \param[in,out] handler Dashboard storage.
 * \param[in] ui_data The snapshot.
 *
 * \retval DASHBOARD_RC_OK on success.
 * \retval DASHBOARD_RC_NULL_POINTER if \p handler or \p ui_data is NULL.
 */
enum DashboardReturnCode dashboard_api_sync(struct DashboardHandler *handler, const struct UIData *ui_data);

#endif // DASHBOARD_API_H
