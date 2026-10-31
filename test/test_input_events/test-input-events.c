/*!
 * \file test-input-events.c
 * \date 2026-10-31
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 *
 * \brief Unit tests for the input event layer.
 *
 * \details Combo hold times run on a real libtimebase watchdog pool with a
 *     fake clock; the sink records every event it receives.
 */

#include "unity.h"
#include "fff.h"
#include "input-events-api.h"
#include "watchdogs-api.h"
#include <string.h>

DEFINE_FFF_GLOBALS;

FAKE_VALUE_FUNC(enum InputEventsReturnCode, fake_sink, const struct InputEvent *);

extern struct InputEventsHandler input_events_handler;

#define TEST_INPUT_EVENTS_CAPACITY (32U)

static uint32_t prv_now;
static struct WatchdogHandler prv_watchdogs;
static struct InputEvent prv_events[TEST_INPUT_EVENTS_CAPACITY];
static size_t prv_event_count;

static uint32_t prv_get_tick(void) {
    return prv_now;
}

static enum InputEventsReturnCode prv_record(const struct InputEvent *event) {
    if (prv_event_count < TEST_INPUT_EVENTS_CAPACITY) {
        prv_events[prv_event_count++] = *event;
    }
    return INPUT_EVENTS_RC_OK;
}

/* Move the clock and let the watchdog pool fire whatever expired. */
static void prv_advance_to(uint32_t tick) {
    prv_now = tick;
    TEST_ASSERT_EQUAL(WATCHDOG_RC_OK, watchdogs_api_routine(&prv_watchdogs));
}

static size_t prv_count_combos(enum InputEventsComboId combo_id) {
    size_t count = 0U;
    for (size_t i = 0U; i < prv_event_count; i++) {
        if (prv_events[i].type == INPUT_EVENT_TYPE_COMBO && prv_events[i].combo == combo_id) {
            count++;
        }
    }
    return count;
}

/* Press (or release) every button of a combo, in ID order. */
static void prv_set_combo_buttons(enum InputEventsComboId combo_id, bool pressed) {
    const uint16_t buttons = input_events_api_get_combo(combo_id)->buttons;
    for (uint8_t i = 0U; i < INPUTS_SHARED_BUTTON_ID_COUNT; i++) {
        if ((buttons & INPUT_EVENTS_BUTTON_BIT(i)) == 0U) {
            continue;
        }
        if (pressed) {
            TEST_ASSERT_EQUAL(INPUTS_RC_OK, input_events_api_on_button_press((enum InputsSharedButtonID)i));
        } else {
            TEST_ASSERT_EQUAL(INPUTS_RC_OK, input_events_api_on_button_release((enum InputsSharedButtonID)i));
        }
    }
}

/* First (lowest ID) button of a combo. */
static enum InputsSharedButtonID prv_first_button(enum InputEventsComboId combo_id) {
    const uint16_t buttons = input_events_api_get_combo(combo_id)->buttons;
    for (uint8_t i = 0U; i < INPUTS_SHARED_BUTTON_ID_COUNT; i++) {
        if ((buttons & INPUT_EVENTS_BUTTON_BIT(i)) != 0U) {
            return (enum InputsSharedButtonID)i;
        }
    }
    TEST_FAIL_MESSAGE("combo without buttons");
    return INPUTS_SHARED_BUTTON_ID_COUNT;
}

/* Last (highest ID) button of a combo. */
static enum InputsSharedButtonID prv_last_button(enum InputEventsComboId combo_id) {
    const uint16_t buttons = input_events_api_get_combo(combo_id)->buttons;
    for (int8_t i = (int8_t)INPUTS_SHARED_BUTTON_ID_COUNT - 1; i >= 0; i--) {
        if ((buttons & INPUT_EVENTS_BUTTON_BIT(i)) != 0U) {
            return (enum InputsSharedButtonID)i;
        }
    }
    TEST_FAIL_MESSAGE("combo without buttons");
    return INPUTS_SHARED_BUTTON_ID_COUNT;
}

/* A button that belongs to no combo. */
static enum InputsSharedButtonID prv_free_button(void) {
    for (uint8_t i = 0U; i < INPUTS_SHARED_BUTTON_ID_COUNT; i++) {
        bool used = false;
        for (uint8_t c = 0U; c < INPUT_EVENTS_COMBO_COUNT; c++) {
            used = used || (input_events_api_get_combo((enum InputEventsComboId)c)->buttons & INPUT_EVENTS_BUTTON_BIT(i)) != 0U;
        }
        if (!used) {
            return (enum InputsSharedButtonID)i;
        }
    }
    TEST_FAIL_MESSAGE("every button is in a combo");
    return INPUTS_SHARED_BUTTON_ID_COUNT;
}

