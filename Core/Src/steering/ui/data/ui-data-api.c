/*!
 * \file ui-data-api.c
 * \date 2026-07-13
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief Implementation of the UIData accessor and the 10 s windows.
 */

#include "ui/data/ui-data-api.h"
#include "vehicle/shutdown/shutdown-api.h"
#include "eagletrt-api.h"
#include "eagletrt.h"

#include <float.h>
#include <stddef.h>

// The ECU state is unknown and every CAN value stale until their first frame (A4).
EAGLETRT_STATIC struct UIData ui_data = {
    .vehicle_state = UI_DATA_VEHICLE_STATE_COUNT,
    .stale = UI_DATA_STALE_ALL,
};

/*! \brief The message each shutdown node's voltage comes in. */
EAGLETRT_STATIC const enum UIDataSource ui_data_shutdown_sources[SHUTDOWN_NODE_COUNT] = {
    [SHUTDOWN_NODE_ECU] = UI_DATA_SOURCE_ECU_SHUTDOWN,
    [SHUTDOWN_NODE_COCKPIT_MUSHROOM] = UI_DATA_SOURCE_DAS_SHUTDOWN,
    [SHUTDOWN_NODE_FRONT_RIGHT_A_ARM] = UI_DATA_SOURCE_SHUTDOWN_EXTRA,
    [SHUTDOWN_NODE_BOTS] = UI_DATA_SOURCE_BRAKE,
    [SHUTDOWN_NODE_FRONT_LEFT_A_ARM] = UI_DATA_SOURCE_SHUTDOWN_EXTRA,
    [SHUTDOWN_NODE_INERTIA] = UI_DATA_SOURCE_DAS_SHUTDOWN,
    [SHUTDOWN_NODE_INVERTER_INTERLOCK] = UI_DATA_SOURCE_SHUTDOWN_EXTRA,
    [SHUTDOWN_NODE_REAR_LEFT_A_ARM] = UI_DATA_SOURCE_SHUTDOWN_EXTRA,
    [SHUTDOWN_NODE_BSPD] = UI_DATA_SOURCE_PIPO_SHUTDOWN,
    [SHUTDOWN_NODE_HVD] = UI_DATA_SOURCE_PIPO_SHUTDOWN,
    [SHUTDOWN_NODE_LEFT_MUSHROOM] = UI_DATA_SOURCE_DAS_SHUTDOWN,
    [SHUTDOWN_NODE_RIGHT_MUSHROOM] = UI_DATA_SOURCE_DAS_SHUTDOWN,
    [SHUTDOWN_NODE_AMS] = UI_DATA_SOURCE_HV_SHUTDOWN,
    [SHUTDOWN_NODE_IMD] = UI_DATA_SOURCE_HV_SHUTDOWN,
    [SHUTDOWN_NODE_REAR_RIGHT_A_ARM] = UI_DATA_SOURCE_SHUTDOWN_EXTRA,
    [SHUTDOWN_NODE_TSMS] = UI_DATA_SOURCE_PIPO_SHUTDOWN,
    [SHUTDOWN_NODE_SD_END] = UI_DATA_SOURCE_SHUTDOWN,
};

struct UIData *ui_data_api_get(void) {
    return &ui_data;
}

void ui_data_api_window_push(struct UIDataWindow *window, const struct UIDataCorners *sample) {
    if (window == NULL || sample == NULL) {
        return;
    }
    const uint8_t head = window->head % UI_DATA_WINDOW_SECONDS;
    struct UIDataCorners *bucket = &window->bucket[head];
    if ((window->filled & (1U << head)) == 0U) {
        *bucket = *sample;
        window->filled |= (uint16_t)(1U << head);
        return;
    }
    bucket->fl = EAGLETRT_API_MAX(bucket->fl, sample->fl);
    bucket->fr = EAGLETRT_API_MAX(bucket->fr, sample->fr);
    bucket->rl = EAGLETRT_API_MAX(bucket->rl, sample->rl);
    bucket->rr = EAGLETRT_API_MAX(bucket->rr, sample->rr);
}

struct UIDataCorners ui_data_api_window_max(const struct UIDataWindow *window) {
    struct UIDataCorners max = { -FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX };
    for (uint8_t i = 0U; i < UI_DATA_WINDOW_SECONDS; i++) {
        if ((window->filled & (1U << i)) == 0U) {
            continue;
        }
        max.fl = EAGLETRT_API_MAX(max.fl, window->bucket[i].fl);
        max.fr = EAGLETRT_API_MAX(max.fr, window->bucket[i].fr);
        max.rl = EAGLETRT_API_MAX(max.rl, window->bucket[i].rl);
        max.rr = EAGLETRT_API_MAX(max.rr, window->bucket[i].rr);
    }
    return max;
}

/*!
 * \brief Move a window to a new, empty second.
 */
EAGLETRT_STATIC void prv_ui_data_api_rotate(struct UIDataWindow *window) {
    window->head = (uint8_t)((window->head + 1U) % UI_DATA_WINDOW_SECONDS);
    window->filled &= (uint16_t)~(1U << window->head);
}

void ui_data_api_rotate_windows(void) {
    prv_ui_data_api_rotate(&ui_data.powertrain.torque_window);
    prv_ui_data_api_rotate(&ui_data.powertrain.rpm_window);
}

bool ui_data_api_shutdown(const struct UIData *ui_data, struct ShutdownChain *chain) {
    if (ui_data == NULL) {
        return false;
    }
    uint32_t available = 0U;
    for (uint8_t node = 0U; node < SHUTDOWN_NODE_COUNT; node++) {
        available |= ((ui_data->stale & UI_DATA_STALE_BIT(ui_data_shutdown_sources[node])) != 0U) ? 0U : SHUTDOWN_NODE_BIT(node);
    }
    return shutdown_api_evaluate(ui_data->shutdown_v, available, chain);
}
