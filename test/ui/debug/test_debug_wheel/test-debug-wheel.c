/*!
 * \file test-debug-wheel.c
 * \date 2026-11-17
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 *
 * \brief Unit tests for the WHEEL debug page: layout, formatting and colours.
 */

#include "unity.h"
#include "ui/debug/wheel/debug-wheel-api.h"

static struct DebugWheelHandler page;
static struct WheelHealth health;

static const char *prv_value(enum DebugWheelCell cell) {
    return page.text[DEBUG_WHEEL_BOX_VALUE(cell)];
}

static uint32_t prv_color(enum DebugWheelCell cell) {
    return page.labels[DEBUG_WHEEL_BOX_VALUE(cell)].color.argb;
}

static void prv_rendered(void) {
    for (uint8_t i = 0U; i < DEBUG_WHEEL_BOX_COUNT; i++) {
        page.boxes[i].updated = false;
    }
}

static unsigned prv_flagged(void) {
    unsigned count = 0U;
    for (uint8_t i = 0U; i < DEBUG_WHEEL_BOX_COUNT; i++) {
        count += page.boxes[i].updated ? 1U : 0U;
    }
    return count;
}

static void prv_sync(void) {
    TEST_ASSERT_EQUAL(DEBUG_WHEEL_RC_OK, debug_wheel_api_sync(&page, &health));
}

void setUp(void) {
    TEST_ASSERT_EQUAL(DEBUG_WHEEL_RC_OK, debug_wheel_api_init(&page));
    health = (struct WheelHealth){ .can_tx_fifo_free = 32U, .can_tx_fifo_size = 32U, .window_ms = 5000U, .reset_cause = WHEEL_HEALTH_RESET_POR };
}

void tearDown(void) {
}

void test_debug_wheel_init_rejects_null_and_starts_with_no_values(void) {
    TEST_ASSERT_EQUAL(DEBUG_WHEEL_RC_NULL_POINTER, debug_wheel_api_init(NULL));
    TEST_ASSERT_EQUAL(DEBUG_WHEEL_RC_NULL_POINTER, debug_wheel_api_sync(NULL, &health));
    TEST_ASSERT_EQUAL(DEBUG_WHEEL_RC_NULL_POINTER, debug_wheel_api_sync(&page, NULL));
    TEST_ASSERT_EQUAL(DEBUG_WHEEL_BOX_COUNT, prv_flagged());
    TEST_ASSERT_EQUAL_STRING(DEBUG_WHEEL_NO_VALUE, prv_value(DEBUG_WHEEL_CELL_CAN_BUS));
    TEST_ASSERT_EQUAL_STRING("BUS-OFFS", page.text[DEBUG_WHEEL_BOX_LABEL(DEBUG_WHEEL_CELL_BUS_OFFS)]);
}

void test_debug_wheel_layout_matches_the_design(void) {
    const struct BoxRectangle first = page.boxes[DEBUG_WHEEL_BOX_LABEL(0U)].rect;
    TEST_ASSERT_EQUAL_UINT16(1U, first.x);
    TEST_ASSERT_EQUAL_UINT16(57U, first.y);
    const struct BoxRectangle last = page.boxes[DEBUG_WHEEL_BOX_VALUE(DEBUG_WHEEL_CELL_LAST_RESET)].rect;
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(799U, last.x + last.width, "Last column ends 1 px from the edge");
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(479U, last.y + last.height, "Last row ends 1 px from the bottom");
    for (uint8_t cell = 0U; cell < DEBUG_WHEEL_CELL_COUNT; cell++) {
        const struct BoxRectangle label = page.boxes[DEBUG_WHEEL_BOX_LABEL(cell)].rect;
        const struct BoxRectangle value = page.boxes[DEBUG_WHEEL_BOX_VALUE(cell)].rect;
        TEST_ASSERT_EQUAL_UINT16_MESSAGE(label.y + label.height, value.y, "Caption and value make one cell");
        TEST_ASSERT_EQUAL_UINT16(DEBUG_WHEEL_CELL_HEIGHT, label.height + value.height);
    }
}