void setUp(void) {
    RESET_FAKE(fake_sink);
    FFF_RESET_HISTORY();
    fake_sink_fake.custom_fake = prv_record;
    prv_now = 0U;
    prv_event_count = 0U;
    memset(prv_events, 0, sizeof(prv_events));
    TEST_ASSERT_EQUAL(WATCHDOG_RC_OK, watchdogs_api_init_pool(&prv_watchdogs, prv_get_tick));
    TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_OK, input_events_api_init(&prv_watchdogs, fake_sink));
}

void tearDown(void) {
}

/*!
 * \defgroup input_events_init Tests for input_events_api_init and the combo table
 * \{
 */

void test_input_events_init_rejects_null_pointers(void) {
    TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_NULL_POINTER, input_events_api_init(NULL, fake_sink));
    TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_NULL_POINTER, input_events_api_init(&prv_watchdogs, NULL));
}

void test_input_events_init_releases_every_button(void) {
    TEST_ASSERT_EQUAL(INPUTS_RC_OK, input_events_api_on_button_press(INPUTS_SHARED_BUTTON_ID_BOTTOM_LEFT));
    TEST_ASSERT_EQUAL(INPUT_EVENTS_RC_OK, input_events_api_init(&prv_watchdogs, fake_sink));
    TEST_ASSERT_FALSE(input_events_api_is_held(INPUTS_SHARED_BUTTON_ID_BOTTOM_LEFT));
}

void test_input_events_get_combo_rejects_invalid_id(void) {
    TEST_ASSERT_NULL(input_events_api_get_combo(INPUT_EVENTS_COMBO_COUNT));
}

void test_input_events_combos_are_real_combinations(void) {
    for (uint8_t i = 0U; i < INPUT_EVENTS_COMBO_COUNT; i++) {
        const struct InputEventsCombo *combo = input_events_api_get_combo((enum InputEventsComboId)i);
        TEST_ASSERT_NOT_NULL(combo);
        TEST_ASSERT_TRUE_MESSAGE(__builtin_popcount(combo->buttons) >= 2, "A combo needs at least two buttons");
        TEST_ASSERT_EQUAL_MESSAGE(0U, combo->buttons >> INPUTS_SHARED_BUTTON_ID_COUNT, "Combo uses a button that doesn't exist");
        TEST_ASSERT_TRUE(combo->hold_ms > 0U);
        for (uint8_t j = 0U; j < i; j++) {
            TEST_ASSERT_NOT_EQUAL_MESSAGE(input_events_api_get_combo((enum InputEventsComboId)j)->buttons, combo->buttons, "Two combos with the same buttons");
        }
    }
}

/*! \} */

/*!
 * \defgroup input_events_forwarding Tests for the plain button and knob events
 * \{
 */

void test_input_events_press_is_forwarded_and_held(void) {
    TEST_ASSERT_EQUAL(INPUTS_RC_OK, input_events_api_on_button_press(INPUTS_SHARED_BUTTON_ID_BOTTOM_LEFT));
    TEST_ASSERT_EQUAL(1U, prv_event_count);
    TEST_ASSERT_EQUAL(INPUT_EVENT_TYPE_BUTTON_PRESS, prv_events[0].type);
    TEST_ASSERT_EQUAL(INPUTS_SHARED_BUTTON_ID_BOTTOM_LEFT, prv_events[0].button);
    TEST_ASSERT_TRUE(input_events_api_is_held(INPUTS_SHARED_BUTTON_ID_BOTTOM_LEFT));
}

void test_input_events_release_is_forwarded_and_not_held(void) {
    TEST_ASSERT_EQUAL(INPUTS_RC_OK, input_events_api_on_button_press(INPUTS_SHARED_BUTTON_ID_TS_ON));
    TEST_ASSERT_EQUAL(INPUTS_RC_OK, input_events_api_on_button_release(INPUTS_SHARED_BUTTON_ID_TS_ON));
    TEST_ASSERT_EQUAL(2U, prv_event_count);
    TEST_ASSERT_EQUAL(INPUT_EVENT_TYPE_BUTTON_RELEASE, prv_events[1].type);
    TEST_ASSERT_EQUAL(INPUTS_SHARED_BUTTON_ID_TS_ON, prv_events[1].button);
    TEST_ASSERT_FALSE(input_events_api_is_held(INPUTS_SHARED_BUTTON_ID_TS_ON));
}

