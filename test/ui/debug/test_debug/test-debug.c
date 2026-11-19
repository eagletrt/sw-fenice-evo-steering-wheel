/*!
 * \file test-debug.c
 * \date 2026-11-19
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 *
 * \brief Unit tests for the debug tabs: layout of every tab, tab cycling,
 *     wiring of every value and the colour rules.
 */

#include "unity.h"
#include "ui/data/ui-data-api.h"
#include "ui/debug/debug-api.h"
#include "ui/debug/debug-tabs.h"

#include "can-primary.h"
#include "can-version.h"

#include <string.h>

static struct DebugHandler debug;
static struct UIData ui;

static const char *prv_value(uint8_t slot) {
    TEST_ASSERT_NOT_EQUAL_MESSAGE(DEBUG_NO_SLOT, debug.slot_box[slot], "slot without a box");
    return debug.text[debug.slot_box[slot]];
}

static uint32_t prv_color(uint8_t slot) {
    return debug.labels[debug.slot_box[slot]].color.argb;
}

static void prv_show(enum DebugTab tab) {
    TEST_ASSERT_EQUAL(DEBUG_RC_OK, debug_api_set_tab(&debug, tab));
    TEST_ASSERT_EQUAL(DEBUG_RC_OK, debug_api_sync(&debug, &ui));
}

static unsigned prv_flagged(void) {
    unsigned count = 0U;
    for (uint16_t i = 0U; i < debug.box_count; i++) {
        count += debug.boxes[i].updated ? 1U : 0U;
    }
    return count;
}

static bool prv_overlap(struct BoxRectangle a, struct BoxRectangle b) {
    return a.x < b.x + b.width && b.x < a.x + a.width && a.y < b.y + b.height && b.y < a.y + a.height;
}

/* Every source fresh, every value plausible. */
static void prv_fill(void) {
    memset(&ui, 0, sizeof(ui));
    ui.vehicle_state = UI_DATA_VEHICLE_STATE_IDLE;
    ui.ecu_status = CAN_PRIMARY_ECUFSM_VEHICLESTATUS_IDLE;
    ui.soc = 69U;
    ui.lv_soc = 87U;
    ui.hv = (struct UIDataHv){ .cell_t_max = 38.0F, .cell_t_min = 31.0F, .cell_v_max = 4.12F, .cell_v_min = 4.05F, .pack_v = 512.0F, .ts_v = 509.0F, .current_a = 12.4F, .power_kw = 6.3F, .airp_closed = 1U, .airn_closed = 1U, .plausible = 1U, .tsal_green_v = 5.0F };
    for (uint8_t i = 0U; i < SHUTDOWN_NODE_COUNT; i++) {
        ui.shutdown_v[i] = 24.0F;
    }
    ui.pedals.pressure_bar = (struct UIDataCorners){ 12.1F, 12.0F, 9.8F, 9.9F };
    ui.pedals.bots_v = 24.0F;
    ui.powertrain.torque_window = (struct UIDataWindow){ 0 };
    ui.powertrain.rpm_window = (struct UIDataWindow){ 0 };
    ui_data_api_window_push(&ui.powertrain.torque_window, &(struct UIDataCorners){ 21.0F, 21.0F, 24.0F, 23.0F });
    ui_data_api_window_push(&ui.powertrain.rpm_window, &(struct UIDataCorners){ 18400.0F, 18350.0F, 17900.0F, 17950.0F });
    ui.wheel = (struct WheelHealth){ .can_tx_fifo_free = 32U, .can_tx_fifo_size = 32U, .window_ms = 5000U, .reset_cause = WHEEL_HEALTH_RESET_POR };
    ui.stale = 0U;
}

void setUp(void) {
    memset(&ui, 0, sizeof(ui));
    ui.vehicle_state = UI_DATA_VEHICLE_STATE_COUNT;
    ui.stale = UI_DATA_STALE_ALL;
    TEST_ASSERT_EQUAL(DEBUG_RC_OK, debug_api_init(&debug, debug_tabs));
}

void tearDown(void) {
}

void test_debug_init_rejects_null_and_opens_the_first_tab(void) {
    TEST_ASSERT_EQUAL(DEBUG_RC_NULL_POINTER, debug_api_init(NULL, debug_tabs));
    TEST_ASSERT_EQUAL(DEBUG_RC_NULL_POINTER, debug_api_init(&debug, NULL));
    TEST_ASSERT_EQUAL(DEBUG_RC_NULL_POINTER, debug_api_sync(&debug, NULL));
    TEST_ASSERT_EQUAL(DEBUG_RC_ERROR, debug_api_set_tab(&debug, DEBUG_TAB_COUNT));
    TEST_ASSERT_EQUAL(DEBUG_TAB_HV, debug.tab);
}