void test_debug_wheel_bus_state_colours(void) {
    prv_sync();
    TEST_ASSERT_EQUAL_STRING("ACTIVE", prv_value(DEBUG_WHEEL_CELL_CAN_BUS));
    TEST_ASSERT_EQUAL_HEX32(DEBUG_WHEEL_COLOR_OK, prv_color(DEBUG_WHEEL_CELL_CAN_BUS));
    health.can_bus_state = WHEEL_HEALTH_BUS_PASSIVE;
    prv_sync();
    TEST_ASSERT_EQUAL_HEX32(DEBUG_WHEEL_COLOR_WARNING, prv_color(DEBUG_WHEEL_CELL_CAN_BUS));
    health.can_bus_state = WHEEL_HEALTH_BUS_OFF;
    prv_sync();
    TEST_ASSERT_EQUAL_STRING("BUS-OFF", prv_value(DEBUG_WHEEL_CELL_CAN_BUS));
    TEST_ASSERT_EQUAL_HEX32(DEBUG_WHEEL_COLOR_ERROR, prv_color(DEBUG_WHEEL_CELL_CAN_BUS));
}

void test_debug_wheel_counters_turn_yellow_above_zero(void) {
    prv_sync();
    TEST_ASSERT_EQUAL_STRING("0 / 0", prv_value(DEBUG_WHEEL_CELL_TEC_REC));
    TEST_ASSERT_EQUAL_HEX32(DEBUG_WHEEL_COLOR_VALUE, prv_color(DEBUG_WHEEL_CELL_TX_UNJAMS));
    health.can_receive_errors = 3U;
    health.can_tx_unjams = 1U;
    health.dropped_runs = 48U;
    prv_sync();
    TEST_ASSERT_EQUAL_STRING("0 / 3", prv_value(DEBUG_WHEEL_CELL_TEC_REC));
    TEST_ASSERT_EQUAL_HEX32(DEBUG_WHEEL_COLOR_WARNING, prv_color(DEBUG_WHEEL_CELL_TEC_REC));
    TEST_ASSERT_EQUAL_STRING("1", prv_value(DEBUG_WHEEL_CELL_TX_UNJAMS));
    TEST_ASSERT_EQUAL_HEX32(DEBUG_WHEEL_COLOR_WARNING, prv_color(DEBUG_WHEEL_CELL_TX_UNJAMS));
    TEST_ASSERT_EQUAL_STRING("48 / 5 s", prv_value(DEBUG_WHEEL_CELL_DROPPED_RUNS));
    TEST_ASSERT_EQUAL_HEX32(DEBUG_WHEEL_COLOR_WARNING, prv_color(DEBUG_WHEEL_CELL_DROPPED_RUNS));
}

void test_debug_wheel_full_tx_fifo_is_red(void) {
    prv_sync();
    TEST_ASSERT_EQUAL_STRING("32/32", prv_value(DEBUG_WHEEL_CELL_TX_FIFO_FREE));
    TEST_ASSERT_EQUAL_HEX32(DEBUG_WHEEL_COLOR_VALUE, prv_color(DEBUG_WHEEL_CELL_TX_FIFO_FREE));
    health.can_tx_fifo_free = 0U;
    prv_sync();
    TEST_ASSERT_EQUAL_HEX32(DEBUG_WHEEL_COLOR_ERROR, prv_color(DEBUG_WHEEL_CELL_TX_FIFO_FREE));
}

void test_debug_wheel_last_error_code_names(void) {
    health.can_last_error = 7U;
    prv_sync();
    TEST_ASSERT_EQUAL_STRING_MESSAGE("NONE", prv_value(DEBUG_WHEEL_CELL_LAST_ERROR), "7 = nothing new since the last read");
    TEST_ASSERT_EQUAL_HEX32(DEBUG_WHEEL_COLOR_VALUE, prv_color(DEBUG_WHEEL_CELL_LAST_ERROR));
    health.can_last_error = 3U;
    prv_sync();
    TEST_ASSERT_EQUAL_STRING("ACK", prv_value(DEBUG_WHEEL_CELL_LAST_ERROR));
    TEST_ASSERT_EQUAL_HEX32(DEBUG_WHEEL_COLOR_WARNING, prv_color(DEBUG_WHEEL_CELL_LAST_ERROR));
}

