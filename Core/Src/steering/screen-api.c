/*!
 * \file screen-api.c
 * \date 2026-03-14
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief Lifecycle wrapper around the main dashboard, the parameter-change
 *     popup and the FLASH page. The actual layouts live in dashboard-api.c,
 *     popup-api.c and splash-api.c; this file only orchestrates init, raster
 *     swaps and the per-field setter facade.
 */

#include "screen-api.h"
#include "dashboard-api.h"
#include "eagletrt.h"
#include "popup-api.h"
#include "raster-api.h"
#include "raster-fonts.h"
#include "splash-api.h"
#include <stddef.h>
#include <string.h>

EAGLETRT_STATIC struct ScreenHandler screen_handler;

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
enum ScreenReturnCode screen_api_on_parameter_change(const enum InputsSharedParameterID parameter_id, uint8_t value, uint32_t tick) {
    // Called from the parameters on-change path: only update the popup state
    // here. The raster interface swap is performed by screen_api_update in
    // the render task. The timeout starts from the change itself, not from the
    // last render: renders can be late or not running at all.
    if (popup_api_show(&screen_handler.popup, parameter_id, value, tick) != POPUP_RC_OK) {
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
    if (splash_api_init(&screen_handler.flash, &font_inter) != SPLASH_RC_OK) {
        return SCREEN_RC_ERROR;
    }
    screen_handler.mounted = SCREEN_INTERFACE_DASHBOARD;
    screen_handler.flash_visible = false;
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
 *     one interface to another the new boxes carry whatever flags were left
 *     behind by the previous render (usually all false), so without this
 *     pass the freshly-mounted interface would not be redrawn until each
 *     individual box is touched again.
 */
EAGLETRT_STATIC void prv_screen_api_dirty_all(struct Box *boxes, uint16_t count) {
    for (uint16_t i = 0U; i < count; i++) {
        boxes[i].updated = true;
    }
}

/*!
 * \brief Render the mounted interface, keeping both framebuffers in step.
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
    const bool draw_logo = (screen_handler.mounted == SCREEN_INTERFACE_FLASH) && boxes[SPLASH_BOX_LOGO].updated;

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
 * \brief Mount an interface on the raster, fully dirty.
 *
 * \param interface The interface to mount.
 *
 * \retval SCREEN_RC_OK on success.
 * \retval SCREEN_RC_ERROR if the raster rejected it.
 */
EAGLETRT_STATIC enum ScreenReturnCode prv_screen_api_mount(enum ScreenInterface interface) {
    struct Box *boxes = screen_handler.dashboard.boxes;
    uint16_t count = DASHBOARD_BOX_COUNT;
    switch (interface) {
        case SCREEN_INTERFACE_POPUP:
            boxes = screen_handler.popup.boxes;
            count = POPUP_BOX_COUNT;
            break;
        case SCREEN_INTERFACE_FLASH:
            boxes = screen_handler.flash.boxes;
            count = SPLASH_BOX_COUNT;
            break;
        case SCREEN_INTERFACE_DASHBOARD:
        default:
            break;
    }

    if (raster_api_set_interface(&screen_handler.raster, boxes, count) != RASTER_RC_OK) {
        return SCREEN_RC_ERROR;
    }
    // Everything is redrawn, in both framebuffers over two renders.
    prv_screen_api_dirty_all(boxes, count);
    memset(screen_handler.redraw_in_back_buffer, 0, sizeof(screen_handler.redraw_in_back_buffer));
    screen_handler.mounted = interface;
    return SCREEN_RC_OK;
}

enum ScreenReturnCode screen_api_update(uint32_t tick) {
    const bool popup_active = popup_api_is_active(&screen_handler.popup, tick);

    // FLASH hides everything, the popup hides the dashboard.
    enum ScreenInterface wanted = SCREEN_INTERFACE_DASHBOARD;
    if (screen_handler.flash_visible) {
        wanted = SCREEN_INTERFACE_FLASH;
    } else if (popup_active) {
        wanted = SCREEN_INTERFACE_POPUP;
    }

    if (wanted != screen_handler.mounted && prv_screen_api_mount(wanted) != SCREEN_RC_OK) {
        return SCREEN_RC_ERROR;
    }

    return prv_screen_api_render();
}

enum ScreenReturnCode screen_api_set_flash(bool visible, const char *board_name) {
    screen_handler.flash_visible = visible;
    if (!visible) {
        return SCREEN_RC_OK;
    }
    return (splash_api_set_caption(&screen_handler.flash, SCREEN_FLASH_TITLE, board_name) == SPLASH_RC_OK) ? SCREEN_RC_OK : SCREEN_RC_ERROR;
}

/*!
 * \brief Translate a DashboardReturnCode into a ScreenReturnCode.
 */
EAGLETRT_STATIC enum ScreenReturnCode prv_screen_api_forward_return_code(enum DashboardReturnCode return_code) {
    return (return_code == DASHBOARD_RC_OK) ? SCREEN_RC_OK : SCREEN_RC_ERROR;
}

enum ScreenReturnCode screen_api_set_car_state(const char *text) {
    return prv_screen_api_forward_return_code(dashboard_api_set_car_state(&screen_handler.dashboard, text));
}

enum ScreenReturnCode screen_api_set_power(uint8_t value) {
    return prv_screen_api_forward_return_code(dashboard_api_set_power(&screen_handler.dashboard, value));
}

enum ScreenReturnCode screen_api_set_regen(uint8_t value) {
    return prv_screen_api_forward_return_code(dashboard_api_set_regen(&screen_handler.dashboard, value));
}

enum ScreenReturnCode screen_api_set_torque(uint8_t value) {
    return prv_screen_api_forward_return_code(dashboard_api_set_torque(&screen_handler.dashboard, value));
}

enum ScreenReturnCode screen_api_set_slip(bool slip_on) {
    return prv_screen_api_forward_return_code(dashboard_api_set_slip(&screen_handler.dashboard, slip_on));
}

enum ScreenReturnCode screen_api_set_ptt(bool active) {
    return prv_screen_api_forward_return_code(dashboard_api_set_ptt(&screen_handler.dashboard, active));
}

enum ScreenReturnCode screen_api_set_soc(uint8_t percent) {
    return prv_screen_api_forward_return_code(dashboard_api_set_soc(&screen_handler.dashboard, percent));
}

enum ScreenReturnCode screen_api_set_hv_temperature(int16_t celsius) {
    return prv_screen_api_forward_return_code(dashboard_api_set_hv_temperature(&screen_handler.dashboard, celsius));
}

enum ScreenReturnCode screen_api_set_inv_temperature(int16_t celsius) {
    return prv_screen_api_forward_return_code(dashboard_api_set_inverter_temperature(&screen_handler.dashboard, celsius));
}

enum ScreenReturnCode screen_api_set_lap(uint8_t current, uint8_t total) {
    return prv_screen_api_forward_return_code(dashboard_api_set_lap(&screen_handler.dashboard, current, total));
}

enum ScreenReturnCode screen_api_set_lap_delta_ms(int32_t delta_ms) {
    return prv_screen_api_forward_return_code(dashboard_api_set_lap_delta_ms(&screen_handler.dashboard, delta_ms));
}

enum ScreenReturnCode screen_api_set_tire_temperatures(int16_t front_left, int16_t front_right, int16_t rear_left, int16_t rear_right) {
    return prv_screen_api_forward_return_code(dashboard_api_set_tire_temperatures(&screen_handler.dashboard, front_left, front_right, rear_left, rear_right));
}

enum ScreenReturnCode screen_api_set_motor_temperatures(int16_t front_left, int16_t front_right, int16_t rear_left, int16_t rear_right) {
    return prv_screen_api_forward_return_code(dashboard_api_set_motor_temperatures(&screen_handler.dashboard, front_left, front_right, rear_left, rear_right));
}

/*!
 * \brief Human-readable name for a UIDataVehicleState value.
 *
 * \details Unknown / out-of-range values fall back to "----" so the dashboard
 *     never shows a NULL pointer.
 */
EAGLETRT_STATIC const char *prv_screen_api_vehicle_state_name(uint8_t state) {
    switch ((enum UIDataVehicleState)state) {
        case UI_DATA_VEHICLE_STATE_IDLE:
            return "IDLE";
        case UI_DATA_VEHICLE_STATE_PRECHARGE:
            return "PRECHARGE";
        case UI_DATA_VEHICLE_STATE_DISCHARGE:
            return "DISCHARGE";
        case UI_DATA_VEHICLE_STATE_TSON:
            return "TSON";
        case UI_DATA_VEHICLE_STATE_DRIVE:
            return "DRIVE";
        case UI_DATA_VEHICLE_STATE_AUTONOMOUS:
            return "AUTONOMOUS";
        case UI_DATA_VEHICLE_STATE_FLASH:
            return "FLASH";
        case UI_DATA_VEHICLE_STATE_ERROR:
            return "ERROR";
        case UI_DATA_VEHICLE_STATE_COUNT:
        default:
            return "----";
    }
}

enum ScreenReturnCode screen_api_sync_data(const struct UIData *ui_data) {
    if (ui_data == NULL) {
        return SCREEN_RC_NULL_POINTER;
    }

    enum ScreenReturnCode return_code = SCREEN_RC_OK;

    if (screen_api_set_car_state(prv_screen_api_vehicle_state_name(ui_data->vehicle_state)) != SCREEN_RC_OK) {
        return_code = SCREEN_RC_ERROR;
    }

    if (screen_api_set_power(ui_data->power) != SCREEN_RC_OK) {
        return_code = SCREEN_RC_ERROR;
    }
    if (screen_api_set_regen(ui_data->regen) != SCREEN_RC_OK) {
        return_code = SCREEN_RC_ERROR;
    }
    if (screen_api_set_torque(ui_data->torque) != SCREEN_RC_OK) {
        return_code = SCREEN_RC_ERROR;
    }
    if (screen_api_set_slip(ui_data->slip_on != 0U) != SCREEN_RC_OK) {
        return_code = SCREEN_RC_ERROR;
    }
    if (screen_api_set_ptt(ui_data->ptt != 0U) != SCREEN_RC_OK) {
        return_code = SCREEN_RC_ERROR;
    }

    if (screen_api_set_soc(ui_data->soc) != SCREEN_RC_OK) {
        return_code = SCREEN_RC_ERROR;
    }
    if (screen_api_set_hv_temperature(ui_data->hv_temp) != SCREEN_RC_OK) {
        return_code = SCREEN_RC_ERROR;
    }
    if (screen_api_set_inv_temperature(ui_data->inverter_temp) != SCREEN_RC_OK) {
        return_code = SCREEN_RC_ERROR;
    }

    if (screen_api_set_lap(ui_data->lap_current, ui_data->lap_total) != SCREEN_RC_OK) {
        return_code = SCREEN_RC_ERROR;
    }
    if (screen_api_set_lap_delta_ms(ui_data->lap_delta_ms) != SCREEN_RC_OK) {
        return_code = SCREEN_RC_ERROR;
    }

    if (screen_api_set_tire_temperatures(ui_data->tire_fl_temp, ui_data->tire_fr_temp, ui_data->tire_rl_temp, ui_data->tire_rr_temp) != SCREEN_RC_OK) {
        return_code = SCREEN_RC_ERROR;
    }
    if (screen_api_set_motor_temperatures(ui_data->motor_fl_temp, ui_data->motor_fr_temp, ui_data->motor_rl_temp, ui_data->motor_rr_temp) != SCREEN_RC_OK) {
        return_code = SCREEN_RC_ERROR;
    }

    return return_code;
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