void test_debug_every_tab_fits_the_screen_and_its_texts_never_overlap(void) {
    for (uint8_t tab = 0U; tab < DEBUG_TAB_COUNT; tab++) {
        TEST_ASSERT_EQUAL_MESSAGE(DEBUG_RC_OK, debug_api_set_tab(&debug, (enum DebugTab)tab), debug_tabs[tab]->name);
        TEST_ASSERT_TRUE(debug.box_count <= DEBUG_MAX_BOXES);
        TEST_ASSERT_EQUAL_MESSAGE(debug.box_count, prv_flagged(), "A new tab is drawn whole");
        for (uint16_t i = 0U; i < debug.box_count; i++) {
            const struct BoxRectangle r = debug.boxes[i].rect;
            TEST_ASSERT_TRUE_MESSAGE(r.x + r.width <= STYLE_SCREEN_WIDTH && r.y + r.height <= STYLE_SCREEN_HEIGHT, debug_tabs[tab]->name);
            TEST_ASSERT_TRUE_MESSAGE(r.width > 0U && r.height > 0U, debug_tabs[tab]->name);
            for (uint16_t j = (uint16_t)(i + 1U); j < debug.box_count; j++) {
                // Panels lie under their texts; two texts must never share pixels.
                if (debug.boxes[i].label != NULL && debug.boxes[j].label != NULL) {
                    TEST_ASSERT_FALSE_MESSAGE(prv_overlap(r, debug.boxes[j].rect), debug_tabs[tab]->name);
                }
            }
        }
    }
}

void test_debug_tab_bar_names_the_neighbours(void) {
    // Boxes 1..4: previous, title, counter, next.
    TEST_ASSERT_EQUAL_STRING("< WHEEL", debug.text[1]);
    TEST_ASSERT_EQUAL_STRING("HV", debug.text[2]);
    TEST_ASSERT_EQUAL_STRING("1/13", debug.text[3]);
    TEST_ASSERT_EQUAL_STRING("MOTORS >", debug.text[4]);
    TEST_ASSERT_TRUE_MESSAGE(debug.boxes[3].rect.x > 420U, "The counter sits right of the centred title (Figma: 10 px after it)");
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(debug.boxes[3].rect.x, debug.boxes[2].rect.x + debug.boxes[2].rect.width, "Title and counter boxes tile the bar");
}

void test_debug_step_wraps_both_ways(void) {
    TEST_ASSERT_EQUAL(DEBUG_RC_OK, debug_api_step(&debug, -1));
    TEST_ASSERT_EQUAL(DEBUG_TAB_WHEEL, debug.tab);
    TEST_ASSERT_EQUAL_STRING("13/13", debug.text[3]);
    TEST_ASSERT_EQUAL_STRING("HV >", debug.text[4]);
    TEST_ASSERT_EQUAL(DEBUG_RC_OK, debug_api_step(&debug, 1));
    TEST_ASSERT_EQUAL(DEBUG_TAB_HV, debug.tab);
    TEST_ASSERT_EQUAL(DEBUG_RC_OK, debug_api_step(&debug, 2));
    TEST_ASSERT_EQUAL(DEBUG_TAB_COOLING, debug.tab);
    TEST_ASSERT_EQUAL(DEBUG_RC_NULL_POINTER, debug_api_step(NULL, 1));
}

void test_debug_stale_sources_show_dashes(void) {
    prv_show(DEBUG_TAB_HV);
    TEST_ASSERT_EQUAL_STRING(DEBUG_NO_VALUE, prv_value(0U));
    TEST_ASSERT_EQUAL_STRING(DEBUG_NO_VALUE, prv_value(15U));
    prv_show(DEBUG_TAB_SW);
    TEST_ASSERT_EQUAL_STRING_MESSAGE("OFFLINE", prv_value(2U), "A board never heard is offline");
}

