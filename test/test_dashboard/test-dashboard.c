/*!
 * \file test-dashboard.c
 * \date 2026-06-07
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 *
 * \brief Unit tests for the dashboard API.
 */

#include "unity.h"
#include "dashboard-api.h"
#include "raster-fonts.h"
#include <string.h>

struct DashboardHandler dashboard_handler;

void setUp(void) {
    memset(&dashboard_handler, 0, sizeof(dashboard_handler));
    dashboard_api_init(&dashboard_handler);
    // Clear updated flags so each test only sees what the test itself triggered.
    for (uint16_t i = 0U; i < DASHBOARD_FIELD_COUNT; i++) {
        dashboard_handler.boxes[i].updated = false;
    }
}

/*!
 * \defgroup dashboard_api_init Test cases for dashboard_api_init function
 * \{
 */

void test_dashboard_init_success(void) {
    enum DashboardReturnCode rc = dashboard_api_init(&dashboard_handler);
    TEST_ASSERT_EQUAL_MESSAGE(DASHBOARD_RC_OK, rc, "init must return DASHBOARD_RC_OK on a valid handler");
}

void test_dashboard_init_null_handler(void) {
    enum DashboardReturnCode rc = dashboard_api_init(NULL);
    TEST_ASSERT_EQUAL(DASHBOARD_RC_NULL_POINTER, rc);
}

void test_dashboard_init_populates_every_box(void) {
    dashboard_api_init(&dashboard_handler);

    uint16_t expected_ids[DASHBOARD_FIELD_COUNT];
    uint16_t actual_ids[DASHBOARD_FIELD_COUNT];
    const struct Label *expected_labels[DASHBOARD_FIELD_COUNT];
    const struct Label *actual_labels[DASHBOARD_FIELD_COUNT];
    for (uint16_t i = 0U; i < DASHBOARD_FIELD_COUNT; i++) {
        expected_ids[i] = i;
        actual_ids[i] = dashboard_handler.boxes[i].id;
        actual_labels[i] = dashboard_handler.boxes[i].label;
        expected_labels[i] = (actual_labels[i] == NULL) ? NULL : &dashboard_handler.labels[i];
    }

    TEST_ASSERT_EQUAL_UINT16_ARRAY_MESSAGE(expected_ids, actual_ids, DASHBOARD_FIELD_COUNT, "Each box id must match its field index");
    TEST_ASSERT_EQUAL_PTR_ARRAY_MESSAGE(expected_labels, actual_labels, DASHBOARD_FIELD_COUNT, "Labelled boxes must reference their matching label slot; background boxes must carry a NULL label");
}

void test_dashboard_init_labels_point_at_internal_buffers(void) {
    dashboard_api_init(&dashboard_handler);

    for (uint16_t i = 0U; i < DASHBOARD_FIELD_COUNT; i++) {
        if (dashboard_handler.boxes[i].label == NULL) {
            continue;
        }
        TEST_ASSERT_EQUAL_PTR_MESSAGE(dashboard_handler.text[i], dashboard_handler.labels[i].text, "Each labelled box's label.text must reference the handler's per-field buffer");
        TEST_ASSERT_EQUAL_PTR_MESSAGE(&font_inter, dashboard_handler.labels[i].font, "All labels must use font_konexy");
    }
}

