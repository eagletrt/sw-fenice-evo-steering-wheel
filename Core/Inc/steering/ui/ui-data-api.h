/*!
 * \file ui-data-api.h
 * \date 2026-07-13
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief Accessor for the dashboard data snapshot.
 *
 * \details The UIData instance lives behind the API so producers and the
 *     screen module share a single well-known storage without every module
 *     owning an extern. Callers go through ui_data_api_get and read or
 *     write the fields directly.
 */

#ifndef UI_DATA_API_H
#define UI_DATA_API_H

#include "ui-data.h"

/*!
 * \brief Return the shared UIData instance.
 *
 * \return Pointer to the UIData snapshot.
 */
struct UIData *ui_data_api_get(void);

#endif // UI_DATA_API_H
