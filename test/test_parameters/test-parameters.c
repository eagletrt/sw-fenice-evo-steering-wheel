/*!
 * \file test_parameters.c
 * \date 2026-04-24
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 *
 * \brief Unit tests for the Parameters API functionality.
 */

#include "parameters-api.h"
#include "inputs-shared.h"
#include "fff.h"
#include "unity.h"

DEFINE_FFF_GLOBALS;

FAKE_VALUE_FUNC(bool, on_change_callback, enum InputsSharedParameterID, uint8_t);

extern struct ParametersHandler parameters_handler;

void setUp(void) {
    RESET_FAKE(on_change_callback);
    FFF_RESET_HISTORY();
    on_change_callback_fake.return_val = true;
    parameters_api_init(on_change_callback);
}

/*!
 * \defgroup parameters_api_init Tests for parameters_api_init function
 * \{
 */

void test_parameters_init_stores_callback(void) {
    enum ParametersReturnCode rc = parameters_api_init(on_change_callback);
    TEST_ASSERT_EQUAL_MESSAGE(PARAMETERS_RC_OK, rc, "Expected success return code from parameters_api_init");
    TEST_ASSERT_EQUAL_MESSAGE(on_change_callback, parameters_handler.on_change, "on_change callback was not stored correctly");
}

void test_parameters_init_rejects_null_callback(void) {
    enum ParametersReturnCode rc = parameters_api_init(NULL);
    TEST_ASSERT_EQUAL_MESSAGE(PARAMETERS_RC_ERROR, rc, "Expected error return code when passing NULL callback");
}

void test_parameters_init_resets_values_to_zero(void) {
    uint8_t expected[INPUTS_SHARED_PARAMETER_ID_COUNT] = { 0 };
    parameters_handler.values[INPUTS_SHARED_PARAMETER_ID_POWER] = 5;
    parameters_handler.values[INPUTS_SHARED_PARAMETER_ID_TELEMETRY_LOG] = 1;
    parameters_api_init(on_change_callback);
    TEST_ASSERT_EQUAL_UINT8_ARRAY_MESSAGE(expected, parameters_handler.values, INPUTS_SHARED_PARAMETER_ID_COUNT, "Parameter values were not reset to zero on init");
}

void test_parameters_init_does_not_fire_callback(void) {
    parameters_api_init(on_change_callback);
    TEST_ASSERT_EQUAL_MESSAGE(0, on_change_callback_fake.call_count, "on_change callback must not fire during init");
}

/*! \} */

/*!
 * \defgroup parameters_api_get Tests for parameters_api_get function
 * \{
 */

void test_parameters_get_returns_current_value(void) {
    parameters_handler.values[INPUTS_SHARED_PARAMETER_ID_POWER] = 7;
    TEST_ASSERT_EQUAL(7, parameters_api_get(INPUTS_SHARED_PARAMETER_ID_POWER));
}

void test_parameters_get_returns_zero_for_invalid_id(void) {
    TEST_ASSERT_EQUAL(0U, parameters_api_get(INPUTS_SHARED_PARAMETER_ID_COUNT));
}

/*! \} */

/*!
 * \defgroup parameters_api_set Tests for parameters_api_set function
 * \{
 */

void test_parameters_set_updates_value_and_fires_callback(void) {
    enum ParametersReturnCode rc = parameters_api_set(INPUTS_SHARED_PARAMETER_ID_POWER, 5);
    TEST_ASSERT_EQUAL_MESSAGE(PARAMETERS_RC_OK, rc, "Expected success return code from parameters_api_set");
    TEST_ASSERT_EQUAL_MESSAGE(5, parameters_handler.values[INPUTS_SHARED_PARAMETER_ID_POWER], "Parameter value was not updated correctly");
    TEST_ASSERT_EQUAL_MESSAGE(1, on_change_callback_fake.call_count, "Expected on_change callback to be fired once");
    TEST_ASSERT_EQUAL_MESSAGE(INPUTS_SHARED_PARAMETER_ID_POWER, on_change_callback_fake.arg0_val, "Expected on_change callback to be called with correct parameter_id");
    TEST_ASSERT_EQUAL_MESSAGE(5, on_change_callback_fake.arg1_val, "Expected on_change callback to be called with correct value");
}