void test_dashboard_init_seeds_placeholder_text(void) {
    dashboard_api_init(&dashboard_handler);
    TEST_ASSERT_EQUAL_STRING_MESSAGE("SCENARIO", dashboard_handler.text[DASHBOARD_FIELD_SCENARIO_HEADER], "Scenario header carries its static label");
    TEST_ASSERT_EQUAL_STRING_MESSAGE("HV", dashboard_handler.text[DASHBOARD_FIELD_HV_HEADER], "HV header carries its static label");
    TEST_ASSERT_EQUAL_STRING_MESSAGE("TRS", dashboard_handler.text[DASHBOARD_FIELD_TIRES_HEADER], "TRS header carries its static label");
    TEST_ASSERT_EQUAL_STRING_MESSAGE("MTR", dashboard_handler.text[DASHBOARD_FIELD_MOTORS_HEADER], "MTR header carries its static label");
    TEST_ASSERT_EQUAL_STRING_MESSAGE("RGN", dashboard_handler.text[DASHBOARD_FIELD_REGEN_LABEL], "Regen inline label is initialised once");
    TEST_ASSERT_EQUAL_STRING_MESSAGE("TQ", dashboard_handler.text[DASHBOARD_FIELD_TORQUE_LABEL], "Torque inline label is initialised once");
    TEST_ASSERT_EQUAL_STRING_MESSAGE("POW", dashboard_handler.text[DASHBOARD_FIELD_POWER_LABEL], "Power inline label is initialised once");
    TEST_ASSERT_EQUAL_STRING_MESSAGE("SLIP", dashboard_handler.text[DASHBOARD_FIELD_SLIP_LABEL], "Slip inline label is initialised once");
    TEST_ASSERT_EQUAL_STRING_MESSAGE("INV", dashboard_handler.text[DASHBOARD_FIELD_INVERTER_LABEL], "Inverter inline label is initialised once");
    TEST_ASSERT_EQUAL_STRING_MESSAGE("LAP", dashboard_handler.text[DASHBOARD_FIELD_LAP_LABEL], "Lap inline label is initialised once");
}

void test_dashboard_init_box_count_matches_field_count(void) {
    TEST_ASSERT_EQUAL_MESSAGE(DASHBOARD_FIELD_COUNT, DASHBOARD_BOX_COUNT, "Box count macro must mirror field count enum");
}

/*! \} */

/*!
 * \defgroup dashboard_api_set_car_state Tests for dashboard_api_set_car_state
 * \{
 */

void test_dashboard_set_car_state_writes_text_and_flags_updated(void) {
    enum DashboardReturnCode rc = dashboard_api_set_car_state(&dashboard_handler, "DRIVE");
    TEST_ASSERT_EQUAL_MESSAGE(DASHBOARD_RC_OK, rc, "Setter must return OK on a valid call");
    TEST_ASSERT_EQUAL_STRING_MESSAGE("DRIVE", dashboard_handler.text[DASHBOARD_FIELD_CAR_STATE], "State buffer must hold the new text");
    TEST_ASSERT_TRUE_MESSAGE(dashboard_handler.boxes[DASHBOARD_FIELD_CAR_STATE].updated, "Box must be flagged updated after a real change");
}

void test_dashboard_set_car_fsm_state_null_handler(void) {
    enum DashboardReturnCode rc = dashboard_api_set_car_state(NULL, "DRIVE");
    TEST_ASSERT_EQUAL(DASHBOARD_RC_NULL_POINTER, rc);
}

void test_dashboard_set_car_fsm_state_null_text(void) {
    enum DashboardReturnCode rc = dashboard_api_set_car_state(&dashboard_handler, NULL);
    TEST_ASSERT_EQUAL(DASHBOARD_RC_NULL_POINTER, rc);
}

/*! \} */

/*!
 * \defgroup dashboard_api_set_numeric Tests for the power/regen/torque setters
 * \{
 */

void test_dashboard_set_power_formats_value(void) {
    dashboard_api_set_power(&dashboard_handler, 7U);
    TEST_ASSERT_EQUAL_STRING_MESSAGE("7", dashboard_handler.text[DASHBOARD_FIELD_POWER], "Power value field holds only the number; the 'POW' label is its own box");
    TEST_ASSERT_TRUE_MESSAGE(dashboard_handler.boxes[DASHBOARD_FIELD_POWER].updated, "Power box must be flagged updated");
}

void test_dashboard_set_regen_formats_value(void) {
    dashboard_api_set_regen(&dashboard_handler, 3U);
    TEST_ASSERT_EQUAL_STRING_MESSAGE("3", dashboard_handler.text[DASHBOARD_FIELD_REGEN], "Regen value field holds only the number; the 'RGN' label is its own box");
    TEST_ASSERT_TRUE_MESSAGE(dashboard_handler.boxes[DASHBOARD_FIELD_REGEN].updated, "Regen box must be flagged updated");
}

