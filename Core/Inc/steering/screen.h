#ifndef SCREEN_H
#define SCREEN_H

#include "dashboard.h"
#include "menu.h"
#include "popup.h"
#include "raster.h"
#include "splash.h"
#include "eagletrt-api.h"
#include <stdbool.h>
#include <stddef.h>

#define SCREEN_WIDTH (800U)
#define SCREEN_HEIGHT (480U)

#define SCREEN_POPUP_TIMEOUT_MS (1500U)
#define SCREEN_FLASH_TITLE "FLASHING"            /*!< First caption line of the FLASH page */
#define SCREEN_PLACEHOLDER_TITLE "NO DESIGN YET" /*!< First caption line of the placeholder page */

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
 * \brief Boxes of the biggest view.
 */
#define SCREEN_MAX_BOX_COUNT (EAGLETRT_API_MAX(EAGLETRT_API_MAX(EAGLETRT_API_MAX(DASHBOARD_BOX_COUNT, POPUP_BOX_COUNT), SPLASH_BOX_COUNT), MENU_BOX_COUNT))

/*!
 * \brief Pages the screen can show.
 *
 * \details One row each in the view table of screen-api.c. POPUP is an
 *     overlay: it is never selected, it covers a view that allows it while a
 *     parameter change is recent.
 */
enum ScreenViewId : uint8_t {
    SCREEN_VIEW_DRIVER,      /*!< Main telemetry dashboard */
    SCREEN_VIEW_FLASH,       /*!< Logo + "FLASHING <board>" while another board is flashed */
    SCREEN_VIEW_PLACEHOLDER, /*!< Logo + page name, for modes whose design isn't ready (TODO(design)) */
    SCREEN_VIEW_LOGO,        /*!< Logo only: boot and screen saver */
    SCREEN_VIEW_MENU,        /*!< Menu page */
    SCREEN_VIEW_POPUP,       /*!< Parameter-change popup, overlay only */
    SCREEN_VIEW_COUNT,
};

/*!
 * \brief Main screen handler structure
 */
struct ScreenHandler {
    enum ScreenViewId view;    /*!< View selected by screen_api_set_view */
    enum ScreenViewId mounted; /*!< View currently mounted on the raster (view or the popup over it) */

    /*!
     * Boxes of the mounted view drawn into one framebuffer by the last
     * render and still to be drawn into the other one (double buffering
     * without copying whole frames: see screen_api_update).
     */
    bool redraw_in_back_buffer[SCREEN_MAX_BOX_COUNT];

    struct RasterHandler raster;       /*!< Handler for raster operations */
    struct DashboardHandler dashboard; /*!< Driver view (telemetry, scenario, lap) */
    struct PopupHandler popup;         /*!< State of the parameter-change popup overlay */
    struct SplashHandler flash;        /*!< FLASH page: logo + "FLASHING" + board name */
    struct SplashHandler placeholder;  /*!< Placeholder page: logo + "NO DESIGN YET" + page name */
    struct SplashHandler logo;         /*!< Logo-only page (boot, screen saver) */
    struct MenuHandler menu;           /*!< Menu page and its cursor */
};

#endif // SCREEN_H
