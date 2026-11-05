/*!
 * \file test-input-router.c
 * \date 2026-10-31
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 *
 * \brief Unit tests for the input router and its per-state binding tables.
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
FAKE_VOID_FUNC(fake_on_request, enum InputRequest);
FAKE_VALUE_FUNC(bool, fake_on_change, enum InputsSharedParameterID, uint8_t);

static enum InputEventsReturnCode prv_button(enum InputsSharedButtonID button, enum InputEventType type) {
    const struct InputEvent event = { .source = INPUT_EVENT_SOURCE_BUTTON, .type = type, .button = button };
    return input_router_api_handle_event(&event);
}

static enum InputEventsReturnCode prv_press(enum InputsSharedButtonID button) {
    return prv_button(button, INPUT_EVENT_TYPE_PRESS);
}

static enum InputEventsReturnCode prv_release(enum InputsSharedButtonID button) {
    return prv_button(button, INPUT_EVENT_TYPE_RELEASE);
}

static enum InputEventsReturnCode prv_turn(enum InputsSharedKnobID knob, int8_t delta) {
    const struct InputEvent event = { .source = INPUT_EVENT_SOURCE_KNOB, .type = INPUT_EVENT_TYPE_ROTATION, .knob = knob, .delta = delta };
    return input_router_api_handle_event(&event);
}

static enum InputEventsReturnCode prv_combo(enum InputEventsComboId combo, enum InputEventType type) {
    const struct InputEvent event = { .source = INPUT_EVENT_SOURCE_COMBO, .type = type, .combo = combo };
    return input_router_api_handle_event(&event);
}

static void prv_init(input_router_request_callback on_request) {
    const struct InputRouterConfig config = { .get_state = fake_get_state, .on_request = on_request };
    TEST_ASSERT_EQUAL(INPUT_ROUTER_RC_OK, input_router_api_init(&config));
}

/* Every IDLE control at once: knob step, both toggles, PTT, TS-on, OK, REPEAT. */
static void prv_use_every_control(void) {
    TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_OK, prv_turn(INPUTS_SHARED_KNOB_ID_FRONT_LEFT, 2));
    TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_OK, prv_press(INPUTS_SHARED_BUTTON_ID_BOTTOM_LEFT));
    TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_OK, prv_press(INPUTS_SHARED_BUTTON_ID_BOTTOM_RIGHT));
    TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_OK, prv_press(INPUTS_SHARED_BUTTON_ID_PADDLE_TOP_LEFT));
    TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_OK, prv_press(INPUTS_SHARED_BUTTON_ID_TS_ON));
    TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_OK, prv_press(INPUTS_SHARED_BUTTON_ID_TOP_LEFT));
    TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_OK, prv_press(INPUTS_SHARED_BUTTON_ID_TOP_RIGHT));
}

/* Check one action of a binding table. */
static void prv_check_action(const struct InputAction *action, bool on_press) {
    TEST_ASSERT_TRUE(action->group <= INPUT_ACTION_GROUP_SYSTEM);
    switch (action->type) {
        case INPUT_ACTION_STEP:
            TEST_ASSERT_TRUE(action->parameter < INPUTS_SHARED_PARAMETER_ID_COUNT);
            TEST_ASSERT_NOT_EQUAL_MESSAGE(0, action->amount, "A step of 0 does nothing");
            break;
        case INPUT_ACTION_TOGGLE:
            TEST_ASSERT_TRUE(action->parameter < INPUTS_SHARED_PARAMETER_ID_COUNT);
            break;
        case INPUT_ACTION_HOLD:
            TEST_ASSERT_TRUE(action->parameter < INPUTS_SHARED_PARAMETER_ID_COUNT);
            TEST_ASSERT_TRUE_MESSAGE(on_press, "A hold is let go by the release: bind it to a press");
            break;
        case INPUT_ACTION_REQUEST:
            TEST_ASSERT_EQUAL_MESSAGE(INPUT_ACTION_GROUP_SYSTEM, action->group, "Requests are never filtered");
            break;
        default:
            TEST_FAIL_MESSAGE("Unknown action type");
    }
}

