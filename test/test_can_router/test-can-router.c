/*!
 * \file test-can-router.c
 * \date 2026-11-02
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 *
 * \brief Unit tests for the CAN router's receive path and FDCAN filter list.
 *
 * \details Frames go through the real flash monitor, on the real timeout
 *     pool with a fake clock; the reset is a fake.
 */

#include "unity.h"
#include "fff.h"
#include "can-communications-router-api.h"
#include "can-primary-api.h"
#include "flash-monitor-api.h"
#include "ui-data-api.h"
#include "timeout-api.h"
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
    CAN_PRIMARY_MESSAGE_FRAME_ID_CURRENTLAP,
    CAN_PRIMARY_MESSAGE_FRAME_ID_LVACESTIMATEDSOC,
    CAN_PRIMARY_MESSAGE_FRAME_ID_ESTIMATEDVEHICLEVELOCITY,
    CAN_PRIMARY_MESSAGE_FRAME_ID_PEDALSTHROTTLE,
    CAN_PRIMARY_MESSAGE_FRAME_ID_PEDALSBRAKE,
    CAN_PRIMARY_MESSAGE_FRAME_ID_TSACMAINBOARDFEEDBACKSHUTDOWN,
};

#define TEST_ROUTER_DASHBOARD_COUNT (sizeof(prv_dashboard_ids) / sizeof(prv_dashboard_ids[0]))

static uint32_t prv_now;

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
    TEST_ASSERT_EQUAL(TIMEOUT_RC_OK, timeout_api_init(prv_get_tick));
    TEST_ASSERT_EQUAL(FLASH_MONITOR_RC_OK, flash_monitor_api_init(fake_reset));
    TEST_ASSERT_EQUAL(CAN_COMMUNICATION_RC_OK, can_communications_router_api_init());
}

static void prv_receive_ecu(uint8_t vehicle_status) {
    union CanPrimaryMessages message = { 0 };
    message.ecufsm.vehiclestatus = vehicle_status;
    TEST_ASSERT_EQUAL(CAN_COMMUNICATION_RC_OK, prv_receive(CAN_PRIMARY_MESSAGE_FRAME_ID_ECUFSM, &message));
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
    message.currentlap.number = 7U;
    message.currentlap.deltabestlap = -0.42F;

    TEST_ASSERT_EQUAL(CAN_COMMUNICATION_RC_OK, prv_receive(CAN_PRIMARY_MESSAGE_FRAME_ID_CURRENTLAP, &message));

    TEST_ASSERT_EQUAL_UINT(7U, ui_data_api_get()->lap_current);
    TEST_ASSERT_INT32_WITHIN(1, -420, ui_data_api_get()->lap_delta_ms);
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

void test_can_router_autonomous_follows_the_as_branch(void) {
    const uint8_t as_branch[] = { CAN_PRIMARY_ECUFSM_VEHICLESTATUS_ASOFF, CAN_PRIMARY_ECUFSM_VEHICLESTATUS_AS_READY, CAN_PRIMARY_ECUFSM_VEHICLESTATUS_AS_R2D, CAN_PRIMARY_ECUFSM_VEHICLESTATUS_AS_DRIVING, CAN_PRIMARY_ECUFSM_VEHICLESTATUS_AS_FINISHED, CAN_PRIMARY_ECUFSM_VEHICLESTATUS_AS_EMERGENCY };
    const uint8_t manual[] = { CAN_PRIMARY_ECUFSM_VEHICLESTATUS_INIT, CAN_PRIMARY_ECUFSM_VEHICLESTATUS_IDLE, CAN_PRIMARY_ECUFSM_VEHICLESTATUS_PRECHARGE, CAN_PRIMARY_ECUFSM_VEHICLESTATUS_TSON, CAN_PRIMARY_ECUFSM_VEHICLESTATUS_R2D, CAN_PRIMARY_ECUFSM_VEHICLESTATUS_DISCHARGE, CAN_PRIMARY_ECUFSM_VEHICLESTATUS_ERROR };
    for (size_t i = 0U; i < sizeof(as_branch); i++) {
        prv_receive_ecu(as_branch[i]);
        TEST_ASSERT_EQUAL_UINT8_MESSAGE(1U, ui_data_api_get()->autonomous, "AS branch, AS_EMERGENCY included: the AS page stays");
    }
    for (size_t i = 0U; i < sizeof(manual); i++) {
        prv_receive_ecu(manual[i]);
        TEST_ASSERT_EQUAL_UINT8(0U, ui_data_api_get()->autonomous);
    }
}

void test_can_router_silent_ecu_becomes_unknown_and_manual(void) {
    prv_receive_ecu(CAN_PRIMARY_ECUFSM_VEHICLESTATUS_AS_READY);
    prv_now = 3U * (uint32_t)can_primary_cycle_time_ecufsm - 1U;
    TEST_ASSERT_EQUAL(TIMEOUT_RC_OK, timeout_api_routine());
    TEST_ASSERT_EQUAL_UINT8(UI_DATA_VEHICLE_STATE_AUTONOMOUS, ui_data_api_get()->vehicle_state);

    prv_now++;
    TEST_ASSERT_EQUAL(TIMEOUT_RC_OK, timeout_api_routine());
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(UI_DATA_VEHICLE_STATE_COUNT, ui_data_api_get()->vehicle_state, "Three missed EcuFsm: unknown (A4)");
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U, ui_data_api_get()->autonomous, "Unknown counts as manual (D20)");
}