void test_debug_fresh_data_fills_every_value_of_every_tab(void) {
    prv_fill();
    for (uint8_t tab = 0U; tab < DEBUG_TAB_COUNT; tab++) {
        prv_show((enum DebugTab)tab);
        for (uint8_t slot = 0U; slot < DEBUG_MAX_SLOTS; slot++) {
            if (debug.slot_box[slot] == DEBUG_NO_SLOT) {
                continue;
            }
            TEST_ASSERT_NULL_MESSAGE(strstr(prv_value(slot), DEBUG_NO_VALUE), debug_tabs[tab]->name);
        }
    }
}

void test_debug_unchanged_data_flags_nothing(void) {
    prv_fill();
    prv_show(DEBUG_TAB_COOLING);
    for (uint16_t i = 0U; i < debug.box_count; i++) {
        debug.boxes[i].updated = false;
    }
    debug_api_sync(&debug, &ui);
    TEST_ASSERT_EQUAL(0U, prv_flagged());
    ui.cooling.fan_percent[1] = 60.0F;
    debug_api_sync(&debug, &ui);
    TEST_ASSERT_EQUAL_MESSAGE(1U, prv_flagged(), "Only FAN R");
    TEST_ASSERT_EQUAL_STRING("60%", prv_value(3U));

    for (uint16_t i = 0U; i < debug.box_count; i++) {
        debug.boxes[i].updated = false;
    }
    debug_api_text(&debug, 3U, DEBUG_COLOR_ERROR, "60%%");
    TEST_ASSERT_EQUAL_MESSAGE(1U, prv_flagged(), "A colour change alone redraws the value");
}

void test_debug_hv_rules(void) {
    prv_fill();
    prv_show(DEBUG_TAB_HV);
    TEST_ASSERT_EQUAL_STRING("70 mV", prv_value(4U));
    TEST_ASSERT_EQUAL_STRING("ON/ON", prv_value(10U));
    TEST_ASSERT_EQUAL_HEX32(DEBUG_COLOR_OK, prv_color(10U));
    ui.hv.airp_mec_v = UI_DATA_AIR_CLOSED_V; // mechanically open while commanded closed: stuck
    ui.boards[UI_DATA_BOARD_TSAC].status = CAN_PRIMARY_TSACSTATUS_MAINBOARDSTATUS_PRECHARGE;
    ui.hv.imd_fault = 1U;
    ui.hv.tsal_green_v = 0.0F;
    debug_api_sync(&debug, &ui);
    TEST_ASSERT_EQUAL_STRING("ON/OFF", prv_value(10U));
    TEST_ASSERT_EQUAL_HEX32(DEBUG_COLOR_ERROR, prv_color(10U));
    TEST_ASSERT_EQUAL_STRING("ACTIVE", prv_value(12U));
    TEST_ASSERT_EQUAL_STRING("RED", prv_value(14U));
    TEST_ASSERT_EQUAL_STRING("FAULT/OK", prv_value(15U));
    TEST_ASSERT_EQUAL_HEX32(DEBUG_COLOR_ERROR, prv_color(15U));
    ui.boards[UI_DATA_BOARD_TSAC].status = CAN_PRIMARY_TSACSTATUS_MAINBOARDSTATUS_TS_ON;
    debug_api_sync(&debug, &ui);
    TEST_ASSERT_EQUAL_STRING("DONE", prv_value(12U));
}

void test_debug_motors_show_the_window_maxima(void) {
    prv_fill();
    prv_show(DEBUG_TAB_MOTORS);
    TEST_ASSERT_EQUAL_STRING("24 Nm", prv_value(2U));
    TEST_ASSERT_EQUAL_STRING("18400", prv_value(4U));
    TEST_ASSERT_EQUAL_HEX32_MESSAGE(DEBUG_COLOR_COLD, prv_color(16U), "A 0 °C tyre is cold");
}

/* SHUTDOWN tab: status and voltage slots of a node. */
#define TEST_DEBUG_STATUS(node) ((uint8_t)(2U * (node)))
#define TEST_DEBUG_VOLTAGE(node) ((uint8_t)((2U * (node)) + 1U))
#define TEST_DEBUG_ROW_COLOR(node, box) (debug.boxes[debug.slot_box[TEST_DEBUG_STATUS(node)] + (box)].color.argb)

