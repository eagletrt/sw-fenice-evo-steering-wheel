/*!
 * \file test-flash-monitor.c
 * \date 2026-10-29
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 *
 * \brief Unit tests for the detection of other boards being flashed.
 *
 * \details The timeout runs on a real libtimebase watchdog pool with a fake
 *     clock.
 */

#include "unity.h"
#include "flash-monitor-api.h"
#include "can-primary.h"
#include "font-api.h"
#include "raster-fonts.h"
#include "splash.h"
#include "watchdogs-api.h"
#include <string.h>

extern struct FlashMonitorHandler flash_monitor_handler;
const struct FlashMonitorTarget *prv_flash_monitor_api_find_target(uint32_t id);

/* First and last libcan flashing identifiers: every board's Tx/Rx pair lies in between. */
#define TEST_FLASH_FIRST_ID ((uint32_t)CAN_PRIMARY_MESSAGE_FRAME_ID_ECUFLASHINGTX)
#define TEST_FLASH_LAST_ID ((uint32_t)CAN_PRIMARY_MESSAGE_FRAME_ID_STEERINGBOARDFLASHINGRX)
/* Space left on each side of a board name on the FLASH page. */
#define TEST_FLASH_NAME_MARGIN (40U)

static uint32_t prv_now;
static struct WatchdogHandler prv_watchdogs;

static uint32_t prv_get_tick(void) {
    return prv_now;
}

/* Move the clock and let the watchdog pool fire whatever expired. */
static void prv_advance_to(uint32_t tick) {
    prv_now = tick;
    TEST_ASSERT_EQUAL(WATCHDOG_RC_OK, watchdogs_api_routine(&prv_watchdogs));
}

static bool prv_is_own_id(uint32_t id) {
    return id == CAN_PRIMARY_MESSAGE_FRAME_ID_STEERINGWHEELFLASHINGTX || id == CAN_PRIMARY_MESSAGE_FRAME_ID_STEERINGWHEELFLASHINGRX;
}

void setUp(void) {
    prv_now = 0U;
    TEST_ASSERT_EQUAL(WATCHDOG_RC_OK, watchdogs_api_init_pool(&prv_watchdogs, prv_get_tick));
    TEST_ASSERT_EQUAL(FLASH_MONITOR_RC_OK, flash_monitor_api_init(&prv_watchdogs));
}

void tearDown(void) {}

/*!
 * \defgroup flash_monitor_table Tests for the board table
 * \{
 */

void test_flash_monitor_table_covers_every_other_board(void) {
    for (uint32_t id = TEST_FLASH_FIRST_ID; id <= TEST_FLASH_LAST_ID; id++) {
        if (prv_is_own_id(id)) {
            continue;
        }
        TEST_ASSERT_NOT_NULL_MESSAGE(prv_flash_monitor_api_find_target(id), "Every libcan flashing ID of another board must be in the table");
    }
}

void test_flash_monitor_table_excludes_the_wheel(void) {
    TEST_ASSERT_NULL_MESSAGE(prv_flash_monitor_api_find_target(CAN_PRIMARY_MESSAGE_FRAME_ID_STEERINGWHEELFLASHINGTX), "The wheel's own pair means reset, not FLASH");
    TEST_ASSERT_NULL(prv_flash_monitor_api_find_target(CAN_PRIMARY_MESSAGE_FRAME_ID_STEERINGWHEELFLASHINGRX));
}

void test_flash_monitor_table_ignores_other_messages(void) {
    TEST_ASSERT_NULL(prv_flash_monitor_api_find_target(TEST_FLASH_FIRST_ID - 1U));
    TEST_ASSERT_NULL(prv_flash_monitor_api_find_target(TEST_FLASH_LAST_ID + 1U));
    TEST_ASSERT_NULL(prv_flash_monitor_api_find_target(CAN_PRIMARY_MESSAGE_FRAME_ID_ECUFSM));
}

void test_flash_monitor_table_pairs_are_consistent(void) {
    for (uint32_t id = TEST_FLASH_FIRST_ID; id <= TEST_FLASH_LAST_ID; id++) {
        const struct FlashMonitorTarget *target = prv_flash_monitor_api_find_target(id);
        if (target == NULL) {
            continue;
        }
        TEST_ASSERT_EQUAL_MESSAGE(target->tx_id + 1U, target->rx_id, "libcan numbers each board's Rx right after its Tx");
        TEST_ASSERT_TRUE_MESSAGE(id == target->tx_id || id == target->rx_id, "An ID must map to its own board");
    }
}

void test_flash_monitor_table_names_fit_the_screen(void) {
    for (uint32_t id = TEST_FLASH_FIRST_ID; id <= TEST_FLASH_LAST_ID; id += 2U) {
        const struct FlashMonitorTarget *target = prv_flash_monitor_api_find_target(id);
        if (target == NULL) {
            continue;
        }
        TEST_ASSERT_NOT_NULL(target->name);
        TEST_ASSERT_TRUE(strlen(target->name) > 0U);
        const uint16_t width = font_api_length(target->name, SPLASH_NAME_FONT_SIZE, &font_inter);
        TEST_ASSERT_TRUE_MESSAGE(width <= SPLASH_SCREEN_WIDTH - 2U * TEST_FLASH_NAME_MARGIN, target->name);
    }
}

/*! \} */

/*!
 * \defgroup flash_monitor_api Tests for the monitor
 * \{
 */

