/*!
 * \file ui-data.h
 * \date 2026-07-13
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief Snapshot of every value the screen renders.
 *
 * \details A single struct holds every value the pages show. Producers
 *     (the CAN router, the parameter on-change handler, the scheduler's
 *     statistics) write the fields whenever new data arrives; the render task
 *     reads them once per frame and pushes any deltas into the mounted page.
 *     Boxes are flagged only when their text or colour changes, so syncing
 *     the struct every frame is cheap.
 *
 *     Adding a field is: extend UIData here, format it in the page that shows
 *     it, write the producer (and its UIDataSource if it comes from CAN).
 */

#ifndef UI_DATA_H
#define UI_DATA_H

#include "vehicle/shutdown/shutdown.h"

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
 * \brief CAN sources of the screen, one stale bit each in UIData.stale.
 *
 * \details A source is stale until its first frame and again when it stops
 *     arriving (the router's timeouts); pages show "--" for it. Sources
 *     marked TODO(libcan) have no message yet (can/can-placeholders.h): they
 *     stay stale until libcan adds it and the router decodes it.
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

    UI_DATA_SOURCE_HV_VOLTAGE,      /*!< TsacMainboardVoltageInfo */
    UI_DATA_SOURCE_HV_CURRENT,      /*!< TsacMainboardCurrentInfo */
    UI_DATA_SOURCE_HV_FEEDBACK,     /*!< TsacMainboardFeedback */
    UI_DATA_SOURCE_HV_STATUS,       /*!< TsacStatus */
    UI_DATA_SOURCE_HV_SHUTDOWN,     /*!< TsacMainboardShutdown */
    UI_DATA_SOURCE_HV_COOLING_1,    /*!< TsacMainboardCoolingTemperature1 */
    UI_DATA_SOURCE_HV_COOLING_2,    /*!< TsacMainboardCoolingTemperature2 */
    UI_DATA_SOURCE_TORQUE,          /*!< MotorTorque */
    UI_DATA_SOURCE_COOLING_MOTOR,   /*!< CoolingMotorTemperature */
    UI_DATA_SOURCE_COOLING_CIRCUIT, /*!< CoolingCircuitTemperature */
    UI_DATA_SOURCE_COOLING_OUT,     /*!< CoolingOut */
    UI_DATA_SOURCE_TYRE_FRONT,      /*!< TyreTemperatureFront */
    UI_DATA_SOURCE_TYRE_REAR,       /*!< TyreTemperatureRear */
    UI_DATA_SOURCE_DAS_SHUTDOWN,    /*!< DasShutdown */
    UI_DATA_SOURCE_PIPO_SHUTDOWN,   /*!< PipoShutdown */
    UI_DATA_SOURCE_EBS,             /*!< BrakesEBS */
    UI_DATA_SOURCE_LV_VOLTAGE,      /*!< LvacVoltageInfo */
    UI_DATA_SOURCE_LV_TEMP,         /*!< LvacTemperatureInfo */
    UI_DATA_SOURCE_LV_STATUS,       /*!< LvacFsm */
    UI_DATA_SOURCE_PIPO_SUPPLY,     /*!< PowerSupply */
    UI_DATA_SOURCE_PIPO_LINES_LOW,  /*!< LineCurrentLowPower */
    UI_DATA_SOURCE_PIPO_LINES_HIGH, /*!< LineCurrentHighPower */
    UI_DATA_SOURCE_PIPO_LINES_AS,   /*!< LineCurrentHighPowerAs */
    UI_DATA_SOURCE_PIPO_STATUS,     /*!< PipoFsm */
    UI_DATA_SOURCE_IMU_ACC,         /*!< ImuAcceleration */
    UI_DATA_SOURCE_IMU_GYRO,        /*!< ImuAngularRate */
    UI_DATA_SOURCE_POSITION,        /*!< EstimatedVehiclePosition */
    UI_DATA_SOURCE_STEERING_STATUS, /*!< SteeringFsm */

    UI_DATA_SOURCE_INVERTER_STATUS, /*!< TODO(libcan): inverter status, control enable, DC link */
    UI_DATA_SOURCE_MOTOR_SPEED,     /*!< TODO(libcan): motor speed (rpm) */
    UI_DATA_SOURCE_TORQUE_ACTUAL,   /*!< TODO(libcan): torque the motors deliver (MotorTorque is the ECU's set-point) */
    UI_DATA_SOURCE_TYRE_PRESSURE,   /*!< TODO(libcan) */
    UI_DATA_SOURCE_DISC_TEMP,       /*!< TODO(libcan) */
    UI_DATA_SOURCE_COOLING_TANK,    /*!< TODO(libcan): tank temperature and level */
    UI_DATA_SOURCE_BRAKE_BIAS_SET,  /*!< TODO(libcan): commanded brake bias */
    UI_DATA_SOURCE_PEDALS_VOLTAGE,  /*!< TODO(libcan): APPS and brake sensor voltages */
    UI_DATA_SOURCE_GPS,             /*!< TODO(libcan): latitude, longitude, GPS speed */
    UI_DATA_SOURCE_PC_AS,           /*!< TODO(libcan): AS computer status */
    UI_DATA_SOURCE_MISSION,         /*!< TODO(libcan): selected mission (#11) */
    UI_DATA_SOURCE_EBS_STATE,       /*!< TODO(libcan): EBS armed state */
    UI_DATA_SOURCE_PIPO_FUSES,      /*!< TODO(libcan): fuse status per line */
    UI_DATA_SOURCE_PIPO_CHARGER,    /*!< TODO(libcan): charger input voltage */
    UI_DATA_SOURCE_SHUTDOWN_EXTRA,  /*!< TODO(libcan): inverter interlock, SDC relay */
    UI_DATA_SOURCE_SUSPENSION,      /*!< TODO(libcan): travel per corner (today heave and roll) */
    UI_DATA_SOURCE_LV_REASON,       /*!< TODO(libcan): LV state reason */
    UI_DATA_SOURCE_COUNT,
    UI_DATA_SOURCE_FIRST_PLACEHOLDER = UI_DATA_SOURCE_INVERTER_STATUS, /*!< Sources from here on have no message yet */
};