void test_dashboard_set_torque_formats_value(void) {
    dashboard_api_set_torque(&dashboard_handler, 10U);
    TEST_ASSERT_EQUAL_STRING_MESSAGE("10", dashboard_handler.text[DASHBOARD_FIELD_TORQUE], "Torque value field holds only the number; the 'TQ' label is its own box");
    TEST_ASSERT_TRUE_MESSAGE(dashboard_handler.boxes[DASHBOARD_FIELD_TORQUE].updated, "Torque box must be flagged updated");
}

void test_dashboard_set_power_null_handler(void) {
    TEST_ASSERT_EQUAL(DASHBOARD_RC_NULL_POINTER, dashboard_api_set_power(NULL, 5U));
}

/*! \} */

/*!
 * \defgroup dashboard_api_set_slip Tests for dashboard_api_set_slip
 * \{
 */

void test_dashboard_set_slip_on(void) {
    dashboard_api_set_slip(&dashboard_handler, true);
    TEST_ASSERT_EQUAL_STRING_MESSAGE("ON", dashboard_handler.text[DASHBOARD_FIELD_SLIP], "Slip value holds only 'ON'; the 'SLIP' label is its own box");
    TEST_ASSERT_TRUE_MESSAGE(dashboard_handler.boxes[DASHBOARD_FIELD_SLIP].updated, "Slip box must be flagged updated");
}

void test_dashboard_set_slip_off(void) {
    dashboard_api_set_slip(&dashboard_handler, false);
    TEST_ASSERT_EQUAL_STRING_MESSAGE("OFF", dashboard_handler.text[DASHBOARD_FIELD_SLIP], "Slip value holds only 'OFF'; the 'SLIP' label is its own box");
    TEST_ASSERT_TRUE_MESSAGE(dashboard_handler.boxes[DASHBOARD_FIELD_SLIP].updated, "Slip box must be flagged updated");
}

void test_dashboard_set_slip_null_handler(void) {
    TEST_ASSERT_EQUAL(DASHBOARD_RC_NULL_POINTER, dashboard_api_set_slip(NULL, true));
}

/*! \} */

/*!
 * \defgroup dashboard_api_set_soc Tests for dashboard_api_set_soc
 * \{
 */

void test_dashboard_set_soc_formats_percent(void) {
    dashboard_api_set_soc(&dashboard_handler, 69U);
    TEST_ASSERT_EQUAL_STRING_MESSAGE("69%", dashboard_handler.text[DASHBOARD_FIELD_HV_SOC], "SoC must format as '<percent>%'");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(DASHBOARD_COLOR_SECONDARY, dashboard_handler.labels[DASHBOARD_FIELD_HV_SOC].color.argb, "SoC color must be set to white");
    TEST_ASSERT_TRUE_MESSAGE(dashboard_handler.boxes[DASHBOARD_FIELD_HV_SOC].updated, "SoC box must be flagged updated");
}

void test_dashboard_set_soc_clamps_above_100(void) {
    dashboard_api_set_soc(&dashboard_handler, 200U);
    TEST_ASSERT_EQUAL_STRING_MESSAGE("100%", dashboard_handler.text[DASHBOARD_FIELD_HV_SOC], "Values above 100 must clamp to '100%%'");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(DASHBOARD_COLOR_SECONDARY, dashboard_handler.labels[DASHBOARD_FIELD_HV_SOC].color.argb, "SoC color must be set to white");
}

void test_dashboard_set_soc_zero(void) {
    dashboard_api_set_soc(&dashboard_handler, 0U);
    TEST_ASSERT_EQUAL_STRING("0%", dashboard_handler.text[DASHBOARD_FIELD_HV_SOC]);
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(DASHBOARD_COLOR_ERROR, dashboard_handler.labels[DASHBOARD_FIELD_HV_SOC].color.argb, "SoC color must be set to red");
}

