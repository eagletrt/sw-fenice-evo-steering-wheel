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
 * \details Call it in the main loop as often as possible. Tasks that missed
 *     triggers (because a previous task ran long) are executed once per
 *     missed trigger.
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
fsm_state_t scheduler_api_get_fsm_state(void);

#endif // SCHEDULER_API_H
