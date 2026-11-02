/*!
 * \file test-input-events.c
 * \date 2026-10-31
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 *
 * \brief Unit tests for the input event layer: single buttons, the combo
 *     window, combos and knobs.
 *
 * \details Times run on the real timeout pool with a fake clock; the sink
 *     records every event with the tick it arrived at.
 */

#include "unity.h"
#include "fff.h"
#include "input-events-api.h"
#include "timeout-api.h"
#include <stdio.h>
#include <string.h>

DEFINE_FFF_GLOBALS;

FAKE_VALUE_FUNC(enum InputEventsReturnCode, fake_sink, const struct InputEvent *);

#define TEST_INPUT_EVENTS_CAPACITY (32U)

/* The menu combo's buttons (in the combo) and a button in no combo. */
#define TEST_COMBO_FIRST INPUTS_SHARED_BUTTON_ID_TOP_LEFT
#define TEST_COMBO_SECOND INPUTS_SHARED_BUTTON_ID_TOP_RIGHT
#define TEST_PLAIN INPUTS_SHARED_BUTTON_ID_BOTTOM_LEFT

struct TestEvent {
    struct InputEvent event;
    uint32_t tick;
};

static uint32_t prv_now;
static uint8_t prv_active;
static struct TestEvent prv_events[TEST_INPUT_EVENTS_CAPACITY];
static size_t prv_event_count;

static uint32_t prv_get_tick(void) {
    return prv_now;
}

static uint8_t prv_active_combos(void) {
    return prv_active;
}

static enum InputEventsReturnCode prv_record(const struct InputEvent *event) {
    if (prv_event_count < TEST_INPUT_EVENTS_CAPACITY) {
        prv_events[prv_event_count++] = (struct TestEvent){ .event = *event, .tick = prv_now };
    }
    return INPUT_EVENTS_RC_OK;
}

/* Move the clock one millisecond at a time, letting due timeouts expire. */
static void prv_advance_to(uint32_t tick) {
    while (prv_now < tick) {
        prv_now++;
        TEST_ASSERT_EQUAL(TIMEOUT_RC_OK, timeout_api_routine());
    }
}

static void prv_press(enum InputsSharedButtonID button) {
    TEST_ASSERT_EQUAL(INPUTS_RC_OK, input_events_api_on_button_press(button));
}

static void prv_release(enum InputsSharedButtonID button) {
    TEST_ASSERT_EQUAL(INPUTS_RC_OK, input_events_api_on_button_release(button));
}

/* Check the recorded event at \p index. */
static void prv_expect(size_t index, enum InputEventSource source, enum InputEventType type, uint8_t id, uint32_t tick) {
    char message[64];
    snprintf(message, sizeof(message), "event %u", (unsigned)index);
    TEST_ASSERT_TRUE_MESSAGE(index < prv_event_count, message);
    const struct TestEvent *recorded = &prv_events[index];
    TEST_ASSERT_EQUAL_MESSAGE(source, recorded->event.source, message);
    TEST_ASSERT_EQUAL_MESSAGE(type, recorded->event.type, message);
    uint8_t recorded_id = (uint8_t)recorded->event.knob;
    if (source == INPUT_EVENT_SOURCE_BUTTON) {
        recorded_id = (uint8_t)recorded->event.button;
    } else if (source == INPUT_EVENT_SOURCE_COMBO) {
        recorded_id = (uint8_t)recorded->event.combo;
    }
    TEST_ASSERT_EQUAL_MESSAGE(id, recorded_id, message);
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(tick, recorded->tick, message);
}

static size_t prv_count(enum InputEventSource source, enum InputEventType type) {
    size_t count = 0U;
    for (size_t i = 0U; i < prv_event_count; i++) {
        if (prv_events[i].event.source == source && prv_events[i].event.type == type) {
            count++;
        }
    }
    return count;
}

