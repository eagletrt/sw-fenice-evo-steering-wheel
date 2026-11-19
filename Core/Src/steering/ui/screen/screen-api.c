/*!
 * \file screen-api.c
 * \date 2026-03-14
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief Lifecycle wrapper around the screen's views: the dashboard, the
 *     parameter-change popup and the splash pages (FLASH, placeholder). The
 *     actual layouts live in dashboard-api.c, popup-api.c and splash-api.c;
 *     this file only holds the view table and orchestrates init, view
 *     switches, raster swaps and the UI data sync.
 */

#include "ui/screen/screen-api.h"
#include "ui/dashboard/dashboard-api.h"
#include "ui/debug/wheel/debug-wheel-api.h"
#include "ui/menu/menu-api.h"
#include "eagletrt.h"
#include "ui/popup/popup-api.h"
#include "raster-api.h"
#include "raster-fonts.h"
#include "ui/splash/splash-api.h"
#include <stddef.h>
#include <string.h>

EAGLETRT_STATIC struct ScreenHandler screen_handler;

/*!
 * \brief A page of the screen: what the raster mounts when it is shown.
 */
struct ScreenView {
    struct Box *boxes;  /*!< Boxes mounted as the raster interface */
    uint16_t box_count; /*!< Number of \c boxes */
    bool popup;         /*!< A recent parameter change shows the popup over it */
    bool logo;          /*!< Splash page: the logo is drawn over SPLASH_BOX_LOGO after it */
};

/*!
 * \brief Every view, indexed by ScreenViewId: a new page is a new row.
 */
EAGLETRT_STATIC const struct ScreenView screen_views[SCREEN_VIEW_COUNT] = {
    [SCREEN_VIEW_DRIVER] = { screen_handler.dashboard.boxes, DASHBOARD_BOX_COUNT, true, false },
    [SCREEN_VIEW_FLASH] = { screen_handler.flash.boxes, SPLASH_BOX_COUNT, false, true },
    [SCREEN_VIEW_PLACEHOLDER] = { screen_handler.placeholder.boxes, SPLASH_BOX_COUNT, false, true },
    // TODO(#15): Figma centres the screen-saver logo (y 152), the boot logo is at y 112.
    [SCREEN_VIEW_LOGO] = { screen_handler.logo.boxes, SPLASH_BOX_COUNT, false, true },
    [SCREEN_VIEW_MENU] = { screen_handler.menu.boxes, MENU_BOX_COUNT, false, false },
    [SCREEN_VIEW_DEBUG] = { screen_handler.debug.boxes, DEBUG_WHEEL_BOX_COUNT, false, false },
    [SCREEN_VIEW_POPUP] = { screen_handler.popup.boxes, POPUP_BOX_COUNT, false, false },
};

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
enum ScreenReturnCode screen_api_on_parameter_change(const enum InputsSharedParameterID parameter_id, uint8_t value) {
    // Called from the parameters on-change path: only update the popup state
    // here. The raster interface swap is performed by screen_api_update in
    // the render task. The timeout starts from the change itself, not from the
    // last render: renders can be late or not running at all.
    if (popup_api_show(&screen_handler.popup, parameter_id, value) != POPUP_RC_OK) {
        return SCREEN_RC_ERROR;
    }
    return SCREEN_RC_OK;
}