void test_input_events_knob_rotation_is_forwarded_with_delta(void) {
    TEST_ASSERT_EQUAL(INPUTS_RC_OK, input_events_api_on_knob_rotation(INPUTS_SHARED_KNOB_ID_FRONT_RIGHT, -3));
    TEST_ASSERT_EQUAL(1U, prv_event_count);
    TEST_ASSERT_EQUAL(INPUT_EVENT_TYPE_KNOB_ROTATION, prv_events[0].type);
    TEST_ASSERT_EQUAL(INPUTS_SHARED_KNOB_ID_FRONT_RIGHT, prv_events[0].knob);
    TEST_ASSERT_EQUAL_INT8(-3, prv_events[0].delta);
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
    TEST_ASSERT_EQUAL(INPUTS_RC_ERROR, input_events_api_on_button_press(INPUTS_SHARED_BUTTON_ID_BOTTOM_RIGHT));
    TEST_ASSERT_EQUAL(INPUTS_RC_ERROR, input_events_api_on_button_release(INPUTS_SHARED_BUTTON_ID_BOTTOM_RIGHT));
    TEST_ASSERT_EQUAL(INPUTS_RC_ERROR, input_events_api_on_knob_rotation(INPUTS_SHARED_KNOB_ID_FRONT_LEFT, 1));
}

/*! \} */

/*!
 * \defgroup input_events_long_press Tests for the long press (watchdog per button)
 * \{
 */

static size_t prv_count_long_presses(enum InputsSharedButtonID button_id) {
    size_t count = 0U;
    for (size_t i = 0U; i < prv_event_count; i++) {
        if (prv_events[i].type == INPUT_EVENT_TYPE_BUTTON_LONG_PRESS && prv_events[i].button == button_id) {
            count++;
        }
    }
    return count;
}

void test_input_events_every_button_long_presses_after_its_hold_time(void) {
    for (uint8_t i = 0U; i < INPUTS_SHARED_BUTTON_ID_COUNT; i++) {
        const enum InputsSharedButtonID button = (enum InputsSharedButtonID)i;
        const uint32_t start = prv_now;
        TEST_ASSERT_EQUAL(INPUTS_RC_OK, input_events_api_on_button_press(button));
        prv_advance_to(start + INPUT_EVENTS_LONG_PRESS_MS - 1U);
        TEST_ASSERT_EQUAL_MESSAGE(0U, prv_count_long_presses(button), "Long press before its hold time");
        prv_advance_to(start + INPUT_EVENTS_LONG_PRESS_MS);
        TEST_ASSERT_EQUAL_MESSAGE(1U, prv_count_long_presses(button), "Every button needs its long-press watchdog");
        TEST_ASSERT_EQUAL(INPUTS_RC_OK, input_events_api_on_button_release(button));
    }
}

void test_input_events_long_press_fires_once_per_hold(void) {
    TEST_ASSERT_EQUAL(INPUTS_RC_OK, input_events_api_on_button_press(INPUTS_SHARED_BUTTON_ID_BOTTOM_LEFT));
    for (uint32_t tick = 1U; tick <= 5U * INPUT_EVENTS_LONG_PRESS_MS; tick += 10U) {
        prv_advance_to(tick);
    }
    TEST_ASSERT_EQUAL(1U, prv_count_long_presses(INPUTS_SHARED_BUTTON_ID_BOTTOM_LEFT));
}

void test_input_events_short_press_is_not_long(void) {
    TEST_ASSERT_EQUAL(INPUTS_RC_OK, input_events_api_on_button_press(INPUTS_SHARED_BUTTON_ID_BOTTOM_LEFT));
    prv_advance_to(INPUT_EVENTS_LONG_PRESS_MS - 1U);
    TEST_ASSERT_EQUAL(INPUTS_RC_OK, input_events_api_on_button_release(INPUTS_SHARED_BUTTON_ID_BOTTOM_LEFT));
    prv_advance_to(10U * INPUT_EVENTS_LONG_PRESS_MS);
    TEST_ASSERT_EQUAL(0U, prv_count_long_presses(INPUTS_SHARED_BUTTON_ID_BOTTOM_LEFT));
}

