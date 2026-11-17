/*!
 * \file ui-data-api.c
 * \date 2026-07-13
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief Implementation of the UIData accessor.
 */

#include "ui-data-api.h"
#include "eagletrt.h"

// The ECU state is unknown and every CAN value stale until their first frame (A4).
EAGLETRT_STATIC struct UIData ui_data = { .vehicle_state = UI_DATA_VEHICLE_STATE_COUNT, .stale = UI_DATA_STALE_ALL };

struct UIData *ui_data_api_get(void) {
    return &ui_data;
}
