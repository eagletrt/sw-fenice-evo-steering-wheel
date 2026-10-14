/*!
 * \file test-popup.c
 * \date 2026-06-03
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 *
 * \brief Unit tests for the popup API functionality.
 */

#include "unity.h"
#include "popup-api.h"

struct PopupHandler popup_handler;

bool prv_popup_is_toggle(enum InputsSharedParameterID parameter_id);
void prv_popup_format_value(struct PopupHandler *handler, enum InputsSharedParameterID parameter_id, uint8_t value);

void setUp(void) {
    popup_api_init(&popup_handler);
}

/*!
 * \defgroup popup_api_init Test cases for popup_api_init function
 * \{
 */

void test_popup_api_init_success(void) {
    enum PopupReturnCode rc = popup_api_init(&popup_handler);
    TEST_ASSERT_EQUAL_MESSAGE(POPUP_RC_OK, rc, "popup_api_init should return POPUP_RC_OK on successful initialization");
    TEST_ASSERT_FALSE_MESSAGE(popup_handler.active, "Popup should be inactive after initialization");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, popup_handler.last_event_tick, "last_event_tick should be initialized to 0");
    TEST_ASSERT_EQUAL_MESSAGE(INPUTS_SHARED_PARAMETER_ID_POWER, popup_handler.current_parameter, "current_parameter should be initialized to a default value");
    TEST_ASSERT_EQUAL_STRING_MESSAGE("", popup_handler.value_buffer, "value_buffer should be initialized to an empty string");
}

void test_popup_api_init_null_pointer(void) {
    enum PopupReturnCode rc = popup_api_init(NULL);
    TEST_ASSERT_EQUAL(POPUP_RC_ERROR, rc);
}

/*! \} */

/*!
 * \defgroup popup_api_show Test cases for popup_api_show function
 * \{
 */

void test_popup_api_show_success(void) {
    enum PopupReturnCode rc = popup_api_show(&popup_handler, INPUTS_SHARED_PARAMETER_ID_POWER, 4, 50);

    TEST_ASSERT_EQUAL_MESSAGE(POPUP_RC_OK, rc, "popup_api_show should return POPUP_RC_OK on successful update");
    TEST_ASSERT_TRUE_MESSAGE(popup_handler.active, "Popup should be active after showing");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(50, popup_handler.last_event_tick, "last_event_tick should be updated to the provided tick");
    TEST_ASSERT_EQUAL_MESSAGE(INPUTS_SHARED_PARAMETER_ID_POWER, popup_handler.current_parameter, "current_parameter should be updated to the provided parameter ID");
    TEST_ASSERT_EQUAL_STRING_MESSAGE("4", popup_handler.value_buffer, "value_buffer should be updated to the formatted parameter value");
    TEST_ASSERT_EQUAL_STRING_MESSAGE("4", popup_handler.labels[0].text, "The first label should display the parameter value");
    TEST_ASSERT_EQUAL_STRING_MESSAGE("POWER", popup_handler.labels[1].text, "The second label should display the parameter name");
    
}

void test_popup_api_show_null_handler(void) {
    enum PopupReturnCode rc = popup_api_show(NULL, INPUTS_SHARED_PARAMETER_ID_POWER, 4, 50);
    TEST_ASSERT_EQUAL(POPUP_RC_ERROR, rc);
}

void test_popup_api_show_invalid_parameter_id(void) {
    enum PopupReturnCode rc = popup_api_show(&popup_handler, INPUTS_SHARED_PARAMETER_ID_COUNT, 4, 50);
    TEST_ASSERT_EQUAL(POPUP_RC_ERROR, rc);
}

/*! \} */

/*!
 * \defgroup popup_api_dismiss Test cases for popup_api_dismiss function
 * \{
 */

void test_popup_api_dismiss(void) {
    popup_handler.active = true; // Simulate an active popup
    popup_api_dismiss(&popup_handler);
    TEST_ASSERT_FALSE_MESSAGE(popup_handler.active, "Popup should be inactive after dismissal");
}