static_assert(UI_DATA_SOURCE_COUNT <= 64U, "UIData.stale has one bit per source");

#define UI_DATA_STALE_BIT(source) ((uint64_t)1U << (uint8_t)(source))                 /*!< Bit of a source in UIData.stale */
#define UI_DATA_STALE_ALL ((UI_DATA_STALE_BIT(UI_DATA_SOURCE_COUNT - 1U) << 1U) - 1U) /*!< Every source stale (boot) */

#define UI_DATA_AIR_CLOSED_V (2.0F) /*!< TODO(tune): AIR mechanical feedback at or above this voltage = closed */
#define UI_DATA_TSAL_GREEN_V (2.0F) /*!< TODO(tune): TSAL green feedback at or above this voltage = green on */

#define UI_DATA_WINDOW_SECONDS (10U)  /*!< Length of the "max in the last 10 s" windows */
#define UI_DATA_HV_COOLING_COUNT (7U) /*!< HV pack cooling sensors: inlet + 6 outlets */

/*!
 * \brief One value per wheel.
 */
struct UIDataCorners {
    float fl;
    float fr;
    float rl;
    float rr;
};

/*!
 * \brief Highest value per wheel over the last UI_DATA_WINDOW_SECONDS, one bucket per second.
 */
struct UIDataWindow {
    struct UIDataCorners bucket[UI_DATA_WINDOW_SECONDS];
    uint16_t filled; /*!< Bit per bucket holding at least one sample (all zero = empty window) */
    uint8_t head;    /*!< Bucket of the current second */
};