void setUp(void) {
    RESET_FAKE(fake_sink);
    FFF_RESET_HISTORY();
    fake_sink_fake.custom_fake = prv_record;
    prv_now = 0U;
    prv_active = INPUT_EVENTS_COMBO_BIT(INPUT_EVENTS_COMBO_MENU);
    prv_event_count = 0U;
    memset(prv_events, 0, sizeof(prv_events));
    TEST_ASSERT_EQUAL(TIMEOUT_RC_OK, timeout_api_init(prv_get_tick));
    TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_OK, input_events_api_init(fake_sink, prv_active_combos));
}

void tearDown(void) {}

/*!
 * \defgroup input_events_init Tests for init and the combo table
 * \{
 */

void test_input_events_init_rejects_null_sink(void) {
    TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_NULL_POINTER, input_events_api_init(NULL, prv_active_combos));
}

void test_input_events_init_fails_when_the_pool_is_full(void) {
    TEST_ASSERT_EQUAL(TIMEOUT_RC_OK, timeout_api_init(prv_get_tick));
    for (uint8_t i = 0U; i < TIMEOUT_MAX_COUNT - INPUT_EVENTS_TIMEOUT_COUNT + 1U; i++) {
        timeout_id id = 0U;
        TEST_ASSERT_EQUAL(TIMEOUT_RC_OK, timeout_api_create(10U, (timeout_expired_callback)prv_record, NULL, &id));
    }
    TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_ERROR, input_events_api_init(fake_sink, prv_active_combos));
}

void test_input_events_init_releases_every_button(void) {
    prv_press(TEST_PLAIN);
    TEST_ASSERT_EQUAL(TIMEOUT_RC_OK, timeout_api_init(prv_get_tick));
    TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_OK, input_events_api_init(fake_sink, prv_active_combos));
    TEST_ASSERT_FALSE(input_events_api_is_held(TEST_PLAIN));
}

void test_input_events_combos_are_real_combinations(void) {
    TEST_ASSERT_EQUAL_UINT16(0U, input_events_api_get_combo_buttons(INPUT_EVENTS_COMBO_COUNT));
    for (uint8_t i = 0U; i < INPUT_EVENTS_COMBO_COUNT; i++) {
        const uint16_t buttons = input_events_api_get_combo_buttons((enum InputEventsComboId)i);
        TEST_ASSERT_TRUE_MESSAGE(__builtin_popcount(buttons) >= 2, "A combo needs at least two buttons");
        TEST_ASSERT_EQUAL_MESSAGE(0U, buttons >> INPUTS_SHARED_BUTTON_ID_COUNT, "Combo uses a button that doesn't exist");
        for (uint8_t j = 0U; j < i; j++) {
            TEST_ASSERT_NOT_EQUAL_MESSAGE(input_events_api_get_combo_buttons((enum InputEventsComboId)j), buttons, "Two combos with the same buttons");
        }
    }
}

/*! \} */

/*!
 * \defgroup input_events_single Tests for buttons in no active combo
 * \{
 */

void test_input_events_plain_press_is_delivered_at_once(void) {
    prv_press(TEST_PLAIN);
    TEST_ASSERT_EQUAL(1U, prv_event_count);
    prv_expect(0U, INPUT_EVENT_SOURCE_BUTTON, INPUT_EVENT_TYPE_PRESS, TEST_PLAIN, 0U);
    TEST_ASSERT_TRUE(input_events_api_is_held(TEST_PLAIN));
}

void test_input_events_short_press_is_release_then_tap(void) {
    prv_press(TEST_PLAIN);
    prv_advance_to(INPUT_EVENTS_LONG_PRESS_MS - 1U);
    prv_release(TEST_PLAIN);

    TEST_ASSERT_EQUAL(3U, prv_event_count);
    prv_expect(1U, INPUT_EVENT_SOURCE_BUTTON, INPUT_EVENT_TYPE_RELEASE, TEST_PLAIN, INPUT_EVENTS_LONG_PRESS_MS - 1U);
    prv_expect(2U, INPUT_EVENT_SOURCE_BUTTON, INPUT_EVENT_TYPE_TAP, TEST_PLAIN, INPUT_EVENTS_LONG_PRESS_MS - 1U);
    TEST_ASSERT_FALSE(input_events_api_is_held(TEST_PLAIN));
}

