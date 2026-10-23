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
    SCREEN_RC_RENDERED,     /*!< The screen was rendered successfully */
    SCREEN_RC_NULL_POINTER, /*!< A required pointer argument was NULL */
    SCREEN_RC_ERROR,        /*!< An error occurred during the operation */
};

/*!
 * \brief Optional hardware accelerators for the screen.
 *
 * \details All fields may be NULL/0, the screen then keeps the plain
 *     rectangle path. With \c draw_mask + \c mask_buffer/\c mask_size labels
 *     are blended as one coverage mask each (libraster raster_api_set_mask)
 *     instead of one rectangle per anti-aliased run.
 */
struct ScreenAccelerators {
    raster_draw_mask_callback draw_mask; /*!< Mask blend, NULL to keep rectangles only */
    uint8_t *mask_buffer;                /*!< Scratch for the coverage masks */
    uint32_t mask_size;                  /*!< Size of \c mask_buffer, in bytes */
};

/*!
 * \brief Boxes of the biggest interface (dashboard or popup).
 */
#define SCREEN_MAX_BOX_COUNT (DASHBOARD_BOX_COUNT > POPUP_BOX_COUNT ? DASHBOARD_BOX_COUNT : POPUP_BOX_COUNT)

/*!
 * \brief Main screen handler structure
 */
struct ScreenHandler {
    bool popup_visible; /*!< Whether the popup interface is currently mounted on the raster */

    /*!
     * Boxes of the mounted interface drawn into one framebuffer by the last
     * render and still to be drawn into the other one (double buffering
     * without copying whole frames: see screen_api_update).
     */
    bool redraw_in_back_buffer[SCREEN_MAX_BOX_COUNT];

    struct RasterHandler raster;       /*!< Handler for raster operations */
    struct DashboardHandler dashboard; /*!< Main always-on interface (telemetry, scenario, lap) */
    struct PopupHandler popup;         /*!< State of the parameter-change popup overlay */
};

#endif // SCREEN_H
