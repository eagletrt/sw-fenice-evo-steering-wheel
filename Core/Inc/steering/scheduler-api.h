/*!
 * \file scheduler-api.h
 * \date 2026-10-16
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief Public API of the cooperative task scheduler.
 */

#ifndef SCHEDULER_API_H
#define SCHEDULER_API_H

#include "scheduler.h"

/*!
 * \brief Run the POST and register every task.
 *
 * \details Runs the FSM INIT state with \c config->post, then schedules every
 *     task to fire on the first routine call. If the POST fails the FSM is
 *     left in ERROR, the scheduler still runs so the error state reaches the
 *     bus.
 *
 * \param config Hardware hooks, copied into the scheduler.
 *
 * \retval SCHEDULER_RC_OK on success.
 * \retval SCHEDULER_RC_NULL_POINTER if \p config or a required callback is NULL.
 * \retval SCHEDULER_RC_ERROR if libtimebase rejected the task list.
 */
enum SchedulerReturnCode scheduler_api_init(const struct SchedulerConfig *config);

/*!
 * \brief Run every task that is due.
 *
 * \details Call it in the main loop as often as possible. A task that is
 *     late by more than its period runs once and its missed triggers are
 *     dropped (counted by libtimebase, logged by the stats task).
 *
 * \retval SCHEDULER_RC_OK on success.
 * \retval SCHEDULER_RC_ERROR if libtimebase failed (e.g. the tick went backwards).
 */
enum SchedulerReturnCode scheduler_api_routine(void);

/*!
 * \brief Current state of the wheel FSM.
 *
 * \return The FSM state.
 */
enum WheelFsmState scheduler_api_get_fsm_state(void);

/*!
 * \brief Stop a task until scheduler_api_resume_task is called.
 *
 * \details The time left until its next run is kept. Meant for modes that
 *     must silence part of the wheel, e.g. FLASH muting every CAN producer
 *     but the FSM status. Can be called from inside a task, on any task.
 *
 * \param task_id The task to pause.
 *
 * \retval SCHEDULER_RC_OK on success (also if it was already paused).
 * \retval SCHEDULER_RC_ERROR if \p task_id is invalid or libtimebase failed.
 */
enum SchedulerReturnCode scheduler_api_pause_task(enum SchedulerTaskId task_id);

/*!
 * \brief Restart a task paused with scheduler_api_pause_task.
 *
 * \details The task runs again after the time that was left when it was
 *     paused. Resuming a running task does nothing.
 *
 * \param task_id The task to resume.
 *
 * \retval SCHEDULER_RC_OK on success.
 * \retval SCHEDULER_RC_ERROR if \p task_id is invalid or libtimebase failed.
 */
enum SchedulerReturnCode scheduler_api_resume_task(enum SchedulerTaskId task_id);

/*!
 * \brief Tick the tasks are running with, in milliseconds.
 *
 * \details Use it to timestamp events raised from inside a task (e.g. a
 *     parameter change during the inputs task), so they share the time base
 *     of every other task. A fresh hardware read could be newer than the tick
 *     the render task compares against later in the same routine.
 *
 * \return The last tick read by the scheduler.
 */
uint32_t scheduler_api_get_tick(void);

/*!
 * \brief Restart a watchdog, even if it has already timed out.
 *
 * \details The watchdog is unregistered and re-registered with the same timeout
 *    and callback. If it was running it is re-armed, if it was timed
 *    it is re-armed and the callback can fire again after the timeout.
 *
 * \param watchdog The watchdog to restart.
 *
 * \retval SCHEDULER_RC_OK on success.
 * \retval SCHEDULER_RC_NULL_POINTER if \p watchdog is NULL.
 * \retval SCHEDULER_RC_ERROR if the watchdog could not be restarted.
 */
enum SchedulerReturnCode scheduler_api_restart_watchdog(struct Watchdog *watchdog);

/*!
 * \brief Reset a watchdog, even if it has already timed out.
 *
 * \details The watchdog is unregistered and its state is set to NOT_RUNNING.
 *   If it was running it is stopped, if it was timed out it is stopped and
 *   the callback will not fire again until the watchdog is restarted.
 *
 * \param watchdog The watchdog to reset.
 *
 * \retval SCHEDULER_RC_OK on success.
 * \retval SCHEDULER_RC_NULL_POINTER if \p watchdog is NULL.
 * \retval SCHEDULER_RC_ERROR if the watchdog could not be reset.
 */
enum SchedulerReturnCode scheduler_api_reset_watchdog(struct Watchdog *watchdog);

/*!
 * \brief Stop a watchdog, even if it has already timed out.
 *
 * \details The watchdog is unregistered and its state is set to NOT_RUNNING.
 *    If it was running it is stopped, if it was timed out it is stopped and
 *    the callback will not fire again until the watchdog is restarted.
 *
 * \param watchdog The watchdog to stop.
 *
 * \retval SCHEDULER_RC_OK on success.
 * \retval SCHEDULER_RC_NULL_POINTER if \p watchdog is NULL.
 * \retval SCHEDULER_RC_ERROR if the watchdog could not be stopped.
 */
enum SchedulerReturnCode scheduler_api_stop_watchdog(struct Watchdog *watchdog);

#endif // SCHEDULER_API_H
