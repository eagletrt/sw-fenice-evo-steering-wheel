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
#include "identity-api.h"
#include "inputs-api.h"
#include "logger-api.h"
#include "screen-api.h"
#include "tasks-api.h"
#include "ui-data-api.h"
#include "watchdogs-api.h"

#include <inttypes.h>
#include <string.h>

static_assert(SCHEDULER_TASK_COUNT <= MAX_TASKS, "libtimebase cannot hold every scheduler task");

/*!
 * \brief Body of a task, called with the tick of the current routine call.
 */
typedef void (*scheduler_task_function)(uint32_t tick_ms);

/*!
 * \brief Period and body of one task.
 */
struct SchedulerTask {
    uint16_t interval_ms;             /*!< Time between two executions */
    scheduler_task_function function; /*!< Task body */
};

EAGLETRT_STATIC struct SchedulerHandler scheduler_handler;

/*!
 * \brief Map the wheel FSM state onto the status signal of SteeringWheelFsm.
 *
 * \param state The current FSM state.
 *
 * \return The matching CanPrimarySteeringwheelfsmStatus value.
 */
EAGLETRT_STATIC enum CanPrimarySteeringwheelfsmStatus prv_scheduler_api_fsm_status(fsm_state_t state) {
    switch (state) {
        case FSM_STATE_IDLE:
            return CAN_PRIMARY_STEERINGWHEELFSM_STATUS_IDLE;
        case FSM_STATE_ERROR:
            return CAN_PRIMARY_STEERINGWHEELFSM_STATUS_ERROR;
        case FSM_STATE_FLASH:
            return CAN_PRIMARY_STEERINGWHEELFSM_STATUS_FLASH;
        case FSM_STATE_AUTONOMOUS:
            return CAN_PRIMARY_STEERINGWHEELFSM_STATUS_AUTONOMOUS;
        case FSM_STATE_INIT:
        default:
            return CAN_PRIMARY_STEERINGWHEELFSM_STATUS_INIT;
    }
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
    scheduler_handler.fsm_data.tick = tick_ms;
    scheduler_handler.fsm_state = fsm_run_state(scheduler_handler.fsm_state, &scheduler_handler.fsm_data);
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
    EAGLETRT_API_UNUSED(identity_api_send_state(prv_scheduler_api_fsm_status(scheduler_handler.fsm_state)));
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
    // Same as before the scheduler: the screen is only driven in IDLE, an
    // ERROR leaves the last frame on screen.
    if (scheduler_handler.fsm_state != FSM_STATE_IDLE) {
        return;
    }

    // Setters are no-ops when nothing changed, so syncing every frame is cheap.
    if (screen_api_sync_data(ui_data_api_get()) != SCREEN_RC_OK) {
        scheduler_handler.fsm_data.error_detected = true;
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
            scheduler_handler.fsm_data.error_detected = true;
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

    for (uint8_t i = 0U; i < SCHEDULER_TASK_COUNT; i++) {
        stats[i].window_max_us = 0U;
    }
}

/*!
 * \brief Period and body of every task, indexed by SchedulerTaskId.
 *
 * \details Cyclic CAN messages use their libcan cycle time.
 */
EAGLETRT_STATIC const struct SchedulerTask prv_scheduler_api_tasks[SCHEDULER_TASK_COUNT] = {
    [SCHEDULER_TASK_CAN_RX] = { 1U, prv_scheduler_api_task_can_rx },
    [SCHEDULER_TASK_INPUTS] = { 10U, prv_scheduler_api_task_inputs },
    [SCHEDULER_TASK_FSM] = { 5U, prv_scheduler_api_task_fsm },
    [SCHEDULER_TASK_CAN_CONTROL_MAPS] = { (uint16_t)can_primary_cycle_time_controlmapsset, prv_scheduler_api_task_can_control_maps },
    [SCHEDULER_TASK_CAN_BUTTON_STATUS] = { (uint16_t)can_primary_cycle_time_steeringwheelbuttonstatus, prv_scheduler_api_task_can_button_status },
    [SCHEDULER_TASK_CAN_FSM_STATUS] = { (uint16_t)can_primary_cycle_time_steeringwheelfsm, prv_scheduler_api_task_can_fsm_status },
    [SCHEDULER_TASK_CAN_VERSION] = { (uint16_t)can_primary_cycle_time_steeringwheelversion, prv_scheduler_api_task_can_version },
    [SCHEDULER_TASK_CAN_TX] = { 1U, prv_scheduler_api_task_can_tx },
    [SCHEDULER_TASK_RENDER] = { 20U, prv_scheduler_api_task_render },
    [SCHEDULER_TASK_STATS] = { SCHEDULER_STATS_LOG_PERIOD_MS, prv_scheduler_api_task_stats },
};

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

    // POST runs once, before any task: a failure leaves the FSM in ERROR
    // and the tasks still report it on the bus.
    scheduler_handler.fsm_state = fsm_run_state(FSM_STATE_INIT, &scheduler_handler.config.post);
    scheduler_handler.fsm_data = (struct FsmData){
        .swap_framebuffers = config->swap_framebuffers,
        .reset = config->reset,
    };

    TaskList task_list = { 0 };
    for (uint8_t i = 0U; i < SCHEDULER_TASK_COUNT; i++) {
        task_list[i] = (struct Task){
            .task_id = i,
            .state = TASKS_STATE_ENABLED,
            .repeats = 0U,
            .function = prv_scheduler_api_run_task,
            .interval = prv_scheduler_api_tasks[i].interval_ms,
            .start = 0U,
        };
    }

    const uint32_t tick = config->get_tick();
    scheduler_handler.tick = tick;
    if (tasks_api_init(&scheduler_handler.tasks, task_list, SCHEDULER_TASK_COUNT, tick) != TASKS_RC_OK) {
        return SCHEDULER_RC_ERROR;
    }
    if (watchdogs_api_init_pool(&scheduler_handler.watchdogs, tick) != WATCHDOG_RC_OK) {
        return SCHEDULER_RC_ERROR;
    }

    return SCHEDULER_RC_OK;
}

enum SchedulerReturnCode scheduler_api_routine(void) {
    if (scheduler_handler.config.get_tick == NULL) {
        return SCHEDULER_RC_NULL_POINTER;
    }

    scheduler_handler.tick = scheduler_handler.config.get_tick();

    enum SchedulerReturnCode return_code = SCHEDULER_RC_OK;
    if (tasks_api_routine(&scheduler_handler.tasks, scheduler_handler.tick) != TASKS_RC_OK) {
        return_code = SCHEDULER_RC_ERROR;
    }
    if (watchdogs_api_routine(&scheduler_handler.watchdogs, scheduler_handler.tick) != WATCHDOG_RC_OK) {
        return_code = SCHEDULER_RC_ERROR;
    }
    return return_code;
}

fsm_state_t scheduler_api_get_fsm_state(void) {
    return scheduler_handler.fsm_state;
}
