/*!
 * \file test-dashboard.c
 * \date 2026-06-07
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 *
 * \brief Unit tests for the driver dashboard: layout, formatting, colours,
 *     stale values and the pedal bars.
 */

#include "unity.h"
#include "ui/dashboard/dashboard-api.h"
#include "vehicle/shutdown/shutdown-api.h"
#include "font-api.h"
#include "raster-fonts.h"
#include <string.h>

static struct DashboardHandler dashboard;
static struct UIData ui;

/* Clear every redraw flag, as a render does. */
static void prv_rendered(void) {
    for (uint8_t i = 0U; i < DASHBOARD_BOX_COUNT; i++) {
        dashboard.boxes[i].updated = false;
    }
}

static void prv_sync(void) {
    TEST_ASSERT_EQUAL(DASHBOARD_RC_OK, dashboard_api_sync(&dashboard, &ui));
}

static const char *prv_text(enum DashboardFieldId field) {
    return dashboard.text[field];
}

static uint32_t prv_text_color(enum DashboardFieldId field) {
    return dashboard.labels[field].color.argb;
}

static uint32_t prv_fill(enum DashboardFieldId field) {
    return dashboard.boxes[field].color.argb;
}

/* A plausible driving snapshot, every source fresh. */
static void prv_driving(void) {
    memset(&ui, 0, sizeof(ui));
    ui.vehicle_state = UI_DATA_VEHICLE_STATE_DRIVE;
    ui.soc = 69U;
    ui.hv_temp = 41;
    ui.lv_soc = 82U;
    ui.speed_kmh = 87U;
    for (uint8_t node = 0U; node < SHUTDOWN_NODE_COUNT; node++) {
        ui.shutdown_v[node] = 24.0F;
    }
    ui.inverter_temp = 52;
    ui.motor_fl_temp = 70;
    ui.motor_fr_temp = 74;
    ui.motor_rl_temp = 66;
    ui.motor_rr_temp = 68;
    ui.lap_current = 7U;
    ui.lap_delta_ms = -420;
    ui.power = 6U;
    ui.regen = 4U;
    ui.torque = 3U;
    ui.slip_on = 1U;
    ui.throttle_percent = 72U;
}

void setUp(void) {
    TEST_ASSERT_EQUAL(DASHBOARD_RC_OK, dashboard_api_init(&dashboard));
    prv_driving();
}

void tearDown(void) {
}

/*!
 * \defgroup dashboard_layout Tests for init and layout
 * \{
 */

void test_dashboard_init_rejects_null(void) {
    TEST_ASSERT_EQUAL(DASHBOARD_RC_NULL_POINTER, dashboard_api_init(NULL));
    TEST_ASSERT_EQUAL(DASHBOARD_RC_NULL_POINTER, dashboard_api_sync(NULL, &ui));
    TEST_ASSERT_EQUAL(DASHBOARD_RC_NULL_POINTER, dashboard_api_sync(&dashboard, NULL));
}

void test_dashboard_init_flags_every_box_inside_the_screen(void) {
    for (uint8_t i = 0U; i < DASHBOARD_BOX_COUNT; i++) {
        const struct BoxRectangle r = dashboard.boxes[i].rect;
        TEST_ASSERT_TRUE(dashboard.boxes[i].updated);
        TEST_ASSERT_TRUE_MESSAGE(r.width > 0U && r.height > 0U, "a box must have an area");
        TEST_ASSERT_TRUE(r.x + r.width <= 800U && r.y + r.height <= 480U);
    }
    TEST_ASSERT_EQUAL_MESSAGE(DASHBOARD_FIELD_BACKGROUND, 0, "the grid background is drawn first");
}

