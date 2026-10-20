/*!
 * \file test-scheduler.c
 * \date 2026-10-16
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 *
 * \brief Unit tests for the cooperative task scheduler.
 *
 * \details The scheduler runs the real modules (POST, CAN router, identity,
 *     screen); only the hardware hooks are faked. CAN frames are captured at
 *     the send callback, i.e. after the TX flush task.
 */

#include "unity.h"
#include "fff.h"
#include "scheduler-api.h"
#include "can-communications-router-api.h"
#include "can-primary.h"
#include "inputs-api.h"
#include "parameters-api.h"
#include "screen.h"
#include "screen-api.h"
#include "tasks-api.h"
#include "ui-data-api.h"
#include <string.h>

DEFINE_FFF_GLOBALS;

extern struct SchedulerHandler scheduler_handler;
extern struct ScreenHandler screen_handler;

/* Private helper, reachable because the test build defines EAGLETRT_STATIC as empty. */
enum CanPrimarySteeringwheelfsmStatus prv_scheduler_api_fsm_status(fsm_state_t state);

FAKE_VALUE_FUNC(uint32_t, fake_get_tick);
FAKE_VOID_FUNC(fake_poll_inputs, uint32_t);
FAKE_VOID_FUNC(fake_swap_framebuffers);
FAKE_VOID_FUNC(fake_reset);
FAKE_VALUE_FUNC(uint32_t, fake_get_cycles);
FAKE_VALUE_FUNC(enum RasterReturnCode, fake_draw_rectangle, uint16_t, uint16_t, uint16_t, uint16_t, struct Color);

#define TEST_SCHEDULER_SENT_CAPACITY (512U)
#define TEST_SCHEDULER_CYCLES_PER_US (10U)
#define TEST_SCHEDULER_CYCLES_PER_CALL (250U) /* 25 us between two get_cycles calls */

static uint32_t prv_now;
static uint32_t prv_cycles;
static uint32_t prv_sent_ids[TEST_SCHEDULER_SENT_CAPACITY];
static size_t prv_sent_count;

static uint32_t prv_get_tick(void) {
    return prv_now;
}

static uint32_t prv_get_cycles(void) {
    prv_cycles += TEST_SCHEDULER_CYCLES_PER_CALL;
    return prv_cycles;
}

static enum CanCommunicationReturnCode prv_capture_send(const struct CanCommunicationFrame *frame) {
    if (prv_sent_count < TEST_SCHEDULER_SENT_CAPACITY) {
        prv_sent_ids[prv_sent_count++] = frame->id;
    }
    return CAN_COMMUNICATION_RC_OK;
}

/* Wall clock that moves forward on every read, like HAL_GetTick during a slow I2C poll. */
static uint32_t prv_get_tick_flowing(void) {
    return prv_now++;
}

/* Turns the power knob by one step, as the real inputs task would on a knob click. */
static void prv_poll_inputs_turn_knob(uint32_t tick_ms) {
    (void)tick_ms;
    parameters_api_handle_knob(INPUTS_SHARED_KNOB_ID_FRONT_LEFT, 1);
}

/* First tick at which libtimebase runs a task (its staggered start). */
static uint32_t prv_task_start(enum SchedulerTaskId task_id) {
    return scheduler_handler.tasks.task_list[task_id].start;
}

/* Ticks at which inputs and render ran, to check they never share a routine. */
#define TEST_SCHEDULER_TRACE_CAPACITY (64U)
static uint32_t prv_inputs_ticks[TEST_SCHEDULER_TRACE_CAPACITY];
static size_t prv_inputs_count;
static uint32_t prv_render_ticks[TEST_SCHEDULER_TRACE_CAPACITY];
static size_t prv_render_count;

static void prv_poll_inputs_trace(uint32_t tick_ms) {
    if (prv_inputs_count < TEST_SCHEDULER_TRACE_CAPACITY) {
        prv_inputs_ticks[prv_inputs_count++] = tick_ms;
    }
}

static void prv_swap_trace(void) {
    if (prv_render_count < TEST_SCHEDULER_TRACE_CAPACITY) {
        prv_render_ticks[prv_render_count++] = scheduler_api_get_tick();
    }
}

/* Holds the top-left PTT paddle down. */
static void prv_poll_inputs_hold_ptt(uint32_t tick_ms) {
    inputs_api_update_button(INPUTS_SHARED_BUTTON_ID_PADDLE_TOP_LEFT, true, tick_ms);
}

