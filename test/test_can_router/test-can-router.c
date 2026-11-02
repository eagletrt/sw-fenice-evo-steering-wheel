/*!
 * \file test-can-router.c
 * \date 2026-11-02
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 *
 * \brief Unit tests for the CAN router's receive path and FDCAN filter list.
 *
 * \details Frames go through the real flash monitor, on a real libtimebase
 *     watchdog pool with a fake clock; the reset is a fake.
 */

#include "unity.h"
#include "fff.h"
#include "can-communications-router-api.h"
#include "can-primary-api.h"
#include "flash-monitor-api.h"
#include "ui-data-api.h"
#include "watchdogs-api.h"
#include <string.h>

DEFINE_FFF_GLOBALS;

FAKE_VOID_FUNC(fake_reset);

/* Every <Board>FlashingTx/Rx pair libcan defines, the wheel's own and Battery's included. */
#define TEST_ROUTER_FLASH_FIRST_ID ((uint32_t)CAN_PRIMARY_MESSAGE_FRAME_ID_ECUFLASHINGTX)
#define TEST_ROUTER_FLASH_LAST_ID ((uint32_t)CAN_PRIMARY_MESSAGE_FRAME_ID_STEERINGBOARDFLASHINGRX)
#define TEST_ROUTER_FLASH_PAIRS (17U)

/* Messages the dashboard decodes. */
static const uint32_t prv_dashboard_ids[] = {
    CAN_PRIMARY_MESSAGE_FRAME_ID_ECUFSM,
    CAN_PRIMARY_MESSAGE_FRAME_ID_TSACMAINBOARDESTIMATEDSOC,
    CAN_PRIMARY_MESSAGE_FRAME_ID_TSACMAINBOARDTEMPERATUREINFO,
    CAN_PRIMARY_MESSAGE_FRAME_ID_INVERTERTEMPERATURE,
    CAN_PRIMARY_MESSAGE_FRAME_ID_MOTORTEMPERATURE,
    CAN_PRIMARY_MESSAGE_FRAME_ID_TYRETEMPERATUREFRONT,
    CAN_PRIMARY_MESSAGE_FRAME_ID_TYRETEMPERATUREREAR,
    CAN_PRIMARY_MESSAGE_FRAME_ID_CURRENTLAP,
    CAN_PRIMARY_MESSAGE_FRAME_ID_LAPS,
};

#define TEST_ROUTER_DASHBOARD_COUNT (sizeof(prv_dashboard_ids) / sizeof(prv_dashboard_ids[0]))

static uint32_t prv_now;
static struct WatchdogHandler prv_watchdogs;

static uint32_t prv_get_tick(void) {
    return prv_now;
}

/* Serialize a libcan message and hand it to the router. */
static enum CanCommunicationReturnCode prv_receive(enum CanPrimaryMessageFrameId id, union CanPrimaryMessages *message) {
    struct CanCommunicationFrame frame = { .id = (uint32_t)id, .length = 8U };
    TEST_ASSERT_TRUE(can_primary_api_serialize_from_id(id, message, frame.data) >= 0);
    return can_communications_router_api_receive_primary(&frame);
}

void setUp(void) {
    RESET_FAKE(fake_reset);
    FFF_RESET_HISTORY();
    prv_now = 0U;
    memset(ui_data_api_get(), 0, sizeof(struct UIData));
    TEST_ASSERT_EQUAL(WATCHDOG_RC_OK, watchdogs_api_init_pool(&prv_watchdogs, prv_get_tick));
    TEST_ASSERT_EQUAL(FLASH_MONITOR_RC_OK, flash_monitor_api_init(&prv_watchdogs, fake_reset));
}

void tearDown(void) {}

/*!
 * \defgroup can_router_receive Tests for can_communications_router_api_receive_primary
 * \{
 */