void test_debug_shutdown_first_open_node_is_the_cause(void) {
    prv_fill();
    ui.shutdown_v[SHUTDOWN_NODE_REAR_LEFT_A_ARM] = 23.9F;
    ui.shutdown_v[SHUTDOWN_NODE_BSPD] = 0.0F;
    ui.shutdown_v[SHUTDOWN_NODE_HVD] = 0.0F;
    prv_show(DEBUG_TAB_SHUTDOWN);
    TEST_ASSERT_EQUAL_STRING("CLOSED", prv_value(TEST_DEBUG_STATUS(SHUTDOWN_NODE_REAR_LEFT_A_ARM)));
    TEST_ASSERT_EQUAL_STRING("23.9 V", prv_value(TEST_DEBUG_VOLTAGE(SHUTDOWN_NODE_REAR_LEFT_A_ARM)));
    TEST_ASSERT_EQUAL_STRING("OPEN", prv_value(TEST_DEBUG_STATUS(SHUTDOWN_NODE_BSPD)));
    TEST_ASSERT_EQUAL_STRING("0.0 V", prv_value(TEST_DEBUG_VOLTAGE(SHUTDOWN_NODE_BSPD)));
    TEST_ASSERT_EQUAL_HEX32_MESSAGE(DEBUG_COLOR_ERROR, TEST_DEBUG_ROW_COLOR(SHUTDOWN_NODE_BSPD, -1), "Name");
    TEST_ASSERT_EQUAL_HEX32_MESSAGE(DEBUG_COLOR_ERROR, TEST_DEBUG_ROW_COLOR(SHUTDOWN_NODE_BSPD, 0), "Status");
    TEST_ASSERT_EQUAL_HEX32_MESSAGE(DEBUG_COLOR_ERROR, TEST_DEBUG_ROW_COLOR(SHUTDOWN_NODE_BSPD, 1), "Voltage: the whole row");
    TEST_ASSERT_EQUAL_HEX32_MESSAGE(DEBUG_COLOR_LABEL, prv_color(TEST_DEBUG_STATUS(SHUTDOWN_NODE_HVD)), "Later open nodes are grey");
    TEST_ASSERT_EQUAL_HEX32(DEBUG_COLOR_CELL, TEST_DEBUG_ROW_COLOR(SHUTDOWN_NODE_HVD, 0));

    ui.shutdown_v[SHUTDOWN_NODE_BSPD] = 24.0F;
    debug_api_sync(&debug, &ui);
    TEST_ASSERT_EQUAL_HEX32_MESSAGE(DEBUG_COLOR_CELL, TEST_DEBUG_ROW_COLOR(SHUTDOWN_NODE_BSPD, 1), "Closed again: no red row");
    TEST_ASSERT_EQUAL_HEX32_MESSAGE(DEBUG_COLOR_ERROR, TEST_DEBUG_ROW_COLOR(SHUTDOWN_NODE_HVD, 0), "HVD is the cause now");
}

void test_debug_shutdown_missing_readings_do_not_break_the_chain(void) {
    prv_fill();
    ui.stale = UI_DATA_STALE_BIT(UI_DATA_SOURCE_DAS_SHUTDOWN) | UI_DATA_STALE_BIT(UI_DATA_SOURCE_SHUTDOWN_EXTRA);
    prv_show(DEBUG_TAB_SHUTDOWN);
    TEST_ASSERT_EQUAL_STRING("UNAVAIL", prv_value(TEST_DEBUG_STATUS(SHUTDOWN_NODE_COCKPIT_MUSHROOM)));
    TEST_ASSERT_EQUAL_STRING(DEBUG_NO_VALUE, prv_value(TEST_DEBUG_VOLTAGE(SHUTDOWN_NODE_COCKPIT_MUSHROOM)));
    TEST_ASSERT_EQUAL_STRING_MESSAGE("CLOSED", prv_value(TEST_DEBUG_STATUS(SHUTDOWN_NODE_BOTS)), "Past the hole the chain goes on");
    TEST_ASSERT_EQUAL_STRING("UNAVAIL", prv_value(TEST_DEBUG_STATUS(SHUTDOWN_NODE_LEFT_MUSHROOM)));
    TEST_ASSERT_EQUAL_STRING("CLOSED", prv_value(TEST_DEBUG_STATUS(SHUTDOWN_NODE_SD_END)));
}