/*! \} */

/*!
 * \defgroup dashboard_api_set_temps Tests for the temperature setters
 * \{
 */

void test_dashboard_set_hv_temperature_positive(void) {
    dashboard_api_set_hv_temperature(&dashboard_handler, 104);
    TEST_ASSERT_EQUAL_STRING_MESSAGE("104°C", dashboard_handler.text[DASHBOARD_FIELD_HV_TEMPERATURE], "HV temp must format as '<value>°C'");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(DASHBOARD_COLOR_ERROR, dashboard_handler.labels[DASHBOARD_FIELD_HV_TEMPERATURE].color.argb, "HV temp color must be set to red");
    TEST_ASSERT_TRUE_MESSAGE(dashboard_handler.boxes[DASHBOARD_FIELD_HV_TEMPERATURE].updated, "HV temp box must be flagged updated");
}

void test_dashboard_set_hv_temperature_negative(void) {
    dashboard_api_set_hv_temperature(&dashboard_handler, -5);
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(DASHBOARD_COLOR_SECONDARY, dashboard_handler.labels[DASHBOARD_FIELD_HV_TEMPERATURE].color.argb, "HV temp color must be set to white");
    TEST_ASSERT_EQUAL_STRING_MESSAGE("-5°C", dashboard_handler.text[DASHBOARD_FIELD_HV_TEMPERATURE], "HV temp must format negative values with a '-' sign as '<value>°C'");
}

void test_dashboard_set_inverter_temperature_formats_value(void) {
    dashboard_api_set_inverter_temperature(&dashboard_handler, 22);
    TEST_ASSERT_EQUAL_STRING_MESSAGE("22°C", dashboard_handler.text[DASHBOARD_FIELD_INVERTER], "Inverter value field holds only the temperature; the 'INV' label is its own box");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(DASHBOARD_COLOR_SECONDARY, dashboard_handler.labels[DASHBOARD_FIELD_INVERTER].color.argb, "Inverter temp color must be set to white");
    TEST_ASSERT_TRUE_MESSAGE(dashboard_handler.boxes[DASHBOARD_FIELD_INVERTER].updated, "INV box must be flagged updated");
}

/*! \} */

/*!
 * \defgroup dashboard_api_set_lap Tests for the lap counter and delta
 * \{
 */

void test_dashboard_set_lap_formats_current_over_total(void) {
    dashboard_api_set_lap(&dashboard_handler, 9U, 11U);
    TEST_ASSERT_EQUAL_STRING_MESSAGE("9/11", dashboard_handler.text[DASHBOARD_FIELD_LAP], "Lap value field holds only 'current/total'; the 'LAP' label is its own box");
    TEST_ASSERT_TRUE_MESSAGE(dashboard_handler.boxes[DASHBOARD_FIELD_LAP].updated, "Lap box must be flagged updated");
}

void test_dashboard_set_lap_delta_positive(void) {
    dashboard_api_set_lap_delta_ms(&dashboard_handler, 1234);
    TEST_ASSERT_EQUAL_STRING_MESSAGE("+1.234", dashboard_handler.text[DASHBOARD_FIELD_LAP_DELTA], "Positive delta must keep the '+' sign and 3-digit ms");
    TEST_ASSERT_TRUE_MESSAGE(dashboard_handler.boxes[DASHBOARD_FIELD_LAP_DELTA].updated, "Lap delta box must be flagged updated");
}

void test_dashboard_set_lap_delta_negative(void) {
    dashboard_api_set_lap_delta_ms(&dashboard_handler, -420);
    TEST_ASSERT_EQUAL_STRING_MESSAGE("-0.420", dashboard_handler.text[DASHBOARD_FIELD_LAP_DELTA], "Negative delta must use a '-' sign and keep 3-digit ms");
}

void test_dashboard_set_lap_delta_zero(void) {
    dashboard_api_set_lap_delta_ms(&dashboard_handler, 0);
    TEST_ASSERT_EQUAL_STRING("+0.000", dashboard_handler.text[DASHBOARD_FIELD_LAP_DELTA]);
}

