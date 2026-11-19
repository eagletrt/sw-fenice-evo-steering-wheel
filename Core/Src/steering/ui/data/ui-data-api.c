/*!
 * \file ui-data-api.c
 * \date 2026-07-13
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief Implementation of the UIData accessor and the 10 s windows.
 */

#include "ui/data/ui-data-api.h"
#include "eagletrt-api.h"
#include "eagletrt.h"

#include <float.h>
#include <stddef.h>

// The ECU state is unknown and every CAN value stale until their first frame (A4).
EAGLETRT_STATIC struct UIData ui_data = {
    .vehicle_state = UI_DATA_VEHICLE_STATE_COUNT,
    .stale = UI_DATA_STALE_ALL,
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
