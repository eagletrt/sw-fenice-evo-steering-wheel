/*!
 * \file test-wheel-fsm.c
 * \date 2026-10-26
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 *
 * \brief Unit tests for the steering wheel state machine.
 */

#include "unity.h"
#include "fff.h"
#include "app/fsm/wheel-fsm-api.h"
#include "can-primary.h"
#include "can/flash-monitor/flash-monitor.h"
#include "util/timeout/timeout-api.h"
#include <string.h>

DEFINE_FFF_GLOBALS;

extern struct WheelFsmHandler wheel_fsm_handler;
extern struct FlashMonitorHandler flash_monitor_handler;

FAKE_VOID_FUNC(fake_enter_flash);
FAKE_VOID_FUNC(fake_leave_flash);
FAKE_VALUE_FUNC(bool, fake_parameters_on_change, enum InputsSharedParameterID, uint8_t);
FAKE_VALUE_FUNC(enum CanCommunicationReturnCode, fake_send, const struct CanCommunicationFrame *);
FAKE_VALUE_FUNC(enum CanCommunicationReturnCode, fake_on_receive, const struct CanCommunicationFrame *);
FAKE_VALUE_FUNC(enum RasterReturnCode, fake_draw_rectangle, uint16_t, uint16_t, uint16_t, uint16_t, struct Color);

static struct PostInitData prv_post;
static uint32_t prv_now;

static uint32_t prv_get_tick(void) {
    return prv_now;
}

/* Move the clock and run the timeout pool, as the scheduler routine does. */
static void prv_at(uint32_t tick) {
    prv_now = tick;
    TEST_ASSERT_EQUAL(TIMEOUT_RC_OK, timeout_api_routine());
}

static struct WheelFsmConfig prv_make_config(void) {
    return (struct WheelFsmConfig){
        .post = &prv_post,
        .enter_flash = fake_enter_flash,
        .leave_flash = fake_leave_flash,
    };
}

/* Init, run the POST, then wait out the boot logo into IDLE. */
static void prv_start_in_idle(void) {
    const struct WheelFsmConfig config = prv_make_config();
    TEST_ASSERT_EQUAL(WHEEL_FSM_RC_OK, wheel_fsm_api_init(&config));
    TEST_ASSERT_EQUAL(WHEEL_FSM_RC_OK, wheel_fsm_api_run_state());
    prv_at(WHEEL_FSM_BOOT_LOGO_MS);
    TEST_ASSERT_EQUAL(WHEEL_FSM_RC_OK, wheel_fsm_api_run_state());
    TEST_ASSERT_EQUAL(WHEEL_FSM_STATE_IDLE, wheel_fsm_api_get_state());
}

/* Request a state and apply it with one step. */
static void prv_go(enum WheelFsmState state) {
    TEST_ASSERT_EQUAL(WHEEL_FSM_RC_OK, wheel_fsm_api_request(state));
    TEST_ASSERT_EQUAL(WHEEL_FSM_RC_OK, wheel_fsm_api_run_state());
    TEST_ASSERT_EQUAL(state, wheel_fsm_api_get_state());
}

void setUp(void) {
    RESET_FAKE(fake_enter_flash);
    RESET_FAKE(fake_leave_flash);
    RESET_FAKE(fake_parameters_on_change);
    RESET_FAKE(fake_send);
    RESET_FAKE(fake_on_receive);
    RESET_FAKE(fake_draw_rectangle);
    FFF_RESET_HISTORY();

    fake_parameters_on_change_fake.return_val = true;
    fake_send_fake.return_val = CAN_COMMUNICATION_RC_OK;
    fake_on_receive_fake.return_val = CAN_COMMUNICATION_RC_OK;
    fake_draw_rectangle_fake.return_val = RASTER_RC_OK;

    prv_post = (struct PostInitData){
        .parameters_on_change = fake_parameters_on_change,
        .can_network_configs = {
            [CAN_COMMUNICATION_NETWORK_PRIMARY] = { .send = fake_send, .on_receive = fake_on_receive },
            [CAN_COMMUNICATION_NETWORK_SECONDARY] = { .send = fake_send, .on_receive = fake_on_receive },
        },
        .draw_rectangle = fake_draw_rectangle,
    };

    prv_now = 0U;
    TEST_ASSERT_EQUAL(TIMEOUT_RC_OK, timeout_api_init(prv_get_tick));
    memset(&wheel_fsm_handler, 0, sizeof(wheel_fsm_handler));
    memset(&flash_monitor_handler, 0, sizeof(flash_monitor_handler));
}

