/*!
 * \file test-screen-sync.c
 * \date 2026-06-07
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 *
 * \brief Unit tests for screen_api_sync_data: the UI data snapshot reaches
 *     the dashboard and locks or unlocks the menu.
 *
 * \details The formatting itself is tested in test_dashboard.
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

void tearDown(void) {
}

void test_screen_api_sync_null_pointer(void) {
    TEST_ASSERT_EQUAL(SCREEN_RC_NULL_POINTER, screen_api_sync_data(NULL));
}

void test_screen_api_sync_reaches_the_dashboard(void) {
    struct UIData snapshot = { 0 };
    snapshot.speed_kmh = 123U;
    TEST_ASSERT_EQUAL(SCREEN_RC_OK, screen_api_sync_data(&snapshot));
    TEST_ASSERT_EQUAL_STRING("123", screen_handler.dashboard.text[DASHBOARD_FIELD_SPEED]);
}

void test_screen_api_sync_unlocks_the_menu_only_with_the_ecu_idle(void) {
    struct UIData snapshot = { .vehicle_state = UI_DATA_VEHICLE_STATE_IDLE };
    screen_api_sync_data(&snapshot);
    TEST_ASSERT_TRUE(screen_handler.menu.unlocked);

    snapshot.vehicle_state = UI_DATA_VEHICLE_STATE_TSON;
    screen_api_sync_data(&snapshot);
    TEST_ASSERT_FALSE(screen_handler.menu.unlocked);

    snapshot.vehicle_state = UI_DATA_VEHICLE_STATE_COUNT;
    screen_api_sync_data(&snapshot);
    TEST_ASSERT_FALSE_MESSAGE(screen_handler.menu.unlocked, "an unknown ECU state locks the menu (A4)");
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_screen_api_sync_null_pointer);
    RUN_TEST(test_screen_api_sync_reaches_the_dashboard);
    RUN_TEST(test_screen_api_sync_unlocks_the_menu_only_with_the_ecu_idle);

    return UNITY_END();
}