void setUp(void) {
    RESET_FAKE(fake_get_state);
    RESET_FAKE(fake_on_request);
    RESET_FAKE(fake_on_change);
    FFF_RESET_HISTORY();
    fake_get_state_fake.return_val = WHEEL_FSM_STATE_IDLE;
    fake_on_change_fake.return_val = true;
    TEST_ASSERT_EQUAL(PARAMETERS_RC_OK, parameters_api_init(fake_on_change));
    prv_init(fake_on_request);
}

void tearDown(void) {}

/*!
 * \defgroup input_router_init Tests for init, the filter and the tables
 * \{
 */

void test_input_router_init_rejects_null_pointers(void) {
    const struct InputRouterConfig no_state = { .get_state = NULL, .on_request = fake_on_request };
    TEST_ASSERT_EQUAL(INPUT_ROUTER_RC_NULL_POINTER, input_router_api_init(NULL));
    TEST_ASSERT_EQUAL(INPUT_ROUTER_RC_NULL_POINTER, input_router_api_init(&no_state));
}

void test_input_router_init_allows_every_group(void) {
    input_router_api_set_filter((struct InputRouterFilter){ 0 });
    prv_init(fake_on_request);
    const struct InputRouterFilter filter = input_router_api_get_filter();
    TEST_ASSERT_TRUE(filter.parameters);
    TEST_ASSERT_TRUE(filter.ptt);
    TEST_ASSERT_TRUE(filter.ts_on);
}

void test_input_router_rejects_null_event(void) {
    TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_NULL_POINTER, input_router_api_handle_event(NULL));
}

void test_input_router_get_bindings_rejects_invalid_state(void) {
    TEST_ASSERT_NULL(input_router_api_get_bindings(WHEEL_FSM_STATE_COUNT));
}

void test_input_router_every_binding_is_valid(void) {
    for (uint8_t state = 0U; state < WHEEL_FSM_STATE_COUNT; state++) {
        const struct InputStateBindings *bindings = input_router_api_get_bindings((enum WheelFsmState)state);
        TEST_ASSERT_NOT_NULL(bindings);
        for (uint8_t i = 0U; i < bindings->button_count; i++) {
            TEST_ASSERT_TRUE(bindings->buttons[i].button < INPUTS_SHARED_BUTTON_ID_COUNT);
            TEST_ASSERT_TRUE(bindings->buttons[i].event <= INPUT_EVENT_TYPE_LONG_PRESS);
            prv_check_action(&bindings->buttons[i].action, bindings->buttons[i].event == INPUT_EVENT_TYPE_PRESS);
        }
        for (uint8_t i = 0U; i < bindings->combo_count; i++) {
            TEST_ASSERT_TRUE(bindings->combos[i].combo < INPUT_EVENTS_COMBO_COUNT);
            TEST_ASSERT_TRUE(bindings->combos[i].event <= INPUT_EVENT_TYPE_LONG_PRESS);
            prv_check_action(&bindings->combos[i].action, bindings->combos[i].event == INPUT_EVENT_TYPE_PRESS);
        }
        for (uint8_t i = 0U; i < bindings->knob_count; i++) {
            TEST_ASSERT_TRUE(bindings->knobs[i].knob < INPUTS_SHARED_KNOB_ID_COUNT);
            prv_check_action(&bindings->knobs[i].action, false);
        }
    }
}

void test_input_router_init_flash_error_bind_nothing(void) {
    const enum WheelFsmState states[] = { WHEEL_FSM_STATE_INIT, WHEEL_FSM_STATE_FLASH, WHEEL_FSM_STATE_ERROR };
    for (size_t i = 0U; i < sizeof(states) / sizeof(states[0]); i++) {
        const struct InputStateBindings *bindings = input_router_api_get_bindings(states[i]);
        TEST_ASSERT_EQUAL(0U, bindings->button_count + bindings->combo_count + bindings->knob_count);
    }
}