enum ScreenReturnCode screen_api_init(raster_draw_rectangle_callback draw_rectangle) {
    if (dashboard_api_init(&screen_handler.dashboard) != DASHBOARD_RC_OK) {
        return SCREEN_RC_ERROR;
    }
    if (raster_api_init(&screen_handler.raster, screen_handler.dashboard.boxes, DASHBOARD_BOX_COUNT, draw_rectangle, NULL) != RASTER_RC_OK) {
        return SCREEN_RC_ERROR;
    }
    if (popup_api_init(&screen_handler.popup) != POPUP_RC_OK) {
        return SCREEN_RC_ERROR;
    }
    if (splash_api_init(&screen_handler.flash, &font_inter) != SPLASH_RC_OK ||
        splash_api_init(&screen_handler.placeholder, &font_inter) != SPLASH_RC_OK ||
        splash_api_init(&screen_handler.logo, &font_inter) != SPLASH_RC_OK ||
        menu_api_init(&screen_handler.menu, &font_inter) != MENU_RC_OK ||
        debug_wheel_api_init(&screen_handler.debug) != DEBUG_WHEEL_RC_OK) {
        return SCREEN_RC_ERROR;
    }
    screen_handler.view = SCREEN_VIEW_DRIVER;
    screen_handler.mounted = SCREEN_VIEW_DRIVER;
    memset(screen_handler.redraw_in_back_buffer, 0, sizeof(screen_handler.redraw_in_back_buffer));

    return SCREEN_RC_OK;
}

enum ScreenReturnCode screen_api_set_accelerators(const struct ScreenAccelerators *accelerators) {
    if (accelerators == NULL) {
        return SCREEN_RC_NULL_POINTER;
    }
    if (raster_api_set_mask(&screen_handler.raster, accelerators->draw_mask, accelerators->mask_buffer, accelerators->mask_size) != RASTER_RC_OK) {
        return SCREEN_RC_ERROR;
    }
    return SCREEN_RC_OK;
}

/*!
 * \brief Flag every box of an interface as updated.
 *
 * \details The raster clears \c updated after rendering. When we swap from
 *     one view to another the new boxes carry whatever flags were left
 *     behind by the previous render (usually all false), so without this
 *     pass the freshly-mounted view would not be redrawn until each
 *     individual box is touched again.
 */
EAGLETRT_STATIC void prv_screen_api_dirty_all(struct Box *boxes, uint16_t count) {
    for (uint16_t i = 0U; i < count; i++) {
        boxes[i].updated = true;
    }
}

/*!
 * \brief Render the mounted view, keeping both framebuffers in step.
 *
 * \details The two framebuffers alternate on every swap, and the one about to
 *     be drawn into is one frame behind: it misses whatever the previous
 *     render drew into the other one. Instead of copying the whole visible
 *     frame into it after each swap (1.5 MB through the SDRAM, ~60 ms with
 *     the LTDC scanning the same memory), the boxes drawn last time are drawn
 *     again here, together with the ones that changed since. Nothing renders
 *     while nothing changed: the pending boxes simply wait for the next
 *     change.
 *
 * \retval SCREEN_RC_RENDERED if a frame was drawn (the caller swaps).
 * \retval SCREEN_RC_OK if nothing changed.
 * \retval SCREEN_RC_ERROR if the raster failed.
 */
EAGLETRT_STATIC enum ScreenReturnCode prv_screen_api_render(void) {
    struct Box *boxes = screen_handler.raster.interface;
    const uint16_t count = screen_handler.raster.box_count;

    if (!screen_api_should_redraw(boxes, count)) {
        return SCREEN_RC_OK;
    }

    bool changed[SCREEN_MAX_BOX_COUNT];
    for (uint16_t i = 0U; i < count; i++) {
        changed[i] = boxes[i].updated;
        boxes[i].updated = boxes[i].updated || screen_handler.redraw_in_back_buffer[i];
    }
    // The logo isn't a box: it goes on top of its box every time the box fill erases it.
    const bool draw_logo = screen_views[screen_handler.mounted].logo && boxes[SPLASH_BOX_LOGO].updated;

    if (raster_api_render(&screen_handler.raster) != RASTER_RC_OK) {
        return SCREEN_RC_ERROR;
    }
    if (draw_logo && splash_api_draw_logo(&screen_handler.raster) != SPLASH_RC_OK) {
        return SCREEN_RC_ERROR;
    }

    // Only what changed now still misses from the other framebuffer: the
    // boxes redrawn because of the previous render are now in both.
    memcpy(screen_handler.redraw_in_back_buffer, changed, (size_t)count * sizeof(changed[0]));
    return SCREEN_RC_RENDERED;
}

