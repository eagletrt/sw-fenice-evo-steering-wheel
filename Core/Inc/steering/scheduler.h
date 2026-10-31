/*!
 * \file scheduler.h
 * \date 2026-10-16
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief Definitions for the cooperative task scheduler of the steering wheel.
 *
 * \details Every periodic job of the wheel (CAN I/O, input polling, the FSM,
 *     rendering, cyclic CAN messages) is a libtimebase task with its own
 *     period. Nothing is preemptive: the main loop calls the scheduler
 *     routine, which runs every task that is due, one after the other.
 *     Hardware is reached only through the callbacks in SchedulerConfig, so
 *     the same task table runs on the wheel and on the PC simulator.
 */

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "post.h"
#include "tasks.h"
#include "watchdogs.h"
#include "wheel-fsm.h"

#include <stdbool.h>
#include <stdint.h>

/*!
 * \brief Period of the scheduler statistics log, in milliseconds.
 */
#define SCHEDULER_STATS_LOG_PERIOD_MS (5000U)

/*!
 * \brief Return codes for scheduler operations.
 */
enum SchedulerReturnCode {
    SCHEDULER_RC_OK,           /*!< Operation completed successfully */
    SCHEDULER_RC_NULL_POINTER, /*!< A required pointer or callback was NULL */
    SCHEDULER_RC_ERROR,        /*!< libtimebase rejected the configuration or failed at runtime */
};

/*!
 * \brief Every task of the wheel, in execution order.
 *
 * \details libtimebase runs tasks due on the same tick in ID order: RX and
 *     inputs come first so the FSM and the producers see fresh data, the TX
 *     flush comes after every producer, rendering comes last.
 */
enum SchedulerTaskId : uint8_t {
    SCHEDULER_TASK_CAN_RX,            /*!< Drain both RX queues into the router */
    SCHEDULER_TASK_INPUTS,            /*!< Poll buttons and manettini, detect long presses */
    SCHEDULER_TASK_FSM,               /*!< Run the wheel FSM (logic only) */
    SCHEDULER_TASK_CAN_CONTROL_MAPS,  /*!< ControlMapsSet */
    SCHEDULER_TASK_CAN_BUTTON_STATUS, /*!< SteeringWheelButtonStatus */
    SCHEDULER_TASK_CAN_FSM_STATUS,    /*!< SteeringWheelFsm */
    SCHEDULER_TASK_CAN_VERSION,       /*!< SteeringWheelVersion and SteeringWheelLibcanVersion (+ info) */
    SCHEDULER_TASK_CAN_TX,            /*!< Flush both TX queues to the bus */
    SCHEDULER_TASK_RENDER,            /*!< Sync UI data, redraw dirty boxes, swap framebuffers */
    SCHEDULER_TASK_STATS,             /*!< Log the task timing statistics */
    SCHEDULER_TASK_COUNT,
};

/*!
 * \brief Current time in milliseconds.
 */
typedef uint32_t (*scheduler_get_tick_callback)(void);

/*!
 * \brief Free-running cycle counter, used only to time the tasks.
 */
typedef uint32_t (*scheduler_get_cycles_callback)(void);

/*!
 * \brief Log hardware-side statistics (e.g. DMA2D timings) and start a new window.
 */
typedef void (*scheduler_report_stats_callback)(void);

/*!
 * \brief Present the frame just rendered.
 */
typedef void (*scheduler_swap_framebuffers_callback)(void);

/*!
 * \brief Sample every physical input and forward changes to the inputs module.
 *
 * \param tick_ms Current time in milliseconds.
 */
typedef void (*scheduler_poll_inputs_callback)(uint32_t tick_ms);

/*!
 * \brief Hardware hooks and init data handed to the scheduler.
 */
struct SchedulerConfig {
    struct PostInitData post;                               /*!< Passed to the FSM INIT state (POST); parameters_on_change is set by the scheduler */
    scheduler_get_tick_callback get_tick;                   /*!< Required: millisecond tick */
    scheduler_poll_inputs_callback poll_inputs;             /*!< Required: input sampling */
    scheduler_swap_framebuffers_callback swap_framebuffers; /*!< Required: present the frame just rendered */
    wheel_fsm_reset_callback reset;                         /*!< Required: system reset (bootloader request) */
    scheduler_get_cycles_callback get_cycles;               /*!< Optional: cycle counter, timing statistics are off when NULL */
    uint32_t cycles_per_us;                                 /*!< Cycle counter frequency in MHz, required when get_cycles is set */
    scheduler_report_stats_callback report_stats;           /*!< Optional: called by the stats task after the task timings */
};

/*!
 * \brief Execution time statistics of one task.
 *
 * \details Durations are in **microseconds**, not scheduler ticks: they come
 *     from the free-running core cycle counter (SchedulerConfig::get_cycles,
 *     the DWT CYCCNT on the wheel) divided by SchedulerConfig::cycles_per_us,
 *     because a 1 ms tick is far too coarse to time a task. Kept in the
 *     scheduler handler so they can be read with a debugger at any time;
 *     window_max_us is also logged and reset every
 *     SCHEDULER_STATS_LOG_PERIOD_MS.
 */
struct SchedulerTaskStats {
    uint32_t runs;          /*!< Number of executions since boot */
    uint32_t last_us;       /*!< Duration of the last execution */
    uint32_t max_us;        /*!< Longest execution since boot */
    uint32_t window_max_us; /*!< Longest execution since the last statistics log */
};

/*!
 * \brief File-static state of the scheduler.
 */
struct SchedulerHandler {
    struct SchedulerConfig config;                         /*!< Copy of the configuration given at init */
    struct TasksHandler tasks;                             /*!< libtimebase tasks */
    struct WatchdogHandler watchdogs;                      /*!< libtimebase watchdogs (pool: flash monitor timeout, combo hold times) */
    uint32_t tick;                                         /*!< Last tick read by libtimebase: the time base of every task */
    struct SchedulerTaskStats stats[SCHEDULER_TASK_COUNT]; /*!< Per-task timing */
    uint16_t dropped_at_last_log;                          /*!< libtimebase dropped-runs counter at the previous stats log */
};

#endif // SCHEDULER_H