/*!
 * \brief PIPO output lines (libcan LineCurrent* signals, spares left out).
 */
enum UIDataPipoLine : uint8_t {
    UI_DATA_PIPO_LINE_TELEMETRY,
    UI_DATA_PIPO_LINE_POWERTRAIN,
    UI_DATA_PIPO_LINE_LIGHTS,
    UI_DATA_PIPO_LINE_DRIVER,
    UI_DATA_PIPO_LINE_SHUTDOWN,
    UI_DATA_PIPO_LINE_COOLING_LEFT,
    UI_DATA_PIPO_LINE_COOLING_RIGHT,
    UI_DATA_PIPO_LINE_VISION,
    UI_DATA_PIPO_LINE_PC_AS,
    UI_DATA_PIPO_LINE_STEERING,
    UI_DATA_PIPO_LINE_EBS,
    UI_DATA_PIPO_LINE_COUNT,
};

/*!
 * \brief Boards of the SW debug tab.
 */
enum UIDataBoard : uint8_t {
    UI_DATA_BOARD_ECU,
    UI_DATA_BOARD_BMS_HV,    /*!< TSAC mainboard */
    UI_DATA_BOARD_PORCHETTO, /*!< LVAC */
    UI_DATA_BOARD_PIPO,
    UI_DATA_BOARD_INVERTERS, /*!< TODO(libcan) */
    UI_DATA_BOARD_STEERING,
    UI_DATA_BOARD_PC_AS, /*!< TODO(libcan) */
    UI_DATA_BOARD_COUNT,
};

#define UI_DATA_BOARD_KNOWN_COMMIT (1U << 0U) /*!< UIDataBoardInfo.known: commit received */
#define UI_DATA_BOARD_KNOWN_LIBCAN (1U << 1U) /*!< UIDataBoardInfo.known: libcan version received */

/*!
 * \brief What a board reports about itself (no stale timeout: versions come every 10 s).
 */
struct UIDataBoardInfo {
    uint32_t commit;    /*!< <Board>VersionInfo.commitHash */
    uint16_t libcan[3]; /*!< <Board>LibcanVersion major, minor, patch */
    uint8_t status;     /*!< Raw <Board>Fsm status */
    uint8_t known;      /*!< UI_DATA_BOARD_KNOWN_* */
};

/*!
 * \brief CAN controller state (WHEEL tab).
 */
enum WheelHealthBusState : uint8_t {
    WHEEL_HEALTH_BUS_ACTIVE,  /*!< Error active: sends and receives normally */
    WHEEL_HEALTH_BUS_PASSIVE, /*!< Error passive: a counter went past 127 */
    WHEEL_HEALTH_BUS_OFF,     /*!< Bus-off: TEC went past 255 */
};

/*!
 * \brief Why the MCU last reset (RCC_RSR at boot, most telling flag).
 */
enum WheelHealthResetCause : uint8_t {
    WHEEL_HEALTH_RESET_UNKNOWN,
    WHEEL_HEALTH_RESET_POR, /*!< Power-on */
    WHEEL_HEALTH_RESET_BOR, /*!< Brown-out: the supply dipped */
    WHEEL_HEALTH_RESET_PIN, /*!< NRST pin */
    WHEEL_HEALTH_RESET_SFT, /*!< Software: CAN reset request or Error_Handler */
    WHEEL_HEALTH_RESET_WDG, /*!< Watchdog */
    WHEEL_HEALTH_RESET_COUNT,
};

/*!
 * \brief Health of the steering wheel itself, one snapshot per statistics window.
 *
 * \details The scheduler fills the window, frame, task and uptime fields;
 *     the hardware hook (scheduler_read_health_callback) fills the CAN, DMA2D
 *     and reset-cause fields. The CAN fields describe the primary bus.
 */