static size_t prv_count_sent(uint32_t id) {
    size_t count = 0U;
    for (size_t i = 0U; i < prv_sent_count; i++) {
        if (prv_sent_ids[i] == id) {
            count++;
        }
    }
    return count;
}

static struct SchedulerConfig prv_make_config(void) {
    return (struct SchedulerConfig){
        .post = {
            .can_network_configs = {
                [CAN_COMMUNICATION_NETWORK_PRIMARY] = {
                    .send = prv_capture_send,
                    .on_receive = can_communications_router_api_receive_primary,
                },
                [CAN_COMMUNICATION_NETWORK_SECONDARY] = {
                    .send = prv_capture_send,
                    .on_receive = can_communications_router_api_receive_secondary,
                },
            },
            .draw_rectangle = fake_draw_rectangle,
        },
        .get_tick = fake_get_tick,
        .poll_inputs = fake_poll_inputs,
        .swap_framebuffers = fake_swap_framebuffers,
        .reset = fake_reset,
        .get_cycles = NULL,
        .cycles_per_us = 0U,
    };
}

/*!
 * \brief Call the routine once per millisecond from the current tick up to \p last_tick included.
 */
static void prv_run_until(uint32_t last_tick) {
    for (;;) {
        TEST_ASSERT_EQUAL(SCHEDULER_RC_OK, scheduler_api_routine());
        if (prv_now >= last_tick) {
            break;
        }
        prv_now++;
    }
}

void setUp(void) {
    RESET_FAKE(fake_get_tick);
    RESET_FAKE(fake_poll_inputs);
    RESET_FAKE(fake_swap_framebuffers);
    RESET_FAKE(fake_reset);
    RESET_FAKE(fake_get_cycles);
    RESET_FAKE(fake_draw_rectangle);
    FFF_RESET_HISTORY();

    prv_now = 0U;
    prv_cycles = 0U;
    prv_sent_count = 0U;
    fake_get_tick_fake.custom_fake = prv_get_tick;
    fake_get_cycles_fake.custom_fake = prv_get_cycles;
    fake_draw_rectangle_fake.return_val = RASTER_RC_OK;
}

/*!
 * \defgroup scheduler_api_init Tests for scheduler_api_init function
 * \{
 */

void test_scheduler_init_rejects_null_config(void) {
    TEST_ASSERT_EQUAL(SCHEDULER_RC_NULL_POINTER, scheduler_api_init(NULL));
}

void test_scheduler_init_rejects_missing_callbacks(void) {
    struct SchedulerConfig config = prv_make_config();
    config.get_tick = NULL;
    TEST_ASSERT_EQUAL(SCHEDULER_RC_NULL_POINTER, scheduler_api_init(&config));

    config = prv_make_config();
    config.poll_inputs = NULL;
    TEST_ASSERT_EQUAL(SCHEDULER_RC_NULL_POINTER, scheduler_api_init(&config));

    config = prv_make_config();
    config.swap_framebuffers = NULL;
    TEST_ASSERT_EQUAL(SCHEDULER_RC_NULL_POINTER, scheduler_api_init(&config));

    config = prv_make_config();
    config.reset = NULL;
    TEST_ASSERT_EQUAL(SCHEDULER_RC_NULL_POINTER, scheduler_api_init(&config));
}

void test_scheduler_init_rejects_cycle_counter_without_frequency(void) {
    struct SchedulerConfig config = prv_make_config();
    config.get_cycles = fake_get_cycles;
    config.cycles_per_us = 0U;
    TEST_ASSERT_EQUAL(SCHEDULER_RC_NULL_POINTER, scheduler_api_init(&config));
}

void test_scheduler_init_runs_post_into_idle(void) {
    struct SchedulerConfig config = prv_make_config();
    TEST_ASSERT_EQUAL(SCHEDULER_RC_OK, scheduler_api_init(&config));
    TEST_ASSERT_EQUAL(FSM_STATE_IDLE, scheduler_api_get_fsm_state());
}

void test_scheduler_init_post_failure_leaves_fsm_in_error(void) {
    struct SchedulerConfig config = prv_make_config();
    config.post.draw_rectangle = NULL;
    TEST_ASSERT_EQUAL_MESSAGE(SCHEDULER_RC_OK, scheduler_api_init(&config), "A failed POST must not stop the scheduler: ERROR still has to reach the bus");
    TEST_ASSERT_EQUAL(FSM_STATE_ERROR, scheduler_api_get_fsm_state());
}

/*! \} */