void test_can_router_rejects_null_frame(void) {
    TEST_ASSERT_EQUAL(CAN_COMMUNICATION_RC_NULL_POINTER, can_communications_router_api_receive_primary(NULL));
}

void test_can_router_decodes_dashboard_messages(void) {
    union CanPrimaryMessages message = { 0 };
    message.laps.totalnumber = 7U;

    TEST_ASSERT_EQUAL(CAN_COMMUNICATION_RC_OK, prv_receive(CAN_PRIMARY_MESSAGE_FRAME_ID_LAPS, &message));

    TEST_ASSERT_EQUAL_UINT(7U, ui_data_api_get()->lap_total);
    TEST_ASSERT_FALSE(flash_monitor_api_is_active());
}

void test_can_router_hands_flashing_traffic_to_the_monitor(void) {
    union CanPrimaryMessages message = { 0 };

    TEST_ASSERT_EQUAL(CAN_COMMUNICATION_RC_OK, prv_receive(CAN_PRIMARY_MESSAGE_FRAME_ID_DASREARFLASHINGRX, &message));

    TEST_ASSERT_TRUE(flash_monitor_api_is_active());
    TEST_ASSERT_EQUAL_STRING("DAS REAR", flash_monitor_api_get_board_name());
    TEST_ASSERT_EQUAL(0, fake_reset_fake.call_count);
}

void test_can_router_own_flashing_frame_resets(void) {
    union CanPrimaryMessages message = { 0 };

    TEST_ASSERT_EQUAL(CAN_COMMUNICATION_RC_OK, prv_receive(CAN_PRIMARY_MESSAGE_FRAME_ID_STEERINGWHEELFLASHINGRX, &message));

    TEST_ASSERT_EQUAL(1, fake_reset_fake.call_count);
    TEST_ASSERT_FALSE(flash_monitor_api_is_active());
}

void test_can_router_other_libcan_messages_change_nothing(void) {
    union CanPrimaryMessages message = { 0 };

    TEST_ASSERT_EQUAL(CAN_COMMUNICATION_RC_OK, prv_receive(CAN_PRIMARY_MESSAGE_FRAME_ID_STEERINGWHEELFSM, &message));

    TEST_ASSERT_FALSE(flash_monitor_api_is_active());
    TEST_ASSERT_EQUAL(0, fake_reset_fake.call_count);
}

void test_can_router_unknown_identifier_is_an_error(void) {
    // Not a libcan message: the FDCAN filter should have dropped it.
    const struct CanCommunicationFrame frame = { .id = 0x7FFU, .length = 8U };

    TEST_ASSERT_EQUAL(CAN_COMMUNICATION_RC_RECEIVE_HANDLER_ERROR, can_communications_router_api_receive_primary(&frame));
}

/*! \} */

/*!
 * \defgroup can_router_filters Tests for the FDCAN filter list
 * \{
 */

void test_can_router_primary_filter_count_matches_the_ioc_constant(void) {
    size_t count = 0U;
    TEST_ASSERT_NOT_NULL(can_communications_router_api_get_filters(CAN_COMMUNICATION_NETWORK_PRIMARY, &count));
    TEST_ASSERT_EQUAL_UINT(CAN_COMMUNICATIONS_ROUTER_PRIMARY_FILTER_COUNT, count);
}

void test_can_router_primary_filters_are_valid_ranges(void) {
    size_t count = 0U;
    const struct CanCommunicationsRouterFilter *filters = can_communications_router_api_get_filters(CAN_COMMUNICATION_NETWORK_PRIMARY, &count);
    for (size_t i = 0U; i < count; i++) {
        TEST_ASSERT_TRUE(filters[i].first_id <= filters[i].last_id);
        TEST_ASSERT_TRUE_MESSAGE(filters[i].last_id < (1U << 11U), "Standard identifiers only");
    }
}

