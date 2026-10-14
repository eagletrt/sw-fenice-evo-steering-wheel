/*!
 * \file test_inputs.c
 * \date 2026-03-18
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 *
 * \brief Unit tests for the Inputs API functionality.
 */

#include "inputs-api.h"
#include "inputs-shared.h"
#include "fff.h"
#include "unity.h"

DEFINE_FFF_GLOBALS;

FAKE_VALUE_FUNC(enum InputsReturnCode, press_cb, enum InputsSharedButtonID);
FAKE_VALUE_FUNC(enum InputsReturnCode, long_press_cb, enum InputsSharedButtonID);
FAKE_VALUE_FUNC(enum InputsReturnCode, release_cb, enum InputsSharedButtonID);
FAKE_VALUE_FUNC(enum InputsReturnCode, rotation_cb, enum InputsSharedKnobID, int8_t);

extern struct InputsHandler inputs_handler;

void setUp(void) {
    RESET_FAKE(press_cb);
    RESET_FAKE(long_press_cb);
    RESET_FAKE(release_cb);
    RESET_FAKE(rotation_cb);
    FFF_RESET_HISTORY();
    press_cb_fake.return_val = INPUTS_RC_OK;
    long_press_cb_fake.return_val = INPUTS_RC_OK;
    release_cb_fake.return_val = INPUTS_RC_OK;
    rotation_cb_fake.return_val = INPUTS_RC_OK;
    inputs_api_init(press_cb, long_press_cb, release_cb, rotation_cb);
}

/*!
 * \defgroup inputs_api_init Tests for inputs_api_init function
 * \{
 */

void test_inputs_init_stores_callbacks(void) {
    enum InputsReturnCode rc = inputs_api_init(press_cb, long_press_cb, release_cb, rotation_cb);
    TEST_ASSERT_EQUAL_MESSAGE(INPUTS_RC_OK, rc, "Expected success return code from inputs_api_init");
    TEST_ASSERT_EQUAL_MESSAGE(press_cb, inputs_handler.on_button_press, "Press callback was not set correctly");
    TEST_ASSERT_EQUAL_MESSAGE(long_press_cb, inputs_handler.on_button_long_press, "Long press callback was not set correctly");
    TEST_ASSERT_EQUAL_MESSAGE(release_cb, inputs_handler.on_button_release, "Release callback was not set correctly");
    TEST_ASSERT_EQUAL_MESSAGE(rotation_cb, inputs_handler.on_knob_rotation, "Rotation callback was not set correctly");
}

void test_inputs_init_accepts_null_callbacks(void) {
    enum InputsReturnCode rc = inputs_api_init(NULL, NULL, NULL, NULL);
    TEST_ASSERT_EQUAL_MESSAGE(INPUTS_RC_OK, rc, "Init should accept NULL callbacks");
    TEST_ASSERT_NULL(inputs_handler.on_button_press);
    TEST_ASSERT_NULL(inputs_handler.on_button_long_press);
    TEST_ASSERT_NULL(inputs_handler.on_button_release);
    TEST_ASSERT_NULL(inputs_handler.on_knob_rotation);
}

/*! \} */

/*!
 * \defgroup inputs_api_update_button Tests for inputs_api_update_button function
 * \{
 */

void test_inputs_update_button_press_invokes_press_callback(void) {
    enum InputsReturnCode ret = inputs_api_update_button(INPUTS_SHARED_BUTTON_ID_BOTTOM_LEFT, true, 100);
    TEST_ASSERT_EQUAL_MESSAGE(INPUTS_RC_OK, ret, "Failed to update button state");
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, press_cb_fake.call_count, "Press callback was not called");
    TEST_ASSERT_EQUAL_MESSAGE(INPUTS_SHARED_BUTTON_ID_BOTTOM_LEFT, press_cb_fake.arg0_val, "Wrong button ID passed to press callback");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, release_cb_fake.call_count, "Release callback should not be called on press");
}

void test_inputs_update_button_press_propagates_callback_error(void) {
    press_cb_fake.return_val = INPUTS_RC_ERROR;
    enum InputsReturnCode ret = inputs_api_update_button(INPUTS_SHARED_BUTTON_ID_BOTTOM_LEFT, true, 100);
    TEST_ASSERT_EQUAL_MESSAGE(INPUTS_RC_ERROR, ret, "Expected error when press callback fails");
}

void test_inputs_update_button_invalid_button_id(void) {
    enum InputsReturnCode ret = inputs_api_update_button(INPUTS_SHARED_BUTTON_ID_COUNT, true, 100);
    TEST_ASSERT_EQUAL_MESSAGE(INPUTS_RC_ERROR, ret, "Expected error return code when button ID is out of range");
}