void test_input_events_long_press_once_then_release_without_tap(void) {
    prv_press(TEST_PLAIN);
    prv_advance_to(5U * INPUT_EVENTS_LONG_PRESS_MS);
    prv_release(TEST_PLAIN);

    TEST_ASSERT_EQUAL(3U, prv_event_count);
    prv_expect(1U, INPUT_EVENT_SOURCE_BUTTON, INPUT_EVENT_TYPE_LONG_PRESS, TEST_PLAIN, INPUT_EVENTS_LONG_PRESS_MS);
    prv_expect(2U, INPUT_EVENT_SOURCE_BUTTON, INPUT_EVENT_TYPE_RELEASE, TEST_PLAIN, 5U * INPUT_EVENTS_LONG_PRESS_MS);
}

void test_input_events_long_press_rearms_after_a_release(void) {
    prv_press(TEST_PLAIN);
    prv_advance_to(INPUT_EVENTS_LONG_PRESS_MS);
    prv_release(TEST_PLAIN);
    prv_advance_to(INPUT_EVENTS_LONG_PRESS_MS + 100U);
    prv_press(TEST_PLAIN);
    prv_advance_to(2U * INPUT_EVENTS_LONG_PRESS_MS + 100U);

    TEST_ASSERT_EQUAL(2U, prv_count(INPUT_EVENT_SOURCE_BUTTON, INPUT_EVENT_TYPE_LONG_PRESS));
}

void test_input_events_repeated_press_changes_nothing(void) {
    prv_press(TEST_PLAIN);
    prv_advance_to(INPUT_EVENTS_LONG_PRESS_MS / 2U);
    prv_press(TEST_PLAIN);
    prv_advance_to(INPUT_EVENTS_LONG_PRESS_MS);

    TEST_ASSERT_EQUAL(1U, prv_count(INPUT_EVENT_SOURCE_BUTTON, INPUT_EVENT_TYPE_PRESS));
    prv_expect(1U, INPUT_EVENT_SOURCE_BUTTON, INPUT_EVENT_TYPE_LONG_PRESS, TEST_PLAIN, INPUT_EVENTS_LONG_PRESS_MS);
}

void test_input_events_release_of_a_released_button_changes_nothing(void) {
    prv_release(TEST_PLAIN);
    TEST_ASSERT_EQUAL(0U, prv_event_count);
}

void test_input_events_long_press_timers_are_per_button(void) {
    prv_press(INPUTS_SHARED_BUTTON_ID_BOTTOM_LEFT);
    prv_advance_to(300U);
    prv_press(INPUTS_SHARED_BUTTON_ID_BOTTOM_RIGHT);
    prv_release(INPUTS_SHARED_BUTTON_ID_BOTTOM_LEFT);
    prv_advance_to(300U + INPUT_EVENTS_LONG_PRESS_MS);

    TEST_ASSERT_EQUAL(1U, prv_count(INPUT_EVENT_SOURCE_BUTTON, INPUT_EVENT_TYPE_LONG_PRESS));
    prv_expect(prv_event_count - 1U, INPUT_EVENT_SOURCE_BUTTON, INPUT_EVENT_TYPE_LONG_PRESS, INPUTS_SHARED_BUTTON_ID_BOTTOM_RIGHT, 300U + INPUT_EVENTS_LONG_PRESS_MS);
}

void test_input_events_every_button_has_a_long_press(void) {
    prv_active = 0U;
    for (uint8_t i = 0U; i < INPUTS_SHARED_BUTTON_ID_COUNT; i++) {
        const size_t before = prv_count(INPUT_EVENT_SOURCE_BUTTON, INPUT_EVENT_TYPE_LONG_PRESS);
        const uint32_t start = prv_now;
        prv_press((enum InputsSharedButtonID)i);
        prv_advance_to(start + INPUT_EVENTS_LONG_PRESS_MS);
        TEST_ASSERT_EQUAL(before + 1U, prv_count(INPUT_EVENT_SOURCE_BUTTON, INPUT_EVENT_TYPE_LONG_PRESS));
        prv_release((enum InputsSharedButtonID)i);
        prv_event_count = 0U;
    }
}