void test_dashboard_label_and_value_share_their_cell(void) {
    const enum DashboardFieldId pairs[][2] = {
        { DASHBOARD_FIELD_MISSION_LABEL, DASHBOARD_FIELD_MISSION },
        { DASHBOARD_FIELD_STATE_LABEL, DASHBOARD_FIELD_STATE },
        { DASHBOARD_FIELD_HV_LABEL, DASHBOARD_FIELD_HV_SOC },
        { DASHBOARD_FIELD_HV_SOC, DASHBOARD_FIELD_HV_TEMPERATURE },
        { DASHBOARD_FIELD_LV_LABEL, DASHBOARD_FIELD_LV_SOC },
        { DASHBOARD_FIELD_SPEED, DASHBOARD_FIELD_SPEED_UNIT },
        { DASHBOARD_FIELD_SHUTDOWN_LABEL, DASHBOARD_FIELD_SHUTDOWN },
        { DASHBOARD_FIELD_MOTOR_LABEL, DASHBOARD_FIELD_MOTOR },
        { DASHBOARD_FIELD_POWER_LABEL, DASHBOARD_FIELD_POWER },
        { DASHBOARD_FIELD_SLIP_LABEL, DASHBOARD_FIELD_SLIP },
        { DASHBOARD_FIELD_THROTTLE_LABEL, DASHBOARD_FIELD_THROTTLE },
    };
    for (size_t i = 0U; i < sizeof(pairs) / sizeof(pairs[0]); i++) {
        const struct BoxRectangle top = dashboard.boxes[pairs[i][0]].rect;
        const struct BoxRectangle bottom = dashboard.boxes[pairs[i][1]].rect;
        TEST_ASSERT_EQUAL_UINT16(top.x, bottom.x);
        TEST_ASSERT_EQUAL_UINT16(top.width, bottom.width);
        TEST_ASSERT_EQUAL_UINT16_MESSAGE(top.y + top.height, bottom.y, "the value box starts where the label box ends");
    }
}

/*! \} */

/*!
 * \defgroup dashboard_values Tests for formatting and colours
 * \{
 */

void test_dashboard_shows_a_driving_snapshot(void) {
    prv_sync();
    TEST_ASSERT_EQUAL_STRING("MANUAL", prv_text(DASHBOARD_FIELD_MISSION));
    TEST_ASSERT_EQUAL_STRING("DRIVE", prv_text(DASHBOARD_FIELD_STATE));
    TEST_ASSERT_EQUAL_HEX32(DASHBOARD_COLOR_ALL_OK, prv_text_color(DASHBOARD_FIELD_STATE));
    TEST_ASSERT_EQUAL_STRING("LAP 7", prv_text(DASHBOARD_FIELD_LAP_LABEL));
    TEST_ASSERT_EQUAL_STRING("-0.420", prv_text(DASHBOARD_FIELD_LAP_DELTA));
    TEST_ASSERT_EQUAL_STRING("69%", prv_text(DASHBOARD_FIELD_HV_SOC));
    TEST_ASSERT_EQUAL_STRING("41°C", prv_text(DASHBOARD_FIELD_HV_TEMPERATURE));
    TEST_ASSERT_EQUAL_STRING("82%", prv_text(DASHBOARD_FIELD_LV_SOC));
    TEST_ASSERT_EQUAL_STRING("87", prv_text(DASHBOARD_FIELD_SPEED));
    TEST_ASSERT_EQUAL_STRING("CLOSED", prv_text(DASHBOARD_FIELD_SHUTDOWN));
    TEST_ASSERT_EQUAL_STRING_MESSAGE("74°C", prv_text(DASHBOARD_FIELD_MOTOR), "MOTOR MAX is the hottest of the four");
    TEST_ASSERT_EQUAL_STRING("52°C", prv_text(DASHBOARD_FIELD_INVERTER));
    TEST_ASSERT_EQUAL_STRING_MESSAGE("--", prv_text(DASHBOARD_FIELD_BRAKE_BIAS), "no brake bias on the bus yet");
    TEST_ASSERT_EQUAL_STRING("6", prv_text(DASHBOARD_FIELD_POWER));
    TEST_ASSERT_EQUAL_STRING("4", prv_text(DASHBOARD_FIELD_REGEN));
    TEST_ASSERT_EQUAL_STRING("3", prv_text(DASHBOARD_FIELD_TORQUE));
    TEST_ASSERT_EQUAL_STRING("ON", prv_text(DASHBOARD_FIELD_SLIP));
    TEST_ASSERT_EQUAL_HEX32(DASHBOARD_COLOR_ALL_OK, prv_text_color(DASHBOARD_FIELD_SLIP));
    TEST_ASSERT_EQUAL_STRING("0%", prv_text(DASHBOARD_FIELD_BRAKE));
    TEST_ASSERT_EQUAL_STRING("72%", prv_text(DASHBOARD_FIELD_THROTTLE));
}