void test_input_events_long_press_rearms_after_a_release(void) {
    TEST_ASSERT_EQUAL(INPUTS_RC_OK, input_events_api_on_button_press(INPUTS_SHARED_BUTTON_ID_PADDLE_TOP_LEFT));
    prv_advance_to(INPUT_EVENTS_LONG_PRESS_MS);
    TEST_ASSERT_EQUAL(INPUTS_RC_OK, input_events_api_on_button_release(INPUTS_SHARED_BUTTON_ID_PADDLE_TOP_LEFT));
    prv_advance_to(INPUT_EVENTS_LONG_PRESS_MS + 100U);
    TEST_ASSERT_EQUAL(INPUTS_RC_OK, input_events_api_on_button_press(INPUTS_SHARED_BUTTON_ID_PADDLE_TOP_LEFT));
    prv_advance_to(2U * INPUT_EVENTS_LONG_PRESS_MS + 100U);
    TEST_ASSERT_EQUAL(2U, prv_count_long_presses(INPUTS_SHARED_BUTTON_ID_PADDLE_TOP_LEFT));
}

void test_input_events_long_press_timers_are_per_button(void) {
    TEST_ASSERT_EQUAL(INPUTS_RC_OK, input_events_api_on_button_press(INPUTS_SHARED_BUTTON_ID_BOTTOM_LEFT));
    prv_advance_to(300U);
    TEST_ASSERT_EQUAL(INPUTS_RC_OK, input_events_api_on_button_press(INPUTS_SHARED_BUTTON_ID_BOTTOM_RIGHT));
    TEST_ASSERT_EQUAL(INPUTS_RC_OK, input_events_api_on_button_release(INPUTS_SHARED_BUTTON_ID_BOTTOM_LEFT));
    prv_advance_to(300U + INPUT_EVENTS_LONG_PRESS_MS);
    TEST_ASSERT_EQUAL(0U, prv_count_long_presses(INPUTS_SHARED_BUTTON_ID_BOTTOM_LEFT));
    TEST_ASSERT_EQUAL(1U, prv_count_long_presses(INPUTS_SHARED_BUTTON_ID_BOTTOM_RIGHT));
}

void test_input_events_repeated_press_keeps_the_long_press_time(void) {
    TEST_ASSERT_EQUAL(INPUTS_RC_OK, input_events_api_on_button_press(INPUTS_SHARED_BUTTON_ID_BOTTOM_LEFT));
    prv_advance_to(INPUT_EVENTS_LONG_PRESS_MS / 2U);
    TEST_ASSERT_EQUAL(INPUTS_RC_OK, input_events_api_on_button_press(INPUTS_SHARED_BUTTON_ID_BOTTOM_LEFT));
    prv_advance_to(INPUT_EVENTS_LONG_PRESS_MS);
    TEST_ASSERT_EQUAL(1U, prv_count_long_presses(INPUTS_SHARED_BUTTON_ID_BOTTOM_LEFT));
}

/*! \} */

/*!
 * \defgroup input_events_combos Tests for the combo detection
 * \{
 */

void test_input_events_combo_fires_after_its_hold_time(void) {
    prv_set_combo_buttons(INPUT_EVENTS_COMBO_MENU, true);
    prv_advance_to(INPUT_EVENTS_COMBO_HOLD_MS - 1U);
    TEST_ASSERT_EQUAL_MESSAGE(0U, prv_count_combos(INPUT_EVENTS_COMBO_MENU), "Combo fired before its hold time");
    prv_advance_to(INPUT_EVENTS_COMBO_HOLD_MS);
    TEST_ASSERT_EQUAL(1U, prv_count_combos(INPUT_EVENTS_COMBO_MENU));
    TEST_ASSERT_EQUAL(INPUT_EVENT_TYPE_COMBO, prv_events[prv_event_count - 1U].type);
}

void test_input_events_combo_buttons_still_produce_presses(void) {
    prv_set_combo_buttons(INPUT_EVENTS_COMBO_MENU, true);
    TEST_ASSERT_EQUAL(2U, prv_event_count);
    TEST_ASSERT_EQUAL(INPUT_EVENT_TYPE_BUTTON_PRESS, prv_events[0].type);
    TEST_ASSERT_EQUAL(INPUT_EVENT_TYPE_BUTTON_PRESS, prv_events[1].type);
}