void test_input_events_knob_rotation_is_delivered_with_delta(void) {
    TEST_ASSERT_EQUAL(INPUTS_RC_OK, input_events_api_on_knob_rotation(INPUTS_SHARED_KNOB_ID_FRONT_RIGHT, -3));
    prv_expect(0U, INPUT_EVENT_SOURCE_KNOB, INPUT_EVENT_TYPE_ROTATION, INPUTS_SHARED_KNOB_ID_FRONT_RIGHT, 0U);
    TEST_ASSERT_EQUAL_INT8(-3, prv_events[0].event.delta);
}

void test_input_events_invalid_ids_are_rejected(void) {
    TEST_ASSERT_EQUAL(INPUTS_RC_ERROR, input_events_api_on_button_press(INPUTS_SHARED_BUTTON_ID_COUNT));
    TEST_ASSERT_EQUAL(INPUTS_RC_ERROR, input_events_api_on_button_release(INPUTS_SHARED_BUTTON_ID_COUNT));
    TEST_ASSERT_EQUAL(INPUTS_RC_ERROR, input_events_api_on_knob_rotation(INPUTS_SHARED_KNOB_ID_COUNT, 1));
    TEST_ASSERT_FALSE(input_events_api_is_held(INPUTS_SHARED_BUTTON_ID_COUNT));
    TEST_ASSERT_EQUAL(0U, prv_event_count);
}

void test_input_events_sink_failure_is_reported(void) {
    fake_sink_fake.custom_fake = NULL;
    fake_sink_fake.return_val = INPUT_EVENTS_RC_ERROR;
    TEST_ASSERT_EQUAL(INPUTS_RC_ERROR, input_events_api_on_button_press(TEST_PLAIN));
    TEST_ASSERT_EQUAL(INPUTS_RC_ERROR, input_events_api_on_button_release(TEST_PLAIN));
    TEST_ASSERT_EQUAL(INPUTS_RC_ERROR, input_events_api_on_knob_rotation(INPUTS_SHARED_KNOB_ID_FRONT_LEFT, 1));
}

/*! \} */

/*!
 * \defgroup input_events_window Tests for a combo button pressed on its own
 * \{
 */

void test_input_events_combo_button_press_waits_for_the_window(void) {
    prv_press(TEST_COMBO_FIRST);
    prv_advance_to(INPUT_EVENTS_COMBO_WINDOW_MS - 1U);
    TEST_ASSERT_EQUAL_MESSAGE(0U, prv_event_count, "Delivered before the combo window ended");
    TEST_ASSERT_TRUE(input_events_api_is_held(TEST_COMBO_FIRST));

    prv_advance_to(INPUT_EVENTS_COMBO_WINDOW_MS);
    TEST_ASSERT_EQUAL(1U, prv_event_count);
    prv_expect(0U, INPUT_EVENT_SOURCE_BUTTON, INPUT_EVENT_TYPE_PRESS, TEST_COMBO_FIRST, INPUT_EVENTS_COMBO_WINDOW_MS);
}

void test_input_events_combo_button_tap_is_delivered_at_the_release(void) {
    prv_press(TEST_COMBO_FIRST);
    prv_advance_to(50U);
    prv_release(TEST_COMBO_FIRST);

    TEST_ASSERT_EQUAL(3U, prv_event_count);
    prv_expect(0U, INPUT_EVENT_SOURCE_BUTTON, INPUT_EVENT_TYPE_PRESS, TEST_COMBO_FIRST, 50U);
    prv_expect(1U, INPUT_EVENT_SOURCE_BUTTON, INPUT_EVENT_TYPE_RELEASE, TEST_COMBO_FIRST, 50U);
    prv_expect(2U, INPUT_EVENT_SOURCE_BUTTON, INPUT_EVENT_TYPE_TAP, TEST_COMBO_FIRST, 50U);

    prv_advance_to(10U * INPUT_EVENTS_COMBO_WINDOW_MS);
    TEST_ASSERT_EQUAL_MESSAGE(3U, prv_event_count, "The window must not deliver it again");
}