void test_input_router_active_combos_follow_the_state(void) {
    TEST_ASSERT_EQUAL_UINT8(INPUT_EVENTS_COMBO_BIT(INPUT_EVENTS_COMBO_MENU), input_router_api_active_combos());
    fake_get_state_fake.return_val = WHEEL_FSM_STATE_FLASH;
    TEST_ASSERT_EQUAL_UINT8(0U, input_router_api_active_combos());
    fake_get_state_fake.return_val = WHEEL_FSM_STATE_COUNT;
    TEST_ASSERT_EQUAL_UINT8(0U, input_router_api_active_combos());
}

/*! \} */

/*!
 * \defgroup input_router_idle Tests for the IDLE (driving) bindings
 * \{
 */

void test_input_router_idle_knobs_step_their_parameters(void) {
    TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_OK, prv_turn(INPUTS_SHARED_KNOB_ID_FRONT_LEFT, 3));
    TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_OK, prv_turn(INPUTS_SHARED_KNOB_ID_FRONT_RIGHT, 2));
    TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_OK, prv_turn(INPUTS_SHARED_KNOB_ID_FRONT_CENTER, 4));
    TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_OK, prv_turn(INPUTS_SHARED_KNOB_ID_FRONT_CENTER, -1));

    TEST_ASSERT_EQUAL(3U, parameters_api_get(INPUTS_SHARED_PARAMETER_ID_POWER));
    TEST_ASSERT_EQUAL(2U, parameters_api_get(INPUTS_SHARED_PARAMETER_ID_REGEN));
    TEST_ASSERT_EQUAL(3U, parameters_api_get(INPUTS_SHARED_PARAMETER_ID_TORQUE_VECTORING));
}

void test_input_router_idle_bottom_buttons_toggle_on_press(void) {
    TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_OK, prv_press(INPUTS_SHARED_BUTTON_ID_BOTTOM_LEFT));
    TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_OK, prv_press(INPUTS_SHARED_BUTTON_ID_BOTTOM_RIGHT));
    TEST_ASSERT_EQUAL(1U, parameters_api_get(INPUTS_SHARED_PARAMETER_ID_TELEMETRY_LOG));
    TEST_ASSERT_EQUAL(1U, parameters_api_get(INPUTS_SHARED_PARAMETER_ID_LAUNCH_CONTROL));

    TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_OK, prv_release(INPUTS_SHARED_BUTTON_ID_BOTTOM_LEFT));
    TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_OK, prv_button(INPUTS_SHARED_BUTTON_ID_BOTTOM_LEFT, INPUT_EVENT_TYPE_TAP));
    TEST_ASSERT_EQUAL_MESSAGE(1U, parameters_api_get(INPUTS_SHARED_PARAMETER_ID_TELEMETRY_LOG), "Release and tap don't toggle again");
}

void test_input_router_idle_ptt_held_by_either_paddle(void) {
    TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_OK, prv_press(INPUTS_SHARED_BUTTON_ID_PADDLE_TOP_LEFT));
    TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_OK, prv_press(INPUTS_SHARED_BUTTON_ID_PADDLE_TOP_RIGHT));
    TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_OK, prv_release(INPUTS_SHARED_BUTTON_ID_PADDLE_TOP_LEFT));
    TEST_ASSERT_EQUAL_MESSAGE(1U, parameters_api_get(INPUTS_SHARED_PARAMETER_ID_PTT), "The other paddle still holds it");
    TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_OK, prv_release(INPUTS_SHARED_BUTTON_ID_PADDLE_TOP_RIGHT));
    TEST_ASSERT_EQUAL(0U, parameters_api_get(INPUTS_SHARED_PARAMETER_ID_PTT));
}

