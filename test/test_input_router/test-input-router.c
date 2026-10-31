/*!
 * \file test-input-router.c
 * \date 2026-10-31
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 *
 * \brief Unit tests for the input router.
 *
 * \details The router drives the real parameters module; its on-change hook
 *     is a fake, so "a parameter changed" is "the fake was called".
 */

#include "unity.h"
#include "fff.h"
#include "input-events-api.h"
#include "input-router-api.h"
#include "parameters-api.h"

DEFINE_FFF_GLOBALS;

FAKE_VALUE_FUNC(enum WheelFsmState, fake_get_state);
FAKE_VOID_FUNC(fake_on_combo, enum InputEventsComboId);
FAKE_VALUE_FUNC(bool, fake_on_change, enum InputsSharedParameterID, uint8_t);

static enum InputEventsReturnCode prv_press(enum InputsSharedButtonID button) {
    const struct InputEvent event = { .type = INPUT_EVENT_TYPE_BUTTON_PRESS, .button = button };
    return input_router_api_handle_event(&event);
}

static enum InputEventsReturnCode prv_release(enum InputsSharedButtonID button) {
    const struct InputEvent event = { .type = INPUT_EVENT_TYPE_BUTTON_RELEASE, .button = button };
    return input_router_api_handle_event(&event);
}

static enum InputEventsReturnCode prv_turn(enum InputsSharedKnobID knob, int8_t delta) {
    const struct InputEvent event = { .type = INPUT_EVENT_TYPE_KNOB_ROTATION, .knob = knob, .delta = delta };
    return input_router_api_handle_event(&event);
}

static enum InputEventsReturnCode prv_combo(enum InputEventsComboId combo) {
    const struct InputEvent event = { .type = INPUT_EVENT_TYPE_COMBO, .combo = combo };
    return input_router_api_handle_event(&event);
}

static void prv_init(input_router_combo_callback on_combo) {
    const struct InputRouterConfig config = { .get_state = fake_get_state, .on_combo = on_combo };
    TEST_ASSERT_EQUAL(INPUT_ROUTER_RC_OK, input_router_api_init(&config));
}

/* Every driver control at once: a knob step, the two toggle buttons, PTT and TS-on. */
static void prv_use_every_control(void) {
    TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_OK, prv_turn(INPUTS_SHARED_KNOB_ID_FRONT_LEFT, 2));
    TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_OK, prv_press(INPUTS_SHARED_BUTTON_ID_BOTTOM_LEFT));
    TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_OK, prv_press(INPUTS_SHARED_BUTTON_ID_BOTTOM_RIGHT));
    TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_OK, prv_press(INPUTS_SHARED_BUTTON_ID_PADDLE_TOP_LEFT));
    TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_OK, prv_press(INPUTS_SHARED_BUTTON_ID_TS_ON));
}

void setUp(void) {
    RESET_FAKE(fake_get_state);
    RESET_FAKE(fake_on_combo);
    RESET_FAKE(fake_on_change);
    FFF_RESET_HISTORY();
    fake_get_state_fake.return_val = WHEEL_FSM_STATE_IDLE;
    fake_on_change_fake.return_val = true;
    TEST_ASSERT_EQUAL(PARAMETERS_RC_OK, parameters_api_init(fake_on_change));
    prv_init(fake_on_combo);
}

void tearDown(void) {
}

/*!
 * \defgroup input_router_init Tests for input_router_api_init and the filter
 * \{
 */

void test_input_router_init_rejects_null_pointers(void) {
    const struct InputRouterConfig no_state = { .get_state = NULL, .on_combo = fake_on_combo };
    TEST_ASSERT_EQUAL(INPUT_ROUTER_RC_NULL_POINTER, input_router_api_init(NULL));
    TEST_ASSERT_EQUAL(INPUT_ROUTER_RC_NULL_POINTER, input_router_api_init(&no_state));
}

void test_input_router_init_allows_every_control(void) {
    input_router_api_set_filter((struct InputRouterFilter){ 0 });
    prv_init(fake_on_combo);
    const struct InputRouterFilter filter = input_router_api_get_filter();
    TEST_ASSERT_TRUE(filter.parameters);
    TEST_ASSERT_TRUE(filter.ptt);
    TEST_ASSERT_TRUE(filter.ts_on);
}

void test_input_router_rejects_null_event(void) {
    TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_NULL_POINTER, input_router_api_handle_event(NULL));
}

/*! \} */

