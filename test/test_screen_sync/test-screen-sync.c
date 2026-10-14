/*!
 * \file test-screen-sync.c
 * \date 2026-06-07
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 *
 * \brief Unit tests for screen_api_sync_data — the UIData -> dashboard pump.
 *
 * \details screen_api_sync_data forwards each field of a UIData snapshot through
 *     the matching screen_api_set_*; we drive it directly and assert on the
 *     formatted text in the file-static screen_handler's dashboard buffers.
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

static struct UIData prv_make_snapshot(void) {
    struct UIData snapshot = { 0 };
    snapshot.power = 5U;
    snapshot.regen = 10U;
    snapshot.torque = 6U;
    snapshot.slip_on = 1U;
    snapshot.vehicle_state = UI_DATA_VEHICLE_STATE_DRIVE;
    snapshot.soc = 69U;
    snapshot.hv_temp = 104;
    snapshot.inverter_temp = 22;
    snapshot.lap_current = 9U;
    snapshot.lap_total = 11U;
    snapshot.lap_delta_ms = -420;
    snapshot.tire_fl_temp = 95;
    snapshot.tire_fr_temp = 60;
    snapshot.tire_rl_temp = 10;
    snapshot.tire_rr_temp = 40;
    snapshot.motor_fl_temp = 22;
    snapshot.motor_fr_temp = 23;
    snapshot.motor_rl_temp = 24;
    snapshot.motor_rr_temp = 25;
    return snapshot;
}

/*!
 * \defgroup screen_api_sync_data Test argument-validation tests
 * \{
 */

void test_screen_api_sync_null_pointer(void) {
    enum ScreenReturnCode rc = screen_api_sync_data(NULL);
    TEST_ASSERT_EQUAL(SCREEN_RC_NULL_POINTER, rc);
}

void test_screen_api_sync_empty_snapshot_succeeds(void) {
    struct UIData empty = { 0 };
    enum ScreenReturnCode rc = screen_api_sync_data(&empty);
    TEST_ASSERT_EQUAL(SCREEN_RC_OK, rc);
}

/*! \} */

/*!
 * \defgroup screen_api_sync_data Full forwarding tests
 * \{
 */

void test_screen_api_sync_pushes_scenario_strip(void) {
    struct UIData snapshot = prv_make_snapshot();
    screen_api_sync_data(&snapshot);
    TEST_ASSERT_EQUAL_STRING_MESSAGE("5", screen_handler.dashboard.text[DASHBOARD_FIELD_POWER], "power value field carries only the number");
    TEST_ASSERT_EQUAL_STRING_MESSAGE("10", screen_handler.dashboard.text[DASHBOARD_FIELD_REGEN], "regen value field carries only the number");
    TEST_ASSERT_EQUAL_STRING_MESSAGE("6", screen_handler.dashboard.text[DASHBOARD_FIELD_TORQUE], "torque value field carries only the number");
    TEST_ASSERT_EQUAL_STRING_MESSAGE("ON", screen_handler.dashboard.text[DASHBOARD_FIELD_SLIP], "slip on translates to 'ON' (label box says 'SLIP')");
}

void test_screen_api_sync_slip_off_when_zero(void) {
    struct UIData snapshot = prv_make_snapshot();
    snapshot.slip_on = 0U;
    screen_api_sync_data(&snapshot);
    TEST_ASSERT_EQUAL_STRING("OFF", screen_handler.dashboard.text[DASHBOARD_FIELD_SLIP]);
}

void test_screen_api_sync_state_idle(void) {
    struct UIData snapshot = { 0 };
    snapshot.vehicle_state = UI_DATA_VEHICLE_STATE_IDLE;
    screen_api_sync_data(&snapshot);
    TEST_ASSERT_EQUAL_STRING("IDLE", screen_handler.dashboard.text[DASHBOARD_FIELD_CAR_STATE]);
}

void test_screen_api_sync_state_drive(void) {
    struct UIData snapshot = { 0 };
    snapshot.vehicle_state = UI_DATA_VEHICLE_STATE_DRIVE;
    screen_api_sync_data(&snapshot);
    TEST_ASSERT_EQUAL_STRING("DRIVE", screen_handler.dashboard.text[DASHBOARD_FIELD_CAR_STATE]);
}

