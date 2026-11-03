/*!
 * \file test-mcp23017.c
 * \date 2026-10-29
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 *
 * \brief Unit tests for the MCP23017 expander driver.
 *
 * \details The expander is a fake register file behind the read/write
 *     callbacks, so a test can reset it or break the bus.
 */

#include "unity.h"
#include "mcp23017-api.h"
#include <stdbool.h>
#include <string.h>

#define TEST_ADDRESS (0x4EU)
#define TEST_REGISTER_COUNT (0x16U)

static uint8_t prv_registers[TEST_REGISTER_COUNT];
static bool prv_bus_broken;
static uint8_t prv_last_address;

static enum Mcp23017ReturnCode prv_read_register(uint8_t device_address, uint8_t register_address, uint8_t *value) {
    prv_last_address = device_address;
    if (prv_bus_broken || register_address >= TEST_REGISTER_COUNT) {
        return MCP23017_RC_BUS_ERROR;
    }
    *value = prv_registers[register_address];
    return MCP23017_RC_OK;
}

static enum Mcp23017ReturnCode prv_write_register(uint8_t device_address, uint8_t register_address, uint8_t value) {
    prv_last_address = device_address;
    if (prv_bus_broken || register_address >= TEST_REGISTER_COUNT) {
        return MCP23017_RC_BUS_ERROR;
    }
    prv_registers[register_address] = value;
    return MCP23017_RC_OK;
}

static struct Mcp23017Handler prv_handler;

void setUp(void) {
    memset(prv_registers, 0, sizeof(prv_registers));
    memset(&prv_handler, 0, sizeof(prv_handler));
    prv_bus_broken = false;
    prv_last_address = 0U;
}

void tearDown(void) {}

void test_mcp23017_init_enables_every_pull_up(void) {
    TEST_ASSERT_EQUAL(MCP23017_RC_OK, mcp23017_api_init(&prv_handler, TEST_ADDRESS, prv_read_register, prv_write_register));

    TEST_ASSERT_EQUAL_HEX8(MCP23017_PULL_UP_ALL, prv_registers[MCP23017_REGISTER_GPPUA]);
    TEST_ASSERT_EQUAL_HEX8(MCP23017_PULL_UP_ALL, prv_registers[MCP23017_REGISTER_GPPUB]);
    TEST_ASSERT_EQUAL_HEX8(TEST_ADDRESS, prv_last_address);
}

void test_mcp23017_init_reports_a_broken_bus(void) {
    prv_bus_broken = true;

    TEST_ASSERT_EQUAL(MCP23017_RC_BUS_ERROR, mcp23017_api_init(&prv_handler, TEST_ADDRESS, prv_read_register, prv_write_register));
}

void test_mcp23017_check_config_passes_after_init(void) {
    TEST_ASSERT_EQUAL(MCP23017_RC_OK, mcp23017_api_init(&prv_handler, TEST_ADDRESS, prv_read_register, prv_write_register));

    TEST_ASSERT_EQUAL(MCP23017_RC_OK, mcp23017_api_check_config(&prv_handler));
}

void test_mcp23017_check_config_detects_a_reset_expander(void) {
    TEST_ASSERT_EQUAL(MCP23017_RC_OK, mcp23017_api_init(&prv_handler, TEST_ADDRESS, prv_read_register, prv_write_register));
    // A power-on reset clears the pull-ups.
    prv_registers[MCP23017_REGISTER_GPPUA] = 0U;

    TEST_ASSERT_EQUAL(MCP23017_RC_VERIFY_ERROR, mcp23017_api_check_config(&prv_handler));
}

void test_mcp23017_check_config_checks_port_b_too(void) {
    TEST_ASSERT_EQUAL(MCP23017_RC_OK, mcp23017_api_init(&prv_handler, TEST_ADDRESS, prv_read_register, prv_write_register));
    prv_registers[MCP23017_REGISTER_GPPUB] = 0x7FU;

    TEST_ASSERT_EQUAL(MCP23017_RC_VERIFY_ERROR, mcp23017_api_check_config(&prv_handler));
}

void test_mcp23017_check_config_reports_a_broken_bus(void) {
    TEST_ASSERT_EQUAL(MCP23017_RC_OK, mcp23017_api_init(&prv_handler, TEST_ADDRESS, prv_read_register, prv_write_register));
    prv_bus_broken = true;

    TEST_ASSERT_EQUAL(MCP23017_RC_BUS_ERROR, mcp23017_api_check_config(&prv_handler));
}

void test_mcp23017_check_config_rejects_null(void) {
    TEST_ASSERT_EQUAL(MCP23017_RC_NULL_POINTER, mcp23017_api_check_config(NULL));
    TEST_ASSERT_EQUAL_MESSAGE(MCP23017_RC_NULL_POINTER, mcp23017_api_check_config(&prv_handler), "A handler that was never initialized has no callbacks");
}

void test_mcp23017_read_port_reads_the_gpio_registers(void) {
    TEST_ASSERT_EQUAL(MCP23017_RC_OK, mcp23017_api_init(&prv_handler, TEST_ADDRESS, prv_read_register, prv_write_register));
    prv_registers[MCP23017_REGISTER_GPIOA] = 0xA5U;
    prv_registers[MCP23017_REGISTER_GPIOB] = 0x5AU;
    uint8_t value = 0U;

    TEST_ASSERT_EQUAL(MCP23017_RC_OK, mcp23017_api_read_port(&prv_handler, MCP23017_PORT_A, &value));
    TEST_ASSERT_EQUAL_HEX8(0xA5U, value);
    TEST_ASSERT_EQUAL(MCP23017_RC_OK, mcp23017_api_read_port(&prv_handler, MCP23017_PORT_B, &value));
    TEST_ASSERT_EQUAL_HEX8(0x5AU, value);
    TEST_ASSERT_EQUAL(MCP23017_RC_INVALID_PORT, mcp23017_api_read_port(&prv_handler, MCP23017_PORT_COUNT, &value));
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_mcp23017_init_enables_every_pull_up);
    RUN_TEST(test_mcp23017_init_reports_a_broken_bus);
    RUN_TEST(test_mcp23017_check_config_passes_after_init);
    RUN_TEST(test_mcp23017_check_config_detects_a_reset_expander);
    RUN_TEST(test_mcp23017_check_config_checks_port_b_too);
    RUN_TEST(test_mcp23017_check_config_reports_a_broken_bus);
    RUN_TEST(test_mcp23017_check_config_rejects_null);
    RUN_TEST(test_mcp23017_read_port_reads_the_gpio_registers);

    return UNITY_END();
}