void test_can_router_flashing_traffic_is_the_first_filter(void) {
    // If the .ioc has too few slots, the first ones still let the wheel be reflashed.
    size_t count = 0U;
    const struct CanCommunicationsRouterFilter *filters = can_communications_router_api_get_filters(CAN_COMMUNICATION_NETWORK_PRIMARY, &count);
    TEST_ASSERT_EQUAL_UINT16(TEST_ROUTER_FLASH_FIRST_ID, filters[0].first_id);
    TEST_ASSERT_EQUAL_UINT16(TEST_ROUTER_FLASH_LAST_ID, filters[0].last_id);
    TEST_ASSERT_EQUAL_UINT(2U * TEST_ROUTER_FLASH_PAIRS, TEST_ROUTER_FLASH_LAST_ID - TEST_ROUTER_FLASH_FIRST_ID + 1U);
}

void test_can_router_accepts_every_flashing_and_dashboard_identifier(void) {
    for (uint32_t id = TEST_ROUTER_FLASH_FIRST_ID; id <= TEST_ROUTER_FLASH_LAST_ID; id++) {
        TEST_ASSERT_TRUE(can_communications_router_api_accepts(CAN_COMMUNICATION_NETWORK_PRIMARY, id));
    }
    for (size_t i = 0U; i < TEST_ROUTER_DASHBOARD_COUNT; i++) {
        TEST_ASSERT_TRUE(can_communications_router_api_accepts(CAN_COMMUNICATION_NETWORK_PRIMARY, prv_dashboard_ids[i]));
    }
}

void test_can_router_rejects_everything_else(void) {
    size_t accepted = 0U;
    for (uint32_t id = 0U; id < (1U << 11U); id++) {
        accepted += can_communications_router_api_accepts(CAN_COMMUNICATION_NETWORK_PRIMARY, id) ? 1U : 0U;
    }
    TEST_ASSERT_EQUAL_UINT(2U * TEST_ROUTER_FLASH_PAIRS + TEST_ROUTER_DASHBOARD_COUNT, accepted);
    TEST_ASSERT_FALSE(can_communications_router_api_accepts(CAN_COMMUNICATION_NETWORK_PRIMARY, CAN_PRIMARY_MESSAGE_FRAME_ID_STEERINGWHEELFSM));
}

void test_can_router_secondary_accepts_nothing(void) {
    size_t count = 1U;
    TEST_ASSERT_NULL(can_communications_router_api_get_filters(CAN_COMMUNICATION_NETWORK_SECONDARY, &count));
    TEST_ASSERT_EQUAL_UINT(0U, count);
    TEST_ASSERT_FALSE(can_communications_router_api_accepts(CAN_COMMUNICATION_NETWORK_SECONDARY, CAN_PRIMARY_MESSAGE_FRAME_ID_ECUFSM));
}

void test_can_router_get_filters_rejects_null_count(void) {
    TEST_ASSERT_NULL(can_communications_router_api_get_filters(CAN_COMMUNICATION_NETWORK_PRIMARY, NULL));
}

/*! \} */

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_can_router_rejects_null_frame);
    RUN_TEST(test_can_router_decodes_dashboard_messages);
    RUN_TEST(test_can_router_hands_flashing_traffic_to_the_monitor);
    RUN_TEST(test_can_router_own_flashing_frame_resets);
    RUN_TEST(test_can_router_other_libcan_messages_change_nothing);
    RUN_TEST(test_can_router_unknown_identifier_is_an_error);

    RUN_TEST(test_can_router_primary_filter_count_matches_the_ioc_constant);
    RUN_TEST(test_can_router_primary_filters_are_valid_ranges);
    RUN_TEST(test_can_router_flashing_traffic_is_the_first_filter);
    RUN_TEST(test_can_router_accepts_every_flashing_and_dashboard_identifier);
    RUN_TEST(test_can_router_rejects_everything_else);
    RUN_TEST(test_can_router_secondary_accepts_nothing);
    RUN_TEST(test_can_router_get_filters_rejects_null_count);

    return UNITY_END();
}