void test_screen_api_sync_state_error(void) {
    struct UIData snapshot = { 0 };
    snapshot.vehicle_state = UI_DATA_VEHICLE_STATE_ERROR;
    screen_api_sync_data(&snapshot);
    TEST_ASSERT_EQUAL_STRING("ERROR", screen_handler.dashboard.text[DASHBOARD_FIELD_CAR_STATE]);
}

void test_screen_api_sync_state_unknown_falls_back(void) {
    struct UIData snapshot = { 0 };
    snapshot.vehicle_state = 200U; /* out of range */
    screen_api_sync_data(&snapshot);
    TEST_ASSERT_EQUAL_STRING_MESSAGE("----", screen_handler.dashboard.text[DASHBOARD_FIELD_CAR_STATE], "Unknown state must fall back to '----' rather than show garbage");
}

void test_screen_api_sync_pushes_hv_block(void) {
    struct UIData snapshot = prv_make_snapshot();
    screen_api_sync_data(&snapshot);
    TEST_ASSERT_EQUAL_STRING_MESSAGE("69%", screen_handler.dashboard.text[DASHBOARD_FIELD_HV_SOC], "SoC must reflect the snapshot");
    TEST_ASSERT_EQUAL_STRING_MESSAGE("104°C", screen_handler.dashboard.text[DASHBOARD_FIELD_HV_TEMPERATURE], "HV temp must reflect the snapshot");
    TEST_ASSERT_EQUAL_STRING_MESSAGE("22°C", screen_handler.dashboard.text[DASHBOARD_FIELD_INVERTER], "Inverter temp value carries the temperature without the 'INV' prefix");
}

void test_screen_api_sync_pushes_lap_block(void) {
    struct UIData snapshot = prv_make_snapshot();
    screen_api_sync_data(&snapshot);
    TEST_ASSERT_EQUAL_STRING_MESSAGE("9/11", screen_handler.dashboard.text[DASHBOARD_FIELD_LAP], "Lap value carries only 'current/total' (label box says 'LAP')");
    TEST_ASSERT_EQUAL_STRING_MESSAGE("-0.420", screen_handler.dashboard.text[DASHBOARD_FIELD_LAP_DELTA], "Lap delta must reflect the snapshot");
}

void test_screen_api_sync_pushes_tire_block(void) {
    struct UIData snapshot = prv_make_snapshot();
    screen_api_sync_data(&snapshot);
    TEST_ASSERT_EQUAL_STRING_MESSAGE("95°C", screen_handler.dashboard.text[DASHBOARD_FIELD_TIRES_FRONT_LEFT], "FL tire must reflect the snapshot");
    TEST_ASSERT_EQUAL_STRING_MESSAGE("60°C", screen_handler.dashboard.text[DASHBOARD_FIELD_TIRES_FRONT_RIGHT], "FR tire must reflect the snapshot");
    TEST_ASSERT_EQUAL_STRING_MESSAGE("10°C", screen_handler.dashboard.text[DASHBOARD_FIELD_TIRES_REAR_LEFT], "RL tire must reflect the snapshot");
    TEST_ASSERT_EQUAL_STRING_MESSAGE("40°C", screen_handler.dashboard.text[DASHBOARD_FIELD_TIRES_REAR_RIGHT], "RR tire must reflect the snapshot");
}

void test_screen_api_sync_pushes_motor_block(void) {
    struct UIData snapshot = prv_make_snapshot();
    screen_api_sync_data(&snapshot);
    TEST_ASSERT_EQUAL_STRING_MESSAGE("22°C", screen_handler.dashboard.text[DASHBOARD_FIELD_MOTORS_FRONT_LEFT], "FL motor must reflect the snapshot");
    TEST_ASSERT_EQUAL_STRING_MESSAGE("23°C", screen_handler.dashboard.text[DASHBOARD_FIELD_MOTORS_FRONT_RIGHT], "FR motor must reflect the snapshot");
    TEST_ASSERT_EQUAL_STRING_MESSAGE("24°C", screen_handler.dashboard.text[DASHBOARD_FIELD_MOTORS_REAR_LEFT], "RL motor must reflect the snapshot");
    TEST_ASSERT_EQUAL_STRING_MESSAGE("25°C", screen_handler.dashboard.text[DASHBOARD_FIELD_MOTORS_REAR_RIGHT], "RR motor must reflect the snapshot");
}