void test_debug_wheel_runtime_formatting(void) {
    health.frames = 250U;
    health.frame_max_us = 20850U;
    health.render_max_us = 1460U;
    health.inputs_max_us = 899U;
    prv_sync();
    TEST_ASSERT_EQUAL_STRING("50", prv_value(DEBUG_WHEEL_CELL_FPS));
    TEST_ASSERT_EQUAL_STRING("20.8 ms", prv_value(DEBUG_WHEEL_CELL_FRAME_MAX));
    TEST_ASSERT_EQUAL_STRING("1.4 ms", prv_value(DEBUG_WHEEL_CELL_RENDER_CPU));
    TEST_ASSERT_EQUAL_STRING("0.8 ms", prv_value(DEBUG_WHEEL_CELL_INPUTS_MAX));
    health.window_ms = 0U;
    prv_sync();
    TEST_ASSERT_EQUAL_STRING_MESSAGE(DEBUG_WHEEL_NO_VALUE, prv_value(DEBUG_WHEEL_CELL_FPS), "No window yet, no division by zero");
    health.window_ms = 3U;
    prv_sync();
    TEST_ASSERT_EQUAL_STRING_MESSAGE("0 / 3 ms", prv_value(DEBUG_WHEEL_CELL_DROPPED_RUNS), "The boot window is shorter than a second");
}

void test_debug_wheel_uptime_units(void) {
    health.uptime_s = 59U;
    prv_sync();
    TEST_ASSERT_EQUAL_STRING("59 s", prv_value(DEBUG_WHEEL_CELL_UPTIME));
    health.uptime_s = 125U;
    prv_sync();
    TEST_ASSERT_EQUAL_STRING("2 m 5 s", prv_value(DEBUG_WHEEL_CELL_UPTIME));
    health.uptime_s = (3600U * 1U) + (23U * 60U) + 59U;
    prv_sync();
    TEST_ASSERT_EQUAL_STRING("1 h 23 m", prv_value(DEBUG_WHEEL_CELL_UPTIME));
}

void test_debug_wheel_unexpected_resets_are_yellow(void) {
    prv_sync();
    TEST_ASSERT_EQUAL_STRING("POR", prv_value(DEBUG_WHEEL_CELL_LAST_RESET));
    TEST_ASSERT_EQUAL_HEX32(DEBUG_WHEEL_COLOR_VALUE, prv_color(DEBUG_WHEEL_CELL_LAST_RESET));
    health.reset_cause = WHEEL_HEALTH_RESET_BOR;
    prv_sync();
    TEST_ASSERT_EQUAL_HEX32(DEBUG_WHEEL_COLOR_WARNING, prv_color(DEBUG_WHEEL_CELL_LAST_RESET));
    health.reset_cause = WHEEL_HEALTH_RESET_UNKNOWN;
    prv_sync();
    TEST_ASSERT_EQUAL_STRING(DEBUG_WHEEL_NO_VALUE, prv_value(DEBUG_WHEEL_CELL_LAST_RESET));
}

void test_debug_wheel_same_snapshot_draws_nothing(void) {
    prv_sync();
    prv_rendered();
    prv_sync();
    TEST_ASSERT_EQUAL(0U, prv_flagged());
    health.uptime_s = 1U;
    prv_sync();
    TEST_ASSERT_EQUAL_MESSAGE(1U, prv_flagged(), "Only the changed value");
    TEST_ASSERT_TRUE(page.boxes[DEBUG_WHEEL_BOX_VALUE(DEBUG_WHEEL_CELL_UPTIME)].updated);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_debug_wheel_init_rejects_null_and_starts_with_no_values);
    RUN_TEST(test_debug_wheel_layout_matches_the_design);
    RUN_TEST(test_debug_wheel_bus_state_colours);
    RUN_TEST(test_debug_wheel_counters_turn_yellow_above_zero);
    RUN_TEST(test_debug_wheel_full_tx_fifo_is_red);
    RUN_TEST(test_debug_wheel_last_error_code_names);
    RUN_TEST(test_debug_wheel_runtime_formatting);
    RUN_TEST(test_debug_wheel_uptime_units);
    RUN_TEST(test_debug_wheel_unexpected_resets_are_yellow);
    RUN_TEST(test_debug_wheel_same_snapshot_draws_nothing);
    return UNITY_END();
}