void tearDown(void) {}

/*!
 * \defgroup wheel_fsm_api_init Tests for wheel_fsm_api_init
 * \{
 */

void test_wheel_fsm_init_rejects_null_config(void) {
    TEST_ASSERT_EQUAL(WHEEL_FSM_RC_NULL_POINTER, wheel_fsm_api_init(NULL));
}

void test_wheel_fsm_init_rejects_missing_fields(void) {
    struct WheelFsmConfig config = prv_make_config();
    config.post = NULL;
    TEST_ASSERT_EQUAL(WHEEL_FSM_RC_NULL_POINTER, wheel_fsm_api_init(&config));

    config = prv_make_config();
    config.enter_flash = NULL;
    TEST_ASSERT_EQUAL(WHEEL_FSM_RC_NULL_POINTER, wheel_fsm_api_init(&config));

    config = prv_make_config();
    config.leave_flash = NULL;
    TEST_ASSERT_EQUAL(WHEEL_FSM_RC_NULL_POINTER, wheel_fsm_api_init(&config));
}

void test_wheel_fsm_init_accepts_the_state_table(void) {
    const struct WheelFsmConfig config = prv_make_config();
    TEST_ASSERT_EQUAL_MESSAGE(WHEEL_FSM_RC_OK, wheel_fsm_api_init(&config), "libfsm must accept the state table");
}

void test_wheel_fsm_init_starts_in_init(void) {
    const struct WheelFsmConfig config = prv_make_config();
    wheel_fsm_api_init(&config);

    TEST_ASSERT_EQUAL(WHEEL_FSM_STATE_INIT, wheel_fsm_api_get_state());
}

void test_wheel_fsm_run_state_before_init_fails(void) {
    TEST_ASSERT_EQUAL(WHEEL_FSM_RC_ERROR, wheel_fsm_api_run_state());
}

/*! \} */

/*!
 * \defgroup wheel_fsm_init_state Tests for the INIT state
 * \{
 */

void test_wheel_fsm_init_shows_the_logo_then_goes_to_idle(void) {
    const struct WheelFsmConfig config = prv_make_config();
    wheel_fsm_api_init(&config);
    wheel_fsm_api_run_state();
    TEST_ASSERT_EQUAL_MESSAGE(WHEEL_FSM_STATE_INIT, wheel_fsm_api_get_state(), "POST passed: the logo stays");

    prv_at(WHEEL_FSM_BOOT_LOGO_MS - 1U);
    wheel_fsm_api_run_state();
    TEST_ASSERT_EQUAL(WHEEL_FSM_STATE_INIT, wheel_fsm_api_get_state());

    prv_at(WHEEL_FSM_BOOT_LOGO_MS);
    wheel_fsm_api_run_state();
    TEST_ASSERT_EQUAL(WHEEL_FSM_STATE_IDLE, wheel_fsm_api_get_state());
}

void test_wheel_fsm_post_failure_moves_to_error(void) {
    prv_post.draw_rectangle = NULL;
    const struct WheelFsmConfig config = prv_make_config();
    wheel_fsm_api_init(&config);

    TEST_ASSERT_EQUAL(WHEEL_FSM_RC_OK, wheel_fsm_api_run_state());

    TEST_ASSERT_EQUAL(WHEEL_FSM_STATE_ERROR, wheel_fsm_api_get_state());
}