/*! \} */

/*!
 * \defgroup screen_api_sync_data No-op tests
 * \{
 */

void test_screen_api_sync_called_twice_keeps_dashboard_clean(void) {
    struct UIData snapshot = prv_make_snapshot();
    screen_api_sync_data(&snapshot);
    for (uint16_t i = 0U; i < DASHBOARD_FIELD_COUNT; i++) {
        screen_handler.dashboard.boxes[i].updated = false;
    }

    enum ScreenReturnCode rc = screen_api_sync_data(&snapshot);

    uint8_t updated_after_replay[DASHBOARD_FIELD_COUNT];
    for (uint16_t i = 0U; i < DASHBOARD_FIELD_COUNT; i++) {
        updated_after_replay[i] = screen_handler.dashboard.boxes[i].updated ? 1U : 0U;
    }

    TEST_ASSERT_EQUAL_MESSAGE(SCREEN_RC_OK, rc, "Second sync must still succeed");
    TEST_ASSERT_EACH_EQUAL_UINT8_MESSAGE(0U, updated_after_replay, DASHBOARD_FIELD_COUNT, "Replaying the same snapshot must not dirty any box");
}

void test_screen_api_sync_flags_only_changed_fields(void) {
    struct UIData snapshot = prv_make_snapshot();
    screen_api_sync_data(&snapshot);
    for (uint16_t i = 0U; i < DASHBOARD_FIELD_COUNT; i++) {
        screen_handler.dashboard.boxes[i].updated = false;
    }

    snapshot.soc = 70U; /* only SoC moves */
    screen_api_sync_data(&snapshot);

    TEST_ASSERT_TRUE_MESSAGE(screen_handler.dashboard.boxes[DASHBOARD_FIELD_HV_SOC].updated, "Changed field must be flagged");
    TEST_ASSERT_FALSE_MESSAGE(screen_handler.dashboard.boxes[DASHBOARD_FIELD_POWER].updated, "Unchanged power must stay clean");
    TEST_ASSERT_FALSE_MESSAGE(screen_handler.dashboard.boxes[DASHBOARD_FIELD_HV_TEMPERATURE].updated, "Unchanged HV temp must stay clean");
    TEST_ASSERT_FALSE_MESSAGE(screen_handler.dashboard.boxes[DASHBOARD_FIELD_LAP].updated, "Unchanged lap must stay clean");
}

void test_screen_api_sync_ptt_tints_scenario_header(void) {
    struct UIData snapshot = prv_make_snapshot();
    snapshot.ptt = 1U;
    screen_api_sync_data(&snapshot);
    TEST_ASSERT_EQUAL_UINT32(DASHBOARD_COLOR_PTT, screen_handler.dashboard.boxes[DASHBOARD_FIELD_SCENARIO_HEADER].color.argb);

    snapshot.ptt = 0U;
    screen_api_sync_data(&snapshot);
    TEST_ASSERT_EQUAL_UINT32(DASHBOARD_COLOR_PRIMARY, screen_handler.dashboard.boxes[DASHBOARD_FIELD_SCENARIO_HEADER].color.argb);
}

/*! \} */

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_screen_api_sync_null_pointer);
    RUN_TEST(test_screen_api_sync_empty_snapshot_succeeds);

    RUN_TEST(test_screen_api_sync_pushes_scenario_strip);
    RUN_TEST(test_screen_api_sync_slip_off_when_zero);

    RUN_TEST(test_screen_api_sync_state_idle);
    RUN_TEST(test_screen_api_sync_state_drive);
    RUN_TEST(test_screen_api_sync_state_error);
    RUN_TEST(test_screen_api_sync_state_unknown_falls_back);

    RUN_TEST(test_screen_api_sync_pushes_hv_block);
    RUN_TEST(test_screen_api_sync_pushes_lap_block);
    RUN_TEST(test_screen_api_sync_pushes_tire_block);
    RUN_TEST(test_screen_api_sync_pushes_motor_block);

    RUN_TEST(test_screen_api_sync_called_twice_keeps_dashboard_clean);
    RUN_TEST(test_screen_api_sync_flags_only_changed_fields);
    RUN_TEST(test_screen_api_sync_ptt_tints_scenario_header);

    return UNITY_END();
}
