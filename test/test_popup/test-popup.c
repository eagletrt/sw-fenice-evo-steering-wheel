/*!
 * \file test-popup.c
 * \date 2026-06-03
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 *
 * \brief Unit tests for the popup API functionality.
 *
 * \details The popup's auto-hide is a timeout of the real pool, driven by a
 *     fake clock through timeout_api_routine (as the scheduler routine does).
 */

#include "unity.h"
#include "popup-api.h"
#include "screen.h"
#include "timeout-api.h"

struct PopupHandler popup_handler;

bool prv_popup_is_toggle(enum InputsSharedParameterID parameter_id);
void prv_popup_format_value(struct PopupHandler *handler, enum InputsSharedParameterID parameter_id, uint8_t value);

static uint32_t prv_now;
static uint32_t prv_get_tick(void) {
    return prv_now;
}

/* Move the clock and run the timeout pool. */
static void prv_at(uint32_t tick) {
    prv_now = tick;
    TEST_ASSERT_EQUAL(TIMEOUT_RC_OK, timeout_api_routine());
}

void setUp(void) {
    prv_now = 0U;
    TEST_ASSERT_EQUAL(TIMEOUT_RC_OK, timeout_api_init(prv_get_tick));
    TEST_ASSERT_EQUAL(POPUP_RC_OK, popup_api_init(&popup_handler));
}

/*!
 * \defgroup popup_api_init Test cases for popup_api_init function
 * \{
 */

void test_popup_api_init_success(void) {
    enum PopupReturnCode rc = popup_api_init(&popup_handler);
    TEST_ASSERT_EQUAL_MESSAGE(POPUP_RC_OK, rc, "popup_api_init should return POPUP_RC_OK on successful initialization");
    TEST_ASSERT_FALSE_MESSAGE(popup_api_is_active(&popup_handler), "Popup should be inactive after initialization");
    TEST_ASSERT_EQUAL_MESSAGE(INPUTS_SHARED_PARAMETER_ID_POWER, popup_handler.current_parameter, "current_parameter should be initialized to a default value");
    TEST_ASSERT_EQUAL_STRING_MESSAGE("", popup_handler.value_buffer, "value_buffer should be initialized to an empty string");
}

void test_popup_api_init_null_pointer(void) {
    enum PopupReturnCode rc = popup_api_init(NULL);
    TEST_ASSERT_EQUAL(POPUP_RC_ERROR, rc);
}

void test_popup_api_init_fails_when_the_timeout_pool_is_full(void) {
    timeout_id id = 0U;
    while (timeout_api_create(1U, NULL, NULL, &id) == TIMEOUT_RC_OK) {
    }
    TEST_ASSERT_EQUAL(POPUP_RC_ERROR, popup_api_init(&popup_handler));
}

/*! \} */

/*!
 * \defgroup popup_api_show Test cases for popup_api_show function
 * \{
 */

void test_popup_api_show_success(void) {
    enum PopupReturnCode rc = popup_api_show(&popup_handler, INPUTS_SHARED_PARAMETER_ID_POWER, 4);

    TEST_ASSERT_EQUAL_MESSAGE(POPUP_RC_OK, rc, "popup_api_show should return POPUP_RC_OK on successful update");
    TEST_ASSERT_TRUE_MESSAGE(popup_api_is_active(&popup_handler), "Popup should be active after showing");
    TEST_ASSERT_EQUAL_MESSAGE(INPUTS_SHARED_PARAMETER_ID_POWER, popup_handler.current_parameter, "current_parameter should be updated to the provided parameter ID");
    TEST_ASSERT_EQUAL_STRING_MESSAGE("4", popup_handler.value_buffer, "value_buffer should be updated to the formatted parameter value");
    TEST_ASSERT_EQUAL_STRING_MESSAGE("4", popup_handler.labels[0].text, "The first label should display the parameter value");
    TEST_ASSERT_EQUAL_STRING_MESSAGE("POWER", popup_handler.labels[1].text, "The second label should display the parameter name");
}

void test_popup_api_show_same_parameter_keeps_name_box_clean(void) {
    popup_api_show(&popup_handler, INPUTS_SHARED_PARAMETER_ID_POWER, 4);
    popup_handler.boxes[0].updated = false;
    popup_handler.boxes[1].updated = false;

    prv_at(10U);
    popup_api_show(&popup_handler, INPUTS_SHARED_PARAMETER_ID_POWER, 5);

    TEST_ASSERT_TRUE_MESSAGE(popup_handler.boxes[0].updated, "the value box shows the new value");
    TEST_ASSERT_FALSE_MESSAGE(popup_handler.boxes[1].updated, "same parameter: the full-width name box must not be redrawn");
}

void test_popup_api_show_other_parameter_redraws_name_box(void) {
    popup_api_show(&popup_handler, INPUTS_SHARED_PARAMETER_ID_POWER, 4);
    popup_handler.boxes[1].updated = false;

    popup_api_show(&popup_handler, INPUTS_SHARED_PARAMETER_ID_REGEN, 2);

    TEST_ASSERT_TRUE(popup_handler.boxes[1].updated);
}

