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
#include "splash.h"
#include <string.h>

extern struct ScreenHandler screen_handler;

DEFINE_FFF_GLOBALS;

FAKE_VALUE_FUNC(enum RasterReturnCode, fake_draw_rectangle, uint16_t, uint16_t, uint16_t, uint16_t, struct Color);
FAKE_VALUE_FUNC(enum RasterReturnCode, fake_draw_mask, uint16_t, uint16_t, uint16_t, uint16_t, const uint8_t *, struct Color);

static uint8_t mask_scratch[64U * 1024U];

static void prv_enable_accelerators(void) {
    const struct ScreenAccelerators accelerators = {
        .draw_mask = fake_draw_mask,
        .mask_buffer = mask_scratch,
        .mask_size = sizeof(mask_scratch),
    };
    TEST_ASSERT_EQUAL(SCREEN_RC_OK, screen_api_set_accelerators(&accelerators));
}

/* Render whatever is pending and forget the fake history. */
static void prv_settle(uint32_t tick) {
    screen_api_update(tick);
    RESET_FAKE(fake_draw_rectangle);
    RESET_FAKE(fake_draw_mask);
    fake_draw_rectangle_fake.return_val = RASTER_RC_OK;
    fake_draw_mask_fake.return_val = RASTER_RC_OK;
}

/* Logo blends recorded by the fake mask callback since the last reset. */
static unsigned prv_logo_blends(void) {
    unsigned count = 0U;
    for (unsigned i = 0U; i < fake_draw_mask_fake.call_count && i < FFF_ARG_HISTORY_LEN; i++) {
        if (fake_draw_mask_fake.arg0_history[i] == SPLASH_LOGO_X && fake_draw_mask_fake.arg2_history[i] == SPLASH_LOGO_WIDTH) {
            count++;
        }
    }
    return count;
}

void setUp(void) {
    RESET_FAKE(fake_draw_rectangle);
    RESET_FAKE(fake_draw_mask);
    FFF_RESET_HISTORY();
    fake_draw_rectangle_fake.return_val = RASTER_RC_OK;
    fake_draw_mask_fake.return_val = RASTER_RC_OK;
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
    TEST_ASSERT_TRUE_MESSAGE((screen_handler.mounted == SCREEN_INTERFACE_POPUP), "Popup must be mounted on the first update after a change");
    TEST_ASSERT_EQUAL_STRING("5", screen_handler.popup.value_buffer);
}

void test_screen_popup_timeout_starts_at_change_not_at_last_render(void) {
    /* Last render long before the change: the old code stamped this tick. */
    screen_api_update(0U);
    screen_api_on_parameter_change(INPUTS_SHARED_PARAMETER_ID_POWER, 5U, 5000U);

    screen_api_update(5001U);
    TEST_ASSERT_TRUE_MESSAGE((screen_handler.mounted == SCREEN_INTERFACE_POPUP), "Popup must not expire because the previous render was old");

    screen_api_update(5000U + SCREEN_POPUP_TIMEOUT_MS - 1U);
    TEST_ASSERT_TRUE_MESSAGE((screen_handler.mounted == SCREEN_INTERFACE_POPUP), "Popup must stay up for the whole timeout after the change");

    screen_api_update(5000U + SCREEN_POPUP_TIMEOUT_MS);
    TEST_ASSERT_FALSE_MESSAGE((screen_handler.mounted == SCREEN_INTERFACE_POPUP), "Popup must go away exactly at the timeout");
}

void test_screen_popup_survives_change_stamped_after_render_tick(void) {
    /* Field bug: the change was stamped with a fresh clock read (105) while
     * the render in the same pass still used the tick read earlier (100). */
    screen_api_on_parameter_change(INPUTS_SHARED_PARAMETER_ID_POWER, 5U, 105U);
    screen_api_update(100U);
    TEST_ASSERT_TRUE_MESSAGE((screen_handler.mounted == SCREEN_INTERFACE_POPUP), "A change stamped after the render tick must still show the popup");
}