void test_parameters_set_does_not_fire_callback_when_value_unchanged(void) {
    parameters_handler.values[INPUTS_SHARED_PARAMETER_ID_POWER] = 5;
    on_change_callback_fake.return_val = true;
    enum ParametersReturnCode rc = parameters_api_set(INPUTS_SHARED_PARAMETER_ID_POWER, 5);
    TEST_ASSERT_EQUAL_MESSAGE(PARAMETERS_RC_OK, rc, "Expected success return code even when setting to the same value");
    TEST_ASSERT_EQUAL_MESSAGE(0, on_change_callback_fake.call_count, "on_change must not fire when value does not change");
}

void test_parameters_set_clamps_numeric_above_max(void) {
    enum ParametersReturnCode rc = parameters_api_set(INPUTS_SHARED_PARAMETER_ID_POWER, 42);
    TEST_ASSERT_EQUAL_MESSAGE(PARAMETERS_RC_OK, rc, "Expected success return code from parameters_api_set even when value is above max");
    TEST_ASSERT_EQUAL_MESSAGE(INPUTS_SHARED_PARAMETER_NUMERIC_MAX, parameters_handler.values[INPUTS_SHARED_PARAMETER_ID_POWER], "Expected parameter value to be clamped to INPUTS_SHARED_PARAMETER_NUMERIC_MAX");
    TEST_ASSERT_EQUAL_MESSAGE(INPUTS_SHARED_PARAMETER_NUMERIC_MAX, on_change_callback_fake.arg1_val, "Expected on_change callback to be called with clamped value");
}

void test_parameters_set_clamps_toggle_above_one(void) {
    enum ParametersReturnCode rc = parameters_api_set(INPUTS_SHARED_PARAMETER_ID_TELEMETRY_LOG, 17);
    TEST_ASSERT_EQUAL_MESSAGE(PARAMETERS_RC_OK, rc, "Expected success return code from parameters_api_set even when value is above max");
    TEST_ASSERT_EQUAL_MESSAGE(1U, parameters_handler.values[INPUTS_SHARED_PARAMETER_ID_TELEMETRY_LOG], "Expected toggle parameter value to be clamped to 1");
    TEST_ASSERT_EQUAL_MESSAGE(1U, on_change_callback_fake.arg1_val, "Expected on_change callback to be called with clamped value");
}

void test_parameters_set_rejects_invalid_id(void) {
    enum ParametersReturnCode rc = parameters_api_set(INPUTS_SHARED_PARAMETER_ID_COUNT, 0);
    TEST_ASSERT_EQUAL_MESSAGE(PARAMETERS_RC_ERROR, rc, "Expected error return code when setting invalid parameter ID");
    TEST_ASSERT_EQUAL_MESSAGE(0, on_change_callback_fake.call_count, "on_change must not fire when setting invalid parameter ID");
}

void test_parameters_set_propagates_callback_failure(void) {
    on_change_callback_fake.return_val = false;
    enum ParametersReturnCode rc = parameters_api_set(INPUTS_SHARED_PARAMETER_ID_POWER, 3);
    TEST_ASSERT_EQUAL_MESSAGE(PARAMETERS_RC_ERROR, rc, "Expected error when on_change returns false");
    TEST_ASSERT_EQUAL_MESSAGE(3, parameters_handler.values[INPUTS_SHARED_PARAMETER_ID_POWER], "Value should still update locally even if callback fails");
}

/*! \} */

/*!
 * \defgroup parameters_api_toggle Tests for parameters_api_toggle
 * \{
 */

void test_parameters_toggle_0_to_1_and_back(void) {
    TEST_ASSERT_EQUAL(PARAMETERS_RC_OK, parameters_api_toggle(INPUTS_SHARED_PARAMETER_ID_TELEMETRY_LOG));
    TEST_ASSERT_EQUAL(1U, parameters_api_get(INPUTS_SHARED_PARAMETER_ID_TELEMETRY_LOG));
    TEST_ASSERT_EQUAL(PARAMETERS_RC_OK, parameters_api_toggle(INPUTS_SHARED_PARAMETER_ID_TELEMETRY_LOG));
    TEST_ASSERT_EQUAL(0U, parameters_api_get(INPUTS_SHARED_PARAMETER_ID_TELEMETRY_LOG));
    TEST_ASSERT_EQUAL(2, on_change_callback_fake.call_count);
    TEST_ASSERT_EQUAL(INPUTS_SHARED_PARAMETER_ID_TELEMETRY_LOG, on_change_callback_fake.arg0_val);
    TEST_ASSERT_EQUAL(0U, on_change_callback_fake.arg1_val);
}