void test_input_events_combo_button_long_press_is_timed_from_the_physical_press(void) {
    prv_press(TEST_COMBO_FIRST);
    prv_advance_to(INPUT_EVENTS_LONG_PRESS_MS);

    TEST_ASSERT_EQUAL(2U, prv_event_count);
    prv_expect(1U, INPUT_EVENT_SOURCE_BUTTON, INPUT_EVENT_TYPE_LONG_PRESS, TEST_COMBO_FIRST, INPUT_EVENTS_LONG_PRESS_MS);
}

void test_input_events_combo_buttons_are_immediate_when_the_combo_is_not_active(void) {
    prv_active = 0U;
    prv_press(TEST_COMBO_FIRST);
    TEST_ASSERT_EQUAL(1U, prv_event_count);
    prv_expect(0U, INPUT_EVENT_SOURCE_BUTTON, INPUT_EVENT_TYPE_PRESS, TEST_COMBO_FIRST, 0U);
}

void test_input_events_without_active_combos_callback_everything_is_immediate(void) {
    TEST_ASSERT_EQUAL(TIMEOUT_RC_OK, timeout_api_init(prv_get_tick));
    TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_OK, input_events_api_init(fake_sink, NULL));
    prv_press(TEST_COMBO_FIRST);
    TEST_ASSERT_EQUAL(1U, prv_event_count);
}

void test_input_events_other_button_ends_the_window(void) {
    prv_press(TEST_COMBO_FIRST);
    prv_advance_to(40U);
    prv_press(TEST_PLAIN);

    TEST_ASSERT_EQUAL(2U, prv_event_count);
    prv_expect(0U, INPUT_EVENT_SOURCE_BUTTON, INPUT_EVENT_TYPE_PRESS, TEST_COMBO_FIRST, 40U);
    prv_expect(1U, INPUT_EVENT_SOURCE_BUTTON, INPUT_EVENT_TYPE_PRESS, TEST_PLAIN, 40U);
    prv_advance_to(10U * INPUT_EVENTS_COMBO_WINDOW_MS);
    TEST_ASSERT_EQUAL(2U, prv_count(INPUT_EVENT_SOURCE_BUTTON, INPUT_EVENT_TYPE_PRESS));
}

void test_input_events_second_button_after_the_window_is_no_combo(void) {
    prv_press(TEST_COMBO_FIRST);
    prv_advance_to(INPUT_EVENTS_COMBO_WINDOW_MS + 10U);
    prv_press(TEST_COMBO_SECOND);
    prv_advance_to(3U * INPUT_EVENTS_COMBO_WINDOW_MS);

    TEST_ASSERT_EQUAL(0U, prv_count(INPUT_EVENT_SOURCE_COMBO, INPUT_EVENT_TYPE_PRESS));
    prv_expect(0U, INPUT_EVENT_SOURCE_BUTTON, INPUT_EVENT_TYPE_PRESS, TEST_COMBO_FIRST, INPUT_EVENTS_COMBO_WINDOW_MS);
    prv_expect(1U, INPUT_EVENT_SOURCE_BUTTON, INPUT_EVENT_TYPE_PRESS, TEST_COMBO_SECOND, 2U * INPUT_EVENTS_COMBO_WINDOW_MS + 10U);
}

/*! \} */

/*!
 * \defgroup input_events_combo Tests for combos
 * \{
 */