void test_dashboard_lap_delta_colours(void) {
    prv_sync();
    TEST_ASSERT_EQUAL_HEX32_MESSAGE(DASHBOARD_COLOR_ALL_OK, prv_text_color(DASHBOARD_FIELD_LAP_DELTA), "faster: green");
    ui.lap_delta_ms = 183;
    prv_sync();
    TEST_ASSERT_EQUAL_STRING("+0.183", prv_text(DASHBOARD_FIELD_LAP_DELTA));
    TEST_ASSERT_EQUAL_HEX32_MESSAGE(DASHBOARD_COLOR_ERROR, prv_text_color(DASHBOARD_FIELD_LAP_DELTA), "slower: red");
    ui.lap_delta_ms = 0;
    prv_sync();
    TEST_ASSERT_EQUAL_STRING("0.000", prv_text(DASHBOARD_FIELD_LAP_DELTA));
    TEST_ASSERT_EQUAL_HEX32(DASHBOARD_COLOR_VALUE, prv_text_color(DASHBOARD_FIELD_LAP_DELTA));
}

void test_dashboard_thresholds(void) {
    ui.soc = 18U;
    ui.hv_temp = 47;
    ui.motor_rr_temp = 100;
    ui.inverter_temp = 60;
    prv_sync();
    TEST_ASSERT_EQUAL_HEX32(DASHBOARD_COLOR_WARNING, prv_text_color(DASHBOARD_FIELD_HV_SOC));
    TEST_ASSERT_EQUAL_HEX32(DASHBOARD_COLOR_WARNING, prv_text_color(DASHBOARD_FIELD_HV_TEMPERATURE));
    TEST_ASSERT_EQUAL_HEX32(DASHBOARD_COLOR_ERROR, prv_text_color(DASHBOARD_FIELD_MOTOR));
    TEST_ASSERT_EQUAL_HEX32(DASHBOARD_COLOR_WARNING, prv_text_color(DASHBOARD_FIELD_INVERTER));
    ui.soc = 10U;
    prv_sync();
    TEST_ASSERT_EQUAL_HEX32(DASHBOARD_COLOR_ERROR, prv_text_color(DASHBOARD_FIELD_HV_SOC));
}

void test_dashboard_state_names_and_colours(void) {
    ui.vehicle_state = UI_DATA_VEHICLE_STATE_ERROR;
    prv_sync();
    TEST_ASSERT_EQUAL_STRING("ERROR", prv_text(DASHBOARD_FIELD_STATE));
    TEST_ASSERT_EQUAL_HEX32(DASHBOARD_COLOR_ERROR, prv_text_color(DASHBOARD_FIELD_STATE));
    ui.vehicle_state = UI_DATA_VEHICLE_STATE_IDLE;
    prv_sync();
    TEST_ASSERT_EQUAL_HEX32(DASHBOARD_COLOR_VALUE, prv_text_color(DASHBOARD_FIELD_STATE));
    ui.vehicle_state = UI_DATA_VEHICLE_STATE_COUNT;
    prv_sync();
    TEST_ASSERT_EQUAL_STRING_MESSAGE("----", prv_text(DASHBOARD_FIELD_STATE), "unknown ECU state");
}

