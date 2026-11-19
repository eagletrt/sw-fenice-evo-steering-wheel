/*!
 * \file ui-data.h
 * \date 2026-07-13
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief Snapshot of every value the dashboard renders.
 *
 * \details A single struct holds every value the dashboard shows. Producers
 *     (the CAN router, the parameter on-change handler) write the fields
 *     whenever new data arrives; the scheduler render task reads them once
 *     per frame and pushes
 *     any deltas into the dashboard through screen_api_sync_data. Boxes are
 *     flagged only when their text or colour changes, so syncing the struct
 *     every frame is cheap.
 *
 *     Adding a new dashboard field is: extend UIData here, format it in
 *     dashboard_api_sync, write the producer (and its UIDataSource if it
 *     comes from CAN).
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
    UI_DATA_VEHICLE_STATE_IDLE,       /*!< "IDLE"       */
    UI_DATA_VEHICLE_STATE_PRECHARGE,  /*!< "PRECHARGE"  */
    UI_DATA_VEHICLE_STATE_DISCHARGE,  /*!< "DISCHARGE"  */
    UI_DATA_VEHICLE_STATE_TSON,       /*!< "TSON"      */
    UI_DATA_VEHICLE_STATE_DRIVE,      /*!< "DRIVE"      */
    UI_DATA_VEHICLE_STATE_AUTONOMOUS, /*!< "AUTONOMOUS" */
    UI_DATA_VEHICLE_STATE_FLASH,      /*!< "FLASH"      */
    UI_DATA_VEHICLE_STATE_ERROR,      /*!< "ERROR"      */
    UI_DATA_VEHICLE_STATE_COUNT,
};

/*!
 * \brief CAN sources of the dashboard, one stale bit each in UIData.stale.
 *
 * \details A source is stale until its first frame and again when it stops
 *     arriving (the router's timeouts); the dashboard shows "--" for it.
 */
enum UIDataSource : uint8_t {
    UI_DATA_SOURCE_HV_SOC,   /*!< TsacMainboardEstimatedSoc */
    UI_DATA_SOURCE_HV_TEMP,  /*!< TsacMainboardTemperatureInfo */
    UI_DATA_SOURCE_LV_SOC,   /*!< LvacEstimatedSoc */
    UI_DATA_SOURCE_SPEED,    /*!< EstimatedVehicleVelocity */
    UI_DATA_SOURCE_SHUTDOWN, /*!< TsacMainboardFeedbackShutdown */
    UI_DATA_SOURCE_INVERTER, /*!< InverterTemperature */
    UI_DATA_SOURCE_MOTOR,    /*!< MotorTemperature */
    UI_DATA_SOURCE_THROTTLE, /*!< PedalsThrottle */
    UI_DATA_SOURCE_BRAKE,    /*!< PedalsBrake */
    UI_DATA_SOURCE_LAP,      /*!< CurrentLap */
    UI_DATA_SOURCE_COUNT,
};

#define UI_DATA_STALE_BIT(source) ((uint16_t)(1U << (uint8_t)(source)))            /*!< Bit of a source in UIData.stale */
#define UI_DATA_STALE_ALL ((uint16_t)((1U << (uint8_t)UI_DATA_SOURCE_COUNT) - 1U)) /*!< Every source stale (boot) */

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
    uint8_t ptt;     /*!< 0/1 (push-to-talk keyed) */

    /* Vehicle state shown in the center-top box. */
    uint8_t vehicle_state; /*!< UIDataVehicleState as a uint8_t to keep the layout packed */
    uint8_t autonomous;    /*!< 1 while EcuFsm reports a state of the AS branch (AS_EMERGENCY included) */

    /* High-voltage pack. */
    uint8_t soc;     /*!< State of charge, 0..100 */
    int16_t hv_temp; /*!< Pack temperature in °C */

    /* Low-voltage pack. */
    uint8_t lv_soc; /*!< State of charge, 0..100 */

    /* Driving. */
    uint16_t speed_kmh;       /*!< Vehicle speed in km/h */
    uint8_t shutdown_closed;  /*!< 1 while the shutdown circuit is closed (sdEnd above the threshold) */
    uint8_t throttle_percent; /*!< Throttle pedal travel, 0..100 */
    uint8_t brake_percent;    /*!< Highest brake pressure, 1 % per bar, 0..100 */

    /* Inverter. */
    int16_t inverter_temp; /*!< Hottest inverter temperature in °C */

    /* Lap. */
    uint8_t lap_current;  /*!< Current lap number */
    int32_t lap_delta_ms; /*!< Signed delta vs the best lap, in ms */

    /* Motor temperatures (FL, FR, RL, RR); the dashboard shows the hottest. */
    int16_t motor_fl_temp;
    int16_t motor_fr_temp;
    int16_t motor_rl_temp;
    int16_t motor_rr_temp;

    uint16_t stale; /*!< UI_DATA_STALE_BIT of every source not received lately */
};

#endif // UI_DATA_H