void test_parameters_toggle_rejects_invalid_id(void) {
    TEST_ASSERT_EQUAL(PARAMETERS_RC_ERROR, parameters_api_toggle(INPUTS_SHARED_PARAMETER_ID_COUNT));
    TEST_ASSERT_EQUAL(0, on_change_callback_fake.call_count);
}

void test_parameters_toggle_propagates_callback_failure(void) {
    on_change_callback_fake.return_val = false;
    TEST_ASSERT_EQUAL(PARAMETERS_RC_ERROR, parameters_api_toggle(INPUTS_SHARED_PARAMETER_ID_LAUNCH_CONTROL));
}

/*! \} */

/*!
 * \defgroup parameters_api_step Tests for parameters_api_step
 * \{
 */

void test_parameters_step_moves_by_delta(void) {
    TEST_ASSERT_EQUAL(PARAMETERS_RC_OK, parameters_api_step(INPUTS_SHARED_PARAMETER_ID_POWER, 3));
    TEST_ASSERT_EQUAL(3U, parameters_api_get(INPUTS_SHARED_PARAMETER_ID_POWER));
    TEST_ASSERT_EQUAL(PARAMETERS_RC_OK, parameters_api_step(INPUTS_SHARED_PARAMETER_ID_POWER, -1));
    TEST_ASSERT_EQUAL(2U, parameters_api_get(INPUTS_SHARED_PARAMETER_ID_POWER));
}

void test_parameters_step_clamps_below_zero(void) {
    TEST_ASSERT_EQUAL(PARAMETERS_RC_OK, parameters_api_step(INPUTS_SHARED_PARAMETER_ID_REGEN, -5));
    TEST_ASSERT_EQUAL(0U, parameters_api_get(INPUTS_SHARED_PARAMETER_ID_REGEN));
    TEST_ASSERT_EQUAL_MESSAGE(0, on_change_callback_fake.call_count, "Clamped to the same value: no change");
}

void test_parameters_step_clamps_above_max(void) {
    TEST_ASSERT_EQUAL(PARAMETERS_RC_OK, parameters_api_step(INPUTS_SHARED_PARAMETER_ID_TORQUE_VECTORING, 100));
    TEST_ASSERT_EQUAL(INPUTS_SHARED_PARAMETER_NUMERIC_MAX, parameters_api_get(INPUTS_SHARED_PARAMETER_ID_TORQUE_VECTORING));
}

void test_parameters_step_clamps_toggles_to_one(void) {
    TEST_ASSERT_EQUAL(PARAMETERS_RC_OK, parameters_api_step(INPUTS_SHARED_PARAMETER_ID_LAUNCH_CONTROL, 4));
    TEST_ASSERT_EQUAL(1U, parameters_api_get(INPUTS_SHARED_PARAMETER_ID_LAUNCH_CONTROL));
}

void test_parameters_step_rejects_invalid_id(void) {
    TEST_ASSERT_EQUAL(PARAMETERS_RC_ERROR, parameters_api_step(INPUTS_SHARED_PARAMETER_ID_COUNT, 1));
}

void test_parameters_step_propagates_callback_failure(void) {
    on_change_callback_fake.return_val = false;
    TEST_ASSERT_EQUAL(PARAMETERS_RC_ERROR, parameters_api_step(INPUTS_SHARED_PARAMETER_ID_POWER, 1));
}

/*! \} */

/*!
 * \defgroup parameters_api_hold Tests for parameters_api_hold and parameters_api_release_source
 * \{
 */

/* Two inputs that can hold PTT, like the top paddles. */
#define TEST_PARAMETERS_SOURCE_A (5U)
#define TEST_PARAMETERS_SOURCE_B (7U)

void test_parameters_hold_sets_one_and_let_go_sets_zero(void) {
    TEST_ASSERT_EQUAL(PARAMETERS_RC_OK, parameters_api_hold(INPUTS_SHARED_PARAMETER_ID_TS_ON, TEST_PARAMETERS_SOURCE_A, true));
    TEST_ASSERT_EQUAL(1U, parameters_api_get(INPUTS_SHARED_PARAMETER_ID_TS_ON));
    TEST_ASSERT_EQUAL(PARAMETERS_RC_OK, parameters_api_hold(INPUTS_SHARED_PARAMETER_ID_TS_ON, TEST_PARAMETERS_SOURCE_A, false));
    TEST_ASSERT_EQUAL(0U, parameters_api_get(INPUTS_SHARED_PARAMETER_ID_TS_ON));
    TEST_ASSERT_EQUAL(2, on_change_callback_fake.call_count);
}