void test_dashboard_open_shutdown_turns_the_cell_red(void) {
    ui.shutdown_v[SHUTDOWN_NODE_BSPD] = 0.0F;
    ui.shutdown_v[SHUTDOWN_NODE_HVD] = 0.0F;
    ui.shutdown_v[SHUTDOWN_NODE_SD_END] = 0.0F;
    prv_sync();
    TEST_ASSERT_EQUAL_STRING_MESSAGE("BSPD", prv_text(DASHBOARD_FIELD_SHUTDOWN), "The first open node, not the ones after it");
    TEST_ASSERT_EQUAL_HEX32(DASHBOARD_COLOR_ERROR, prv_fill(DASHBOARD_FIELD_SHUTDOWN));
    TEST_ASSERT_EQUAL_HEX32(DASHBOARD_COLOR_ERROR, prv_fill(DASHBOARD_FIELD_SHUTDOWN_LABEL));
    TEST_ASSERT_EQUAL_HEX32(DASHBOARD_COLOR_VALUE, prv_text_color(DASHBOARD_FIELD_SHUTDOWN_LABEL));

    ui.shutdown_v[SHUTDOWN_NODE_BSPD] = 24.0F;
    ui.shutdown_v[SHUTDOWN_NODE_HVD] = 24.0F;
    ui.shutdown_v[SHUTDOWN_NODE_SD_END] = 24.0F;
    prv_sync();
    TEST_ASSERT_EQUAL_STRING("CLOSED", prv_text(DASHBOARD_FIELD_SHUTDOWN));
    TEST_ASSERT_EQUAL_HEX32(DASHBOARD_COLOR_CELL, prv_fill(DASHBOARD_FIELD_SHUTDOWN_LABEL));
    TEST_ASSERT_EQUAL_HEX32(DASHBOARD_COLOR_ALL_OK, prv_text_color(DASHBOARD_FIELD_SHUTDOWN));
}

void test_dashboard_shutdown_agrees_with_the_chain_even_if_sd_end_reads_closed(void) {
    ui.shutdown_v[SHUTDOWN_NODE_HVD] = 0.0F; // sdEnd still reads 24 V: the SHUTDOWN tab says HVD open, so does the dashboard
    prv_sync();
    TEST_ASSERT_EQUAL_STRING("HVD", prv_text(DASHBOARD_FIELD_SHUTDOWN));
}

void test_dashboard_open_node_without_a_reading_shows_sd_end(void) {
    // An A-arm opened: it has no reading and BOTS (after it) is stale too, so sdEnd is the first open node seen.
    ui.stale = UI_DATA_STALE_BIT(UI_DATA_SOURCE_SHUTDOWN_EXTRA) | UI_DATA_STALE_BIT(UI_DATA_SOURCE_BRAKE);
    ui.shutdown_v[SHUTDOWN_NODE_BOTS] = 0.0F;
    ui.shutdown_v[SHUTDOWN_NODE_SD_END] = 0.0F;
    prv_sync();
    TEST_ASSERT_EQUAL_STRING("SD END", prv_text(DASHBOARD_FIELD_SHUTDOWN));
    TEST_ASSERT_EQUAL_HEX32(DASHBOARD_COLOR_ERROR, prv_fill(DASHBOARD_FIELD_SHUTDOWN));
}

void test_dashboard_every_shutdown_node_name_fits_the_cell(void) {
    const struct BoxRectangle cell = dashboard.boxes[DASHBOARD_FIELD_SHUTDOWN].rect;
    for (uint8_t node = 0U; node < SHUTDOWN_NODE_COUNT; node++) {
        struct FontBounds bounds = { 0 };
        TEST_ASSERT_EQUAL(RASTER_RC_OK, font_api_bounds(0U, 0U, FONT_ALIGN_LEFT, &font_inter, shutdown_api_node_name((enum ShutdownNode)node), 32U, &bounds));
        TEST_ASSERT_LESS_OR_EQUAL_UINT16_MESSAGE(cell.width - 16U, bounds.width, shutdown_api_node_name((enum ShutdownNode)node));
    }
}

