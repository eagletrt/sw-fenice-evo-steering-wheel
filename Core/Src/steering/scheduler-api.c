/*!
 * \file scheduler-api.c
 * \date 2026-10-16
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief Implementation of the cooperative task scheduler.
 */

#include "scheduler-api.h"

#include "can-communications-api.h"
#include "can-communications-router-api.h"
#include "can-primary.h"
#include "eagletrt-api.h"
#include "flash-monitor-api.h"
#include "identity-api.h"
#include "inputs-api.h"
#include "logger-api.h"
#include "parameters-api.h"
#include "screen-api.h"
#include "tasks-api.h"
#include "ui-data-api.h"
#include "watchdogs-api.h"
#include "wheel-fsm-api.h"

#include <inttypes.h>
#include <string.h>

static_assert(SCHEDULER_TASK_COUNT <= MAX_TASKS, "libtimebase cannot hold every scheduler task");

/*!
 * \brief Body of a task, called with the tick of the current routine call.
 */
typedef void (*scheduler_task_function)(uint32_t tick_ms);

/*!
 * \brief Period, start offset and body of one task.
 */
struct SchedulerTask {
    uint16_t interval_ms;             /*!< Time between two executions */
    uint16_t start_ms;                /*!< Delay of the first execution after init (staggering) */
    scheduler_task_function function; /*!< Task body */
};

EAGLETRT_STATIC struct SchedulerHandler scheduler_handler;

/*!
 * \brief Mirror a parameter transition into the UI snapshot.
 *
 * \param parameter_id Parameter that changed.
 * \param value New value of the parameter.
 */
EAGLETRT_STATIC void prv_scheduler_api_sync_parameter_to_ui(enum InputsSharedParameterID parameter_id, uint8_t value) {
    struct UIData *ui_data = ui_data_api_get();
    switch (parameter_id) {
        case INPUTS_SHARED_PARAMETER_ID_POWER:
            ui_data->power = value;
            break;
        case INPUTS_SHARED_PARAMETER_ID_REGEN:
            ui_data->regen = value;
            break;
        case INPUTS_SHARED_PARAMETER_ID_TORQUE_VECTORING:
            ui_data->torque = value;
            break;
        case INPUTS_SHARED_PARAMETER_ID_LAUNCH_CONTROL:
            ui_data->slip_on = value;
            break;
        case INPUTS_SHARED_PARAMETER_ID_PTT:
            ui_data->ptt = value;
            break;
        default:
            break;
    }
}

/*!
 * \brief Parameters on-change hook: bus, dashboard and popup.
 *
 * \details Wired by the scheduler itself (not by main), so the wheel and the
 *     PC simulator react to inputs in exactly the same way.
 *
 * \param parameter_id Parameter that changed.
 * \param value New value of the parameter.
 *
 * \retval true if every consumer accepted the change.
 * \retval false if the CAN queue or the popup rejected it.
 */
EAGLETRT_STATIC bool prv_scheduler_api_on_parameter_change(enum InputsSharedParameterID parameter_id, uint8_t value) {
    // Every transition goes on the bus, shared with the UI or not: TS-on
    // and PTT are exactly the two that never reach the popup.
    if (can_communications_router_api_on_parameter_change(parameter_id, value) != CAN_COMMUNICATION_RC_OK) {
        return false;
    }

    // Every parameter reaches the dashboard; parameters_api_is_shared only
    // decides which ones additionally raise the popup. PTT is the case that
    // matters: it never pops up, but it does tint the SCENARIO header.
    prv_scheduler_api_sync_parameter_to_ui(parameter_id, value);

    if (!parameters_api_is_shared(parameter_id)) {
        return true;
    }

    // Same time base as the render task that checks the popup timeout.
    return screen_api_on_parameter_change(parameter_id, value, scheduler_handler.tick) == SCREEN_RC_OK;
}

EAGLETRT_STATIC void prv_scheduler_api_task_can_rx(uint32_t tick_ms) {
    EAGLETRT_API_UNUSED(tick_ms);
    EAGLETRT_API_UNUSED(can_communications_api_process_rx(CAN_COMMUNICATION_NETWORK_PRIMARY));
    EAGLETRT_API_UNUSED(can_communications_api_process_rx(CAN_COMMUNICATION_NETWORK_SECONDARY));
}