/*!
 * \defgroup scheduler_api_routine Tests for the task table
 * \{
 */

void test_scheduler_first_routine_runs_every_unstaggered_task(void) {
    struct SchedulerConfig config = prv_make_config();
    scheduler_api_init(&config);

    prv_run_until(0U);

    TEST_ASSERT_EQUAL(1, fake_poll_inputs_fake.call_count);
    TEST_ASSERT_EQUAL_MESSAGE(0, fake_swap_framebuffers_fake.call_count, "Render is staggered, it must not run at tick 0");
    TEST_ASSERT_EQUAL(1, prv_count_sent(CAN_PRIMARY_MESSAGE_FRAME_ID_CONTROLMAPSSET));
    TEST_ASSERT_EQUAL(1, prv_count_sent(CAN_PRIMARY_MESSAGE_FRAME_ID_STEERINGWHEELBUTTONSTATUS));
    TEST_ASSERT_EQUAL(1, prv_count_sent(CAN_PRIMARY_MESSAGE_FRAME_ID_STEERINGWHEELFSM));
    TEST_ASSERT_EQUAL_MESSAGE(1, prv_count_sent(CAN_PRIMARY_MESSAGE_FRAME_ID_STEERINGWHEELVERSION), "Version goes out at boot, not after the first 10 s");
    TEST_ASSERT_EQUAL(1, prv_count_sent(CAN_PRIMARY_MESSAGE_FRAME_ID_STEERINGWHEELVERSIONINFO));
    TEST_ASSERT_EQUAL(1, prv_count_sent(CAN_PRIMARY_MESSAGE_FRAME_ID_STEERINGWHEELLIBCANVERSION));
    TEST_ASSERT_EQUAL(1, prv_count_sent(CAN_PRIMARY_MESSAGE_FRAME_ID_STEERINGWHEELLIBCANVERSIONINFO));
}

void test_scheduler_first_frame_presented_at_render_start(void) {
    struct SchedulerConfig config = prv_make_config();
    scheduler_api_init(&config);

    prv_run_until(prv_task_start(SCHEDULER_TASK_RENDER));

    TEST_ASSERT_EQUAL_MESSAGE(1, fake_swap_framebuffers_fake.call_count, "The first frame is fully dirty and must be presented");
}

void test_scheduler_inputs_and_render_never_share_a_routine(void) {
    struct SchedulerConfig config = prv_make_config();
    config.poll_inputs = prv_poll_inputs_trace;
    config.swap_framebuffers = prv_swap_trace;
    prv_inputs_count = 0U;
    prv_render_count = 0U;
    scheduler_api_init(&config);

    /* Something changes every tick, so every render presents a frame (and is traced). */
    for (;;) {
        screen_api_set_power((uint8_t)(prv_now % 10U));
        TEST_ASSERT_EQUAL(SCHEDULER_RC_OK, scheduler_api_routine());
        if (prv_now >= 199U) {
            break;
        }
        prv_now++;
    }

    TEST_ASSERT_TRUE(prv_render_count > 0U);
    for (size_t r = 0U; r < prv_render_count; r++) {
        for (size_t i = 0U; i < prv_inputs_count; i++) {
            TEST_ASSERT_NOT_EQUAL_MESSAGE(prv_inputs_ticks[i], prv_render_ticks[r], "The ~1.6 ms I2C poll and a frame must not pile up in the same routine");
        }
    }
}

void test_scheduler_inputs_polled_every_10_ms(void) {
    struct SchedulerConfig config = prv_make_config();
    scheduler_api_init(&config);

    prv_run_until(99U);

    TEST_ASSERT_EQUAL(10, fake_poll_inputs_fake.call_count);
    TEST_ASSERT_EQUAL_UINT32(90U, fake_poll_inputs_fake.arg0_val);
}

void test_scheduler_can_messages_follow_libcan_cycle_times(void) {
    struct SchedulerConfig config = prv_make_config();
    scheduler_api_init(&config);

    prv_run_until(599U);

    TEST_ASSERT_EQUAL(600U / (uint32_t)can_primary_cycle_time_controlmapsset, prv_count_sent(CAN_PRIMARY_MESSAGE_FRAME_ID_CONTROLMAPSSET));
    TEST_ASSERT_EQUAL(600U / (uint32_t)can_primary_cycle_time_steeringwheelbuttonstatus, prv_count_sent(CAN_PRIMARY_MESSAGE_FRAME_ID_STEERINGWHEELBUTTONSTATUS));
    TEST_ASSERT_EQUAL(600U / (uint32_t)can_primary_cycle_time_steeringwheelfsm, prv_count_sent(CAN_PRIMARY_MESSAGE_FRAME_ID_STEERINGWHEELFSM));
    TEST_ASSERT_EQUAL(1, prv_count_sent(CAN_PRIMARY_MESSAGE_FRAME_ID_STEERINGWHEELVERSION));
}