/*! \} */

/*!
 * \defgroup dashboard_api_set_tire_motor_temps Tests for the four-temp setters
 * \{
 */

void test_dashboard_set_tire_temperatures_updates_all_four(void) {
    dashboard_api_set_tire_temperatures(&dashboard_handler, 95, 60, 10, 40);
    TEST_ASSERT_EQUAL_STRING_MESSAGE("95°C", dashboard_handler.text[DASHBOARD_FIELD_TIRES_FRONT_LEFT], "Front-left tire reads the FL argument");
    TEST_ASSERT_EQUAL_STRING_MESSAGE("60°C", dashboard_handler.text[DASHBOARD_FIELD_TIRES_FRONT_RIGHT], "Front-right tire reads the FR argument");
    TEST_ASSERT_EQUAL_STRING_MESSAGE("10°C", dashboard_handler.text[DASHBOARD_FIELD_TIRES_REAR_LEFT], "Rear-left tire reads the RL argument");
    TEST_ASSERT_EQUAL_STRING_MESSAGE("40°C", dashboard_handler.text[DASHBOARD_FIELD_TIRES_REAR_RIGHT], "Rear-right tire reads the RR argument");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(DASHBOARD_COLOR_WARNING, dashboard_handler.labels[DASHBOARD_FIELD_TIRES_FRONT_LEFT].color.argb, "Front left tire color must be set to yellow");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(DASHBOARD_COLOR_SECONDARY, dashboard_handler.labels[DASHBOARD_FIELD_TIRES_FRONT_RIGHT].color.argb, "Front right tire color must be set to white");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(DASHBOARD_COLOR_COLD_TIRES, dashboard_handler.labels[DASHBOARD_FIELD_TIRES_REAR_LEFT].color.argb, "Rear left tire color must be set to cyan");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(DASHBOARD_COLOR_SECONDARY, dashboard_handler.labels[DASHBOARD_FIELD_TIRES_REAR_RIGHT].color.argb, "Rear right tire color must be set to white");
    TEST_ASSERT_TRUE_MESSAGE(dashboard_handler.boxes[DASHBOARD_FIELD_TIRES_FRONT_LEFT].updated, "FL tire box must be flagged updated");
    TEST_ASSERT_TRUE_MESSAGE(dashboard_handler.boxes[DASHBOARD_FIELD_TIRES_REAR_RIGHT].updated, "RR tire box must be flagged updated");
}

void test_dashboard_set_motor_temperatures_updates_all_four(void) {
    dashboard_api_set_motor_temperatures(&dashboard_handler, 22, 23, 81, 105);
    TEST_ASSERT_EQUAL_STRING_MESSAGE("22°C", dashboard_handler.text[DASHBOARD_FIELD_MOTORS_FRONT_LEFT], "Front-left motor reads the FL argument");
    TEST_ASSERT_EQUAL_STRING_MESSAGE("23°C", dashboard_handler.text[DASHBOARD_FIELD_MOTORS_FRONT_RIGHT], "Front-right motor reads the FR argument");
    TEST_ASSERT_EQUAL_STRING_MESSAGE("81°C", dashboard_handler.text[DASHBOARD_FIELD_MOTORS_REAR_LEFT], "Rear-left motor reads the RL argument");
    TEST_ASSERT_EQUAL_STRING_MESSAGE("105°C", dashboard_handler.text[DASHBOARD_FIELD_MOTORS_REAR_RIGHT], "Rear-right motor reads the RR argument");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(DASHBOARD_COLOR_SECONDARY, dashboard_handler.labels[DASHBOARD_FIELD_MOTORS_FRONT_LEFT].color.argb, "Front left motor color must be set to white");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(DASHBOARD_COLOR_SECONDARY, dashboard_handler.labels[DASHBOARD_FIELD_MOTORS_FRONT_RIGHT].color.argb, "Front right motor color must be set to white");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(DASHBOARD_COLOR_WARNING, dashboard_handler.labels[DASHBOARD_FIELD_MOTORS_REAR_LEFT].color.argb, "Rear left motor color must be set to yellow");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(DASHBOARD_COLOR_ERROR, dashboard_handler.labels[DASHBOARD_FIELD_MOTORS_REAR_RIGHT].color.argb, "Rear right motor color must be set to red");
}