EAGLETRT_STATIC void prv_scheduler_api_task_inputs(uint32_t tick_ms) {
    scheduler_handler.config.poll_inputs(tick_ms);
    // A transient NAK from the input expanders is not worth bricking the
    // wheel over: ERROR has no way back and the display is the one thing
    // the driver actually needs.
    EAGLETRT_API_UNUSED(inputs_api_poll_for_long_press(tick_ms));
}

EAGLETRT_STATIC void prv_scheduler_api_task_fsm(uint32_t tick_ms) {
    EAGLETRT_API_UNUSED(tick_ms);
    EAGLETRT_API_UNUSED(wheel_fsm_api_run_state());
}

EAGLETRT_STATIC void prv_scheduler_api_task_can_control_maps(uint32_t tick_ms) {
    EAGLETRT_API_UNUSED(tick_ms);
    EAGLETRT_API_UNUSED(can_communications_router_api_send_control_maps());
}

EAGLETRT_STATIC void prv_scheduler_api_task_can_button_status(uint32_t tick_ms) {
    EAGLETRT_API_UNUSED(tick_ms);
    EAGLETRT_API_UNUSED(can_communications_router_api_send_button_status());
}

EAGLETRT_STATIC void prv_scheduler_api_task_can_fsm_status(uint32_t tick_ms) {
    EAGLETRT_API_UNUSED(tick_ms);
    EAGLETRT_API_UNUSED(identity_api_send_state(wheel_fsm_api_get_can_status(wheel_fsm_api_get_state())));
}

EAGLETRT_STATIC void prv_scheduler_api_task_can_version(uint32_t tick_ms) {
    EAGLETRT_API_UNUSED(tick_ms);
    EAGLETRT_API_UNUSED(identity_api_send_version());
    EAGLETRT_API_UNUSED(identity_api_send_libcan_version());
}

EAGLETRT_STATIC void prv_scheduler_api_task_can_tx(uint32_t tick_ms) {
    EAGLETRT_API_UNUSED(tick_ms);
    EAGLETRT_API_UNUSED(can_communications_api_process_tx(CAN_COMMUNICATION_NETWORK_PRIMARY));
    EAGLETRT_API_UNUSED(can_communications_api_process_tx(CAN_COMMUNICATION_NETWORK_SECONDARY));
}

EAGLETRT_STATIC void prv_scheduler_api_task_render(uint32_t tick_ms) {
    // The screen is only driven in IDLE and FLASH: an ERROR leaves the last
    // frame on screen.
    const enum WheelFsmState state = wheel_fsm_api_get_state();
    if (state != WHEEL_FSM_STATE_IDLE && state != WHEEL_FSM_STATE_FLASH) {
        return;
    }

    const bool flashing = (state == WHEEL_FSM_STATE_FLASH);
    if (screen_api_set_flash(flashing, flashing ? flash_monitor_api_get_board_name() : NULL) != SCREEN_RC_OK) {
        EAGLETRT_API_UNUSED(wheel_fsm_api_request(WHEEL_FSM_STATE_ERROR));
        return;
    }

    // Setters are no-ops when nothing changed, so syncing every frame is cheap.
    if (screen_api_sync_data(ui_data_api_get()) != SCREEN_RC_OK) {
        EAGLETRT_API_UNUSED(wheel_fsm_api_request(WHEEL_FSM_STATE_ERROR));
        return;
    }

    switch (screen_api_update(tick_ms)) {
        case SCREEN_RC_OK:
            break;
        case SCREEN_RC_RENDERED:
            scheduler_handler.config.swap_framebuffers();
            break;
        case SCREEN_RC_ERROR:
        case SCREEN_RC_NULL_POINTER:
        default:
            EAGLETRT_API_UNUSED(wheel_fsm_api_request(WHEEL_FSM_STATE_ERROR));
            break;
    }
}