void test_wheel_fsm_init_follows_flash_and_comes_back(void) {
    const struct WheelFsmConfig config = prv_make_config();
    wheel_fsm_api_init(&config);
    wheel_fsm_api_run_state();

    flash_monitor_handler.active = true;
    wheel_fsm_api_run_state();
    TEST_ASSERT_EQUAL(WHEEL_FSM_STATE_FLASH, wheel_fsm_api_get_state());

    flash_monitor_handler.active = false;
    wheel_fsm_api_run_state();
    TEST_ASSERT_EQUAL_MESSAGE(WHEEL_FSM_STATE_INIT, wheel_fsm_api_get_state(), "FLASH returns to the logo it interrupted");
    TEST_ASSERT_EQUAL(1, fake_leave_flash_fake.call_count);

    prv_at(WHEEL_FSM_BOOT_LOGO_MS);
    wheel_fsm_api_run_state();
    TEST_ASSERT_EQUAL_MESSAGE(WHEEL_FSM_STATE_IDLE, wheel_fsm_api_get_state(), "The POST doesn't run twice: straight to IDLE");
}

/*! \} */

/*!
 * \defgroup wheel_fsm_api_request Tests for transitions requested by the tasks
 * \{
 */

void test_wheel_fsm_request_before_init_is_rejected(void) {
    TEST_ASSERT_EQUAL(WHEEL_FSM_RC_INVALID_TRANSITION, wheel_fsm_api_request(WHEEL_FSM_STATE_ERROR));
}

void test_wheel_fsm_request_applied_at_the_end_of_the_next_step(void) {
    prv_start_in_idle();

    TEST_ASSERT_EQUAL(WHEEL_FSM_RC_OK, wheel_fsm_api_request(WHEEL_FSM_STATE_FLASH));
    TEST_ASSERT_EQUAL_MESSAGE(WHEEL_FSM_STATE_IDLE, wheel_fsm_api_get_state(), "A request waits for the FSM task");

    wheel_fsm_api_run_state();
    TEST_ASSERT_EQUAL(WHEEL_FSM_STATE_FLASH, wheel_fsm_api_get_state());
}

void test_wheel_fsm_states_stay_without_requests(void) {
    prv_start_in_idle();

    for (uint8_t i = 0U; i < 10U; i++) {
        wheel_fsm_api_run_state();
    }

    TEST_ASSERT_EQUAL(WHEEL_FSM_STATE_IDLE, wheel_fsm_api_get_state());
}

void test_wheel_fsm_flash_returns_to_run(void) {
    prv_start_in_idle();
    wheel_fsm_api_request(WHEEL_FSM_STATE_FLASH);
    wheel_fsm_api_run_state();

    TEST_ASSERT_EQUAL(WHEEL_FSM_RC_OK, wheel_fsm_api_request(WHEEL_FSM_STATE_IDLE));
    wheel_fsm_api_run_state();

    TEST_ASSERT_EQUAL(WHEEL_FSM_STATE_IDLE, wheel_fsm_api_get_state());
}

void test_wheel_fsm_flash_can_fail_into_error(void) {
    prv_start_in_idle();
    wheel_fsm_api_request(WHEEL_FSM_STATE_FLASH);
    wheel_fsm_api_run_state();

    TEST_ASSERT_EQUAL(WHEEL_FSM_RC_OK, wheel_fsm_api_request(WHEEL_FSM_STATE_ERROR));
    wheel_fsm_api_run_state();

    TEST_ASSERT_EQUAL(WHEEL_FSM_STATE_ERROR, wheel_fsm_api_get_state());
}

void test_wheel_fsm_run_cannot_go_back_to_init(void) {
    prv_start_in_idle();

    TEST_ASSERT_EQUAL(WHEEL_FSM_RC_INVALID_TRANSITION, wheel_fsm_api_request(WHEEL_FSM_STATE_INIT));
    wheel_fsm_api_run_state();

    TEST_ASSERT_EQUAL(WHEEL_FSM_STATE_IDLE, wheel_fsm_api_get_state());
}

