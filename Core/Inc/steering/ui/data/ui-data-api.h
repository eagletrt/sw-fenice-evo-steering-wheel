/*!
 * \file ui-data-api.h
 * \date 2026-07-13
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief Accessor to the shared UIData snapshot.
 *
 * \details The router, the scheduler (parameter mirror, statistics) and the
 *     screen module share a single well-known storage without every module
 *     owning an extern. Callers go through ui_data_api_get and read or
 *     write the fields directly.
 */

#ifndef UI_DATA_API_H
#define UI_DATA_API_H

#include "ui/data/ui-data.h"

/*!
 * \brief Return the shared UIData instance.
 *
 * \return Pointer to the UIData snapshot.
 */
struct UIData *ui_data_api_get(void);

/*!
 * \brief Fold a sample into the current second of a window.
 *
 * \param[in,out] window The window (NULL is ignored).
 * \param[in] sample One value per wheel (NULL is ignored).
 */
void ui_data_api_window_push(struct UIDataWindow *window, const struct UIDataCorners *sample);

/*!
 * \brief Highest value per wheel over the whole window.
 *
 * \param[in] window The window.
 *
 * \return The maxima (-FLT_MAX for a wheel with no sample in the window).
 */
struct UIDataCorners ui_data_api_window_max(const struct UIDataWindow *window);

/*!
 * \brief Start a new second in every window of the shared snapshot, dropping the oldest.
 *
 * \details Called once per second by the scheduler.
 */
void ui_data_api_rotate_windows(void);

#endif // UI_DATA_API_H