void test_scheduler_render_skipped_outside_idle(void) {
    struct SchedulerConfig config = prv_make_config();
    scheduler_api_init(&config);
    scheduler_handler.fsm_state = FSM_STATE_ERROR;

    prv_run_until(100U);

    TEST_ASSERT_EQUAL_MESSAGE(0, fake_swap_framebuffers_fake.call_count, "ERROR freezes the last frame on screen");
    TEST_ASSERT_TRUE_MESSAGE(prv_count_sent(CAN_PRIMARY_MESSAGE_FRAME_ID_STEERINGWHEELFSM) > 0U, "The FSM status must still reach the bus in ERROR");
}

void test_scheduler_render_failure_moves_fsm_to_error(void) {
    struct SchedulerConfig config = prv_make_config();
    scheduler_api_init(&config);
    fake_draw_rectangle_fake.return_val = RASTER_RC_ERROR;

    prv_run_until(10U);

    TEST_ASSERT_EQUAL(FSM_STATE_ERROR, scheduler_api_get_fsm_state());
}

void test_scheduler_reset_request_resets_the_board(void) {
    struct SchedulerConfig config = prv_make_config();
    scheduler_api_init(&config);

    const struct CanCommunicationFrame frame = { .id = CAN_PRIMARY_MESSAGE_FRAME_ID_STEERINGWHEELFLASHINGRX, .length = 0U };
    can_communications_router_api_receive_primary(&frame);
    prv_run_until(5U);

    TEST_ASSERT_TRUE(fake_reset_fake.call_count > 0U);
}

void test_scheduler_fsm_status_mapping(void) {
    TEST_ASSERT_EQUAL(CAN_PRIMARY_STEERINGWHEELFSM_STATUS_INIT, prv_scheduler_api_fsm_status(FSM_STATE_INIT));
    TEST_ASSERT_EQUAL(CAN_PRIMARY_STEERINGWHEELFSM_STATUS_IDLE, prv_scheduler_api_fsm_status(FSM_STATE_IDLE));
    TEST_ASSERT_EQUAL(CAN_PRIMARY_STEERINGWHEELFSM_STATUS_ERROR, prv_scheduler_api_fsm_status(FSM_STATE_ERROR));
    TEST_ASSERT_EQUAL(CAN_PRIMARY_STEERINGWHEELFSM_STATUS_FLASH, prv_scheduler_api_fsm_status(FSM_STATE_FLASH));
}

void test_scheduler_routine_rejects_time_going_backwards(void) {
    struct SchedulerConfig config = prv_make_config();
    prv_now = 100U;
    scheduler_api_init(&config);
    prv_now = 50U;
    TEST_ASSERT_EQUAL(SCHEDULER_RC_ERROR, scheduler_api_routine());
}

void test_scheduler_knob_popup_shows_on_next_frame(void) {
    struct SchedulerConfig config = prv_make_config();
    config.poll_inputs = prv_poll_inputs_turn_knob;
    fake_get_tick_fake.custom_fake = prv_get_tick_flowing;
    scheduler_api_init(&config);

    /* Time keeps flowing on every clock read, like HAL_GetTick during a slow I2C poll. */
    for (uint32_t calls = 0U; calls < 100U && fake_swap_framebuffers_fake.call_count == 0U; calls++) {
        TEST_ASSERT_EQUAL(SCHEDULER_RC_OK, scheduler_api_routine());
    }

    TEST_ASSERT_TRUE_MESSAGE(screen_handler.popup_visible, "The popup raised by the knob must be mounted by the next frame");
}

void test_scheduler_tasks_get_the_tick_libtimebase_acted_on(void) {
    struct SchedulerConfig config = prv_make_config();
    scheduler_api_init(&config);
    prv_now = 40U;

    TEST_ASSERT_EQUAL(SCHEDULER_RC_OK, scheduler_api_routine());

    TEST_ASSERT_EQUAL_UINT32(40U, scheduler_api_get_tick());
    TEST_ASSERT_EQUAL_UINT32(40U, fake_poll_inputs_fake.arg0_val);
}