void test_screen_popup_new_change_extends_timeout(void) {
    screen_api_on_parameter_change(INPUTS_SHARED_PARAMETER_ID_POWER, 5U, 1000U);
    screen_api_update(1000U);
    screen_api_on_parameter_change(INPUTS_SHARED_PARAMETER_ID_POWER, 6U, 2000U);

    screen_api_update(1000U + SCREEN_POPUP_TIMEOUT_MS);
    TEST_ASSERT_TRUE_MESSAGE((screen_handler.mounted == SCREEN_INTERFACE_POPUP), "A later change must restart the timeout");
    TEST_ASSERT_EQUAL_STRING("6", screen_handler.popup.value_buffer);

    screen_api_update(2000U + SCREEN_POPUP_TIMEOUT_MS);
    TEST_ASSERT_FALSE((screen_handler.mounted == SCREEN_INTERFACE_POPUP));
}

void test_screen_popup_dismiss_remounts_dashboard_dirty(void) {
    screen_api_on_parameter_change(INPUTS_SHARED_PARAMETER_ID_REGEN, 3U, 0U);
    screen_api_update(0U);
    screen_api_update(SCREEN_POPUP_TIMEOUT_MS);

    TEST_ASSERT_FALSE((screen_handler.mounted == SCREEN_INTERFACE_POPUP));
    TEST_ASSERT_EQUAL_PTR_MESSAGE(screen_handler.dashboard.boxes, screen_handler.raster.interface, "Dashboard must be mounted again after the popup");
}

void test_screen_popup_rejects_invalid_parameter(void) {
    TEST_ASSERT_EQUAL(SCREEN_RC_ERROR, screen_api_on_parameter_change(INPUTS_SHARED_PARAMETER_ID_COUNT, 1U, 0U));
    screen_api_update(1U);
    TEST_ASSERT_FALSE((screen_handler.mounted == SCREEN_INTERFACE_POPUP));
}

/*! \} */

/*!
 * \defgroup screen_api_set_accelerators Mask blend and frame cache
 * \{
 */

void test_screen_accelerators_reject_null(void) {
    TEST_ASSERT_EQUAL(SCREEN_RC_NULL_POINTER, screen_api_set_accelerators(NULL));
}

void test_screen_accelerators_reject_mask_without_buffer(void) {
    const struct ScreenAccelerators accelerators = { .draw_mask = fake_draw_mask, .mask_buffer = NULL, .mask_size = 0U };
    TEST_ASSERT_EQUAL(SCREEN_RC_ERROR, screen_api_set_accelerators(&accelerators));
}

void test_screen_labels_go_through_the_mask(void) {
    prv_enable_accelerators();
    prv_settle(0U); /* first frame: every box, also pending for the back buffer */
    screen_api_set_power(7U);
    prv_settle(1U); /* back buffer catches up */

    screen_api_set_power(8U);
    TEST_ASSERT_EQUAL(SCREEN_RC_RENDERED, screen_api_update(2U));

    TEST_ASSERT_EQUAL_MESSAGE(1, fake_draw_rectangle_fake.call_count, "only the POW background is a rectangle");
    TEST_ASSERT_EQUAL_MESSAGE(1, fake_draw_mask_fake.call_count, "the POW value is one mask blend");
}

/*! \} */

/*!
 * \defgroup screen_back_buffer Double buffering without full-frame copies
 * \{
 */

/* Two renders with no change in between: both framebuffers hold the full frame. */
static void prv_settle_both_buffers(void) {
    prv_enable_accelerators();
    prv_settle(0U);
    screen_api_set_power(1U);
    prv_settle(1U);
    screen_api_set_power(2U);
    prv_settle(2U);
}

void test_screen_back_buffer_gets_the_previous_change(void) {
    prv_settle_both_buffers(); /* POW changed in the last render */

    screen_api_set_soc(42U);
    TEST_ASSERT_EQUAL(SCREEN_RC_RENDERED, screen_api_update(3U));

    TEST_ASSERT_EQUAL_MESSAGE(2, fake_draw_rectangle_fake.call_count, "the new SOC box plus the POW box the other buffer missed");
    TEST_ASSERT_EQUAL(2, fake_draw_mask_fake.call_count);
}

