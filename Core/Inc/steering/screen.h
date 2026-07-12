#ifndef SCREEN_H
#define SCREEN_H

#include "dashboard.h"
#include "popup.h"
#include "raster.h"
#include <stdbool.h>
#include <stddef.h>

#define SCREEN_WIDTH (800U)
#define SCREEN_HEIGHT (480U)

#define SCREEN_POPUP_TIMEOUT_MS (1500U)

/*!
 * \brief Return codes for screen operations
 */
enum ScreenReturnCode {
    SCREEN_RC_OK,           /*!< Operation completed successfully */
    SCREEN_RC_NULL_POINTER, /*!< A required pointer argument was NULL */
    SCREEN_RC_ERROR,        /*!< An error occurred during the operation */
};

/*!
 * \brief Main screen handler structure
 */
struct ScreenHandler {
    uint32_t last_event_tick; /*!< Timestamp of the last screen event, in milliseconds */
    bool popup_visible;       /*!< Whether the popup interface is currently mounted on the raster */

    struct RasterHandler raster;       /*!< Handler for raster operations */
    struct DashboardHandler dashboard; /*!< Main always-on interface (telemetry, scenario, lap) */
    struct PopupHandler popup;         /*!< State of the parameter-change popup overlay */
};

#endif // SCREEN_H