void test_input_events_combo_hold_starts_with_the_last_button(void) {
    TEST_ASSERT_EQUAL(INPUTS_RC_OK, input_events_api_on_button_press(prv_first_button(INPUT_EVENTS_COMBO_MENU)));
    prv_advance_to(600U);
    TEST_ASSERT_EQUAL(INPUTS_RC_OK, input_events_api_on_button_press(prv_last_button(INPUT_EVENTS_COMBO_MENU)));
    prv_advance_to(600U + INPUT_EVENTS_COMBO_HOLD_MS - 1U);
    TEST_ASSERT_EQUAL(0U, prv_count_combos(INPUT_EVENTS_COMBO_MENU));
    prv_advance_to(600U + INPUT_EVENTS_COMBO_HOLD_MS);
    TEST_ASSERT_EQUAL(1U, prv_count_combos(INPUT_EVENTS_COMBO_MENU));
}

void test_input_events_one_button_alone_never_fires(void) {
    TEST_ASSERT_EQUAL(INPUTS_RC_OK, input_events_api_on_button_press(prv_first_button(INPUT_EVENTS_COMBO_MENU)));
    prv_advance_to(10U * INPUT_EVENTS_COMBO_HOLD_MS);
    TEST_ASSERT_EQUAL(0U, prv_count_combos(INPUT_EVENTS_COMBO_MENU));
}

void test_input_events_early_release_cancels_the_combo(void) {
    prv_set_combo_buttons(INPUT_EVENTS_COMBO_MENU, true);
    prv_advance_to(INPUT_EVENTS_COMBO_HOLD_MS / 2U);
    TEST_ASSERT_EQUAL(INPUTS_RC_OK, input_events_api_on_button_release(prv_last_button(INPUT_EVENTS_COMBO_MENU)));
    prv_advance_to(10U * INPUT_EVENTS_COMBO_HOLD_MS);
    TEST_ASSERT_EQUAL(0U, prv_count_combos(INPUT_EVENTS_COMBO_MENU));
}

void test_input_events_combo_fires_once_per_hold(void) {
    prv_set_combo_buttons(INPUT_EVENTS_COMBO_MENU, true);
    for (uint32_t tick = 1U; tick <= 5U * INPUT_EVENTS_COMBO_HOLD_MS; tick += 10U) {
        prv_advance_to(tick);
    }
    TEST_ASSERT_EQUAL(1U, prv_count_combos(INPUT_EVENTS_COMBO_MENU));
}

void test_input_events_combo_rearms_after_a_release(void) {
    prv_set_combo_buttons(INPUT_EVENTS_COMBO_MENU, true);
    prv_advance_to(INPUT_EVENTS_COMBO_HOLD_MS);
    const enum InputsSharedButtonID button = prv_last_button(INPUT_EVENTS_COMBO_MENU);
    TEST_ASSERT_EQUAL(INPUTS_RC_OK, input_events_api_on_button_release(button));
    prv_advance_to(INPUT_EVENTS_COMBO_HOLD_MS + 100U);
    TEST_ASSERT_EQUAL(INPUTS_RC_OK, input_events_api_on_button_press(button));
    prv_advance_to(2U * INPUT_EVENTS_COMBO_HOLD_MS + 99U);
    TEST_ASSERT_EQUAL(1U, prv_count_combos(INPUT_EVENTS_COMBO_MENU));
    prv_advance_to(2U * INPUT_EVENTS_COMBO_HOLD_MS + 100U);
    TEST_ASSERT_EQUAL(2U, prv_count_combos(INPUT_EVENTS_COMBO_MENU));
}

void test_input_events_extra_button_does_not_restart_the_hold(void) {
    prv_set_combo_buttons(INPUT_EVENTS_COMBO_MENU, true);
    prv_advance_to(INPUT_EVENTS_COMBO_HOLD_MS / 2U);
    TEST_ASSERT_EQUAL(INPUTS_RC_OK, input_events_api_on_button_press(prv_free_button()));
    prv_advance_to(INPUT_EVENTS_COMBO_HOLD_MS);
    TEST_ASSERT_EQUAL(1U, prv_count_combos(INPUT_EVENTS_COMBO_MENU));
}