void test_parameters_hold_stays_while_another_source_holds(void) {
    TEST_ASSERT_EQUAL(PARAMETERS_RC_OK, parameters_api_hold(INPUTS_SHARED_PARAMETER_ID_PTT, TEST_PARAMETERS_SOURCE_A, true));
    TEST_ASSERT_EQUAL(PARAMETERS_RC_OK, parameters_api_hold(INPUTS_SHARED_PARAMETER_ID_PTT, TEST_PARAMETERS_SOURCE_B, true));
    TEST_ASSERT_EQUAL_MESSAGE(1, on_change_callback_fake.call_count, "A second holder changes nothing");

    TEST_ASSERT_EQUAL(PARAMETERS_RC_OK, parameters_api_hold(INPUTS_SHARED_PARAMETER_ID_PTT, TEST_PARAMETERS_SOURCE_A, false));
    TEST_ASSERT_EQUAL_MESSAGE(1U, parameters_api_get(INPUTS_SHARED_PARAMETER_ID_PTT), "Still held by the other source");

    TEST_ASSERT_EQUAL(PARAMETERS_RC_OK, parameters_api_hold(INPUTS_SHARED_PARAMETER_ID_PTT, TEST_PARAMETERS_SOURCE_B, false));
    TEST_ASSERT_EQUAL(0U, parameters_api_get(INPUTS_SHARED_PARAMETER_ID_PTT));
    TEST_ASSERT_EQUAL(2, on_change_callback_fake.call_count);
}

void test_parameters_hold_rejects_invalid_arguments(void) {
    TEST_ASSERT_EQUAL(PARAMETERS_RC_ERROR, parameters_api_hold(INPUTS_SHARED_PARAMETER_ID_COUNT, TEST_PARAMETERS_SOURCE_A, true));
    TEST_ASSERT_EQUAL(PARAMETERS_RC_ERROR, parameters_api_hold(INPUTS_SHARED_PARAMETER_ID_PTT, PARAMETERS_HOLD_SOURCE_COUNT, true));
    TEST_ASSERT_EQUAL(0, on_change_callback_fake.call_count);
}

void test_parameters_hold_propagates_callback_failure(void) {
    on_change_callback_fake.return_val = false;
    TEST_ASSERT_EQUAL(PARAMETERS_RC_ERROR, parameters_api_hold(INPUTS_SHARED_PARAMETER_ID_DRIVER_OK, TEST_PARAMETERS_SOURCE_A, true));
}

void test_parameters_release_source_lets_go_of_everything_it_holds(void) {
    TEST_ASSERT_EQUAL(PARAMETERS_RC_OK, parameters_api_hold(INPUTS_SHARED_PARAMETER_ID_DRIVER_OK, TEST_PARAMETERS_SOURCE_A, true));
    TEST_ASSERT_EQUAL(PARAMETERS_RC_OK, parameters_api_hold(INPUTS_SHARED_PARAMETER_ID_TS_ON, TEST_PARAMETERS_SOURCE_A, true));
    TEST_ASSERT_EQUAL(PARAMETERS_RC_OK, parameters_api_hold(INPUTS_SHARED_PARAMETER_ID_PTT, TEST_PARAMETERS_SOURCE_B, true));

    TEST_ASSERT_EQUAL(PARAMETERS_RC_OK, parameters_api_release_source(TEST_PARAMETERS_SOURCE_A));

    TEST_ASSERT_EQUAL(0U, parameters_api_get(INPUTS_SHARED_PARAMETER_ID_DRIVER_OK));
    TEST_ASSERT_EQUAL(0U, parameters_api_get(INPUTS_SHARED_PARAMETER_ID_TS_ON));
    TEST_ASSERT_EQUAL_MESSAGE(1U, parameters_api_get(INPUTS_SHARED_PARAMETER_ID_PTT), "Another source's hold stays");
}

void test_parameters_release_source_holding_nothing_changes_nothing(void) {
    TEST_ASSERT_EQUAL(PARAMETERS_RC_OK, parameters_api_release_source(TEST_PARAMETERS_SOURCE_B));
    TEST_ASSERT_EQUAL(0, on_change_callback_fake.call_count);
}

void test_parameters_release_source_rejects_invalid_source(void) {
    TEST_ASSERT_EQUAL(PARAMETERS_RC_ERROR, parameters_api_release_source(PARAMETERS_HOLD_SOURCE_COUNT));
}

