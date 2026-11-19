/*!
 * \file debug-dynamics.c
 * \date 2026-11-19
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief DYNAMICS debug tab (Figma "Debug 11/12 - DYNAMICS").
 */

#include "ui/debug/debug-api.h"
#include "ui/debug/debug-tabs.h"

#include "eagletrt.h"

enum DebugDynamicsSlot : uint8_t {
    DEBUG_DYNAMICS_LAT,
    DEBUG_DYNAMICS_LON,
    DEBUG_DYNAMICS_GPS_SPEED,
    DEBUG_DYNAMICS_HEADING,
    DEBUG_DYNAMICS_ACC, /*!< x, y, z */
    DEBUG_DYNAMICS_IMU_TEMP = DEBUG_DYNAMICS_ACC + 3U,
    DEBUG_DYNAMICS_GYRO, /*!< x, y, z */
    DEBUG_DYNAMICS_SUSPENSION = DEBUG_DYNAMICS_GYRO + 3U,
};

// clang-format off
EAGLETRT_STATIC const struct DebugElement debug_dynamics_elements[] = {
    DEBUG_GRID_CELL(0, 0, "GPS LAT", DEBUG_DYNAMICS_LAT),           DEBUG_GRID_CELL(1, 0, "GPS LON", DEBUG_DYNAMICS_LON),
    DEBUG_GRID_CELL(2, 0, "GPS SPEED", DEBUG_DYNAMICS_GPS_SPEED),   DEBUG_GRID_CELL(3, 0, "HEADING", DEBUG_DYNAMICS_HEADING),
    DEBUG_GRID_CELL(0, 1, "ACC X", DEBUG_DYNAMICS_ACC),             DEBUG_GRID_CELL(1, 1, "ACC Y", DEBUG_DYNAMICS_ACC + 1U),
    DEBUG_GRID_CELL(2, 1, "ACC Z", DEBUG_DYNAMICS_ACC + 2U),        DEBUG_GRID_CELL(3, 1, "IMU TEMP", DEBUG_DYNAMICS_IMU_TEMP),
    DEBUG_CELL(  1U, 269U, 265U, 104U, "GYRO X", DEBUG_DYNAMICS_GYRO, 32U),
    DEBUG_CELL(268U, 269U, 264U, 104U, "GYRO Y", DEBUG_DYNAMICS_GYRO + 1U, 32U),
    DEBUG_CELL(534U, 269U, 265U, 104U, "GYRO Z", DEBUG_DYNAMICS_GYRO + 2U, 32U),
    { .kind = DEBUG_ELEMENT_COLUMNS, .rect = { 1U, 375U, 798U, 104U }, .caption = "SUSPENSION TRAVEL mm", .labels = debug_api_corner_labels, .slot = DEBUG_DYNAMICS_SUSPENSION, .value_size = 26U },
};
// clang-format on

EAGLETRT_STATIC void prv_debug_dynamics_sync(struct DebugHandler *handler, const struct UIData *ui) {
    const bool gps_stale = debug_api_stale(ui, UI_DATA_SOURCE_GPS);
    debug_api_number(handler, DEBUG_DYNAMICS_LAT, gps_stale, ui->chassis.gps_lat, 4U, "");
    debug_api_number(handler, DEBUG_DYNAMICS_LON, gps_stale, ui->chassis.gps_lon, 4U, "");
    debug_api_number(handler, DEBUG_DYNAMICS_GPS_SPEED, gps_stale, ui->chassis.gps_speed_kmh, 0U, " km/h");
    debug_api_number(handler, DEBUG_DYNAMICS_HEADING, debug_api_stale(ui, UI_DATA_SOURCE_POSITION), ui->chassis.heading_deg, 0U, "°");

    const bool acc_stale = debug_api_stale(ui, UI_DATA_SOURCE_IMU_ACC);
    const bool gyro_stale = debug_api_stale(ui, UI_DATA_SOURCE_IMU_GYRO);
    for (uint8_t axis = 0U; axis < 3U; axis++) {
        debug_api_number(handler, (uint8_t)(DEBUG_DYNAMICS_ACC + axis), acc_stale, ui->chassis.acc_g[axis], 2U, " g");
        debug_api_number(handler, (uint8_t)(DEBUG_DYNAMICS_GYRO + axis), gyro_stale, ui->chassis.gyro_dps[axis], 1U, " °/s");
    }
    debug_api_number(handler, DEBUG_DYNAMICS_IMU_TEMP, acc_stale, ui->chassis.imu_temp, 0U, "°C");
    debug_api_corners(handler, DEBUG_DYNAMICS_SUSPENSION, debug_api_stale(ui, UI_DATA_SOURCE_SUSPENSION), &ui->chassis.suspension_mm, 0U, "");
}

const struct DebugPage debug_dynamics_page = { "DYNAMICS", debug_dynamics_elements, sizeof(debug_dynamics_elements) / sizeof(debug_dynamics_elements[0]), prv_debug_dynamics_sync };