EAGLETRT_STATIC void prv_scheduler_api_task_stats(uint32_t tick_ms) {
    EAGLETRT_API_UNUSED(tick_ms);
    if (scheduler_handler.config.get_cycles == NULL) {
        return;
    }

    // Two short lines: the logger truncates long records. The UART write is
    // blocking, so this task itself shows up as the slow one in the stats.
    struct SchedulerTaskStats *stats = scheduler_handler.stats;
    EAGLETRT_API_UNUSED(logger_api_log(LOGGER_LEVEL_INFO, "tasks 0-4 max us %" PRIu32 " %" PRIu32 " %" PRIu32 " %" PRIu32 " %" PRIu32, stats[0].window_max_us, stats[1].window_max_us, stats[2].window_max_us, stats[3].window_max_us, stats[4].window_max_us));
    EAGLETRT_API_UNUSED(logger_api_log(LOGGER_LEVEL_INFO, "tasks 5-9 max us %" PRIu32 " %" PRIu32 " %" PRIu32 " %" PRIu32 " %" PRIu32, stats[5].window_max_us, stats[6].window_max_us, stats[7].window_max_us, stats[8].window_max_us, stats[9].window_max_us));
    // Executions libtimebase skipped in this window because a task was late by
    // more than its period. libtimebase's counter is cumulative and saturates
    // at UINT16_MAX, after that the delta reads 0.
    const uint16_t dropped = tasks_api_get_dropped_task_executions(&scheduler_handler.tasks);
    const uint16_t dropped_in_window = (uint16_t)(dropped - scheduler_handler.dropped_at_last_log);
    scheduler_handler.dropped_at_last_log = dropped;
    EAGLETRT_API_UNUSED(logger_api_log(LOGGER_LEVEL_INFO, "tasks dropped runs %" PRIu16 " (total %" PRIu16 ")", dropped_in_window, dropped));

    for (uint8_t i = 0U; i < SCHEDULER_TASK_COUNT; i++) {
        stats[i].window_max_us = 0U;
    }

    if (scheduler_handler.config.report_stats != NULL) {
        scheduler_handler.config.report_stats();
    }
}

/*!
 * \brief Tasks silenced while another board is being flashed.
 *
 * \details Every CAN producer except the FSM status (which must report FLASH),
 *     and the inputs: a knob turned meanwhile would go on the bus right away
 *     through the parameters on-change hook. RX, the TX flush and the render
 *     keep running.
 */
EAGLETRT_STATIC const enum SchedulerTaskId prv_scheduler_api_flash_muted_tasks[] = {
    SCHEDULER_TASK_INPUTS,
    SCHEDULER_TASK_CAN_CONTROL_MAPS,
    SCHEDULER_TASK_CAN_BUTTON_STATUS,
    SCHEDULER_TASK_CAN_VERSION,
};

#define SCHEDULER_FLASH_MUTED_TASK_COUNT (sizeof(prv_scheduler_api_flash_muted_tasks) / sizeof(prv_scheduler_api_flash_muted_tasks[0]))

/*!
 * \brief FSM hook, IDLE -> FLASH: pause the muted tasks.
 */
EAGLETRT_STATIC void prv_scheduler_api_enter_flash(void) {
    for (size_t i = 0U; i < SCHEDULER_FLASH_MUTED_TASK_COUNT; i++) {
        EAGLETRT_API_UNUSED(scheduler_api_pause_task(prv_scheduler_api_flash_muted_tasks[i]));
    }
}

/*!
 * \brief FSM hook, FLASH -> IDLE: resume the muted tasks.
 */
EAGLETRT_STATIC void prv_scheduler_api_leave_flash(void) {
    for (size_t i = 0U; i < SCHEDULER_FLASH_MUTED_TASK_COUNT; i++) {
        EAGLETRT_API_UNUSED(scheduler_api_resume_task(prv_scheduler_api_flash_muted_tasks[i]));
    }
}

/*!
 * \brief Period, start offset and body of every task, indexed by SchedulerTaskId.
 *
 * \details Cyclic CAN messages use their libcan cycle time.
 *
 *     Start offsets stagger the tasks so the slow ones don't pile up in the
 *     same routine call. Measured on hardware (idle): every task is at the µs
 *     level except INPUTS (~1.6 ms of blocking I2C) and STATS (~10 ms of
 *     blocking UART), so RENDER is shifted by half the INPUTS period and STATS
 *     away from both: with 10/20 ms periods they never meet. Staggering the
 *     µs-level CAN tasks buys nothing measurable, they keep starting at 0.
 */