void test_wheel_fsm_request_invalid_state_is_rejected(void) {
    prv_start_in_idle();
    TEST_ASSERT_EQUAL(WHEEL_FSM_RC_INVALID_TRANSITION, wheel_fsm_api_request(WHEEL_FSM_STATE_COUNT));
}

void test_wheel_fsm_last_request_wins(void) {
    prv_start_in_idle();
    wheel_fsm_api_request(WHEEL_FSM_STATE_FLASH);
    wheel_fsm_api_run_state();

    TEST_ASSERT_EQUAL(WHEEL_FSM_RC_OK, wheel_fsm_api_request(WHEEL_FSM_STATE_IDLE));
    TEST_ASSERT_EQUAL(WHEEL_FSM_RC_OK, wheel_fsm_api_request(WHEEL_FSM_STATE_ERROR));
    wheel_fsm_api_run_state();

    TEST_ASSERT_EQUAL(WHEEL_FSM_STATE_ERROR, wheel_fsm_api_get_state());
}

void test_wheel_fsm_pending_error_is_not_overwritten(void) {
    prv_start_in_idle();

    TEST_ASSERT_EQUAL(WHEEL_FSM_RC_OK, wheel_fsm_api_request(WHEEL_FSM_STATE_ERROR));
    TEST_ASSERT_EQUAL_MESSAGE(WHEEL_FSM_RC_INVALID_TRANSITION, wheel_fsm_api_request(WHEEL_FSM_STATE_FLASH), "A fatal error must win over a later request in the same step");
    TEST_ASSERT_EQUAL(WHEEL_FSM_RC_OK, wheel_fsm_api_request(WHEEL_FSM_STATE_ERROR));
    wheel_fsm_api_run_state();

    TEST_ASSERT_EQUAL(WHEEL_FSM_STATE_ERROR, wheel_fsm_api_get_state());
}

void test_wheel_fsm_error_is_terminal(void) {
    prv_start_in_idle();
    wheel_fsm_api_request(WHEEL_FSM_STATE_ERROR);
    wheel_fsm_api_run_state();

    TEST_ASSERT_EQUAL(WHEEL_FSM_RC_INVALID_TRANSITION, wheel_fsm_api_request(WHEEL_FSM_STATE_IDLE));
    TEST_ASSERT_EQUAL(WHEEL_FSM_RC_INVALID_TRANSITION, wheel_fsm_api_request(WHEEL_FSM_STATE_FLASH));
    TEST_ASSERT_EQUAL(WHEEL_FSM_RC_INVALID_TRANSITION, wheel_fsm_api_request(WHEEL_FSM_STATE_INIT));
    for (uint8_t i = 0U; i < 10U; i++) {
        wheel_fsm_api_run_state();
    }

    TEST_ASSERT_EQUAL(WHEEL_FSM_STATE_ERROR, wheel_fsm_api_get_state());
}

/*! \} */

/*!
 * \defgroup wheel_fsm_run_state Tests for the IDLE state
 * \{
 */

void test_wheel_fsm_idle_stays_idle_without_flashing(void) {
    prv_start_in_idle();

    wheel_fsm_api_run_state();
    wheel_fsm_api_run_state();

    TEST_ASSERT_EQUAL(WHEEL_FSM_STATE_IDLE, wheel_fsm_api_get_state());
    TEST_ASSERT_EQUAL(0, fake_enter_flash_fake.call_count);
}

/*! \} */

/*!
 * \defgroup wheel_fsm_flash Tests for FLASH, driven by the flash monitor
 * \{
 */