struct WheelHealth {
    enum WheelHealthBusState can_bus_state;
    uint8_t can_last_error; /*!< FDCAN PSR.LEC: 0 none, 1 stuff, 2 form, 3 ack, 4 bit1, 5 bit0, 6 crc, 7 unchanged */
    enum WheelHealthResetCause reset_cause;
    uint32_t can_transmit_errors; /*!< TEC */
    uint32_t can_receive_errors;  /*!< REC */
    uint32_t can_bus_offs;        /*!< Bus-off restarts since boot */
    uint32_t can_tx_fifo_free;    /*!< Free Tx FIFO elements */
    uint32_t can_tx_fifo_size;    /*!< Tx FIFO elements */
    uint32_t can_tx_refused;      /*!< Frames refused by a full Tx FIFO since boot */
    uint32_t can_tx_unjams;       /*!< Times every pending Tx request was cancelled */
    uint32_t can_restricted;      /*!< Times restricted operation mode was left */
    uint32_t frame_max_us;        /*!< Longest frame (first DMA2D transfer to the swap) in the window */
    uint32_t dma2d_errors;        /*!< DMA2D transfer errors since boot */

    uint32_t window_ms;     /*!< Length of the statistics window */
    uint32_t frames;        /*!< Frames presented in the window */
    uint32_t render_max_us; /*!< Longest render task run in the window (CPU) */
    uint32_t inputs_max_us; /*!< Longest inputs task run in the window */
    uint32_t dropped_runs;  /*!< libtimebase task runs dropped in the window */
    uint32_t uptime_s;
};

/*!
 * \brief High-voltage pack (TSAC).
 */
struct UIDataHv {
    float cell_t_max;   /*!< °C */
    float cell_t_min;   /*!< °C */
    float cell_v_max;   /*!< V */
    float cell_v_min;   /*!< V */
    float pack_v;       /*!< Pack voltage */
    float ts_v;         /*!< Tractive system (DC link) voltage */
    float current_a;    /*!< Pack current */
    float power_kw;     /*!< Pack power */
    float airp_mec_v;   /*!< AIR+ mechanical feedback voltage */
    float airn_mec_v;   /*!< AIR- mechanical feedback voltage */
    float tsal_green_v; /*!< TSAL green feedback voltage */
    float cooling[UI_DATA_HV_COOLING_COUNT];
    uint8_t airp_closed; /*!< AIR+ commanded closed */
    uint8_t airn_closed; /*!< AIR- commanded closed */
    uint8_t plausible;   /*!< TS plausibility */
    uint8_t ams_fault;   /*!< AMS fault cockpit LED */
    uint8_t imd_fault;   /*!< IMD fault cockpit LED */
};

/*!
 * \brief Motors and inverters.
 */
struct UIDataPowertrain {
    struct UIDataCorners torque;        /*!< MotorTorque: what the ECU sets, Nm */
    struct UIDataWindow torque_window;  /*!< Highest delivered torque in the last 10 s (TODO(libcan)) */
    struct UIDataWindow rpm_window;     /*!< Highest speed in the last 10 s (TODO(libcan)) */
    struct UIDataCorners motor_temp;    /*!< MotorTemperature, °C */
    struct UIDataCorners inverter_temp; /*!< InverterTemperature, °C */
    float dc_link_v;                    /*!< Inverter DC link (TODO(libcan)) */
    uint8_t inverter_status[4];         /*!< UIDataInverterStatus per wheel (TODO(libcan)) */
    uint8_t control_enable[4];          /*!< Enable input as read by each inverter (TODO(libcan)) */
};

/*!
 * \brief Inverter status (TODO(libcan): replace with the inverter message's enum).
 */
enum UIDataInverterStatus : uint8_t {
    UI_DATA_INVERTER_OFF,
    UI_DATA_INVERTER_RUN,
    UI_DATA_INVERTER_DERATE,
    UI_DATA_INVERTER_FAULT,
    UI_DATA_INVERTER_STATUS_COUNT,
};