void test_debug_sw_states(void) {
    prv_fill();
    ui.boards[UI_DATA_BOARD_PIPO] = (struct UIDataBoardInfo){ .commit = 0x3d2e9aaU, .libcan = { 17U, 0U, 0U }, .known = UI_DATA_BOARD_KNOWN_COMMIT | UI_DATA_BOARD_KNOWN_LIBCAN };
    ui.boards[UI_DATA_BOARD_ECU] = (struct UIDataBoardInfo){ .libcan = { (uint16_t)can_version_major, (uint16_t)can_version_minor, (uint16_t)can_version_patch }, .status = CAN_PRIMARY_ECUFSM_VEHICLESTATUS_AS_READY, .known = UI_DATA_BOARD_KNOWN_LIBCAN };
    ui.stale = UI_DATA_STALE_BIT(UI_DATA_SOURCE_STEERING_STATUS);
    prv_show(DEBUG_TAB_SW);
    TEST_ASSERT_EQUAL_STRING("AS READY", prv_value(2U));
    TEST_ASSERT_EQUAL_STRING("-", prv_value(1U));
    const uint8_t pipo = (uint8_t)(3U * UI_DATA_BOARD_PIPO);
    TEST_ASSERT_EQUAL_STRING("v17.0.0", prv_value(pipo));
    TEST_ASSERT_EQUAL_STRING("3d2e9aa", prv_value((uint8_t)(pipo + 1U)));
    TEST_ASSERT_EQUAL_STRING("CANLIB MISMATCH", prv_value((uint8_t)(pipo + 2U)));
    TEST_ASSERT_EQUAL_HEX32(DEBUG_COLOR_ERROR, prv_color(pipo));
    TEST_ASSERT_EQUAL_STRING("OFFLINE", prv_value((uint8_t)((3U * UI_DATA_BOARD_STEERING) + 2U)));
}

void test_debug_sw_telemetry_names_and_own_row(void) {
    prv_fill();
    ui.boards[UI_DATA_BOARD_TELEMETRY].status = CAN_PRIMARY_TELEMETRYFSM_STATUS_RUN;
    ui.boards[UI_DATA_BOARD_STEERING_WHEEL].status = CAN_PRIMARY_STEERINGWHEELFSM_STATUS_IDLE;
    ui.stale = UINT64_MAX & ~UI_DATA_STALE_BIT(UI_DATA_SOURCE_TELEMETRY_STATUS); // every bit, even past the last source
    prv_show(DEBUG_TAB_SW);
    TEST_ASSERT_EQUAL_STRING_MESSAGE("RUN", prv_value((uint8_t)((3U * UI_DATA_BOARD_TELEMETRY) + 2U)), "TelemetryFsm has its own names");
    TEST_ASSERT_EQUAL_STRING_MESSAGE("IDLE", prv_value((uint8_t)((3U * UI_DATA_BOARD_STEERING_WHEEL) + 2U)), "This board is never offline");
    TEST_ASSERT_EQUAL_STRING("OFFLINE", prv_value((uint8_t)((3U * UI_DATA_BOARD_DAS_REAR) + 2U)));
}

void test_debug_pipo_blown_fuse_is_red(void) {
    prv_fill();
    ui.lv.fuse_blown = 1U << UI_DATA_PIPO_LINE_LIGHTS;
    ui.lv.line_a[UI_DATA_PIPO_LINE_STEERING] = 0.4F;
    prv_show(DEBUG_TAB_PIPO);
    const uint8_t lights = (uint8_t)(2U + (2U * UI_DATA_PIPO_LINE_LIGHTS));
    TEST_ASSERT_EQUAL_STRING("FUSE BLOWN", prv_value(lights));
    TEST_ASSERT_EQUAL_HEX32(DEBUG_COLOR_ERROR, prv_color((uint8_t)(lights + 1U)));
    TEST_ASSERT_EQUAL_STRING("0.4 A", prv_value((uint8_t)(3U + (2U * UI_DATA_PIPO_LINE_STEERING))));
    prv_show(DEBUG_TAB_LV);
    TEST_ASSERT_EQUAL_STRING("1 BLOWN", prv_value(14U));
}

void test_debug_inverters_control_enable_is_one_for_all_four(void) {
    prv_fill();
    ui.stale = UI_DATA_STALE_BIT(UI_DATA_SOURCE_INVERTER_STATUS);
    prv_show(DEBUG_TAB_INVERTERS);
    TEST_ASSERT_EQUAL_STRING(DEBUG_NO_VALUE, prv_value(16U));
    ui.stale = 0U;
    ui.powertrain.control_enable = 1U;
    debug_api_sync(&debug, &ui);
    TEST_ASSERT_EQUAL_STRING("ON", prv_value(16U));
    TEST_ASSERT_EQUAL_HEX32(DEBUG_COLOR_OK, prv_color(16U));
    TEST_ASSERT_EQUAL_MESSAGE(DEBUG_NO_SLOT, debug.slot_box[18U], "A single value, not four");
}