void test_wheel_fsm_idle_follows_the_monitor_into_flash(void) {
    prv_start_in_idle();
    flash_monitor_handler.active = true;

    wheel_fsm_api_run_state();

    TEST_ASSERT_EQUAL(WHEEL_FSM_STATE_FLASH, wheel_fsm_api_get_state());
    TEST_ASSERT_EQUAL_MESSAGE(1, fake_enter_flash_fake.call_count, "Entering FLASH must silence the wheel");
    TEST_ASSERT_EQUAL(0, fake_leave_flash_fake.call_count);
}

void test_wheel_fsm_flash_stays_while_the_monitor_is_active(void) {
    prv_start_in_idle();
    flash_monitor_handler.active = true;

    for (uint8_t i = 0U; i < 10U; i++) {
        wheel_fsm_api_run_state();
    }

    TEST_ASSERT_EQUAL(WHEEL_FSM_STATE_FLASH, wheel_fsm_api_get_state());
    TEST_ASSERT_EQUAL_MESSAGE(1, fake_enter_flash_fake.call_count, "The hook runs once per transition, not per step");
}

void test_wheel_fsm_flash_returns_to_idle_when_the_monitor_stops(void) {
    prv_start_in_idle();
    flash_monitor_handler.active = true;
    wheel_fsm_api_run_state();

    flash_monitor_handler.active = false;
    wheel_fsm_api_run_state();

    TEST_ASSERT_EQUAL(WHEEL_FSM_STATE_IDLE, wheel_fsm_api_get_state());
    TEST_ASSERT_EQUAL_MESSAGE(1, fake_leave_flash_fake.call_count, "Leaving FLASH must restore the wheel");
}

void test_wheel_fsm_flash_survives_a_late_frame(void) {
    prv_start_in_idle();
    flash_monitor_handler.active = true;
    wheel_fsm_api_run_state();
    flash_monitor_handler.active = false;
    wheel_fsm_api_run_state();

    // A frame right after the timeout: the next step goes back to FLASH.
    flash_monitor_handler.active = true;
    wheel_fsm_api_run_state();

    TEST_ASSERT_EQUAL(WHEEL_FSM_STATE_FLASH, wheel_fsm_api_get_state());
    TEST_ASSERT_EQUAL(2, fake_enter_flash_fake.call_count);
}

void test_wheel_fsm_flash_returns_to_the_interrupted_mode(void) {
    const enum WheelFsmState path[] = { WHEEL_FSM_STATE_MENU, WHEEL_FSM_STATE_DEBUG, WHEEL_FSM_STATE_MENU, WHEEL_FSM_STATE_CALIBRATION, WHEEL_FSM_STATE_MENU, WHEEL_FSM_STATE_SCREEN_SAVER };
    prv_start_in_idle();
    for (size_t i = 0U; i < sizeof(path) / sizeof(path[0]); i++) {
        prv_go(path[i]);
        flash_monitor_handler.active = true;
        wheel_fsm_api_run_state();
        TEST_ASSERT_EQUAL(WHEEL_FSM_STATE_FLASH, wheel_fsm_api_get_state());
        flash_monitor_handler.active = false;
        wheel_fsm_api_run_state();
        TEST_ASSERT_EQUAL(path[i], wheel_fsm_api_get_state());
    }
    TEST_ASSERT_EQUAL(6, fake_enter_flash_fake.call_count);
    TEST_ASSERT_EQUAL(6, fake_leave_flash_fake.call_count);
}

void test_wheel_fsm_error_ignores_the_monitor(void) {
    prv_start_in_idle();
    wheel_fsm_api_request(WHEEL_FSM_STATE_ERROR);
    flash_monitor_handler.active = true;
    wheel_fsm_api_run_state();
    wheel_fsm_api_run_state();

    TEST_ASSERT_EQUAL(WHEEL_FSM_STATE_ERROR, wheel_fsm_api_get_state());
    TEST_ASSERT_EQUAL(0, fake_enter_flash_fake.call_count);
}

/*! \} */

/*!
 * \defgroup wheel_fsm_modes Tests for the edges between the modes
 * \{
 */