void test_scheduler_late_routine_drops_missed_runs(void) {
    struct SchedulerConfig config = prv_make_config();
    scheduler_api_init(&config);
    prv_run_until(0U);
    const unsigned polls_before = fake_poll_inputs_fake.call_count;

    prv_now = 100U; /* 10 inputs periods late */
    TEST_ASSERT_EQUAL(SCHEDULER_RC_OK, scheduler_api_routine());

    TEST_ASSERT_EQUAL_MESSAGE(polls_before + 1U, fake_poll_inputs_fake.call_count, "A late task runs once, not once per missed trigger");
    TEST_ASSERT_TRUE(tasks_api_get_dropped_task_executions(&scheduler_handler.tasks) > 0U);
}

void test_scheduler_parameter_change_reaches_ui_bus_and_popup(void) {
    struct SchedulerConfig config = prv_make_config();
    config.poll_inputs = prv_poll_inputs_turn_knob;
    scheduler_api_init(&config);
    memset(ui_data_api_get(), 0, sizeof(struct UIData));

    prv_run_until(prv_task_start(SCHEDULER_TASK_RENDER));

    TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, ui_data_api_get()->power, "The knob step must be mirrored into the UI snapshot");
    TEST_ASSERT_TRUE_MESSAGE(prv_count_sent(CAN_PRIMARY_MESSAGE_FRAME_ID_CONTROLMAPSSET) >= 2U, "The change must go on the bus right away, on top of the cyclic send");
    TEST_ASSERT_TRUE_MESSAGE(screen_handler.popup_visible, "A shared parameter must raise the popup");
}

void test_scheduler_ptt_tints_dashboard_without_popup(void) {
    struct SchedulerConfig config = prv_make_config();
    config.poll_inputs = prv_poll_inputs_hold_ptt;
    scheduler_api_init(&config);
    memset(ui_data_api_get(), 0, sizeof(struct UIData));

    prv_run_until(prv_task_start(SCHEDULER_TASK_RENDER));

    TEST_ASSERT_EQUAL_UINT8(1U, ui_data_api_get()->ptt);
    TEST_ASSERT_FALSE_MESSAGE(screen_handler.popup_visible, "PTT is not shared: no popup");
}

void test_scheduler_paused_task_stops_until_resumed(void) {
    struct SchedulerConfig config = prv_make_config();
    scheduler_api_init(&config);
    prv_run_until(0U);
    const size_t fsm_frames = prv_count_sent(CAN_PRIMARY_MESSAGE_FRAME_ID_STEERINGWHEELFSM);

    TEST_ASSERT_EQUAL(SCHEDULER_RC_OK, scheduler_api_pause_task(SCHEDULER_TASK_CAN_FSM_STATUS));
    prv_run_until(1000U);
    TEST_ASSERT_EQUAL_MESSAGE(fsm_frames, prv_count_sent(CAN_PRIMARY_MESSAGE_FRAME_ID_STEERINGWHEELFSM), "A paused task must not run");
    TEST_ASSERT_TRUE_MESSAGE(prv_count_sent(CAN_PRIMARY_MESSAGE_FRAME_ID_CONTROLMAPSSET) > 1U, "Other tasks keep running");

    TEST_ASSERT_EQUAL(SCHEDULER_RC_OK, scheduler_api_resume_task(SCHEDULER_TASK_CAN_FSM_STATUS));
    prv_run_until(1000U + (uint32_t)can_primary_cycle_time_steeringwheelfsm);
    TEST_ASSERT_TRUE_MESSAGE(prv_count_sent(CAN_PRIMARY_MESSAGE_FRAME_ID_STEERINGWHEELFSM) > fsm_frames, "A resumed task must run again");
}

void test_scheduler_resume_running_task_is_harmless(void) {
    struct SchedulerConfig config = prv_make_config();
    scheduler_api_init(&config);
    TEST_ASSERT_EQUAL(SCHEDULER_RC_OK, scheduler_api_resume_task(SCHEDULER_TASK_CAN_RX));
}

void test_scheduler_pause_resume_reject_invalid_task(void) {
    struct SchedulerConfig config = prv_make_config();
    scheduler_api_init(&config);
    TEST_ASSERT_EQUAL(SCHEDULER_RC_ERROR, scheduler_api_pause_task(SCHEDULER_TASK_COUNT));
    TEST_ASSERT_EQUAL(SCHEDULER_RC_ERROR, scheduler_api_resume_task(SCHEDULER_TASK_COUNT));
}

/*! \} */