void test_dashboard_ptt_turns_the_mission_cell_blue(void) {
    ui.ptt = 1U;
    prv_sync();
    TEST_ASSERT_EQUAL_HEX32(DASHBOARD_COLOR_PTT, prv_fill(DASHBOARD_FIELD_MISSION));
    TEST_ASSERT_EQUAL_HEX32(DASHBOARD_COLOR_PTT, prv_fill(DASHBOARD_FIELD_MISSION_LABEL));
    prv_rendered();
    ui.ptt = 0U;
    prv_sync();
    TEST_ASSERT_TRUE_MESSAGE(dashboard.boxes[DASHBOARD_FIELD_MISSION].updated, "only the fill changes: still redrawn");
    TEST_ASSERT_EQUAL_HEX32(DASHBOARD_COLOR_CELL, prv_fill(DASHBOARD_FIELD_MISSION));
}

void test_dashboard_stale_sources_show_dashes(void) {
    ui.stale = UI_DATA_STALE_ALL;
    prv_sync();
    const enum DashboardFieldId values[] = { DASHBOARD_FIELD_LAP_DELTA, DASHBOARD_FIELD_HV_SOC, DASHBOARD_FIELD_HV_TEMPERATURE, DASHBOARD_FIELD_LV_SOC, DASHBOARD_FIELD_SPEED, DASHBOARD_FIELD_SHUTDOWN, DASHBOARD_FIELD_MOTOR, DASHBOARD_FIELD_INVERTER, DASHBOARD_FIELD_BRAKE, DASHBOARD_FIELD_THROTTLE };
    for (size_t i = 0U; i < sizeof(values) / sizeof(values[0]); i++) {
        TEST_ASSERT_EQUAL_STRING("--", prv_text(values[i]));
    }
    TEST_ASSERT_EQUAL_STRING("LAP --", prv_text(DASHBOARD_FIELD_LAP_LABEL));
    TEST_ASSERT_EQUAL_HEX32_MESSAGE(DASHBOARD_COLOR_VALUE, prv_text_color(DASHBOARD_FIELD_BRAKE), "a stale pedal is not red");
    TEST_ASSERT_EQUAL_HEX32_MESSAGE(DASHBOARD_COLOR_CELL, prv_fill(DASHBOARD_FIELD_SHUTDOWN), "a silent shutdown is unknown, not open");
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(1U, dashboard.boxes[DASHBOARD_FIELD_THROTTLE_BAR].rect.width, "no bar for a stale pedal");
}

void test_dashboard_one_stale_source_leaves_the_others(void) {
    ui.stale = UI_DATA_STALE_BIT(UI_DATA_SOURCE_SPEED);
    prv_sync();
    TEST_ASSERT_EQUAL_STRING("--", prv_text(DASHBOARD_FIELD_SPEED));
    TEST_ASSERT_EQUAL_STRING("69%", prv_text(DASHBOARD_FIELD_HV_SOC));
}

void test_dashboard_same_snapshot_twice_flags_nothing(void) {
    prv_sync();
    prv_rendered();
    prv_sync();
    for (uint8_t i = 0U; i < DASHBOARD_BOX_COUNT; i++) {
        TEST_ASSERT_FALSE(dashboard.boxes[i].updated);
    }
}

void test_dashboard_one_change_flags_one_box(void) {
    prv_sync();
    prv_rendered();
    ui.lv_soc = 81U;
    prv_sync();
    for (uint8_t i = 0U; i < DASHBOARD_BOX_COUNT; i++) {
        TEST_ASSERT_EQUAL(i == DASHBOARD_FIELD_LV_SOC, dashboard.boxes[i].updated);
    }
}

/*! \} */

/*!
 * \defgroup dashboard_pedals Tests for the pedal bars
 * \{
 */

void test_dashboard_throttle_bar_grows_right_of_the_marker(void) {
    prv_sync();
    const struct Box *bar = &dashboard.boxes[DASHBOARD_FIELD_THROTTLE_BAR];
    TEST_ASSERT_EQUAL_UINT16(DASHBOARD_PEDAL_BAR_X, bar->rect.x);
    TEST_ASSERT_EQUAL_UINT16((72U * DASHBOARD_PEDAL_BAR_MAX) / 100U, bar->rect.width);
    TEST_ASSERT_EQUAL_HEX32(DASHBOARD_COLOR_ALL_OK, bar->color.argb);
}