/*!
 * \brief Mount a view on the raster, fully dirty.
 *
 * \param view The view to mount.
 *
 * \retval SCREEN_RC_OK on success.
 * \retval SCREEN_RC_ERROR if the raster rejected it.
 */
EAGLETRT_STATIC enum ScreenReturnCode prv_screen_api_mount(enum ScreenViewId view) {
    const struct ScreenView *page = &screen_views[view];
    if (raster_api_set_interface(&screen_handler.raster, page->boxes, page->box_count) != RASTER_RC_OK) {
        return SCREEN_RC_ERROR;
    }
    // Everything is redrawn, in both framebuffers over two renders.
    prv_screen_api_dirty_all(page->boxes, page->box_count);
    memset(screen_handler.redraw_in_back_buffer, 0, sizeof(screen_handler.redraw_in_back_buffer));
    screen_handler.mounted = view;
    return SCREEN_RC_OK;
}

enum ScreenReturnCode screen_api_update(void) {
    const bool popup_active = popup_api_is_active(&screen_handler.popup);
    const enum ScreenViewId wanted = (popup_active && screen_views[screen_handler.view].popup) ? SCREEN_VIEW_POPUP : screen_handler.view;

    if (wanted != screen_handler.mounted && prv_screen_api_mount(wanted) != SCREEN_RC_OK) {
        return SCREEN_RC_ERROR;
    }

    return prv_screen_api_render();
}

enum ScreenReturnCode screen_api_set_view(enum ScreenViewId view, const char *caption) {
    enum SplashReturnCode return_code = SPLASH_RC_OK;
    switch (view) {
        case SCREEN_VIEW_DRIVER:
        case SCREEN_VIEW_LOGO:
        case SCREEN_VIEW_MENU:
        case SCREEN_VIEW_DEBUG:
            break;
        case SCREEN_VIEW_FLASH:
            return_code = splash_api_set_caption(&screen_handler.flash, STYLE_FLASH_TITLE, caption);
            break;
        case SCREEN_VIEW_PLACEHOLDER:
            return_code = splash_api_set_caption(&screen_handler.placeholder, STYLE_PLACEHOLDER_TITLE, caption);
            break;
        case SCREEN_VIEW_POPUP:
        case SCREEN_VIEW_COUNT:
        default:
            return SCREEN_RC_ERROR;
    }
    if (return_code != SPLASH_RC_OK) {
        return SCREEN_RC_ERROR;
    }
    screen_handler.view = view;
    return SCREEN_RC_OK;
}

enum ScreenReturnCode screen_api_sync_data(const struct UIData *ui_data) {
    if (ui_data == NULL) {
        return SCREEN_RC_NULL_POINTER;
    }
    // D22 + A4: only an ECU known to be IDLE unlocks the menu (unknown or stale is not IDLE).
    menu_api_set_unlocked(&screen_handler.menu, ui_data->vehicle_state == UI_DATA_VEHICLE_STATE_IDLE);
    return (dashboard_api_sync(&screen_handler.dashboard, ui_data) == DASHBOARD_RC_OK) ? SCREEN_RC_OK : SCREEN_RC_ERROR;
}

enum ScreenReturnCode screen_api_sync_health(const struct WheelHealth *health) {
    return (debug_wheel_api_sync(&screen_handler.debug, health) == DEBUG_WHEEL_RC_OK) ? SCREEN_RC_OK : SCREEN_RC_NULL_POINTER;
}

bool screen_api_should_redraw(struct Box *boxes, uint16_t count) {
    if (boxes == NULL) {
        return false;
    }

    for (uint16_t i = 0U; i < count; i++) {
        if (boxes[i].updated) {
            return true;
        }
    }

    return false;
}

void screen_api_menu_open(void) {
    menu_api_open(&screen_handler.menu);
}

enum MenuAction screen_api_menu_handle(enum MenuInput input, int8_t amount) {
    return menu_api_handle(&screen_handler.menu, input, amount);
}

bool screen_api_menu_is_mission_open(void) {
    return screen_handler.menu.mission_open;
}