void test_wheel_fsm_idle_only_opens_the_menu(void) {
    prv_start_in_idle();
    TEST_ASSERT_EQUAL(WHEEL_FSM_RC_INVALID_TRANSITION, wheel_fsm_api_request(WHEEL_FSM_STATE_DEBUG));
    TEST_ASSERT_EQUAL(WHEEL_FSM_RC_INVALID_TRANSITION, wheel_fsm_api_request(WHEEL_FSM_STATE_CALIBRATION));
    TEST_ASSERT_EQUAL(WHEEL_FSM_RC_INVALID_TRANSITION, wheel_fsm_api_request(WHEEL_FSM_STATE_SCREEN_SAVER));
    prv_go(WHEEL_FSM_STATE_MENU);
}

void test_wheel_fsm_menu_reaches_every_mode(void) {
    const enum WheelFsmState targets[] = { WHEEL_FSM_STATE_DEBUG, WHEEL_FSM_STATE_CALIBRATION, WHEEL_FSM_STATE_SCREEN_SAVER, WHEEL_FSM_STATE_IDLE };
    prv_start_in_idle();
    for (size_t i = 0U; i < sizeof(targets) / sizeof(targets[0]); i++) {
        prv_go(WHEEL_FSM_STATE_MENU);
        prv_go(targets[i]);
    }
}

void test_wheel_fsm_pages_only_go_back_to_the_menu(void) {
    const enum WheelFsmState pages[] = { WHEEL_FSM_STATE_DEBUG, WHEEL_FSM_STATE_CALIBRATION, WHEEL_FSM_STATE_SCREEN_SAVER };
    prv_start_in_idle();
    for (size_t i = 0U; i < sizeof(pages) / sizeof(pages[0]); i++) {
        prv_go(WHEEL_FSM_STATE_MENU);
        prv_go(pages[i]);
        TEST_ASSERT_EQUAL(WHEEL_FSM_RC_INVALID_TRANSITION, wheel_fsm_api_request(WHEEL_FSM_STATE_IDLE));
        prv_go(WHEEL_FSM_STATE_MENU);
    }
}

/*! \} */

/*!
 * \defgroup wheel_fsm_names Tests for the CAN status mapping and the state names
 * \{
 */

void test_wheel_fsm_can_status_mapping(void) {
    TEST_ASSERT_EQUAL(CAN_PRIMARY_STEERINGWHEELFSM_STATUS_INIT, wheel_fsm_api_get_can_status(WHEEL_FSM_STATE_INIT));
    TEST_ASSERT_EQUAL_MESSAGE(CAN_PRIMARY_STEERINGWHEELFSM_STATUS_IDLE, wheel_fsm_api_get_can_status(WHEEL_FSM_STATE_IDLE), "IDLE keeps the IDLE value until libcan has one for it");
    TEST_ASSERT_EQUAL(CAN_PRIMARY_STEERINGWHEELFSM_STATUS_FLASH, wheel_fsm_api_get_can_status(WHEEL_FSM_STATE_FLASH));
    TEST_ASSERT_EQUAL(CAN_PRIMARY_STEERINGWHEELFSM_STATUS_ERROR, wheel_fsm_api_get_can_status(WHEEL_FSM_STATE_ERROR));
    TEST_ASSERT_EQUAL(CAN_PRIMARY_STEERINGWHEELFSM_STATUS_IDLE, wheel_fsm_api_get_can_status(WHEEL_FSM_STATE_MENU));
    TEST_ASSERT_EQUAL(CAN_PRIMARY_STEERINGWHEELFSM_STATUS_IDLE, wheel_fsm_api_get_can_status(WHEEL_FSM_STATE_DEBUG));
    TEST_ASSERT_EQUAL(CAN_PRIMARY_STEERINGWHEELFSM_STATUS_IDLE, wheel_fsm_api_get_can_status(WHEEL_FSM_STATE_CALIBRATION));
    TEST_ASSERT_EQUAL(CAN_PRIMARY_STEERINGWHEELFSM_STATUS_IDLE, wheel_fsm_api_get_can_status(WHEEL_FSM_STATE_SCREEN_SAVER));
}