/*! \} */

/*!
 * \defgroup dashboard_api_set_ptt Tests for dashboard_api_set_ptt function
 * \{
 */

void test_dashboard_set_ptt_tints_scenario_header_blue(void) {
    enum DashboardReturnCode rc = dashboard_api_set_ptt(&dashboard_handler, true);
    TEST_ASSERT_EQUAL(DASHBOARD_RC_OK, rc);
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(DASHBOARD_COLOR_PTT, dashboard_handler.boxes[DASHBOARD_FIELD_SCENARIO_HEADER].color.argb, "PTT must paint the scenario header blue");
    TEST_ASSERT_TRUE_MESSAGE(dashboard_handler.boxes[DASHBOARD_FIELD_SCENARIO_HEADER].updated, "A fill-only change must still flag the box, or the raster skips it");
}

void test_dashboard_set_ptt_release_restores_primary_color(void) {
    dashboard_api_set_ptt(&dashboard_handler, true);
    dashboard_handler.boxes[DASHBOARD_FIELD_SCENARIO_HEADER].updated = false;

    dashboard_api_set_ptt(&dashboard_handler, false);

    TEST_ASSERT_EQUAL_UINT32(DASHBOARD_COLOR_PRIMARY, dashboard_handler.boxes[DASHBOARD_FIELD_SCENARIO_HEADER].color.argb);
    TEST_ASSERT_TRUE(dashboard_handler.boxes[DASHBOARD_FIELD_SCENARIO_HEADER].updated);
}

void test_dashboard_set_ptt_unchanged_does_not_re_flag(void) {
    dashboard_api_set_ptt(&dashboard_handler, true);
    dashboard_handler.boxes[DASHBOARD_FIELD_SCENARIO_HEADER].updated = false;

    dashboard_api_set_ptt(&dashboard_handler, true);

    TEST_ASSERT_FALSE_MESSAGE(dashboard_handler.boxes[DASHBOARD_FIELD_SCENARIO_HEADER].updated, "Re-applying the same PTT state must not force a redraw");
}

void test_dashboard_set_ptt_only_touches_scenario_header(void) {
    dashboard_api_set_ptt(&dashboard_handler, true);
    TEST_ASSERT_EQUAL_UINT32(DASHBOARD_COLOR_PRIMARY, dashboard_handler.boxes[DASHBOARD_FIELD_REGEN_BACKGROUND].color.argb);
    TEST_ASSERT_FALSE(dashboard_handler.boxes[DASHBOARD_FIELD_REGEN_BACKGROUND].updated);
}

void test_dashboard_set_ptt_null_handler(void) {
    TEST_ASSERT_EQUAL(DASHBOARD_RC_NULL_POINTER, dashboard_api_set_ptt(NULL, true));
}

/*! \} */

/*!
 * \defgroup dashboard_no_op Tests for the no-op skip optimization
 * \{
 */

void test_dashboard_setter_does_not_re_flag_when_value_unchanged(void) {
    dashboard_api_set_power(&dashboard_handler, 5U);
    dashboard_handler.boxes[DASHBOARD_FIELD_POWER].updated = false;

    dashboard_api_set_power(&dashboard_handler, 5U);

    TEST_ASSERT_FALSE_MESSAGE(dashboard_handler.boxes[DASHBOARD_FIELD_POWER].updated, "Setting the same value twice must not re-flag the box");
    TEST_ASSERT_EQUAL_STRING_MESSAGE("5", dashboard_handler.text[DASHBOARD_FIELD_POWER], "The text buffer must still hold the previous value");
}