/*!
 * \defgroup scheduler_stats Tests for the task timing statistics
 * \{
 */

void test_scheduler_stats_record_task_duration(void) {
    struct SchedulerConfig config = prv_make_config();
    config.get_cycles = fake_get_cycles;
    config.cycles_per_us = TEST_SCHEDULER_CYCLES_PER_US;
    scheduler_api_init(&config);

    prv_run_until(0U);

    const struct SchedulerTaskStats *stats = &scheduler_handler.stats[SCHEDULER_TASK_CAN_RX];
    TEST_ASSERT_EQUAL_UINT32(1U, stats->runs);
    TEST_ASSERT_EQUAL_UINT32(TEST_SCHEDULER_CYCLES_PER_CALL / TEST_SCHEDULER_CYCLES_PER_US, stats->last_us);
    TEST_ASSERT_EQUAL_UINT32(stats->last_us, stats->max_us);
}

void test_scheduler_stats_off_without_cycle_counter(void) {
    struct SchedulerConfig config = prv_make_config();
    scheduler_api_init(&config);

    prv_run_until(10U);

    TEST_ASSERT_EQUAL_UINT32(0U, scheduler_handler.stats[SCHEDULER_TASK_CAN_RX].runs);
    TEST_ASSERT_EQUAL(0, fake_get_cycles_fake.call_count);
}

void test_scheduler_stats_window_reset_by_stats_task(void) {
    struct SchedulerConfig config = prv_make_config();
    config.get_cycles = fake_get_cycles;
    config.cycles_per_us = TEST_SCHEDULER_CYCLES_PER_US;
    scheduler_api_init(&config);

    const uint32_t second_log = prv_task_start(SCHEDULER_TASK_STATS) + SCHEDULER_STATS_LOG_PERIOD_MS;

    prv_run_until(second_log - 1U);
    TEST_ASSERT_NOT_EQUAL_UINT32(0U, scheduler_handler.stats[SCHEDULER_TASK_CAN_RX].window_max_us);

    prv_run_until(second_log);
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0U, scheduler_handler.stats[SCHEDULER_TASK_CAN_RX].window_max_us, "The stats task must start a new window");
    TEST_ASSERT_NOT_EQUAL_UINT32_MESSAGE(0U, scheduler_handler.stats[SCHEDULER_TASK_CAN_RX].max_us, "The all-time max is never reset");
}

/*! \} */

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_scheduler_init_rejects_null_config);
    RUN_TEST(test_scheduler_init_rejects_missing_callbacks);
    RUN_TEST(test_scheduler_init_rejects_cycle_counter_without_frequency);
    RUN_TEST(test_scheduler_init_runs_post_into_idle);
    RUN_TEST(test_scheduler_init_post_failure_leaves_fsm_in_error);

    RUN_TEST(test_scheduler_first_routine_runs_every_unstaggered_task);
    RUN_TEST(test_scheduler_first_frame_presented_at_render_start);
    RUN_TEST(test_scheduler_inputs_and_render_never_share_a_routine);
    RUN_TEST(test_scheduler_inputs_polled_every_10_ms);
    RUN_TEST(test_scheduler_can_messages_follow_libcan_cycle_times);
    RUN_TEST(test_scheduler_render_skipped_outside_idle);
    RUN_TEST(test_scheduler_render_failure_moves_fsm_to_error);
    RUN_TEST(test_scheduler_reset_request_resets_the_board);
    RUN_TEST(test_scheduler_fsm_status_mapping);
    RUN_TEST(test_scheduler_routine_rejects_time_going_backwards);
    RUN_TEST(test_scheduler_knob_popup_shows_on_next_frame);
    RUN_TEST(test_scheduler_tasks_get_the_tick_libtimebase_acted_on);
    RUN_TEST(test_scheduler_late_routine_drops_missed_runs);
    RUN_TEST(test_scheduler_parameter_change_reaches_ui_bus_and_popup);
    RUN_TEST(test_scheduler_ptt_tints_dashboard_without_popup);
    RUN_TEST(test_scheduler_paused_task_stops_until_resumed);
    RUN_TEST(test_scheduler_resume_running_task_is_harmless);
    RUN_TEST(test_scheduler_pause_resume_reject_invalid_task);

    RUN_TEST(test_scheduler_stats_record_task_duration);
    RUN_TEST(test_scheduler_stats_off_without_cycle_counter);
    RUN_TEST(test_scheduler_stats_window_reset_by_stats_task);

    return UNITY_END();
}
