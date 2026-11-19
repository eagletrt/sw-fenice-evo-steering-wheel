/*!
 * \file test-shutdown.c
 * \date 2026-11-19
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 *
 * \brief Unit tests for the shutdown chain evaluation.
 */

#include "unity.h"
#include "vehicle/shutdown/shutdown-api.h"

static float voltage[SHUTDOWN_NODE_COUNT];
static struct ShutdownChain chain;

#define TEST_SHUTDOWN_ALL ((uint32_t)((1U << SHUTDOWN_NODE_COUNT) - 1U))

void setUp(void) {
    for (uint8_t i = 0U; i < SHUTDOWN_NODE_COUNT; i++) {
        voltage[i] = 24.0F;
    }
}

void tearDown(void) {
}

void test_shutdown_rejects_null(void) {
    TEST_ASSERT_FALSE(shutdown_api_evaluate(NULL, 0U, &chain));
    TEST_ASSERT_FALSE(shutdown_api_evaluate(voltage, 0U, NULL));
}

void test_shutdown_all_closed_has_no_cause(void) {
    TEST_ASSERT_TRUE(shutdown_api_evaluate(voltage, TEST_SHUTDOWN_ALL, &chain));
    TEST_ASSERT_EQUAL(SHUTDOWN_NODE_COUNT, chain.cause);
    for (uint8_t i = 0U; i < SHUTDOWN_NODE_COUNT; i++) {
        TEST_ASSERT_EQUAL(SHUTDOWN_STATE_CLOSED, chain.state[i]);
    }
}

void test_shutdown_threshold_is_closed(void) {
    voltage[SHUTDOWN_NODE_AMS] = SHUTDOWN_CLOSED_V;
    shutdown_api_evaluate(voltage, TEST_SHUTDOWN_ALL, &chain);
    TEST_ASSERT_EQUAL(SHUTDOWN_STATE_CLOSED, chain.state[SHUTDOWN_NODE_AMS]);
}

void test_shutdown_first_open_node_is_the_cause_and_the_rest_downstream(void) {
    voltage[SHUTDOWN_NODE_BSPD] = 0.0F;
    voltage[SHUTDOWN_NODE_HVD] = 0.0F;
    voltage[SHUTDOWN_NODE_EBS] = 24.0F; // can't be closed past an open node
    shutdown_api_evaluate(voltage, TEST_SHUTDOWN_ALL, &chain);
    TEST_ASSERT_EQUAL(SHUTDOWN_NODE_BSPD, chain.cause);
    TEST_ASSERT_EQUAL(SHUTDOWN_STATE_CLOSED, chain.state[SHUTDOWN_NODE_INERTIA]);
    TEST_ASSERT_EQUAL(SHUTDOWN_STATE_OPEN, chain.state[SHUTDOWN_NODE_BSPD]);
    TEST_ASSERT_EQUAL(SHUTDOWN_STATE_DOWNSTREAM, chain.state[SHUTDOWN_NODE_HVD]);
    TEST_ASSERT_EQUAL(SHUTDOWN_STATE_DOWNSTREAM, chain.state[SHUTDOWN_NODE_EBS]);
}

void test_shutdown_unavailable_nodes_are_skipped(void) {
    const uint32_t available = TEST_SHUTDOWN_ALL & ~(SHUTDOWN_NODE_BIT(SHUTDOWN_NODE_IMD) | SHUTDOWN_NODE_BIT(SHUTDOWN_NODE_HVD));
    voltage[SHUTDOWN_NODE_IMD] = 0.0F; // no reading: whatever it holds is ignored
    voltage[SHUTDOWN_NODE_TSAC_INTERLOCK] = 0.0F;
    shutdown_api_evaluate(voltage, available, &chain);
    TEST_ASSERT_EQUAL(SHUTDOWN_STATE_UNAVAILABLE, chain.state[SHUTDOWN_NODE_IMD]);
    TEST_ASSERT_EQUAL_MESSAGE(SHUTDOWN_STATE_CLOSED, chain.state[SHUTDOWN_NODE_TSMS], "A hole doesn't open the rest of the chain");
    TEST_ASSERT_EQUAL(SHUTDOWN_STATE_UNAVAILABLE, chain.state[SHUTDOWN_NODE_HVD]);
    TEST_ASSERT_EQUAL(SHUTDOWN_NODE_TSAC_INTERLOCK, chain.cause);
    TEST_ASSERT_EQUAL(SHUTDOWN_STATE_DOWNSTREAM, chain.state[SHUTDOWN_NODE_ASMS]);
}

void test_shutdown_nothing_available_has_no_cause(void) {
    voltage[SHUTDOWN_NODE_AMS] = 0.0F;
    shutdown_api_evaluate(voltage, 0U, &chain);
    TEST_ASSERT_EQUAL(SHUTDOWN_NODE_COUNT, chain.cause);
    TEST_ASSERT_EQUAL(SHUTDOWN_STATE_UNAVAILABLE, chain.state[SHUTDOWN_NODE_AMS]);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_shutdown_rejects_null);
    RUN_TEST(test_shutdown_all_closed_has_no_cause);
    RUN_TEST(test_shutdown_threshold_is_closed);
    RUN_TEST(test_shutdown_first_open_node_is_the_cause_and_the_rest_downstream);
    RUN_TEST(test_shutdown_unavailable_nodes_are_skipped);
    RUN_TEST(test_shutdown_nothing_available_has_no_cause);
    return UNITY_END();
}
