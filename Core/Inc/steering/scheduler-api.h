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

#endif // SCHEDULER_API_H