EAGLETRT_STATIC const struct SchedulerTask prv_scheduler_api_tasks[SCHEDULER_TASK_COUNT] = {
    [SCHEDULER_TASK_CAN_RX] = { 1U, 0U, prv_scheduler_api_task_can_rx },
    [SCHEDULER_TASK_INPUTS] = { 10U, 0U, prv_scheduler_api_task_inputs },
    [SCHEDULER_TASK_FSM] = { 5U, 0U, prv_scheduler_api_task_fsm },
    [SCHEDULER_TASK_CAN_CONTROL_MAPS] = { (uint16_t)can_primary_cycle_time_controlmapsset, 0U, prv_scheduler_api_task_can_control_maps },
    [SCHEDULER_TASK_CAN_BUTTON_STATUS] = { (uint16_t)can_primary_cycle_time_steeringwheelbuttonstatus, 0U, prv_scheduler_api_task_can_button_status },
    [SCHEDULER_TASK_CAN_FSM_STATUS] = { (uint16_t)can_primary_cycle_time_steeringwheelfsm, 0U, prv_scheduler_api_task_can_fsm_status },
    [SCHEDULER_TASK_CAN_VERSION] = { (uint16_t)can_primary_cycle_time_steeringwheelversion, 0U, prv_scheduler_api_task_can_version },
    [SCHEDULER_TASK_CAN_TX] = { 1U, 0U, prv_scheduler_api_task_can_tx },
    [SCHEDULER_TASK_RENDER] = { 20U, 5U, prv_scheduler_api_task_render },
    [SCHEDULER_TASK_STATS] = { SCHEDULER_STATS_LOG_PERIOD_MS, 3U, prv_scheduler_api_task_stats },
};

/*!
 * \brief Tick getter handed to libtimebase.
 *
 * \details libtimebase reads the time by itself (once per routine, and on
 *     every API call). Caching the value here gives the tasks, and anyone
 *     calling scheduler_api_get_tick, exactly the tick the scheduler acted on:
 *     mixing it with a fresher read of the hardware clock makes "now" older
 *     than an event stamped during the same routine.
 *
 * \return The current tick, in milliseconds.
 */
EAGLETRT_STATIC uint32_t prv_scheduler_api_read_tick(void) {
    if (scheduler_handler.config.get_tick == NULL) {
        EAGLETRT_API_UNUSED(wheel_fsm_api_request(WHEEL_FSM_STATE_ERROR));
        return 0U;
    }
    scheduler_handler.tick = scheduler_handler.config.get_tick();
    return scheduler_handler.tick;
}

/*!
 * \brief Single libtimebase callback: runs a task body and times it.
 *
 * \param task_id The task being executed.
 */
EAGLETRT_STATIC void prv_scheduler_api_run_task(uint8_t task_id) {
    if (task_id >= SCHEDULER_TASK_COUNT) {
        return;
    }

    const scheduler_get_cycles_callback get_cycles = scheduler_handler.config.get_cycles;
    const uint32_t start = (get_cycles != NULL) ? get_cycles() : 0U;

    prv_scheduler_api_tasks[task_id].function(scheduler_handler.tick);

    if (get_cycles == NULL) {
        return;
    }

    // Unsigned subtraction survives the counter wrapping once.
    const uint32_t elapsed_us = (get_cycles() - start) / scheduler_handler.config.cycles_per_us;
    struct SchedulerTaskStats *stats = &scheduler_handler.stats[task_id];
    stats->runs++;
    stats->last_us = elapsed_us;
    stats->max_us = EAGLETRT_API_MAX(stats->max_us, elapsed_us);
    stats->window_max_us = EAGLETRT_API_MAX(stats->window_max_us, elapsed_us);
}