void test_input_router_idle_ts_on_held_while_pressed(void) {
    TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_OK, prv_press(INPUTS_SHARED_BUTTON_ID_TS_ON));
    TEST_ASSERT_EQUAL(1U, parameters_api_get(INPUTS_SHARED_PARAMETER_ID_TS_ON));
    TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_OK, prv_release(INPUTS_SHARED_BUTTON_ID_TS_ON));
    TEST_ASSERT_EQUAL(0U, parameters_api_get(INPUTS_SHARED_PARAMETER_ID_TS_ON));
}

void test_input_router_idle_ok_on_the_right_repeat_on_the_left(void) {
    TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_OK, prv_press(INPUTS_SHARED_BUTTON_ID_TOP_RIGHT));
    TEST_ASSERT_EQUAL(1U, parameters_api_get(INPUTS_SHARED_PARAMETER_ID_DRIVER_OK));
    TEST_ASSERT_EQUAL(0U, parameters_api_get(INPUTS_SHARED_PARAMETER_ID_DRIVER_REPEAT));

    TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_OK, prv_press(INPUTS_SHARED_BUTTON_ID_TOP_LEFT));
    TEST_ASSERT_EQUAL(1U, parameters_api_get(INPUTS_SHARED_PARAMETER_ID_DRIVER_REPEAT));

    TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_OK, prv_release(INPUTS_SHARED_BUTTON_ID_TOP_RIGHT));
    TEST_ASSERT_EQUAL(0U, parameters_api_get(INPUTS_SHARED_PARAMETER_ID_DRIVER_OK));
    TEST_ASSERT_EQUAL(1U, parameters_api_get(INPUTS_SHARED_PARAMETER_ID_DRIVER_REPEAT));
    TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_OK, prv_release(INPUTS_SHARED_BUTTON_ID_TOP_LEFT));
    TEST_ASSERT_EQUAL(0U, parameters_api_get(INPUTS_SHARED_PARAMETER_ID_DRIVER_REPEAT));
}

void test_input_router_idle_menu_combo_requests_the_menu_on_long_press_only(void) {
    TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_OK, prv_combo(INPUT_EVENTS_COMBO_MENU, INPUT_EVENT_TYPE_PRESS));
    TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_OK, prv_combo(INPUT_EVENTS_COMBO_MENU, INPUT_EVENT_TYPE_RELEASE));
    TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_OK, prv_combo(INPUT_EVENTS_COMBO_MENU, INPUT_EVENT_TYPE_TAP));
    TEST_ASSERT_EQUAL(0U, fake_on_request_fake.call_count);

    TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_OK, prv_combo(INPUT_EVENTS_COMBO_MENU, INPUT_EVENT_TYPE_LONG_PRESS));
    TEST_ASSERT_EQUAL(1U, fake_on_request_fake.call_count);
    TEST_ASSERT_EQUAL(INPUT_REQUEST_MENU, fake_on_request_fake.arg0_val);
    TEST_ASSERT_EQUAL_MESSAGE(0U, fake_on_change_fake.call_count, "A combo changes no parameter");
}

void test_input_router_idle_wake_combo_does_nothing(void) {
    TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_OK, prv_combo(INPUT_EVENTS_COMBO_WAKE, INPUT_EVENT_TYPE_LONG_PRESS));
    TEST_ASSERT_EQUAL(0U, fake_on_request_fake.call_count);
}

void test_input_router_request_without_hook_is_dropped(void) {
    prv_init(NULL);
    TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_OK, prv_combo(INPUT_EVENTS_COMBO_MENU, INPUT_EVENT_TYPE_LONG_PRESS));
}

void test_input_router_unbound_events_change_nothing(void) {
    TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_OK, prv_button(INPUTS_SHARED_BUTTON_ID_BOTTOM_LEFT, INPUT_EVENT_TYPE_LONG_PRESS));
    TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_OK, prv_press(INPUTS_SHARED_BUTTON_ID_PADDLE_BOTTOM_LEFT));
    TEST_ASSERT_EQUAL(0U, fake_on_change_fake.call_count);
}