void test_input_events_combo_press_inside_the_window(void) {
    prv_press(TEST_COMBO_FIRST);
    prv_advance_to(INPUT_EVENTS_COMBO_WINDOW_MS - 1U);
    prv_press(TEST_COMBO_SECOND);

    TEST_ASSERT_EQUAL(1U, prv_event_count);
    prv_expect(0U, INPUT_EVENT_SOURCE_COMBO, INPUT_EVENT_TYPE_PRESS, INPUT_EVENTS_COMBO_MENU, INPUT_EVENTS_COMBO_WINDOW_MS - 1U);
    prv_advance_to(10U * INPUT_EVENTS_COMBO_WINDOW_MS);
    TEST_ASSERT_EQUAL_MESSAGE(0U, prv_count(INPUT_EVENT_SOURCE_BUTTON, INPUT_EVENT_TYPE_PRESS), "Combo buttons have no press of their own");
}

void test_input_events_combo_tap(void) {
    prv_press(TEST_COMBO_FIRST);
    prv_press(TEST_COMBO_SECOND);
    prv_advance_to(INPUT_EVENTS_COMBO_LONG_PRESS_MS - 1U);
    prv_release(TEST_COMBO_SECOND);
    prv_advance_to(INPUT_EVENTS_COMBO_LONG_PRESS_MS + 100U);
    prv_release(TEST_COMBO_FIRST);

    TEST_ASSERT_EQUAL(3U, prv_event_count);
    prv_expect(1U, INPUT_EVENT_SOURCE_COMBO, INPUT_EVENT_TYPE_RELEASE, INPUT_EVENTS_COMBO_MENU, INPUT_EVENTS_COMBO_LONG_PRESS_MS - 1U);
    prv_expect(2U, INPUT_EVENT_SOURCE_COMBO, INPUT_EVENT_TYPE_TAP, INPUT_EVENTS_COMBO_MENU, INPUT_EVENTS_COMBO_LONG_PRESS_MS - 1U);
}

void test_input_events_combo_long_press(void) {
    prv_press(TEST_COMBO_FIRST);
    prv_advance_to(20U);
    prv_press(TEST_COMBO_SECOND);
    prv_advance_to(20U + 5U * INPUT_EVENTS_COMBO_LONG_PRESS_MS);
    prv_release(TEST_COMBO_FIRST);
    prv_release(TEST_COMBO_SECOND);

    TEST_ASSERT_EQUAL(3U, prv_event_count);
    prv_expect(1U, INPUT_EVENT_SOURCE_COMBO, INPUT_EVENT_TYPE_LONG_PRESS, INPUT_EVENTS_COMBO_MENU, 20U + INPUT_EVENTS_COMBO_LONG_PRESS_MS);
    prv_expect(2U, INPUT_EVENT_SOURCE_COMBO, INPUT_EVENT_TYPE_RELEASE, INPUT_EVENTS_COMBO_MENU, 20U + 5U * INPUT_EVENTS_COMBO_LONG_PRESS_MS);
    TEST_ASSERT_EQUAL_MESSAGE(0U, prv_count(INPUT_EVENT_SOURCE_BUTTON, INPUT_EVENT_TYPE_LONG_PRESS), "Combo buttons have no long press of their own");
}

void test_input_events_combo_rearms_after_its_buttons_are_released(void) {
    prv_press(TEST_COMBO_FIRST);
    prv_press(TEST_COMBO_SECOND);
    prv_release(TEST_COMBO_FIRST);
    prv_release(TEST_COMBO_SECOND);
    prv_advance_to(100U);
    prv_press(TEST_COMBO_SECOND);
    prv_press(TEST_COMBO_FIRST);

    TEST_ASSERT_EQUAL(2U, prv_count(INPUT_EVENT_SOURCE_COMBO, INPUT_EVENT_TYPE_PRESS));
    TEST_ASSERT_EQUAL(0U, prv_count(INPUT_EVENT_SOURCE_BUTTON, INPUT_EVENT_TYPE_PRESS));
}

