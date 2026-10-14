/*!
 * \file test-screen.c
 * \date 2026-10-14
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 *
 * \brief Unit tests for the popup lifecycle driven through the screen API.
 *
 * \details screen_api_on_parameter_change stamps the popup and
 *     screen_api_update mounts / unmounts it on the raster. The popup
 *     timeout must start from the parameter change, not from the last
 *     render: renders can be late or not running at all.
 */

#include "unity.h"
#include "fff.h"
#include "screen-api.h"
#include "raster.h"
#include <string.h>

extern struct ScreenHandler screen_handler;

DEFINE_FFF_GLOBALS;

FAKE_VALUE_FUNC(enum RasterReturnCode, fake_draw_rectangle, uint16_t, uint16_t, uint16_t, uint16_t, struct Color);

void setUp(void) {
    RESET_FAKE(fake_draw_rectangle);
    FFF_RESET_HISTORY();
    fake_draw_rectangle_fake.return_val = RASTER_RC_OK;
    memset(&screen_handler, 0, sizeof(screen_handler));
    screen_api_init(fake_draw_rectangle);
}

/*!
 * \defgroup screen_api_on_parameter_change Popup timing tests
 * \{
 */

void test_screen_popup_mounts_on_next_update(void) {
    TEST_ASSERT_EQUAL(SCREEN_RC_OK, screen_api_on_parameter_change(INPUTS_SHARED_PARAMETER_ID_POWER, 5U, 100U));
    screen_api_update(110U);
    TEST_ASSERT_TRUE_MESSAGE(screen_handler.popup_visible, "Popup must be mounted on the first update after a change");
    TEST_ASSERT_EQUAL_STRING("5", screen_handler.popup.value_buffer);
}

void test_screen_popup_timeout_starts_at_change_not_at_last_render(void) {
    /* Last render long before the change: the old code stamped this tick. */
    screen_api_update(0U);
    screen_api_on_parameter_change(INPUTS_SHARED_PARAMETER_ID_POWER, 5U, 5000U);

    screen_api_update(5001U);
    TEST_ASSERT_TRUE_MESSAGE(screen_handler.popup_visible, "Popup must not expire because the previous render was old");

    screen_api_update(5000U + SCREEN_POPUP_TIMEOUT_MS - 1U);
    TEST_ASSERT_TRUE_MESSAGE(screen_handler.popup_visible, "Popup must stay up for the whole timeout after the change");

    screen_api_update(5000U + SCREEN_POPUP_TIMEOUT_MS);
    TEST_ASSERT_FALSE_MESSAGE(screen_handler.popup_visible, "Popup must go away exactly at the timeout");
}

void test_screen_popup_new_change_extends_timeout(void) {
    screen_api_on_parameter_change(INPUTS_SHARED_PARAMETER_ID_POWER, 5U, 1000U);
    screen_api_update(1000U);
    screen_api_on_parameter_change(INPUTS_SHARED_PARAMETER_ID_POWER, 6U, 2000U);

    screen_api_update(1000U + SCREEN_POPUP_TIMEOUT_MS);
    TEST_ASSERT_TRUE_MESSAGE(screen_handler.popup_visible, "A later change must restart the timeout");
    TEST_ASSERT_EQUAL_STRING("6", screen_handler.popup.value_buffer);

    screen_api_update(2000U + SCREEN_POPUP_TIMEOUT_MS);
    TEST_ASSERT_FALSE(screen_handler.popup_visible);
}

void test_screen_popup_dismiss_remounts_dashboard_dirty(void) {
    screen_api_on_parameter_change(INPUTS_SHARED_PARAMETER_ID_REGEN, 3U, 0U);
    screen_api_update(0U);
    screen_api_update(SCREEN_POPUP_TIMEOUT_MS);

    TEST_ASSERT_FALSE(screen_handler.popup_visible);
    TEST_ASSERT_EQUAL_PTR_MESSAGE(screen_handler.dashboard.boxes, screen_handler.raster.interface, "Dashboard must be mounted again after the popup");
}

void test_screen_popup_rejects_invalid_parameter(void) {
    TEST_ASSERT_EQUAL(SCREEN_RC_ERROR, screen_api_on_parameter_change(INPUTS_SHARED_PARAMETER_ID_COUNT, 1U, 0U));
    screen_api_update(1U);
    TEST_ASSERT_FALSE(screen_handler.popup_visible);
}

/*! \} */

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_screen_popup_mounts_on_next_update);
    RUN_TEST(test_screen_popup_timeout_starts_at_change_not_at_last_render);
    RUN_TEST(test_screen_popup_new_change_extends_timeout);
    RUN_TEST(test_screen_popup_dismiss_remounts_dashboard_dirty);
    RUN_TEST(test_screen_popup_rejects_invalid_parameter);

    return UNITY_END();
}