/*! \} */

/*!
 * \defgroup parameters_api_is_shared Tests for parameters_api_is_shared function
 * \{
 */

void test_parameters_is_shared_for_dashboard_parameters(void) {
    TEST_ASSERT_TRUE(parameters_api_is_shared(INPUTS_SHARED_PARAMETER_ID_POWER));
    TEST_ASSERT_TRUE(parameters_api_is_shared(INPUTS_SHARED_PARAMETER_ID_REGEN));
    TEST_ASSERT_TRUE(parameters_api_is_shared(INPUTS_SHARED_PARAMETER_ID_TORQUE_VECTORING));
    TEST_ASSERT_TRUE(parameters_api_is_shared(INPUTS_SHARED_PARAMETER_ID_TELEMETRY_LOG));
    TEST_ASSERT_TRUE(parameters_api_is_shared(INPUTS_SHARED_PARAMETER_ID_LAUNCH_CONTROL));
}

void test_parameters_is_not_shared_for_momentary_parameters(void) {
    TEST_ASSERT_FALSE_MESSAGE(parameters_api_is_shared(INPUTS_SHARED_PARAMETER_ID_PTT), "PTT must not raise a popup");
    TEST_ASSERT_FALSE_MESSAGE(parameters_api_is_shared(INPUTS_SHARED_PARAMETER_ID_TS_ON), "TS-on must not raise a popup");
    TEST_ASSERT_FALSE_MESSAGE(parameters_api_is_shared(INPUTS_SHARED_PARAMETER_ID_DRIVER_OK), "OK must not raise a popup");
    TEST_ASSERT_FALSE_MESSAGE(parameters_api_is_shared(INPUTS_SHARED_PARAMETER_ID_DRIVER_REPEAT), "REPEAT must not raise a popup");
}

void test_parameters_is_shared_rejects_invalid_id(void) {
    TEST_ASSERT_FALSE(parameters_api_is_shared(INPUTS_SHARED_PARAMETER_ID_COUNT));
}

/*! \} */

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_parameters_init_stores_callback);
    RUN_TEST(test_parameters_init_rejects_null_callback);
    RUN_TEST(test_parameters_init_resets_values_to_zero);
    RUN_TEST(test_parameters_init_does_not_fire_callback);

    RUN_TEST(test_parameters_get_returns_current_value);
    RUN_TEST(test_parameters_get_returns_zero_for_invalid_id);

    RUN_TEST(test_parameters_set_updates_value_and_fires_callback);
    RUN_TEST(test_parameters_set_does_not_fire_callback_when_value_unchanged);
    RUN_TEST(test_parameters_set_clamps_numeric_above_max);
    RUN_TEST(test_parameters_set_clamps_toggle_above_one);
    RUN_TEST(test_parameters_set_rejects_invalid_id);
    RUN_TEST(test_parameters_set_propagates_callback_failure);

    RUN_TEST(test_parameters_toggle_0_to_1_and_back);
    RUN_TEST(test_parameters_toggle_rejects_invalid_id);
    RUN_TEST(test_parameters_toggle_propagates_callback_failure);

    RUN_TEST(test_parameters_step_moves_by_delta);
    RUN_TEST(test_parameters_step_clamps_below_zero);
    RUN_TEST(test_parameters_step_clamps_above_max);
    RUN_TEST(test_parameters_step_clamps_toggles_to_one);
    RUN_TEST(test_parameters_step_rejects_invalid_id);
    RUN_TEST(test_parameters_step_propagates_callback_failure);

    RUN_TEST(test_parameters_hold_sets_one_and_let_go_sets_zero);
    RUN_TEST(test_parameters_hold_stays_while_another_source_holds);
    RUN_TEST(test_parameters_hold_rejects_invalid_arguments);
    RUN_TEST(test_parameters_hold_propagates_callback_failure);
    RUN_TEST(test_parameters_release_source_lets_go_of_everything_it_holds);
    RUN_TEST(test_parameters_release_source_holding_nothing_changes_nothing);
    RUN_TEST(test_parameters_release_source_rejects_invalid_source);

    RUN_TEST(test_parameters_is_shared_for_dashboard_parameters);
    RUN_TEST(test_parameters_is_not_shared_for_momentary_parameters);
    RUN_TEST(test_parameters_is_shared_rejects_invalid_id);

    return UNITY_END();
}