void test_inputs_update_button_disabled(void) {
    inputs_handler.buttons[INPUTS_SHARED_BUTTON_ID_BOTTOM_LEFT].enabled = false;
    enum InputsReturnCode ret = inputs_api_update_button(INPUTS_SHARED_BUTTON_ID_BOTTOM_LEFT, true, 100);
    TEST_ASSERT_EQUAL_MESSAGE(INPUTS_RC_OK, ret, "Expected success return code when button is disabled");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, press_cb_fake.call_count, "Press callback should not be called when button is disabled");
}

void test_inputs_update_button_release_invokes_release_callback(void) {
    inputs_api_update_button(INPUTS_SHARED_BUTTON_ID_BOTTOM_LEFT, true, 100);
    enum InputsReturnCode ret = inputs_api_update_button(INPUTS_SHARED_BUTTON_ID_BOTTOM_LEFT, false, 200);
    TEST_ASSERT_EQUAL_MESSAGE(INPUTS_RC_OK, ret, "Failed to update button state on release");
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, press_cb_fake.call_count, "Press callback should be called once");
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, release_cb_fake.call_count, "Release callback should be called once");
    TEST_ASSERT_EQUAL_MESSAGE(INPUTS_SHARED_BUTTON_ID_BOTTOM_LEFT, release_cb_fake.arg0_val, "Wrong button ID passed to release callback");
}

void test_inputs_update_button_null_callback_drops_event(void) {
    inputs_api_init(NULL, NULL, NULL, NULL);
    enum InputsReturnCode ret = inputs_api_update_button(INPUTS_SHARED_BUTTON_ID_BOTTOM_LEFT, true, 100);
    TEST_ASSERT_EQUAL_MESSAGE(INPUTS_RC_OK, ret, "NULL callback should not produce an error");
}

/*! \} */

/*!
 * \defgroup inputs_api_update_knob Tests for inputs_api_update_knob function
 * \{
 */

void test_inputs_update_knob_invokes_rotation_callback(void) {
    enum InputsReturnCode ret = inputs_api_update_knob(INPUTS_SHARED_KNOB_ID_FRONT_LEFT, 10);
    TEST_ASSERT_EQUAL_MESSAGE(INPUTS_RC_OK, ret, "Failed to update knob state");
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, rotation_cb_fake.call_count, "Rotation callback was not called");
    TEST_ASSERT_EQUAL_MESSAGE(INPUTS_SHARED_KNOB_ID_FRONT_LEFT, rotation_cb_fake.arg0_val, "Wrong knob ID passed");
    TEST_ASSERT_EQUAL_INT_MESSAGE(10, rotation_cb_fake.arg1_val, "Wrong delta passed on first report");
}

void test_inputs_update_knob_propagates_callback_error(void) {
    rotation_cb_fake.return_val = INPUTS_RC_ERROR;
    enum InputsReturnCode ret = inputs_api_update_knob(INPUTS_SHARED_KNOB_ID_FRONT_LEFT, 10);
    TEST_ASSERT_EQUAL_MESSAGE(INPUTS_RC_ERROR, ret, "Expected error when rotation callback fails");
}

void test_inputs_update_knob_invalid_knob_id(void) {
    enum InputsReturnCode ret = inputs_api_update_knob(INPUTS_SHARED_KNOB_ID_COUNT, 10);
    TEST_ASSERT_EQUAL_MESSAGE(INPUTS_RC_ERROR, ret, "Expected error return code when knob ID is out of range");
}

void test_inputs_update_knob_disabled(void) {
    inputs_handler.knobs[INPUTS_SHARED_KNOB_ID_FRONT_LEFT].enabled = false;
    enum InputsReturnCode ret = inputs_api_update_knob(INPUTS_SHARED_KNOB_ID_FRONT_LEFT, 10);
    TEST_ASSERT_EQUAL_MESSAGE(INPUTS_RC_OK, ret, "Expected success return code when knob is disabled");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, rotation_cb_fake.call_count, "Rotation callback should not be called when knob is disabled");
}

void test_inputs_update_knob_no_movement(void) {
    inputs_handler.knobs[INPUTS_SHARED_KNOB_ID_FRONT_LEFT].last_position = 10;
    enum InputsReturnCode ret = inputs_api_update_knob(INPUTS_SHARED_KNOB_ID_FRONT_LEFT, 10);
    TEST_ASSERT_EQUAL_MESSAGE(INPUTS_RC_OK, ret, "Expected success return code when knob position does not change");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, rotation_cb_fake.call_count, "Rotation callback should not be called when knob position does not change");
}

