/*!
 * \file screen-api.c
 * \date 2026-03-14
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief Lifecycle wrapper around the main dashboard and the parameter-change
 *     popup. The actual dashboard layout lives in dashboard-api.c; this file
 *     only orchestrates init, raster swaps and the per-field setter facade.
 */

#include "screen-api.h"
#include "dashboard-api.h"
#include "eagletrt.h"
#include "popup-api.h"
#include "raster-api.h"
#include <stddef.h>

EAGLETRT_STATIC struct ScreenHandler screen_handler;

enum ScreenReturnCode screen_api_on_parameter_change(const enum InputsSharedParameterID parameter_id, uint8_t value) {
    // Called from the parameters on-change path: only update the popup state
    // here. The raster interface swap is performed by screen_update in the
    // FSM idle loop.
    if (popup_api_show(&screen_handler.popup, parameter_id, value, screen_handler.last_event_tick) != POPUP_RC_OK) {
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
    screen_handler.popup_visible = false;
    screen_handler.last_event_tick = 0U;

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

enum ScreenReturnCode screen_api_update(uint32_t tick) {
    screen_handler.last_event_tick = tick;

    bool popup_active = popup_api_is_active(&screen_handler.popup, tick);

    if (popup_active && !screen_handler.popup_visible) {
        if (raster_api_set_interface(&screen_handler.raster, screen_handler.popup.boxes, POPUP_BOX_COUNT) != RASTER_RC_OK) {
            return SCREEN_RC_ERROR;
        }
        prv_screen_api_dirty_all(screen_handler.popup.boxes, POPUP_BOX_COUNT);
        screen_handler.popup_visible = true;
    } else if (!popup_active && screen_handler.popup_visible) {
        if (raster_api_set_interface(&screen_handler.raster, screen_handler.dashboard.boxes, DASHBOARD_BOX_COUNT) != RASTER_RC_OK) {
            return SCREEN_RC_ERROR;
        }
        prv_screen_api_dirty_all(screen_handler.dashboard.boxes, DASHBOARD_BOX_COUNT);
        screen_handler.popup_visible = false;
    }

    if (screen_api_should_redraw(screen_handler.raster.interface, screen_handler.raster.box_count)) {
        if (raster_api_render(&screen_handler.raster) != RASTER_RC_OK) {
            return SCREEN_RC_ERROR;
        }
        return SCREEN_RC_RENDERED;
    }
    return SCREEN_RC_OK;
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
        case UI_DATA_VEHICLE_STATE_READY:
            return "READY";
        case UI_DATA_VEHICLE_STATE_DRIVE:
            return "DRIVE";
        case UI_DATA_VEHICLE_STATE_AUTONOMOUS:
            return "AUTO";
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