void test_popup_api_dismiss_null_handler(void) {
    // Should not crash or modify any state when a null handler is provided
    popup_api_dismiss(NULL);
    TEST_PASS();
}

/*! \} */

/*!
 * \defgroup popup_api_is_active Test cases for popup_api_is_active function
 * \{
 */

void test_popup_api_is_active(void) {
    popup_handler.active = true;
    popup_handler.last_event_tick = 100;

    TEST_ASSERT_TRUE_MESSAGE(popup_api_is_active(&popup_handler, 150), "Popup should be active within timeout");

    TEST_ASSERT_FALSE_MESSAGE(popup_api_is_active(&popup_handler, 2651), "Popup should be inactive after timeout");
    TEST_ASSERT_FALSE_MESSAGE(popup_handler.active, "Popup handler should be set to inactive after timeout");
}

void test_popup_api_is_active_null_handler(void) {
    TEST_ASSERT_FALSE(popup_api_is_active(NULL, 100));
}

void test_popup_api_is_active_inactive(void) {
    popup_handler.active = false;
    TEST_ASSERT_FALSE(popup_api_is_active(&popup_handler, 100));
}

/*! \} */

/*!
 * \defgroup prv_popup_is_toggle Test cases for prv_popup_is_toggle function
 * \{
 */

void test_prv_popup_is_toggle(void) {
TEST_ASSERT_TRUE_MESSAGE(prv_popup_is_toggle(INPUTS_SHARED_PARAMETER_ID_TELEMETRY_LOG), "INPUTS_SHARED_PARAMETER_ID_TELEMETRY_LOG should be toggle");
    TEST_ASSERT_TRUE_MESSAGE(prv_popup_is_toggle(INPUTS_SHARED_PARAMETER_ID_LAUNCH_CONTROL), "INPUTS_SHARED_PARAMETER_ID_LAUNCH_CONTROL should be toggle");
}

/*! \} */

/*!
 * \defgroup prv_popup_format_label Test cases for prv_popup_format_label function
 * \{
 */

void test_prv_popup_format_value_toggle(void) {
    prv_popup_format_value(&popup_handler, INPUTS_SHARED_PARAMETER_ID_LAUNCH_CONTROL, 1);
    TEST_ASSERT_EQUAL_STRING("ON", popup_handler.value_buffer);
}

void test_prv_popup_format_value_numeric(void) {
    prv_popup_format_value(&popup_handler, INPUTS_SHARED_PARAMETER_ID_POWER, 4);
    TEST_ASSERT_EQUAL_STRING("4", popup_handler.value_buffer);
}

void test_prv_popup_format_value_toggle_off(void) {
    prv_popup_format_value(&popup_handler, INPUTS_SHARED_PARAMETER_ID_LAUNCH_CONTROL, 0);
    TEST_ASSERT_EQUAL_STRING("OFF", popup_handler.value_buffer);
}

/*! \} */

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_popup_api_init_success);
    RUN_TEST(test_popup_api_init_null_pointer);

    RUN_TEST(test_popup_api_show_success);
    RUN_TEST(test_popup_api_show_null_handler);
    RUN_TEST(test_popup_api_show_invalid_parameter_id);

    RUN_TEST(test_popup_api_dismiss);
    RUN_TEST(test_popup_api_dismiss_null_handler);

    RUN_TEST(test_popup_api_is_active);
    RUN_TEST(test_popup_api_is_active_null_handler);
    RUN_TEST(test_popup_api_is_active_inactive);

    RUN_TEST(test_prv_popup_is_toggle);

    RUN_TEST(test_prv_popup_format_value_toggle);
    RUN_TEST(test_prv_popup_format_value_numeric);
    RUN_TEST(test_prv_popup_format_value_toggle_off);

    return UNITY_END();
}