void test_wheel_fsm_state_names(void) {
    TEST_ASSERT_EQUAL_STRING("INIT", wheel_fsm_api_get_state_name(WHEEL_FSM_STATE_INIT));
    TEST_ASSERT_EQUAL_STRING("IDLE", wheel_fsm_api_get_state_name(WHEEL_FSM_STATE_IDLE));
    TEST_ASSERT_EQUAL_STRING("MENU", wheel_fsm_api_get_state_name(WHEEL_FSM_STATE_MENU));
    TEST_ASSERT_EQUAL_STRING("SCREEN_SAVER", wheel_fsm_api_get_state_name(WHEEL_FSM_STATE_SCREEN_SAVER));
    TEST_ASSERT_EQUAL_STRING("FLASH", wheel_fsm_api_get_state_name(WHEEL_FSM_STATE_FLASH));
    TEST_ASSERT_EQUAL_STRING("ERROR", wheel_fsm_api_get_state_name(WHEEL_FSM_STATE_ERROR));
    TEST_ASSERT_EQUAL_STRING("?", wheel_fsm_api_get_state_name(WHEEL_FSM_STATE_COUNT));
}

/*! \} */

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_wheel_fsm_init_rejects_null_config);
    RUN_TEST(test_wheel_fsm_init_rejects_missing_fields);
    RUN_TEST(test_wheel_fsm_init_accepts_the_state_table);
    RUN_TEST(test_wheel_fsm_init_starts_in_init);
    RUN_TEST(test_wheel_fsm_run_state_before_init_fails);

    RUN_TEST(test_wheel_fsm_init_shows_the_logo_then_goes_to_idle);
    RUN_TEST(test_wheel_fsm_post_failure_moves_to_error);
    RUN_TEST(test_wheel_fsm_init_follows_flash_and_comes_back);

    RUN_TEST(test_wheel_fsm_request_before_init_is_rejected);
    RUN_TEST(test_wheel_fsm_request_applied_at_the_end_of_the_next_step);
    RUN_TEST(test_wheel_fsm_states_stay_without_requests);
    RUN_TEST(test_wheel_fsm_flash_returns_to_run);
    RUN_TEST(test_wheel_fsm_flash_can_fail_into_error);
    RUN_TEST(test_wheel_fsm_run_cannot_go_back_to_init);
    RUN_TEST(test_wheel_fsm_request_invalid_state_is_rejected);
    RUN_TEST(test_wheel_fsm_last_request_wins);
    RUN_TEST(test_wheel_fsm_pending_error_is_not_overwritten);
    RUN_TEST(test_wheel_fsm_error_is_terminal);

    RUN_TEST(test_wheel_fsm_idle_stays_idle_without_flashing);

    RUN_TEST(test_wheel_fsm_idle_follows_the_monitor_into_flash);
    RUN_TEST(test_wheel_fsm_flash_stays_while_the_monitor_is_active);
    RUN_TEST(test_wheel_fsm_flash_returns_to_idle_when_the_monitor_stops);
    RUN_TEST(test_wheel_fsm_flash_survives_a_late_frame);
    RUN_TEST(test_wheel_fsm_flash_returns_to_the_interrupted_mode);
    RUN_TEST(test_wheel_fsm_error_ignores_the_monitor);

    RUN_TEST(test_wheel_fsm_idle_only_opens_the_menu);
    RUN_TEST(test_wheel_fsm_menu_reaches_every_mode);
    RUN_TEST(test_wheel_fsm_pages_only_go_back_to_the_menu);

    RUN_TEST(test_wheel_fsm_can_status_mapping);
    RUN_TEST(test_wheel_fsm_state_names);

    return UNITY_END();
}
