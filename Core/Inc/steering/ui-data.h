/*!
 * \file ui-data.h
 * \date 2026-07-13
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief Snapshot of every value the dashboard renders.
 *
 * \details A single struct holds every value the dashboard shows. Producers
 *     (the CAN router once canlib is wired in, the parameter on-change
 *     handler, the FSM state mirror in main) write the fields whenever new
 *     data arrives; the FSM idle loop reads them once per tick and pushes
 *     any deltas into the dashboard through screen_api_sync_data. Setters
 *     are no-ops when nothing changed, so polling the struct every tick is
 *     cheap.
 *
 *     Adding a new dashboard field is: extend UIData here, extend
 *     screen_api_sync_data to forward the value, write the producer.
 */

#ifndef UI_DATA_H
#define UI_DATA_H

#include <stdint.h>

/*!
 * \brief High-level vehicle state shown in the center-top box.
 *
 * \details Generic states that map onto the FSM; the dashboard turns the
 *     enum into a short human-readable string. Add an entry here when a new
 *     state needs to surface on the UI.
 */
enum UIDataVehicleState {
    UI_DATA_VEHICLE_STATE_IDLE,       /*!< "IDLE"   */
    UI_DATA_VEHICLE_STATE_READY,      /*!< "READY"  */
    UI_DATA_VEHICLE_STATE_DRIVE,      /*!< "DRIVE"  */
    UI_DATA_VEHICLE_STATE_AUTONOMOUS, /*!< "AUTO"   */
    UI_DATA_VEHICLE_STATE_FLASH,      /*!< "FLASH"  */
    UI_DATA_VEHICLE_STATE_ERROR,      /*!< "ERROR"  */
    UI_DATA_VEHICLE_STATE_COUNT,
};

/*!
 * \brief Snapshot rendered by the dashboard.
 *
 * \details The layout is grouped by visual section to make the matching
 *     screen_api_sync_data code easy to read.
 */
struct UIData {
    /* Scenario presets (mirror of the parameters module). */
    uint8_t power;   /*!< 0..10 */
    uint8_t regen;   /*!< 0..10 */
    uint8_t torque;  /*!< 0..10 */
    uint8_t slip_on; /*!< 0/1 (slip / traction control toggle) */

    /* Vehicle state shown in the center-top box. */
    uint8_t vehicle_state; /*!< UIDataVehicleState as a uint8_t to keep the layout packed */

    /* High-voltage pack. */
    uint8_t soc;     /*!< State of charge, 0..100 */
    int16_t hv_temp; /*!< Pack temperature in °C */

    /* Inverter. */
    int16_t inverter_temp; /*!< Inverter temperature in °C */

    /* Lap. */
    uint8_t lap_current;  /*!< Current lap number */
    uint8_t lap_total;    /*!< Total laps in the session */
    int32_t lap_delta_ms; /*!< Signed delta vs reference lap, in ms */

    /* Tire temperatures (clockwise FL, FR, RL, RR). */
    int16_t tire_fl_temp;
    int16_t tire_fr_temp;
    int16_t tire_rl_temp;
    int16_t tire_rr_temp;

    /* Motor temperatures (same layout as tires). */
    int16_t motor_fl_temp;
    int16_t motor_fr_temp;
    int16_t motor_rl_temp;
    int16_t motor_rr_temp;
};

#endif // UI_DATA_H