void test_inputs_update_knob_movement(void) {
    inputs_handler.knobs[INPUTS_SHARED_KNOB_ID_FRONT_LEFT].last_position = 10;
    enum InputsReturnCode ret = inputs_api_update_knob(INPUTS_SHARED_KNOB_ID_FRONT_LEFT, 15);
    TEST_ASSERT_EQUAL_MESSAGE(INPUTS_RC_OK, ret, "Failed to update knob state on movement");
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, rotation_cb_fake.call_count, "Rotation callback should be called once for knob movement");
    TEST_ASSERT_EQUAL_INT_MESSAGE(5, rotation_cb_fake.arg1_val, "Wrong delta passed to rotation callback");
}

void test_inputs_update_knob_null_callback_drops_event(void) {
    inputs_api_init(NULL, NULL, NULL, NULL);
    enum InputsReturnCode ret = inputs_api_update_knob(INPUTS_SHARED_KNOB_ID_FRONT_LEFT, 10);
    TEST_ASSERT_EQUAL_MESSAGE(INPUTS_RC_OK, ret, "NULL callback should not produce an error");
}

/*! \} */

/*!
 * \defgroup inputs_api_poll_for_long_press Tests for inputs_api_poll_for_long_press function
 * \{
 */

void test_inputs_poll_for_long_press_invokes_long_press_callback(void) {
    enum InputsReturnCode ret = inputs_api_update_button(INPUTS_SHARED_BUTTON_ID_BOTTOM_LEFT, true, 100);
    TEST_ASSERT_EQUAL_MESSAGE(INPUTS_RC_OK, ret, "Failed to update button state for long press test");
    ret = inputs_api_poll_for_long_press(100 + INPUTS_LONG_PRESS_THRESHOLD_MS);
    TEST_ASSERT_EQUAL_MESSAGE(INPUTS_RC_OK, ret, "Failed to poll for long press");
    TEST_ASSERT_EQUAL_INT_MESSAGE(1, long_press_cb_fake.call_count, "Long press callback should be called once");
    TEST_ASSERT_EQUAL_MESSAGE(INPUTS_SHARED_BUTTON_ID_BOTTOM_LEFT, long_press_cb_fake.arg0_val, "Wrong button ID passed to long press callback");
}

void test_inputs_poll_for_long_press_propagates_callback_error(void) {
    enum InputsReturnCode ret = inputs_api_update_button(INPUTS_SHARED_BUTTON_ID_BOTTOM_LEFT, true, 100);
    TEST_ASSERT_EQUAL_MESSAGE(INPUTS_RC_OK, ret, "Failed to update button state for long press test");
    long_press_cb_fake.return_val = INPUTS_RC_ERROR;
    ret = inputs_api_poll_for_long_press(100 + INPUTS_LONG_PRESS_THRESHOLD_MS);
    TEST_ASSERT_EQUAL_MESSAGE(INPUTS_RC_ERROR, ret, "Expected error return code when long press callback fails");
}

void test_inputs_poll_for_long_press_no_long_press(void) {
    enum InputsReturnCode ret = inputs_api_update_button(INPUTS_SHARED_BUTTON_ID_BOTTOM_LEFT, true, 100);
    TEST_ASSERT_EQUAL_MESSAGE(INPUTS_RC_OK, ret, "Failed to update button state for long press test");
    ret = inputs_api_poll_for_long_press(100 + INPUTS_LONG_PRESS_THRESHOLD_MS - 1);
    TEST_ASSERT_EQUAL_MESSAGE(INPUTS_RC_OK, ret, "Failed to poll for long press when threshold not reached");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, long_press_cb_fake.call_count, "Long press callback should not be called before threshold");
}

/*! \} */

int main() {
    UNITY_BEGIN();

    RUN_TEST(test_inputs_init_stores_callbacks);
    RUN_TEST(test_inputs_init_accepts_null_callbacks);

    RUN_TEST(test_inputs_update_button_press_invokes_press_callback);
    RUN_TEST(test_inputs_update_button_press_propagates_callback_error);
    RUN_TEST(test_inputs_update_button_invalid_button_id);
    RUN_TEST(test_inputs_update_button_disabled);
    RUN_TEST(test_inputs_update_button_release_invokes_release_callback);
    RUN_TEST(test_inputs_update_button_null_callback_drops_event);

    RUN_TEST(test_inputs_update_knob_invokes_rotation_callback);
    RUN_TEST(test_inputs_update_knob_propagates_callback_error);
    RUN_TEST(test_inputs_update_knob_invalid_knob_id);
    RUN_TEST(test_inputs_update_knob_disabled);
    RUN_TEST(test_inputs_update_knob_no_movement);
    RUN_TEST(test_inputs_update_knob_movement);
    RUN_TEST(test_inputs_update_knob_null_callback_drops_event);

    RUN_TEST(test_inputs_poll_for_long_press_invokes_long_press_callback);
    RUN_TEST(test_inputs_poll_for_long_press_propagates_callback_error);
    RUN_TEST(test_inputs_poll_for_long_press_no_long_press);

    return UNITY_END();
}