void test_debug_brake_bias_comes_from_the_pedals_board(void) {
    prv_fill();
    ui.pedals.pressure_bar = (struct UIDataCorners){ 12.0F, 12.0F, 8.0F, 8.0F };
    ui.stale = UI_DATA_STALE_BIT(UI_DATA_SOURCE_BRAKE_BIAS);
    prv_show(DEBUG_TAB_PEDALS);
    TEST_ASSERT_EQUAL_STRING_MESSAGE(DEBUG_NO_VALUE, prv_value(10U), "Not computed from the pressures");
    TEST_ASSERT_EQUAL_STRING(DEBUG_NO_VALUE, prv_value(11U));
    ui.stale = 0U;
    ui.pedals.bias = 54.2F;
    ui.pedals.bias_set = 55.0F;
    debug_api_sync(&debug, &ui);
    TEST_ASSERT_EQUAL_STRING("54.2%", prv_value(10U));
    TEST_ASSERT_EQUAL_STRING("55.0%", prv_value(11U));
}

void test_debug_wheel_rules(void) {
    prv_fill();
    ui.wheel.can_bus_state = WHEEL_HEALTH_BUS_OFF;
    ui.wheel.can_tx_unjams = 1U;
    ui.wheel.can_tx_fifo_free = 0U;
    ui.wheel.can_last_error = 7U;
    ui.wheel.uptime_s = 3600U + (23U * 60U) + 59U;
    ui.wheel.reset_cause = WHEEL_HEALTH_RESET_WDG;
    ui.wheel.frames = 250U;
    ui.wheel.render_max_us = 1460U;
    prv_show(DEBUG_TAB_WHEEL);
    TEST_ASSERT_EQUAL_STRING("BUS-OFF", prv_value(0U));
    TEST_ASSERT_EQUAL_HEX32(DEBUG_COLOR_ERROR, prv_color(0U));
    TEST_ASSERT_EQUAL_STRING_MESSAGE("NONE", prv_value(2U), "7 = nothing new since the last read");
    TEST_ASSERT_EQUAL_HEX32(DEBUG_COLOR_ERROR, prv_color(4U));
    TEST_ASSERT_EQUAL_HEX32(DEBUG_COLOR_WARNING, prv_color(6U));
    TEST_ASSERT_EQUAL_STRING("50", prv_value(8U));
    TEST_ASSERT_EQUAL_STRING("1.4 ms", prv_value(10U));
    TEST_ASSERT_EQUAL_STRING("1 h 23 m", prv_value(14U));
    TEST_ASSERT_EQUAL_HEX32(DEBUG_COLOR_WARNING, prv_color(15U));
    ui.wheel.window_ms = 3U;
    ui.wheel.dropped_runs = 0U;
    debug_api_sync(&debug, &ui);
    TEST_ASSERT_EQUAL_STRING_MESSAGE("0 / 3 ms", prv_value(12U), "The boot window is shorter than a second");
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_debug_init_rejects_null_and_opens_the_first_tab);
    RUN_TEST(test_debug_every_tab_fits_the_screen_and_its_texts_never_overlap);
    RUN_TEST(test_debug_tab_bar_names_the_neighbours);
    RUN_TEST(test_debug_step_wraps_both_ways);
    RUN_TEST(test_debug_stale_sources_show_dashes);
    RUN_TEST(test_debug_fresh_data_fills_every_value_of_every_tab);
    RUN_TEST(test_debug_unchanged_data_flags_nothing);
    RUN_TEST(test_debug_hv_rules);
    RUN_TEST(test_debug_motors_show_the_window_maxima);
    RUN_TEST(test_debug_shutdown_first_open_node_is_the_cause);
    RUN_TEST(test_debug_shutdown_missing_readings_do_not_break_the_chain);
    RUN_TEST(test_debug_sw_states);
    RUN_TEST(test_debug_sw_telemetry_names_and_own_row);
    RUN_TEST(test_debug_pipo_blown_fuse_is_red);
    RUN_TEST(test_debug_inverters_control_enable_is_one_for_all_four);
    RUN_TEST(test_debug_brake_bias_comes_from_the_pedals_board);
    RUN_TEST(test_debug_wheel_rules);
    return UNITY_END();
}