void test_input_router_parameter_failure_is_reported(void) {
    fake_on_change_fake.return_val = false;
    TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_ERROR, prv_turn(INPUTS_SHARED_KNOB_ID_FRONT_RIGHT, 1));
    TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_ERROR, prv_press(INPUTS_SHARED_BUTTON_ID_TS_ON));
    TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_ERROR, prv_release(INPUTS_SHARED_BUTTON_ID_TS_ON));
}

/*! \} */

/*!
 * \defgroup input_router_states Tests for the states that bind nothing
 * \{
 */

void test_input_router_init_flash_error_ignore_every_control(void) {
    const enum WheelFsmState states[] = { WHEEL_FSM_STATE_INIT, WHEEL_FSM_STATE_FLASH, WHEEL_FSM_STATE_ERROR, WHEEL_FSM_STATE_COUNT };
    for (size_t i = 0U; i < sizeof(states) / sizeof(states[0]); i++) {
        fake_get_state_fake.return_val = states[i];
        prv_use_every_control();
        TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_OK, prv_combo(INPUT_EVENTS_COMBO_MENU, INPUT_EVENT_TYPE_LONG_PRESS));
    }
    TEST_ASSERT_EQUAL(0U, fake_on_change_fake.call_count);
    TEST_ASSERT_EQUAL(0U, fake_on_request_fake.call_count);
}

void test_input_router_release_lets_go_in_every_state(void) {
    TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_OK, prv_press(INPUTS_SHARED_BUTTON_ID_TS_ON));
    TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_OK, prv_press(INPUTS_SHARED_BUTTON_ID_PADDLE_TOP_LEFT));
    TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_OK, prv_press(INPUTS_SHARED_BUTTON_ID_TOP_RIGHT));
    fake_get_state_fake.return_val = WHEEL_FSM_STATE_ERROR;

    TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_OK, prv_release(INPUTS_SHARED_BUTTON_ID_TS_ON));
    TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_OK, prv_release(INPUTS_SHARED_BUTTON_ID_PADDLE_TOP_LEFT));
    TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_OK, prv_release(INPUTS_SHARED_BUTTON_ID_TOP_RIGHT));

    TEST_ASSERT_EQUAL_MESSAGE(0U, parameters_api_get(INPUTS_SHARED_PARAMETER_ID_TS_ON), "TS-on stuck after an ERROR");
    TEST_ASSERT_EQUAL_MESSAGE(0U, parameters_api_get(INPUTS_SHARED_PARAMETER_ID_PTT), "PTT stuck after an ERROR");
    TEST_ASSERT_EQUAL_MESSAGE(0U, parameters_api_get(INPUTS_SHARED_PARAMETER_ID_DRIVER_OK), "OK stuck after an ERROR");
}

/*! \} */

/*!
 * \defgroup input_router_filter Tests for the driving-context filter
 * \{
 */

void test_input_router_filter_drops_parameters_group_only(void) {
    input_router_api_set_filter((struct InputRouterFilter){ .parameters = false, .ptt = true, .ts_on = true });
    prv_use_every_control();

    TEST_ASSERT_EQUAL(0U, parameters_api_get(INPUTS_SHARED_PARAMETER_ID_POWER));
    TEST_ASSERT_EQUAL(0U, parameters_api_get(INPUTS_SHARED_PARAMETER_ID_TELEMETRY_LOG));
    TEST_ASSERT_EQUAL(0U, parameters_api_get(INPUTS_SHARED_PARAMETER_ID_LAUNCH_CONTROL));
    TEST_ASSERT_EQUAL(0U, parameters_api_get(INPUTS_SHARED_PARAMETER_ID_DRIVER_OK));
    TEST_ASSERT_EQUAL(0U, parameters_api_get(INPUTS_SHARED_PARAMETER_ID_DRIVER_REPEAT));
    TEST_ASSERT_EQUAL(1U, parameters_api_get(INPUTS_SHARED_PARAMETER_ID_PTT));
    TEST_ASSERT_EQUAL(1U, parameters_api_get(INPUTS_SHARED_PARAMETER_ID_TS_ON));
}

