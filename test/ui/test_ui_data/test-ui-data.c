/*!
 * \file test-ui-data.c
 * \date 2026-11-19
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 *
 * \brief Unit tests for the "max in the last 10 s" windows and the shutdown
 *     chain read from the UI data.
 */

#include "unity.h"
#include "ui/data/ui-data-api.h"

#include <float.h>
#include <stddef.h>

void setUp(void) {
}

void tearDown(void) {
}

void test_ui_data_window_keeps_the_highest_sample_for_10_seconds(void) {
    struct UIData *ui = ui_data_api_get();
    TEST_ASSERT_EQUAL_FLOAT_MESSAGE(-FLT_MAX, ui_data_api_window_max(&ui->powertrain.torque_window).fl, "Empty at boot");

    ui_data_api_window_push(&ui->powertrain.torque_window, &(struct UIDataCorners){ 24.0F, 1.0F, -5.0F, 3.0F });
    ui_data_api_window_push(&ui->powertrain.torque_window, &(struct UIDataCorners){ 10.0F, 2.0F, -7.0F, 3.0F });
    struct UIDataCorners max = ui_data_api_window_max(&ui->powertrain.torque_window);
    TEST_ASSERT_EQUAL_FLOAT(24.0F, max.fl);
    TEST_ASSERT_EQUAL_FLOAT(2.0F, max.fr);
    TEST_ASSERT_EQUAL_FLOAT_MESSAGE(-5.0F, max.rl, "Negative torque (regen) is a value too");

    for (uint8_t second = 1U; second < UI_DATA_WINDOW_SECONDS; second++) {
        ui_data_api_rotate_windows();
        ui_data_api_window_push(&ui->powertrain.torque_window, &(struct UIDataCorners){ 1.0F, 1.0F, -9.0F, 1.0F });
    }
    TEST_ASSERT_EQUAL_FLOAT_MESSAGE(24.0F, ui_data_api_window_max(&ui->powertrain.torque_window).fl, "Still inside the 10 s");
    ui_data_api_rotate_windows();
    TEST_ASSERT_EQUAL_FLOAT_MESSAGE(1.0F, ui_data_api_window_max(&ui->powertrain.torque_window).fl, "Older than 10 s: gone");
}

void test_ui_data_window_ignores_null(void) {
    ui_data_api_window_push(NULL, &(struct UIDataCorners){ 0 });
    ui_data_api_window_push(&ui_data_api_get()->powertrain.rpm_window, NULL);
}

void test_ui_data_shutdown_node_follows_its_message(void) {
    struct UIData ui = { 0 };
    for (uint8_t node = 0U; node < SHUTDOWN_NODE_COUNT; node++) {
        ui.shutdown_v[node] = 24.0F;
    }
    ui.stale = UI_DATA_STALE_BIT(UI_DATA_SOURCE_DAS_SHUTDOWN);
    struct ShutdownChain chain;
    TEST_ASSERT_TRUE(ui_data_api_shutdown(&ui, &chain));
    const enum ShutdownNode das[] = { SHUTDOWN_NODE_COCKPIT_MUSHROOM, SHUTDOWN_NODE_INERTIA, SHUTDOWN_NODE_LEFT_MUSHROOM, SHUTDOWN_NODE_RIGHT_MUSHROOM };
    for (size_t i = 0U; i < sizeof(das) / sizeof(das[0]); i++) {
        TEST_ASSERT_EQUAL_MESSAGE(SHUTDOWN_STATE_UNAVAILABLE, chain.state[das[i]], "DasShutdown nodes");
    }
    TEST_ASSERT_EQUAL(SHUTDOWN_STATE_CLOSED, chain.state[SHUTDOWN_NODE_FRONT_RIGHT_A_ARM]);
    TEST_ASSERT_EQUAL(SHUTDOWN_NODE_COUNT, chain.cause);
    TEST_ASSERT_FALSE(ui_data_api_shutdown(NULL, &chain));
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_ui_data_window_keeps_the_highest_sample_for_10_seconds);
    RUN_TEST(test_ui_data_window_ignores_null);
    RUN_TEST(test_ui_data_shutdown_node_follows_its_message);
    return UNITY_END();
}