void test_dashboard_setter_flags_on_real_change(void) {
    dashboard_api_set_power(&dashboard_handler, 5U);
    dashboard_handler.boxes[DASHBOARD_FIELD_POWER].updated = false;

    dashboard_api_set_power(&dashboard_handler, 6U);

    TEST_ASSERT_TRUE_MESSAGE(dashboard_handler.boxes[DASHBOARD_FIELD_POWER].updated, "A genuine change must flag the box as updated");
    TEST_ASSERT_EQUAL_STRING_MESSAGE("6", dashboard_handler.text[DASHBOARD_FIELD_POWER], "The text buffer must hold the new value");
}

void test_dashboard_setter_only_touches_target_box(void) {
    dashboard_api_set_power(&dashboard_handler, 5U);
    TEST_ASSERT_TRUE_MESSAGE(dashboard_handler.boxes[DASHBOARD_FIELD_POWER].updated, "Power box must be updated");
    TEST_ASSERT_FALSE_MESSAGE(dashboard_handler.boxes[DASHBOARD_FIELD_REGEN].updated, "Regen box must stay clean when only power changed");
    TEST_ASSERT_FALSE_MESSAGE(dashboard_handler.boxes[DASHBOARD_FIELD_TORQUE].updated, "Torque box must stay clean when only power changed");
}

/*! \} */

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_dashboard_init_success);
    RUN_TEST(test_dashboard_init_null_handler);
    RUN_TEST(test_dashboard_init_populates_every_box);
    RUN_TEST(test_dashboard_init_labels_point_at_internal_buffers);
    RUN_TEST(test_dashboard_init_seeds_placeholder_text);
    RUN_TEST(test_dashboard_init_box_count_matches_field_count);

    RUN_TEST(test_dashboard_set_car_state_writes_text_and_flags_updated);
    RUN_TEST(test_dashboard_set_car_fsm_state_null_handler);
    RUN_TEST(test_dashboard_set_car_fsm_state_null_text);

    RUN_TEST(test_dashboard_set_power_formats_value);
    RUN_TEST(test_dashboard_set_regen_formats_value);
    RUN_TEST(test_dashboard_set_torque_formats_value);
    RUN_TEST(test_dashboard_set_power_null_handler);

    RUN_TEST(test_dashboard_set_slip_on);
    RUN_TEST(test_dashboard_set_slip_off);
    RUN_TEST(test_dashboard_set_slip_null_handler);

    RUN_TEST(test_dashboard_set_soc_formats_percent);
    RUN_TEST(test_dashboard_set_soc_clamps_above_100);
    RUN_TEST(test_dashboard_set_soc_zero);

    RUN_TEST(test_dashboard_set_hv_temperature_positive);
    RUN_TEST(test_dashboard_set_hv_temperature_negative);
    RUN_TEST(test_dashboard_set_inverter_temperature_formats_value);

    RUN_TEST(test_dashboard_set_lap_formats_current_over_total);
    RUN_TEST(test_dashboard_set_lap_delta_positive);
    RUN_TEST(test_dashboard_set_lap_delta_negative);
    RUN_TEST(test_dashboard_set_lap_delta_zero);

    RUN_TEST(test_dashboard_set_tire_temperatures_updates_all_four);
    RUN_TEST(test_dashboard_set_motor_temperatures_updates_all_four);

    RUN_TEST(test_dashboard_set_ptt_tints_scenario_header_blue);
    RUN_TEST(test_dashboard_set_ptt_release_restores_primary_color);
    RUN_TEST(test_dashboard_set_ptt_unchanged_does_not_re_flag);
    RUN_TEST(test_dashboard_set_ptt_only_touches_scenario_header);
    RUN_TEST(test_dashboard_set_ptt_null_handler);

    RUN_TEST(test_dashboard_setter_does_not_re_flag_when_value_unchanged);
    RUN_TEST(test_dashboard_setter_flags_on_real_change);
    RUN_TEST(test_dashboard_setter_only_touches_target_box);

    return UNITY_END();
}