enum SchedulerReturnCode scheduler_api_init(const struct SchedulerConfig *config) {
    if (config == NULL ||
        config->get_tick == NULL ||
        config->poll_inputs == NULL ||
        config->swap_framebuffers == NULL ||
        config->reset == NULL ||
        (config->get_cycles != NULL && config->cycles_per_us == 0U)) {
        return SCHEDULER_RC_NULL_POINTER;
    }

    memset(&scheduler_handler, 0, sizeof(scheduler_handler));
    scheduler_handler.config = *config;
    scheduler_handler.config.post.parameters_on_change = prv_scheduler_api_on_parameter_change;

    const struct WheelFsmConfig fsm_config = {
        .post = &scheduler_handler.config.post,
        .reset = config->reset,
        .enter_flash = prv_scheduler_api_enter_flash,
        .leave_flash = prv_scheduler_api_leave_flash,
    };
    if (wheel_fsm_api_init(&fsm_config) != WHEEL_FSM_RC_OK) {
        return SCHEDULER_RC_ERROR;
    }
    // POST runs once, before any task: a failure leaves the FSM in ERROR
    // and the tasks still report it on the bus. The first step runs INIT,
    // the transition it requests is applied at the end of the same step.
    if (wheel_fsm_api_run_state() != WHEEL_FSM_RC_OK) {
        return SCHEDULER_RC_ERROR;
    }

    TaskList task_list = { 0 };
    for (uint8_t i = 0U; i < SCHEDULER_TASK_COUNT; i++) {
        task_list[i] = (struct Task){
            .task_id = i,
            .state = TASKS_STATE_ENABLED,
            .repeats = 0U,
            .function = prv_scheduler_api_run_task,
            .interval = prv_scheduler_api_tasks[i].interval_ms,
            .start = prv_scheduler_api_tasks[i].start_ms,
        };
    }

    if (tasks_api_init(&scheduler_handler.tasks, task_list, SCHEDULER_TASK_COUNT, prv_scheduler_api_read_tick) != TASKS_RC_OK) {
        return SCHEDULER_RC_ERROR;
    }
    if (watchdogs_api_init_pool(&scheduler_handler.watchdogs, prv_scheduler_api_read_tick) != WATCHDOG_RC_OK) {
        return SCHEDULER_RC_ERROR;
    }
    if (flash_monitor_api_init(&scheduler_handler.watchdogs) != FLASH_MONITOR_RC_OK) {
        return SCHEDULER_RC_ERROR;
    }

    return SCHEDULER_RC_OK;
}

enum SchedulerReturnCode scheduler_api_routine(void) {
    if (scheduler_handler.config.get_tick == NULL) {
        return SCHEDULER_RC_NULL_POINTER;
    }

    enum SchedulerReturnCode return_code = SCHEDULER_RC_OK;
    if (tasks_api_routine(&scheduler_handler.tasks) != TASKS_RC_OK) {
        return_code = SCHEDULER_RC_ERROR;
    }
    if (watchdogs_api_routine(&scheduler_handler.watchdogs) != WATCHDOG_RC_OK) {
        return_code = SCHEDULER_RC_ERROR;
    }
    return return_code;
}

enum WheelFsmState scheduler_api_get_fsm_state(void) {
    return wheel_fsm_api_get_state();
}

enum SchedulerReturnCode scheduler_api_pause_task(enum SchedulerTaskId task_id) {
    if (task_id >= SCHEDULER_TASK_COUNT) {
        return SCHEDULER_RC_ERROR;
    }
    return (tasks_api_pause_task(&scheduler_handler.tasks, task_id) == TASKS_RC_OK) ? SCHEDULER_RC_OK : SCHEDULER_RC_ERROR;
}

enum SchedulerReturnCode scheduler_api_resume_task(enum SchedulerTaskId task_id) {
    if (task_id >= SCHEDULER_TASK_COUNT) {
        return SCHEDULER_RC_ERROR;
    }
    // ENABLED from PAUSED keeps the time that was left until the next run.
    return (tasks_api_enable_task(&scheduler_handler.tasks, task_id) == TASKS_RC_OK) ? SCHEDULER_RC_OK : SCHEDULER_RC_ERROR;
}

uint32_t scheduler_api_get_tick(void) {
    return scheduler_handler.tick;
}