void test_flash_monitor_init_rejects_null(void) {
    TEST_ASSERT_EQUAL(FLASH_MONITOR_RC_NULL_POINTER, flash_monitor_api_init(NULL));
}

void test_flash_monitor_starts_inactive(void) {
    TEST_ASSERT_FALSE(flash_monitor_api_is_active());
    TEST_ASSERT_NULL(flash_monitor_api_get_board_name());
}

void test_flash_monitor_flashing_frame_activates(void) {
    TEST_ASSERT_TRUE(flash_monitor_api_on_frame(CAN_PRIMARY_MESSAGE_FRAME_ID_PEDALSFLASHINGRX));

    TEST_ASSERT_TRUE(flash_monitor_api_is_active());
    TEST_ASSERT_EQUAL_STRING("PEDALS", flash_monitor_api_get_board_name());
}

void test_flash_monitor_other_frames_are_not_consumed(void) {
    TEST_ASSERT_FALSE(flash_monitor_api_on_frame(CAN_PRIMARY_MESSAGE_FRAME_ID_ECUFSM));
    TEST_ASSERT_FALSE_MESSAGE(flash_monitor_api_on_frame(CAN_PRIMARY_MESSAGE_FRAME_ID_STEERINGWHEELFLASHINGRX), "The router needs the wheel's own frames for the reset");

    TEST_ASSERT_FALSE(flash_monitor_api_is_active());
}

void test_flash_monitor_expires_after_the_timeout(void) {
    flash_monitor_api_on_frame(CAN_PRIMARY_MESSAGE_FRAME_ID_COOLINGFLASHINGTX);

    prv_advance_to(FLASH_MONITOR_TIMEOUT_MS - 1U);
    TEST_ASSERT_TRUE_MESSAGE(flash_monitor_api_is_active(), "Still flashing just before the timeout");

    prv_advance_to(FLASH_MONITOR_TIMEOUT_MS);
    TEST_ASSERT_FALSE_MESSAGE(flash_monitor_api_is_active(), "Flashing is over after FLASH_MONITOR_TIMEOUT_MS of silence");
    TEST_ASSERT_EQUAL_STRING_MESSAGE("COOLING", flash_monitor_api_get_board_name(), "The last board is kept for the record");
}

void test_flash_monitor_each_frame_restarts_the_timeout(void) {
    flash_monitor_api_on_frame(CAN_PRIMARY_MESSAGE_FRAME_ID_COOLINGFLASHINGTX);
    prv_advance_to(400U);
    flash_monitor_api_on_frame(CAN_PRIMARY_MESSAGE_FRAME_ID_COOLINGFLASHINGRX);

    prv_advance_to(800U);
    TEST_ASSERT_TRUE(flash_monitor_api_is_active());

    prv_advance_to(400U + FLASH_MONITOR_TIMEOUT_MS);
    TEST_ASSERT_FALSE(flash_monitor_api_is_active());
}

void test_flash_monitor_reactivates_after_expiry(void) {
    flash_monitor_api_on_frame(CAN_PRIMARY_MESSAGE_FRAME_ID_ECUFLASHINGTX);
    prv_advance_to(FLASH_MONITOR_TIMEOUT_MS);
    TEST_ASSERT_FALSE(flash_monitor_api_is_active());

    prv_advance_to(2000U);
    TEST_ASSERT_TRUE(flash_monitor_api_on_frame(CAN_PRIMARY_MESSAGE_FRAME_ID_DASREARFLASHINGTX));
    TEST_ASSERT_TRUE_MESSAGE(flash_monitor_api_is_active(), "An expired watchdog must restart on the next flashing frame");
    TEST_ASSERT_EQUAL_STRING("DAS REAR", flash_monitor_api_get_board_name());

    prv_advance_to(2000U + FLASH_MONITOR_TIMEOUT_MS);
    TEST_ASSERT_FALSE(flash_monitor_api_is_active());
}

void test_flash_monitor_name_follows_the_last_board(void) {
    flash_monitor_api_on_frame(CAN_PRIMARY_MESSAGE_FRAME_ID_TSACCELLBOARD1FLASHINGTX);
    flash_monitor_api_on_frame(CAN_PRIMARY_MESSAGE_FRAME_ID_TSACCELLBOARD2FLASHINGRX);

    TEST_ASSERT_EQUAL_STRING("CELLBOARD 2", flash_monitor_api_get_board_name());
}

/*! \} */

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_flash_monitor_table_covers_every_other_board);
    RUN_TEST(test_flash_monitor_table_excludes_the_wheel);
    RUN_TEST(test_flash_monitor_table_ignores_other_messages);
    RUN_TEST(test_flash_monitor_table_pairs_are_consistent);
    RUN_TEST(test_flash_monitor_table_names_fit_the_screen);

    RUN_TEST(test_flash_monitor_init_rejects_null);
    RUN_TEST(test_flash_monitor_starts_inactive);
    RUN_TEST(test_flash_monitor_flashing_frame_activates);
    RUN_TEST(test_flash_monitor_other_frames_are_not_consumed);
    RUN_TEST(test_flash_monitor_expires_after_the_timeout);
    RUN_TEST(test_flash_monitor_each_frame_restarts_the_timeout);
    RUN_TEST(test_flash_monitor_reactivates_after_expiry);
    RUN_TEST(test_flash_monitor_name_follows_the_last_board);

    return UNITY_END();
}