void test_screen_back_buffer_catch_up_happens_once(void) {
    prv_settle_both_buffers();
    screen_api_set_soc(42U);
    prv_settle(3U); /* draws SOC + POW */

    screen_api_set_hv_temperature(55);
    screen_api_update(4U);

    TEST_ASSERT_EQUAL_MESSAGE(2, fake_draw_rectangle_fake.call_count, "HV temperature plus SOC only: POW is already in both buffers");
}

void test_screen_nothing_to_render_without_changes(void) {
    prv_settle_both_buffers(); /* POW still pending for the back buffer */

    TEST_ASSERT_EQUAL_MESSAGE(SCREEN_RC_OK, screen_api_update(3U), "a box pending for the back buffer alone does not trigger a frame");
    TEST_ASSERT_EQUAL(0, fake_draw_rectangle_fake.call_count);
}

void test_screen_popup_is_drawn_into_both_buffers(void) {
    prv_settle_both_buffers();

    screen_api_on_parameter_change(INPUTS_SHARED_PARAMETER_ID_POWER, 5U, 10U);
    TEST_ASSERT_EQUAL(SCREEN_RC_RENDERED, screen_api_update(10U));
    TEST_ASSERT_EQUAL_MESSAGE(POPUP_BOX_COUNT, fake_draw_rectangle_fake.call_count, "first popup frame: every popup box");
    prv_settle(10U);

    screen_api_on_parameter_change(INPUTS_SHARED_PARAMETER_ID_POWER, 6U, 20U);
    screen_api_update(20U);
    TEST_ASSERT_EQUAL_MESSAGE(POPUP_BOX_COUNT, fake_draw_rectangle_fake.call_count, "second popup frame: the back buffer still had the dashboard");
}

void test_screen_knob_steps_redraw_only_the_popup_value(void) {
    prv_settle_both_buffers();
    screen_api_on_parameter_change(INPUTS_SHARED_PARAMETER_ID_POWER, 5U, 10U);
    prv_settle(10U); /* popup appears: both boxes */
    screen_api_on_parameter_change(INPUTS_SHARED_PARAMETER_ID_POWER, 6U, 20U);
    prv_settle(20U); /* other buffer catches up */

    screen_api_on_parameter_change(INPUTS_SHARED_PARAMETER_ID_POWER, 7U, 30U);
    screen_api_update(30U);

    TEST_ASSERT_EQUAL_MESSAGE(1, fake_draw_rectangle_fake.call_count, "turning the same knob only redraws the value box");
}

void test_screen_popup_close_redraws_dashboard_into_both_buffers(void) {
    prv_settle_both_buffers();
    screen_api_on_parameter_change(INPUTS_SHARED_PARAMETER_ID_POWER, 5U, 10U);
    prv_settle(10U);

    TEST_ASSERT_EQUAL(SCREEN_RC_RENDERED, screen_api_update(10U + SCREEN_POPUP_TIMEOUT_MS));
    TEST_ASSERT_EQUAL_MESSAGE(DASHBOARD_BOX_COUNT, fake_draw_rectangle_fake.call_count, "whole dashboard back");
    prv_settle(10U + SCREEN_POPUP_TIMEOUT_MS);

    screen_api_set_soc(42U);
    screen_api_update(20U + SCREEN_POPUP_TIMEOUT_MS);
    TEST_ASSERT_EQUAL_MESSAGE(DASHBOARD_BOX_COUNT, fake_draw_rectangle_fake.call_count, "the other buffer still had the popup: whole dashboard again");
}

/*! \} */

/*!
 * \defgroup screen_api_set_flash FLASH page tests
 * \{
 */

void test_screen_flash_page_replaces_the_dashboard(void) {
    prv_settle(0U);

    TEST_ASSERT_EQUAL(SCREEN_RC_OK, screen_api_set_flash(true, "PEDALS"));
    TEST_ASSERT_EQUAL(SCREEN_RC_RENDERED, screen_api_update(10U));
    TEST_ASSERT_TRUE(screen_handler.mounted == SCREEN_INTERFACE_FLASH);
    TEST_ASSERT_EQUAL_STRING("PEDALS", screen_handler.flash.name.text);

    TEST_ASSERT_EQUAL(SCREEN_RC_OK, screen_api_set_flash(false, NULL));
    TEST_ASSERT_EQUAL(SCREEN_RC_RENDERED, screen_api_update(20U));
    TEST_ASSERT_TRUE(screen_handler.mounted == SCREEN_INTERFACE_DASHBOARD);
}