/*!
 * \defgroup input_router_idle Tests for the routing in IDLE (unchanged driving behaviour)
 * \{
 */

void test_input_router_idle_knob_changes_its_parameter(void) {
    TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_OK, prv_turn(INPUTS_SHARED_KNOB_ID_FRONT_LEFT, 3));
    TEST_ASSERT_EQUAL(3U, parameters_api_get(INPUTS_SHARED_PARAMETER_ID_POWER));
    TEST_ASSERT_EQUAL(1U, fake_on_change_fake.call_count);
}

void test_input_router_idle_button_toggles_its_parameter(void) {
    TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_OK, prv_press(INPUTS_SHARED_BUTTON_ID_BOTTOM_LEFT));
    TEST_ASSERT_EQUAL(1U, parameters_api_get(INPUTS_SHARED_PARAMETER_ID_TELEMETRY_LOG));
}

void test_input_router_idle_ptt_follows_the_paddle(void) {
    TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_OK, prv_press(INPUTS_SHARED_BUTTON_ID_PADDLE_TOP_RIGHT));
    TEST_ASSERT_EQUAL(1U, parameters_api_get(INPUTS_SHARED_PARAMETER_ID_PTT));
    TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_OK, prv_release(INPUTS_SHARED_BUTTON_ID_PADDLE_TOP_RIGHT));
    TEST_ASSERT_EQUAL(0U, parameters_api_get(INPUTS_SHARED_PARAMETER_ID_PTT));
}

void test_input_router_idle_ts_on_follows_the_button(void) {
    TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_OK, prv_press(INPUTS_SHARED_BUTTON_ID_TS_ON));
    TEST_ASSERT_EQUAL(1U, parameters_api_get(INPUTS_SHARED_PARAMETER_ID_TS_ON));
    TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_OK, prv_release(INPUTS_SHARED_BUTTON_ID_TS_ON));
    TEST_ASSERT_EQUAL(0U, parameters_api_get(INPUTS_SHARED_PARAMETER_ID_TS_ON));
}

void test_input_router_parameter_failure_is_reported(void) {
    fake_on_change_fake.return_val = false;
    TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_ERROR, prv_turn(INPUTS_SHARED_KNOB_ID_FRONT_RIGHT, 1));
    TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_ERROR, prv_press(INPUTS_SHARED_BUTTON_ID_TS_ON));
    TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_ERROR, prv_release(INPUTS_SHARED_BUTTON_ID_TS_ON));
}

void test_input_router_long_press_changes_nothing(void) {
    const struct InputEvent event = { .type = INPUT_EVENT_TYPE_BUTTON_LONG_PRESS, .button = INPUTS_SHARED_BUTTON_ID_BOTTOM_LEFT };
    TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_OK, input_router_api_handle_event(&event));
    TEST_ASSERT_EQUAL(0U, fake_on_change_fake.call_count);
}

void test_input_router_combo_buttons_alone_change_no_parameter(void) {
    // Combo buttons also produce their own presses: in a state where the
    // combo is recognised they must not drive anything.
    for (uint8_t c = 0U; c < INPUT_EVENTS_COMBO_COUNT; c++) {
        const uint16_t buttons = input_events_api_get_combo((enum InputEventsComboId)c)->buttons;
        for (uint8_t b = 0U; b < INPUTS_SHARED_BUTTON_ID_COUNT; b++) {
            if ((buttons & INPUT_EVENTS_BUTTON_BIT(b)) != 0U) {
                TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_OK, prv_press((enum InputsSharedButtonID)b));
                TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_OK, prv_release((enum InputsSharedButtonID)b));
            }
        }
    }
    TEST_ASSERT_EQUAL(0U, fake_on_change_fake.call_count);
}

/*! \} */

/*!
 * \defgroup input_router_states Tests for the states that ignore inputs
 * \{
 */

void test_input_router_init_flash_error_ignore_controls(void) {
    const enum WheelFsmState states[] = { WHEEL_FSM_STATE_INIT, WHEEL_FSM_STATE_FLASH, WHEEL_FSM_STATE_ERROR };
    for (size_t i = 0U; i < sizeof(states) / sizeof(states[0]); i++) {
        fake_get_state_fake.return_val = states[i];
        prv_use_every_control();
    }
    TEST_ASSERT_EQUAL(0U, fake_on_change_fake.call_count);
}

void test_input_router_invalid_state_ignores_controls(void) {
    fake_get_state_fake.return_val = WHEEL_FSM_STATE_COUNT;
    prv_use_every_control();
    TEST_ASSERT_EQUAL(0U, fake_on_change_fake.call_count);
}