/*!
 * \brief Cooling circuit.
 */
struct UIDataCooling {
    float pump_percent[2];       /*!< Left, right */
    float fan_percent[2];        /*!< Left, right */
    float radiator[2];           /*!< Left, right, °C */
    float tank_temp[2];          /*!< Left, right, °C (TODO(libcan)) */
    float tank_level_percent[2]; /*!< Left, right (TODO(libcan)) */
    struct UIDataCorners motor;  /*!< CoolingMotorTemperature, °C */
};

/*!
 * \brief Pedals, brakes and EBS.
 */
struct UIDataPedals {
    float apps_percent[3];
    float apps_v[3];       /*!< TODO(libcan) */
    float brake_sensor_v;  /*!< TODO(libcan) */
    float brake_travel;    /*!< Brake pedal, 0..100 */
    float bots_v;          /*!< Brake over-travel switch, shutdown voltage */
    float bias_set;        /*!< Commanded brake bias, % front (TODO(libcan)) */
    float ebs_tank_bar[2]; /*!< EBS tanks 1, 2 */
    float ebs_line_bar[2]; /*!< EBS lines 1, 2 */
    float ebs_load_n[2];   /*!< EBS strain gauges 1, 2 */
    struct UIDataCorners pressure_bar;
    uint8_t plausibility; /*!< Raw ThrottlePlausibility */
    uint8_t ebs_armed;    /*!< TODO(libcan) */
};

/*!
 * \brief Vehicle dynamics and the DAS boards.
 */
struct UIDataChassis {
    struct UIDataCorners tyre_temp;
    struct UIDataCorners tyre_pressure_bar; /*!< TODO(libcan) */
    struct UIDataCorners disc_temp;         /*!< TODO(libcan) */
    struct UIDataCorners suspension_mm;     /*!< TODO(libcan) */
    float gps_lat;                          /*!< TODO(libcan) */
    float gps_lon;                          /*!< TODO(libcan) */
    float gps_speed_kmh;                    /*!< TODO(libcan) */
    float heading_deg;                      /*!< From the estimated yaw */
    float acc_g[3];                         /*!< x, y, z */
    float gyro_dps[3];                      /*!< x, y, z */
    float imu_temp;
};

/*!
 * \brief Low-voltage pack and PIPO.
 */
struct UIDataLv {
    float total_v;
    float cell_v_max;
    float cell_v_min;
    float current_a;
    float charge_a; /*!< TODO(libcan) */
    float t_max;
    float input_v;   /*!< PIPO input (LV battery) */
    float charger_v; /*!< PIPO charger input (TODO(libcan)) */
    float out_a;     /*!< PIPO total output current */
    float out_w;     /*!< PIPO total output power */
    float line_a[UI_DATA_PIPO_LINE_COUNT];
    uint16_t fuse_blown; /*!< Bit per UIDataPipoLine (TODO(libcan)) */
    uint8_t reason;      /*!< TODO(libcan) */
};

/*!
 * \brief Snapshot rendered by the pages.
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
    uint8_t ecu_status;    /*!< Raw EcuFsm vehicle status */
    uint8_t mission;       /*!< TODO(libcan, #11) */
    uint8_t pc_as_status;  /*!< TODO(libcan) */

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

    /* Debug tabs. */
    struct UIDataHv hv;
    struct UIDataPowertrain powertrain;
    struct UIDataCooling cooling;
    struct UIDataPedals pedals;
    struct UIDataChassis chassis;
    struct UIDataLv lv;
    float shutdown_v[SHUTDOWN_NODE_COUNT]; /*!< Voltage after each node */
    struct UIDataBoardInfo boards[UI_DATA_BOARD_COUNT];
    struct WheelHealth wheel;

    uint64_t stale; /*!< UI_DATA_STALE_BIT of every source not received lately */
};

#endif // UI_DATA_H