void test_input_events_combo_held_button_can_join_a_new_combo_only_after_release(void) {
    // One button still down from the last combo: pressing the other again is a fresh press.
    prv_press(TEST_COMBO_FIRST);
    prv_press(TEST_COMBO_SECOND);
    prv_release(TEST_COMBO_SECOND);
    prv_advance_to(100U);
    prv_press(TEST_COMBO_SECOND);
    prv_advance_to(100U + INPUT_EVENTS_COMBO_WINDOW_MS);

    TEST_ASSERT_EQUAL(1U, prv_count(INPUT_EVENT_SOURCE_COMBO, INPUT_EVENT_TYPE_PRESS));
    prv_expect(prv_event_count - 1U, INPUT_EVENT_SOURCE_BUTTON, INPUT_EVENT_TYPE_PRESS, TEST_COMBO_SECOND, 100U + INPUT_EVENTS_COMBO_WINDOW_MS);
}

void test_input_events_only_active_combos_are_recognised(void) {
    prv_press(INPUTS_SHARED_BUTTON_ID_PADDLE_BOTTOM_LEFT);
    prv_press(INPUTS_SHARED_BUTTON_ID_PADDLE_BOTTOM_RIGHT);
    TEST_ASSERT_EQUAL_MESSAGE(2U, prv_count(INPUT_EVENT_SOURCE_BUTTON, INPUT_EVENT_TYPE_PRESS), "Wake is not active: plain buttons");

    TEST_ASSERT_EQUAL(TIMEOUT_RC_OK, timeout_api_init(prv_get_tick));
    TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_OK, input_events_api_init(fake_sink, prv_active_combos));
    prv_event_count = 0U;
    prv_active = INPUT_EVENTS_COMBO_BIT(INPUT_EVENTS_COMBO_WAKE);
    prv_press(INPUTS_SHARED_BUTTON_ID_PADDLE_BOTTOM_LEFT);
    prv_press(INPUTS_SHARED_BUTTON_ID_PADDLE_BOTTOM_RIGHT);
    TEST_ASSERT_EQUAL(1U, prv_event_count);
    prv_expect(0U, INPUT_EVENT_SOURCE_COMBO, INPUT_EVENT_TYPE_PRESS, INPUT_EVENTS_COMBO_WAKE, prv_now);
}

/*! \} */

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_input_events_init_rejects_null_sink);
    RUN_TEST(test_input_events_init_fails_when_the_pool_is_full);
    RUN_TEST(test_input_events_init_releases_every_button);
    RUN_TEST(test_input_events_combos_are_real_combinations);

    RUN_TEST(test_input_events_plain_press_is_delivered_at_once);
    RUN_TEST(test_input_events_short_press_is_release_then_tap);
    RUN_TEST(test_input_events_long_press_once_then_release_without_tap);
    RUN_TEST(test_input_events_long_press_rearms_after_a_release);
    RUN_TEST(test_input_events_repeated_press_changes_nothing);
    RUN_TEST(test_input_events_release_of_a_released_button_changes_nothing);
    RUN_TEST(test_input_events_long_press_timers_are_per_button);
    RUN_TEST(test_input_events_every_button_has_a_long_press);
    RUN_TEST(test_input_events_knob_rotation_is_delivered_with_delta);
    RUN_TEST(test_input_events_invalid_ids_are_rejected);
    RUN_TEST(test_input_events_sink_failure_is_reported);

    RUN_TEST(test_input_events_combo_button_press_waits_for_the_window);
    RUN_TEST(test_input_events_combo_button_tap_is_delivered_at_the_release);
    RUN_TEST(test_input_events_combo_button_long_press_is_timed_from_the_physical_press);
    RUN_TEST(test_input_events_combo_buttons_are_immediate_when_the_combo_is_not_active);
    RUN_TEST(test_input_events_without_active_combos_callback_everything_is_immediate);
    RUN_TEST(test_input_events_other_button_ends_the_window);
    RUN_TEST(test_input_events_second_button_after_the_window_is_no_combo);

    RUN_TEST(test_input_events_combo_press_inside_the_window);
    RUN_TEST(test_input_events_combo_tap);
    RUN_TEST(test_input_events_combo_long_press);
    RUN_TEST(test_input_events_combo_rearms_after_its_buttons_are_released);
    RUN_TEST(test_input_events_combo_held_button_can_join_a_new_combo_only_after_release);
    RUN_TEST(test_input_events_only_active_combos_are_recognised);

    return UNITY_END();
}