void test_input_router_release_passes_in_error(void) {
    TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_OK, prv_press(INPUTS_SHARED_BUTTON_ID_TS_ON));
    TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_OK, prv_press(INPUTS_SHARED_BUTTON_ID_PADDLE_TOP_LEFT));
    fake_get_state_fake.return_val = WHEEL_FSM_STATE_ERROR;
    TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_OK, prv_release(INPUTS_SHARED_BUTTON_ID_TS_ON));
    TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_OK, prv_release(INPUTS_SHARED_BUTTON_ID_PADDLE_TOP_LEFT));
    TEST_ASSERT_EQUAL_MESSAGE(0U, parameters_api_get(INPUTS_SHARED_PARAMETER_ID_TS_ON), "TS-on stuck after an ERROR");
    TEST_ASSERT_EQUAL_MESSAGE(0U, parameters_api_get(INPUTS_SHARED_PARAMETER_ID_PTT), "PTT stuck after an ERROR");
}

/*! \} */

/*!
 * \defgroup input_router_filter Tests for the driving-context filter
 * \{
 */

void test_input_router_filter_drops_parameters_only(void) {
    input_router_api_set_filter((struct InputRouterFilter){ .parameters = false, .ptt = true, .ts_on = true });
    TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_OK, prv_turn(INPUTS_SHARED_KNOB_ID_FRONT_LEFT, 2));
    TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_OK, prv_press(INPUTS_SHARED_BUTTON_ID_BOTTOM_RIGHT));
    TEST_ASSERT_EQUAL(0U, parameters_api_get(INPUTS_SHARED_PARAMETER_ID_POWER));
    TEST_ASSERT_EQUAL(0U, parameters_api_get(INPUTS_SHARED_PARAMETER_ID_LAUNCH_CONTROL));

    TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_OK, prv_press(INPUTS_SHARED_BUTTON_ID_PADDLE_TOP_LEFT));
    TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_OK, prv_press(INPUTS_SHARED_BUTTON_ID_TS_ON));
    TEST_ASSERT_EQUAL(1U, parameters_api_get(INPUTS_SHARED_PARAMETER_ID_PTT));
    TEST_ASSERT_EQUAL(1U, parameters_api_get(INPUTS_SHARED_PARAMETER_ID_TS_ON));
}

void test_input_router_filter_drops_ptt_only(void) {
    input_router_api_set_filter((struct InputRouterFilter){ .parameters = true, .ptt = false, .ts_on = true });
    TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_OK, prv_press(INPUTS_SHARED_BUTTON_ID_PADDLE_TOP_LEFT));
    TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_OK, prv_press(INPUTS_SHARED_BUTTON_ID_PADDLE_TOP_RIGHT));
    TEST_ASSERT_EQUAL(0U, parameters_api_get(INPUTS_SHARED_PARAMETER_ID_PTT));

    TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_OK, prv_turn(INPUTS_SHARED_KNOB_ID_FRONT_CENTER, 1));
    TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_OK, prv_press(INPUTS_SHARED_BUTTON_ID_TS_ON));
    TEST_ASSERT_EQUAL(1U, parameters_api_get(INPUTS_SHARED_PARAMETER_ID_TORQUE_VECTORING));
    TEST_ASSERT_EQUAL(1U, parameters_api_get(INPUTS_SHARED_PARAMETER_ID_TS_ON));
}

void test_input_router_filter_drops_ts_on_only(void) {
    input_router_api_set_filter((struct InputRouterFilter){ .parameters = true, .ptt = true, .ts_on = false });
    TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_OK, prv_press(INPUTS_SHARED_BUTTON_ID_TS_ON));
    TEST_ASSERT_EQUAL(0U, parameters_api_get(INPUTS_SHARED_PARAMETER_ID_TS_ON));

    TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_OK, prv_turn(INPUTS_SHARED_KNOB_ID_FRONT_RIGHT, 1));
    TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_OK, prv_press(INPUTS_SHARED_BUTTON_ID_PADDLE_TOP_LEFT));
    TEST_ASSERT_EQUAL(1U, parameters_api_get(INPUTS_SHARED_PARAMETER_ID_REGEN));
    TEST_ASSERT_EQUAL(1U, parameters_api_get(INPUTS_SHARED_PARAMETER_ID_PTT));
}