void test_can_router_decodes_the_driver_values(void) {
    union CanPrimaryMessages message = { 0 };
    message.lvacestimatedsoc.soc = 0.82F;
    prv_receive(CAN_PRIMARY_MESSAGE_FRAME_ID_LVACESTIMATEDSOC, &message);
    memset(&message, 0, sizeof(message));
    message.estimatedvehiclevelocity.u = 24.2F;
    prv_receive(CAN_PRIMARY_MESSAGE_FRAME_ID_ESTIMATEDVEHICLEVELOCITY, &message);
    memset(&message, 0, sizeof(message));
    message.pedalsthrottle.travel = 0.72F;
    prv_receive(CAN_PRIMARY_MESSAGE_FRAME_ID_PEDALSTHROTTLE, &message);

    const struct UIData *ui = ui_data_api_get();
    TEST_ASSERT_UINT8_WITHIN(1, 82U, ui->lv_soc);
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(87U, ui->speed_kmh, "24.2 m/s rounded to km/h");
    TEST_ASSERT_UINT8_WITHIN(1, 72U, ui->throttle_percent);
}

void test_can_router_brake_is_the_highest_pressure_one_percent_per_bar(void) {
    union CanPrimaryMessages message = { 0 };
    message.pedalsbrake.pressurefl = 40.0F;
    message.pedalsbrake.pressurefr = 40.0F;
    message.pedalsbrake.pressurerl = 65.4F;
    message.pedalsbrake.pressurerr = 65.4F;
    prv_receive(CAN_PRIMARY_MESSAGE_FRAME_ID_PEDALSBRAKE, &message);
    TEST_ASSERT_EQUAL_UINT8(65U, ui_data_api_get()->brake_percent);

    memset(&message, 0, sizeof(message));
    message.pedalsbrake.pressurefl = 0.9F;
    prv_receive(CAN_PRIMARY_MESSAGE_FRAME_ID_PEDALSBRAKE, &message);
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(0U, ui_data_api_get()->brake_percent, "0-1 bar is 0 %");
}

void test_can_router_shutdown_closed_above_the_threshold(void) {
    union CanPrimaryMessages message = { 0 };
    message.tsacmainboardfeedbackshutdown.sdend = 24.0F;
    prv_receive(CAN_PRIMARY_MESSAGE_FRAME_ID_TSACMAINBOARDFEEDBACKSHUTDOWN, &message);
    TEST_ASSERT_EQUAL_UINT8(1U, ui_data_api_get()->shutdown_closed);
    message.tsacmainboardfeedbackshutdown.sdend = 3.0F;
    prv_receive(CAN_PRIMARY_MESSAGE_FRAME_ID_TSACMAINBOARDFEEDBACKSHUTDOWN, &message);
    TEST_ASSERT_EQUAL_UINT8(0U, ui_data_api_get()->shutdown_closed);
}

void test_can_router_a_frame_makes_its_source_fresh_until_it_goes_silent(void) {
    ui_data_api_get()->stale = UI_DATA_STALE_ALL;
    union CanPrimaryMessages message = { 0 };
    message.pedalsthrottle.travel = 0.5F;
    prv_receive(CAN_PRIMARY_MESSAGE_FRAME_ID_PEDALSTHROTTLE, &message);
    TEST_ASSERT_EQUAL_HEX16_MESSAGE(UI_DATA_STALE_ALL & (uint16_t)~UI_DATA_STALE_BIT(UI_DATA_SOURCE_THROTTLE), ui_data_api_get()->stale, "only the throttle is fresh");

    // 5 ms message: stale after the 500 ms floor, not after 15 ms.
    prv_now = 499U;
    TEST_ASSERT_EQUAL(TIMEOUT_RC_OK, timeout_api_routine());
    TEST_ASSERT_EQUAL_HEX16(0U, ui_data_api_get()->stale & UI_DATA_STALE_BIT(UI_DATA_SOURCE_THROTTLE));
    prv_now = 500U;
    TEST_ASSERT_EQUAL(TIMEOUT_RC_OK, timeout_api_routine());
    TEST_ASSERT_EQUAL_HEX16(UI_DATA_STALE_BIT(UI_DATA_SOURCE_THROTTLE), ui_data_api_get()->stale & UI_DATA_STALE_BIT(UI_DATA_SOURCE_THROTTLE));
}

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

    RUN_TEST(test_can_router_autonomous_follows_the_as_branch);
    RUN_TEST(test_can_router_silent_ecu_becomes_unknown_and_manual);

    RUN_TEST(test_can_router_decodes_the_driver_values);
    RUN_TEST(test_can_router_brake_is_the_highest_pressure_one_percent_per_bar);
    RUN_TEST(test_can_router_shutdown_closed_above_the_threshold);
    RUN_TEST(test_can_router_a_frame_makes_its_source_fresh_until_it_goes_silent);

    return UNITY_END();
}