void test_screen_flash_logo_is_drawn_into_both_buffers(void) {
    prv_enable_accelerators();
    prv_settle(0U);

    screen_api_set_flash(true, "PEDALS");
    TEST_ASSERT_EQUAL(SCREEN_RC_RENDERED, screen_api_update(10U));
    TEST_ASSERT_TRUE_MESSAGE(prv_logo_blends() > 0U, "The logo goes on top of its box");

    // Static page: nothing renders, the frame on screen already has the logo.
    RESET_FAKE(fake_draw_mask);
    fake_draw_mask_fake.return_val = RASTER_RC_OK;
    TEST_ASSERT_EQUAL(SCREEN_RC_OK, screen_api_update(20U));

    // The next change is drawn into the other framebuffer: the logo catches up there.
    screen_api_set_flash(true, "COOLING");
    TEST_ASSERT_EQUAL(SCREEN_RC_RENDERED, screen_api_update(30U));
    TEST_ASSERT_TRUE_MESSAGE(prv_logo_blends() > 0U, "The other framebuffer needs the logo too");
}

void test_screen_flash_board_change_keeps_the_logo(void) {
    prv_enable_accelerators();
    screen_api_set_flash(true, "PEDALS");
    prv_settle(0U);
    screen_api_set_flash(true, "COOLING");
    prv_settle(1U); // both framebuffers have the whole page now

    screen_api_set_flash(true, "ECU");
    TEST_ASSERT_EQUAL(SCREEN_RC_RENDERED, screen_api_update(10U));
    TEST_ASSERT_EQUAL_MESSAGE(0U, prv_logo_blends(), "Only the name box changed: the logo stays as it is");
}

void test_screen_flash_hides_the_popup(void) {
    prv_settle(0U);
    screen_api_on_parameter_change(INPUTS_SHARED_PARAMETER_ID_POWER, 3U, 0U);
    screen_api_set_flash(true, "ECU");

    screen_api_update(10U);
    TEST_ASSERT_TRUE_MESSAGE(screen_handler.mounted == SCREEN_INTERFACE_FLASH, "FLASH wins over the popup");

    screen_api_set_flash(false, NULL);
    screen_api_update(20U);
    TEST_ASSERT_TRUE_MESSAGE(screen_handler.mounted == SCREEN_INTERFACE_POPUP, "A popup still in its timeout comes back");
}

/*! \} */

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_screen_popup_mounts_on_next_update);
    RUN_TEST(test_screen_popup_timeout_starts_at_change_not_at_last_render);
    RUN_TEST(test_screen_popup_survives_change_stamped_after_render_tick);
    RUN_TEST(test_screen_popup_new_change_extends_timeout);
    RUN_TEST(test_screen_popup_dismiss_remounts_dashboard_dirty);
    RUN_TEST(test_screen_popup_rejects_invalid_parameter);

    RUN_TEST(test_screen_accelerators_reject_null);
    RUN_TEST(test_screen_accelerators_reject_mask_without_buffer);
    RUN_TEST(test_screen_labels_go_through_the_mask);

    RUN_TEST(test_screen_back_buffer_gets_the_previous_change);
    RUN_TEST(test_screen_back_buffer_catch_up_happens_once);
    RUN_TEST(test_screen_nothing_to_render_without_changes);
    RUN_TEST(test_screen_popup_is_drawn_into_both_buffers);
    RUN_TEST(test_screen_knob_steps_redraw_only_the_popup_value);
    RUN_TEST(test_screen_popup_close_redraws_dashboard_into_both_buffers);

    RUN_TEST(test_screen_flash_page_replaces_the_dashboard);
    RUN_TEST(test_screen_flash_logo_is_drawn_into_both_buffers);
    RUN_TEST(test_screen_flash_board_change_keeps_the_logo);
    RUN_TEST(test_screen_flash_hides_the_popup);

    return UNITY_END();
}