void test_input_router_filter_never_blocks_a_release(void) {
    TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_OK, prv_press(INPUTS_SHARED_BUTTON_ID_PADDLE_TOP_LEFT));
    TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_OK, prv_press(INPUTS_SHARED_BUTTON_ID_TS_ON));
    // E.g. the ECU switched to an autonomous mission while both were held.
    input_router_api_set_filter((struct InputRouterFilter){ .parameters = false, .ptt = false, .ts_on = false });
    TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_OK, prv_release(INPUTS_SHARED_BUTTON_ID_PADDLE_TOP_LEFT));
    TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_OK, prv_release(INPUTS_SHARED_BUTTON_ID_TS_ON));
    TEST_ASSERT_EQUAL(0U, parameters_api_get(INPUTS_SHARED_PARAMETER_ID_PTT));
    TEST_ASSERT_EQUAL(0U, parameters_api_get(INPUTS_SHARED_PARAMETER_ID_TS_ON));
}

/*! \} */

/*!
 * \defgroup input_router_combos Tests for the combo routing
 * \{
 */

void test_input_router_idle_recognises_the_menu_combo(void) {
    TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_OK, prv_combo(INPUT_EVENTS_COMBO_MENU));
    TEST_ASSERT_EQUAL(1U, fake_on_combo_fake.call_count);
    TEST_ASSERT_EQUAL(INPUT_EVENTS_COMBO_MENU, fake_on_combo_fake.arg0_val);
}

void test_input_router_idle_ignores_the_wake_combo(void) {
    TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_OK, prv_combo(INPUT_EVENTS_COMBO_WAKE));
    TEST_ASSERT_EQUAL(0U, fake_on_combo_fake.call_count);
}

void test_input_router_combos_ignored_outside_idle(void) {
    const enum WheelFsmState states[] = { WHEEL_FSM_STATE_INIT, WHEEL_FSM_STATE_FLASH, WHEEL_FSM_STATE_ERROR };
    for (size_t i = 0U; i < sizeof(states) / sizeof(states[0]); i++) {
        fake_get_state_fake.return_val = states[i];
        for (uint8_t c = 0U; c < INPUT_EVENTS_COMBO_COUNT; c++) {
            TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_OK, prv_combo((enum InputEventsComboId)c));
        }
    }
    TEST_ASSERT_EQUAL(0U, fake_on_combo_fake.call_count);
}

void test_input_router_combo_without_hook_is_dropped(void) {
    prv_init(NULL);
    TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_OK, prv_combo(INPUT_EVENTS_COMBO_MENU));
    TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_OK, prv_combo(INPUT_EVENTS_COMBO_COUNT));
}

void test_input_router_filter_does_not_block_combos(void) {
    input_router_api_set_filter((struct InputRouterFilter){ .parameters = false, .ptt = false, .ts_on = false });
    TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_OK, prv_combo(INPUT_EVENTS_COMBO_MENU));
    TEST_ASSERT_EQUAL(1U, fake_on_combo_fake.call_count);
}

/*! \} */

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_input_router_init_rejects_null_pointers);
    RUN_TEST(test_input_router_init_allows_every_control);
    RUN_TEST(test_input_router_rejects_null_event);

    RUN_TEST(test_input_router_idle_knob_changes_its_parameter);
    RUN_TEST(test_input_router_idle_button_toggles_its_parameter);
    RUN_TEST(test_input_router_idle_ptt_follows_the_paddle);
    RUN_TEST(test_input_router_idle_ts_on_follows_the_button);
    RUN_TEST(test_input_router_parameter_failure_is_reported);
    RUN_TEST(test_input_router_long_press_changes_nothing);
    RUN_TEST(test_input_router_combo_buttons_alone_change_no_parameter);

    RUN_TEST(test_input_router_init_flash_error_ignore_controls);
    RUN_TEST(test_input_router_invalid_state_ignores_controls);
    RUN_TEST(test_input_router_release_passes_in_error);

    RUN_TEST(test_input_router_filter_drops_parameters_only);
    RUN_TEST(test_input_router_filter_drops_ptt_only);
    RUN_TEST(test_input_router_filter_drops_ts_on_only);
    RUN_TEST(test_input_router_filter_never_blocks_a_release);

    RUN_TEST(test_input_router_idle_recognises_the_menu_combo);
    RUN_TEST(test_input_router_idle_ignores_the_wake_combo);
    RUN_TEST(test_input_router_combos_ignored_outside_idle);
    RUN_TEST(test_input_router_combo_without_hook_is_dropped);
    RUN_TEST(test_input_router_filter_does_not_block_combos);

    return UNITY_END();
}