void test_input_router_filter_drops_ptt_only(void) {
    input_router_api_set_filter((struct InputRouterFilter){ .parameters = true, .ptt = false, .ts_on = true });
    prv_use_every_control();

    TEST_ASSERT_EQUAL(0U, parameters_api_get(INPUTS_SHARED_PARAMETER_ID_PTT));
    TEST_ASSERT_EQUAL(2U, parameters_api_get(INPUTS_SHARED_PARAMETER_ID_POWER));
    TEST_ASSERT_EQUAL(1U, parameters_api_get(INPUTS_SHARED_PARAMETER_ID_TS_ON));
}

void test_input_router_filter_drops_ts_on_only(void) {
    input_router_api_set_filter((struct InputRouterFilter){ .parameters = true, .ptt = true, .ts_on = false });
    prv_use_every_control();

    TEST_ASSERT_EQUAL(0U, parameters_api_get(INPUTS_SHARED_PARAMETER_ID_TS_ON));
    TEST_ASSERT_EQUAL(1U, parameters_api_get(INPUTS_SHARED_PARAMETER_ID_PTT));
    TEST_ASSERT_EQUAL(1U, parameters_api_get(INPUTS_SHARED_PARAMETER_ID_DRIVER_OK));
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

void test_input_router_filter_never_blocks_requests(void) {
    input_router_api_set_filter((struct InputRouterFilter){ .parameters = false, .ptt = false, .ts_on = false });
    TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_OK, prv_combo(INPUT_EVENTS_COMBO_MENU, INPUT_EVENT_TYPE_LONG_PRESS));
    TEST_ASSERT_EQUAL(1U, fake_on_request_fake.call_count);
}

/*! \} */

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_input_router_init_rejects_null_pointers);
    RUN_TEST(test_input_router_init_allows_every_group);
    RUN_TEST(test_input_router_rejects_null_event);
    RUN_TEST(test_input_router_get_bindings_rejects_invalid_state);
    RUN_TEST(test_input_router_every_binding_is_valid);
    RUN_TEST(test_input_router_init_flash_error_bind_nothing);
    RUN_TEST(test_input_router_active_combos_follow_the_state);

    RUN_TEST(test_input_router_idle_knobs_step_their_parameters);
    RUN_TEST(test_input_router_idle_bottom_buttons_toggle_on_press);
    RUN_TEST(test_input_router_idle_ptt_held_by_either_paddle);
    RUN_TEST(test_input_router_idle_ts_on_held_while_pressed);
    RUN_TEST(test_input_router_idle_ok_on_the_right_repeat_on_the_left);
    RUN_TEST(test_input_router_idle_menu_combo_requests_the_menu_on_long_press_only);
    RUN_TEST(test_input_router_idle_wake_combo_does_nothing);
    RUN_TEST(test_input_router_request_without_hook_is_dropped);
    RUN_TEST(test_input_router_unbound_events_change_nothing);
    RUN_TEST(test_input_router_parameter_failure_is_reported);

    RUN_TEST(test_input_router_init_flash_error_ignore_every_control);
    RUN_TEST(test_input_router_release_lets_go_in_every_state);

    RUN_TEST(test_input_router_filter_drops_parameters_group_only);
    RUN_TEST(test_input_router_filter_drops_ptt_only);
    RUN_TEST(test_input_router_filter_drops_ts_on_only);
    RUN_TEST(test_input_router_filter_never_blocks_a_release);
    RUN_TEST(test_input_router_filter_never_blocks_requests);

    return UNITY_END();
}