void test_input_events_releasing_another_button_keeps_the_combo(void) {
    const enum InputsSharedButtonID other = prv_free_button();
    TEST_ASSERT_EQUAL(INPUTS_RC_OK, input_events_api_on_button_press(other));
    prv_set_combo_buttons(INPUT_EVENTS_COMBO_MENU, true);
    prv_advance_to(INPUT_EVENTS_COMBO_HOLD_MS / 2U);
    TEST_ASSERT_EQUAL(INPUTS_RC_OK, input_events_api_on_button_release(other));
    prv_advance_to(INPUT_EVENTS_COMBO_HOLD_MS);
    TEST_ASSERT_EQUAL(1U, prv_count_combos(INPUT_EVENTS_COMBO_MENU));
}

void test_input_events_repeated_press_does_not_restart_the_hold(void) {
    prv_set_combo_buttons(INPUT_EVENTS_COMBO_MENU, true);
    prv_advance_to(INPUT_EVENTS_COMBO_HOLD_MS / 2U);
    TEST_ASSERT_EQUAL(INPUTS_RC_OK, input_events_api_on_button_press(prv_last_button(INPUT_EVENTS_COMBO_MENU)));
    prv_advance_to(INPUT_EVENTS_COMBO_HOLD_MS);
    TEST_ASSERT_EQUAL(1U, prv_count_combos(INPUT_EVENTS_COMBO_MENU));
    TEST_ASSERT_EQUAL(INPUTS_RC_OK, input_events_api_on_button_press(prv_last_button(INPUT_EVENTS_COMBO_MENU)));
    prv_advance_to(3U * INPUT_EVENTS_COMBO_HOLD_MS);
    TEST_ASSERT_EQUAL_MESSAGE(1U, prv_count_combos(INPUT_EVENTS_COMBO_MENU), "A repeated press re-fired the combo");
}

void test_input_events_combos_are_independent(void) {
    prv_set_combo_buttons(INPUT_EVENTS_COMBO_WAKE, true);
    prv_advance_to(INPUT_EVENTS_COMBO_HOLD_MS);
    TEST_ASSERT_EQUAL(1U, prv_count_combos(INPUT_EVENTS_COMBO_WAKE));
    TEST_ASSERT_EQUAL(0U, prv_count_combos(INPUT_EVENTS_COMBO_MENU));

    prv_set_combo_buttons(INPUT_EVENTS_COMBO_MENU, true);
    prv_advance_to(2U * INPUT_EVENTS_COMBO_HOLD_MS);
    TEST_ASSERT_EQUAL(1U, prv_count_combos(INPUT_EVENTS_COMBO_MENU));
    TEST_ASSERT_EQUAL_MESSAGE(1U, prv_count_combos(INPUT_EVENTS_COMBO_WAKE), "Wake fired again without a release");
}

/*! \} */

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_input_events_init_rejects_null_pointers);
    RUN_TEST(test_input_events_init_releases_every_button);
    RUN_TEST(test_input_events_get_combo_rejects_invalid_id);
    RUN_TEST(test_input_events_combos_are_real_combinations);

    RUN_TEST(test_input_events_press_is_forwarded_and_held);
    RUN_TEST(test_input_events_release_is_forwarded_and_not_held);
    RUN_TEST(test_input_events_knob_rotation_is_forwarded_with_delta);
    RUN_TEST(test_input_events_invalid_ids_are_rejected);
    RUN_TEST(test_input_events_sink_failure_is_reported);

    RUN_TEST(test_input_events_every_button_long_presses_after_its_hold_time);
    RUN_TEST(test_input_events_long_press_fires_once_per_hold);
    RUN_TEST(test_input_events_short_press_is_not_long);
    RUN_TEST(test_input_events_long_press_rearms_after_a_release);
    RUN_TEST(test_input_events_long_press_timers_are_per_button);
    RUN_TEST(test_input_events_repeated_press_keeps_the_long_press_time);

    RUN_TEST(test_input_events_combo_fires_after_its_hold_time);
    RUN_TEST(test_input_events_combo_buttons_still_produce_presses);
    RUN_TEST(test_input_events_combo_hold_starts_with_the_last_button);
    RUN_TEST(test_input_events_one_button_alone_never_fires);
    RUN_TEST(test_input_events_early_release_cancels_the_combo);
    RUN_TEST(test_input_events_combo_fires_once_per_hold);
    RUN_TEST(test_input_events_combo_rearms_after_a_release);
    RUN_TEST(test_input_events_extra_button_does_not_restart_the_hold);
    RUN_TEST(test_input_events_releasing_another_button_keeps_the_combo);
    RUN_TEST(test_input_events_repeated_press_does_not_restart_the_hold);
    RUN_TEST(test_input_events_combos_are_independent);

    return UNITY_END();
}