void test_dashboard_brake_bar_grows_left_of_the_marker(void) {
    ui.brake_percent = 100U;
    prv_sync();
    const struct Box *bar = &dashboard.boxes[DASHBOARD_FIELD_BRAKE_BAR];
    TEST_ASSERT_EQUAL_UINT16(DASHBOARD_PEDAL_BAR_MAX, bar->rect.width);
    TEST_ASSERT_EQUAL_UINT16(DASHBOARD_PEDAL_CENTER_X - DASHBOARD_PEDAL_BAR_MAX, bar->rect.x);
    TEST_ASSERT_EQUAL_HEX32(DASHBOARD_COLOR_ERROR, bar->color.argb);
    TEST_ASSERT_EQUAL_STRING("100%", prv_text(DASHBOARD_FIELD_BRAKE));
}

void test_dashboard_released_pedal_leaves_a_hidden_bar(void) {
    prv_sync();
    const struct Box *bar = &dashboard.boxes[DASHBOARD_FIELD_BRAKE_BAR];
    TEST_ASSERT_EQUAL_UINT16(1U, bar->rect.width);
    TEST_ASSERT_EQUAL_HEX32_MESSAGE(DASHBOARD_COLOR_TRACK, bar->color.argb, "a 0 % bar is drawn in the track colour");
}

void test_dashboard_a_shrinking_bar_redraws_the_track(void) {
    prv_sync();
    prv_rendered();
    ui.throttle_percent = 30U;
    prv_sync();
    TEST_ASSERT_TRUE_MESSAGE(dashboard.boxes[DASHBOARD_FIELD_PEDAL_TRACK].updated, "the old bar pixels must be covered");
    TEST_ASSERT_TRUE(dashboard.boxes[DASHBOARD_FIELD_THROTTLE_BAR].updated);
    TEST_ASSERT_TRUE(dashboard.boxes[DASHBOARD_FIELD_BRAKE_BAR].updated);
    TEST_ASSERT_TRUE_MESSAGE(dashboard.boxes[DASHBOARD_FIELD_PEDAL_CENTER].updated, "the marker goes back on top");
}

/*! \} */

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_dashboard_init_rejects_null);
    RUN_TEST(test_dashboard_init_flags_every_box_inside_the_screen);
    RUN_TEST(test_dashboard_label_and_value_share_their_cell);

    RUN_TEST(test_dashboard_shows_a_driving_snapshot);
    RUN_TEST(test_dashboard_lap_delta_colours);
    RUN_TEST(test_dashboard_thresholds);
    RUN_TEST(test_dashboard_state_names_and_colours);
    RUN_TEST(test_dashboard_open_shutdown_turns_the_cell_red);
    RUN_TEST(test_dashboard_open_node_without_a_reading_shows_sd_end);
    RUN_TEST(test_dashboard_shutdown_agrees_with_the_chain_even_if_sd_end_reads_closed);
    RUN_TEST(test_dashboard_every_shutdown_node_name_fits_the_cell);
    RUN_TEST(test_dashboard_ptt_turns_the_mission_cell_blue);
    RUN_TEST(test_dashboard_stale_sources_show_dashes);
    RUN_TEST(test_dashboard_one_stale_source_leaves_the_others);
    RUN_TEST(test_dashboard_same_snapshot_twice_flags_nothing);
    RUN_TEST(test_dashboard_one_change_flags_one_box);

    RUN_TEST(test_dashboard_throttle_bar_grows_right_of_the_marker);
    RUN_TEST(test_dashboard_brake_bar_grows_left_of_the_marker);
    RUN_TEST(test_dashboard_released_pedal_leaves_a_hidden_bar);
    RUN_TEST(test_dashboard_a_shrinking_bar_redraws_the_track);

    return UNITY_END();
}