void test_popup_api_show_after_dismiss_redraws_name_box(void) {
    popup_api_show(&popup_handler, INPUTS_SHARED_PARAMETER_ID_POWER, 4);
    popup_handler.boxes[1].updated = false;
    popup_api_dismiss(&popup_handler);

    popup_api_show(&popup_handler, INPUTS_SHARED_PARAMETER_ID_POWER, 5);

    TEST_ASSERT_TRUE_MESSAGE(popup_handler.boxes[1].updated, "a popup that comes back is drawn whole");
}

void test_popup_api_show_after_timeout_redraws_name_box(void) {
    popup_api_show(&popup_handler, INPUTS_SHARED_PARAMETER_ID_POWER, 4);
    prv_at(STYLE_POPUP_TIMEOUT_MS);
    popup_handler.boxes[1].updated = false;

    popup_api_show(&popup_handler, INPUTS_SHARED_PARAMETER_ID_POWER, 5);

    TEST_ASSERT_TRUE_MESSAGE(popup_handler.boxes[1].updated, "an expired popup that comes back is drawn whole");
    TEST_ASSERT_TRUE(popup_api_is_active(&popup_handler));
}

void test_popup_api_show_null_handler(void) {
    enum PopupReturnCode rc = popup_api_show(NULL, INPUTS_SHARED_PARAMETER_ID_POWER, 4);
    TEST_ASSERT_EQUAL(POPUP_RC_ERROR, rc);
}

void test_popup_api_show_invalid_parameter_id(void) {
    enum PopupReturnCode rc = popup_api_show(&popup_handler, INPUTS_SHARED_PARAMETER_ID_COUNT, 4);
    TEST_ASSERT_EQUAL(POPUP_RC_ERROR, rc);
    TEST_ASSERT_FALSE_MESSAGE(popup_api_is_active(&popup_handler), "A rejected change shows nothing");
}

/*! \} */

/*!
 * \defgroup popup_api_dismiss Test cases for popup_api_dismiss function
 * \{
 */

void test_popup_api_dismiss(void) {
    popup_api_show(&popup_handler, INPUTS_SHARED_PARAMETER_ID_POWER, 4);
    popup_api_dismiss(&popup_handler);
    TEST_ASSERT_FALSE_MESSAGE(popup_api_is_active(&popup_handler), "Popup should be inactive after dismissal");
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

void test_popup_api_is_active_until_the_timeout(void) {
    prv_at(100U);
    popup_api_show(&popup_handler, INPUTS_SHARED_PARAMETER_ID_POWER, 4);

    prv_at(100U + STYLE_POPUP_TIMEOUT_MS - 1U);
    TEST_ASSERT_TRUE_MESSAGE(popup_api_is_active(&popup_handler), "Popup should be active within timeout");

    prv_at(100U + STYLE_POPUP_TIMEOUT_MS);
    TEST_ASSERT_FALSE_MESSAGE(popup_api_is_active(&popup_handler), "Popup should be inactive exactly at the timeout");
}

void test_popup_api_is_active_new_change_restarts_the_timeout(void) {
    popup_api_show(&popup_handler, INPUTS_SHARED_PARAMETER_ID_POWER, 4);
    prv_at(1000U);
    popup_api_show(&popup_handler, INPUTS_SHARED_PARAMETER_ID_POWER, 5);

    prv_at(STYLE_POPUP_TIMEOUT_MS);
    TEST_ASSERT_TRUE_MESSAGE(popup_api_is_active(&popup_handler), "The second change restarted the timeout");

    prv_at(1000U + STYLE_POPUP_TIMEOUT_MS);
    TEST_ASSERT_FALSE(popup_api_is_active(&popup_handler));
}

void test_popup_api_is_active_null_handler(void) {
    TEST_ASSERT_FALSE(popup_api_is_active(NULL));
}

void test_popup_api_is_active_inactive(void) {
    prv_at(5000U);
    TEST_ASSERT_FALSE_MESSAGE(popup_api_is_active(&popup_handler), "Never shown: time alone doesn't activate it");
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
    RUN_TEST(test_popup_api_init_fails_when_the_timeout_pool_is_full);

    RUN_TEST(test_popup_api_show_success);
    RUN_TEST(test_popup_api_show_same_parameter_keeps_name_box_clean);
    RUN_TEST(test_popup_api_show_other_parameter_redraws_name_box);
    RUN_TEST(test_popup_api_show_after_dismiss_redraws_name_box);
    RUN_TEST(test_popup_api_show_after_timeout_redraws_name_box);
    RUN_TEST(test_popup_api_show_null_handler);
    RUN_TEST(test_popup_api_show_invalid_parameter_id);

    RUN_TEST(test_popup_api_dismiss);
    RUN_TEST(test_popup_api_dismiss_null_handler);

    RUN_TEST(test_popup_api_is_active_until_the_timeout);
    RUN_TEST(test_popup_api_is_active_new_change_restarts_the_timeout);
    RUN_TEST(test_popup_api_is_active_null_handler);
    RUN_TEST(test_popup_api_is_active_inactive);

    RUN_TEST(test_prv_popup_is_toggle);

    RUN_TEST(test_prv_popup_format_value_toggle);
    RUN_TEST(test_prv_popup_format_value_numeric);
    RUN_TEST(test_prv_popup_format_value_toggle_off);

    return UNITY_END();
}
