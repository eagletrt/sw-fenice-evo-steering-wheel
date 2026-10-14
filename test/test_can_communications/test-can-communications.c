/*!
 * \file test-can-communications.c
 * \date 2026-06-12
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 *
 * \brief Unit tests for the CAN-bus communication module.
 */

#include "can-communications-api.h"
#include "fff.h"
#include "unity.h"

#include <string.h>

DEFINE_FFF_GLOBALS;

FAKE_VALUE_FUNC(enum CanCommunicationReturnCode, fake_send, const struct CanCommunicationFrame *);
FAKE_VALUE_FUNC(enum CanCommunicationReturnCode, fake_on_receive, const struct CanCommunicationFrame *);
FAKE_VOID_FUNC(fake_cs_enter);
FAKE_VOID_FUNC(fake_cs_exit);

extern struct CanCommunicationsHandler handler;

#define CAPTURE_BUFFER_SIZE (64U)

static struct CanCommunicationFrame captured_send[CAPTURE_BUFFER_SIZE];
static size_t captured_send_count;
static struct CanCommunicationFrame captured_receive[CAPTURE_BUFFER_SIZE];
static size_t captured_receive_count;

static enum CanCommunicationReturnCode capturing_send(const struct CanCommunicationFrame *frame) {
    if (captured_send_count < CAPTURE_BUFFER_SIZE) {
        captured_send[captured_send_count] = *frame;
        captured_send_count++;
    }
    return CAN_COMMUNICATION_RC_OK;
}

static enum CanCommunicationReturnCode capturing_receive(const struct CanCommunicationFrame *frame) {
    if (captured_receive_count < CAPTURE_BUFFER_SIZE) {
        captured_receive[captured_receive_count] = *frame;
        captured_receive_count++;
    }
    return CAN_COMMUNICATION_RC_OK;
}

static struct CanCommunicationsNetworkConfig default_config(void) {
    return (struct CanCommunicationsNetworkConfig){
        .send = fake_send,
        .on_receive = fake_on_receive,
        .cs_enter = NULL,
        .cs_exit = NULL,
    };
}

static void fill_default_configs(struct CanCommunicationsNetworkConfig configs[CAN_COMMUNICATION_NETWORK_COUNT]) {
    for (enum CanCommunicationNetwork network = 0; network < CAN_COMMUNICATION_NETWORK_COUNT; ++network) {
        configs[network] = default_config();
    }
}

static struct CanCommunicationFrame make_frame(uint32_t id, uint8_t length) {
    struct CanCommunicationFrame frame = {
        .id = id,
        .length = length,
    };
    for (uint8_t i = 0U; i < length; i++) {
        frame.data[i] = (uint8_t)(0xA0U + i);
    }
    return frame;
}

void setUp(void) {
    RESET_FAKE(fake_send);
    RESET_FAKE(fake_on_receive);
    RESET_FAKE(fake_cs_enter);
    RESET_FAKE(fake_cs_exit);
    FFF_RESET_HISTORY();
    fake_send_fake.return_val = CAN_COMMUNICATION_RC_OK;
    fake_on_receive_fake.return_val = CAN_COMMUNICATION_RC_OK;

    captured_send_count = 0;
    captured_receive_count = 0;
    memset(captured_send, 0, sizeof(captured_send));
    memset(captured_receive, 0, sizeof(captured_receive));

    struct CanCommunicationsNetworkConfig configs[CAN_COMMUNICATION_NETWORK_COUNT];
    fill_default_configs(configs);
    can_communications_api_init(configs);
}

/*!
 * \defgroup can_communications_api_init Tests for can_communications_api_init
 * \{
 */

void test_init_success(void) {
    struct CanCommunicationsNetworkConfig configs[CAN_COMMUNICATION_NETWORK_COUNT];
    fill_default_configs(configs);
    enum CanCommunicationReturnCode rc = can_communications_api_init(configs);
    TEST_ASSERT_EQUAL_MESSAGE(CAN_COMMUNICATION_RC_OK, rc, "Expected init to succeed on a valid configs array");
    TEST_ASSERT_EQUAL_PTR_MESSAGE(fake_send, handler.networks[CAN_COMMUNICATION_NETWORK_PRIMARY].send, "Primary send callback must be stored");
    TEST_ASSERT_EQUAL_PTR_MESSAGE(fake_on_receive, handler.networks[CAN_COMMUNICATION_NETWORK_PRIMARY].on_receive, "Primary receive callback must be stored");
    TEST_ASSERT_EQUAL_PTR_MESSAGE(fake_send, handler.networks[CAN_COMMUNICATION_NETWORK_SECONDARY].send, "Secondary send callback must be stored");
    TEST_ASSERT_EQUAL_PTR_MESSAGE(fake_on_receive, handler.networks[CAN_COMMUNICATION_NETWORK_SECONDARY].on_receive, "Secondary receive callback must be stored");
}

void test_init_null_configs(void) {
    TEST_ASSERT_EQUAL(CAN_COMMUNICATION_RC_NULL_POINTER, can_communications_api_init(NULL));
}

void test_init_null_send_in_any_slot(void) {
    struct CanCommunicationsNetworkConfig configs[CAN_COMMUNICATION_NETWORK_COUNT];
    fill_default_configs(configs);
    configs[CAN_COMMUNICATION_NETWORK_SECONDARY].send = NULL;
    TEST_ASSERT_EQUAL(CAN_COMMUNICATION_RC_NULL_POINTER, can_communications_api_init(configs));
}

void test_init_null_on_receive_in_any_slot(void) {
    struct CanCommunicationsNetworkConfig configs[CAN_COMMUNICATION_NETWORK_COUNT];
    fill_default_configs(configs);
    configs[CAN_COMMUNICATION_NETWORK_PRIMARY].on_receive = NULL;
    TEST_ASSERT_EQUAL(CAN_COMMUNICATION_RC_NULL_POINTER, can_communications_api_init(configs));
}

void test_init_accepts_null_critical_section_callbacks(void) {
    struct CanCommunicationsNetworkConfig configs[CAN_COMMUNICATION_NETWORK_COUNT];
    fill_default_configs(configs);
    for (enum CanCommunicationNetwork network = 0; network < CAN_COMMUNICATION_NETWORK_COUNT; ++network) {
        configs[network].cs_enter = NULL;
        configs[network].cs_exit = NULL;
    }
    TEST_ASSERT_EQUAL_MESSAGE(CAN_COMMUNICATION_RC_OK, can_communications_api_init(configs), "Critical-section callbacks are optional; NULL on both is allowed");
}

void test_init_resets_state_on_repeated_call(void) {
    struct CanCommunicationsNetworkConfig configs[CAN_COMMUNICATION_NETWORK_COUNT];
    fill_default_configs(configs);
    enum CanCommunicationReturnCode first = can_communications_api_init(configs);
    enum CanCommunicationReturnCode second = can_communications_api_init(configs);
    TEST_ASSERT_EQUAL_MESSAGE(CAN_COMMUNICATION_RC_OK, first, "First init must succeed");
    TEST_ASSERT_EQUAL_MESSAGE(CAN_COMMUNICATION_RC_OK, second, "Re-init must succeed and reset state");
}

/*! \} */

/*!
 * \defgroup can_communications_api_add_to_tx_buffer Tests for can_communications_api_add_to_tx_buffer
 * \{
 */

void test_add_to_tx_success(void) {
    struct CanCommunicationFrame frame = make_frame(0x123U, 4U);
    TEST_ASSERT_EQUAL(CAN_COMMUNICATION_RC_OK, can_communications_api_add_to_tx_buffer(CAN_COMMUNICATION_NETWORK_PRIMARY, &frame));
}

void test_add_to_tx_null_frame(void) {
    TEST_ASSERT_EQUAL(CAN_COMMUNICATION_RC_NULL_POINTER, can_communications_api_add_to_tx_buffer(CAN_COMMUNICATION_NETWORK_PRIMARY, NULL));
}

void test_add_to_tx_invalid_network(void) {
    struct CanCommunicationFrame frame = make_frame(0x123U, 4U);
    TEST_ASSERT_EQUAL(CAN_COMMUNICATION_RC_INVALID_NETWORK, can_communications_api_add_to_tx_buffer(CAN_COMMUNICATION_NETWORK_COUNT, &frame));
}

void test_add_to_tx_invalid_length(void) {
    struct CanCommunicationFrame frame = make_frame(0x123U, 4U);
    frame.length = (uint8_t)(CAN_COMMUNICATIONS_FRAME_DATA_SIZE + 1U);
    TEST_ASSERT_EQUAL(CAN_COMMUNICATION_RC_INVALID_LENGTH, can_communications_api_add_to_tx_buffer(CAN_COMMUNICATION_NETWORK_PRIMARY, &frame));
}

void test_add_to_tx_queue_full(void) {
    struct CanCommunicationFrame frame = make_frame(0x100U, 1U);
    enum CanCommunicationReturnCode rc = CAN_COMMUNICATION_RC_OK;
    uint32_t accepted = 0U;
    while (rc == CAN_COMMUNICATION_RC_OK) {
        rc = can_communications_api_add_to_tx_buffer(CAN_COMMUNICATION_NETWORK_PRIMARY, &frame);
        if (rc == CAN_COMMUNICATION_RC_OK) {
            accepted++;
        }
    }
    TEST_ASSERT_EQUAL_MESSAGE(CAN_COMMUNICATION_RC_QUEUE_FULL, rc, "Push past capacity must return QUEUE_FULL");
    TEST_ASSERT_TRUE_MESSAGE(accepted > 0U, "At least one frame must have been accepted before saturation");
}

/*! \} */

/*!
 * \defgroup can_communications_api_add_to_rx_buffer Tests for can_communications_api_add_to_rx_buffer
 * \{
 */

void test_add_to_rx_success(void) {
    struct CanCommunicationFrame frame = make_frame(0x123U, 4U);
    TEST_ASSERT_EQUAL(CAN_COMMUNICATION_RC_OK, can_communications_api_add_to_rx_buffer(CAN_COMMUNICATION_NETWORK_PRIMARY, &frame));
}

void test_add_to_rx_null_frame(void) {
    TEST_ASSERT_EQUAL(CAN_COMMUNICATION_RC_NULL_POINTER, can_communications_api_add_to_rx_buffer(CAN_COMMUNICATION_NETWORK_PRIMARY, NULL));
}

void test_add_to_rx_invalid_network(void) {
    struct CanCommunicationFrame frame = make_frame(0x123U, 4U);
    TEST_ASSERT_EQUAL(CAN_COMMUNICATION_RC_INVALID_NETWORK, can_communications_api_add_to_rx_buffer(CAN_COMMUNICATION_NETWORK_COUNT, &frame));
}

void test_add_to_rx_invalid_length(void) {
    struct CanCommunicationFrame frame = make_frame(0x123U, 4U);
    frame.length = (uint8_t)(CAN_COMMUNICATIONS_FRAME_DATA_SIZE + 1U);
    TEST_ASSERT_EQUAL(CAN_COMMUNICATION_RC_INVALID_LENGTH, can_communications_api_add_to_rx_buffer(CAN_COMMUNICATION_NETWORK_PRIMARY, &frame));
}

/*! \} */

/*!
 * \defgroup can_communications_api_process_tx Tests for can_communications_api_process_tx
 * \{
 */

void test_process_tx_invalid_network(void) {
    TEST_ASSERT_EQUAL(CAN_COMMUNICATION_RC_INVALID_NETWORK, can_communications_api_process_tx(CAN_COMMUNICATION_NETWORK_COUNT));
}

void test_process_tx_empty_queue_succeeds_without_calling_send(void) {
    enum CanCommunicationReturnCode rc = can_communications_api_process_tx(CAN_COMMUNICATION_NETWORK_PRIMARY);
    TEST_ASSERT_EQUAL_MESSAGE(CAN_COMMUNICATION_RC_OK, rc, "Empty drain must return OK");
    TEST_ASSERT_EQUAL_MESSAGE(0, fake_send_fake.call_count, "Send must not be called when the queue was empty");
}

void test_process_tx_drains_a_single_frame(void) {
    struct CanCommunicationFrame frame = make_frame(0x456U, 6U);
    can_communications_api_add_to_tx_buffer(CAN_COMMUNICATION_NETWORK_PRIMARY, &frame);

    enum CanCommunicationReturnCode rc = can_communications_api_process_tx(CAN_COMMUNICATION_NETWORK_PRIMARY);

    TEST_ASSERT_EQUAL_MESSAGE(CAN_COMMUNICATION_RC_OK, rc, "Drain must return OK when send succeeded");
    TEST_ASSERT_EQUAL_MESSAGE(1, fake_send_fake.call_count, "Send must be called once per queued frame");
}

void test_process_tx_drains_every_queued_frame_in_one_call(void) {
    handler.networks[CAN_COMMUNICATION_NETWORK_PRIMARY].send = capturing_send;
    for (uint32_t i = 0U; i < 5U; i++) {
        struct CanCommunicationFrame frame = make_frame(0x200U + i, (uint8_t)(i + 1U));
        can_communications_api_add_to_tx_buffer(CAN_COMMUNICATION_NETWORK_PRIMARY, &frame);
    }

    enum CanCommunicationReturnCode rc = can_communications_api_process_tx(CAN_COMMUNICATION_NETWORK_PRIMARY);

    TEST_ASSERT_EQUAL_MESSAGE(CAN_COMMUNICATION_RC_OK, rc, "Drain must succeed");
    TEST_ASSERT_EQUAL_MESSAGE(5U, captured_send_count, "Every queued frame must be sent in one process_tx call");
}

void test_process_tx_preserves_frame_contents_and_order(void) {
    handler.networks[CAN_COMMUNICATION_NETWORK_PRIMARY].send = capturing_send;
    struct CanCommunicationFrame first = make_frame(0x111U, 3U);
    struct CanCommunicationFrame second = make_frame(0x222U, 8U);
    can_communications_api_add_to_tx_buffer(CAN_COMMUNICATION_NETWORK_PRIMARY, &first);
    can_communications_api_add_to_tx_buffer(CAN_COMMUNICATION_NETWORK_PRIMARY, &second);

    can_communications_api_process_tx(CAN_COMMUNICATION_NETWORK_PRIMARY);

    TEST_ASSERT_EQUAL_MESSAGE(2U, captured_send_count, "Both queued frames must be delivered");
    TEST_ASSERT_EQUAL_MESSAGE(0x111U, captured_send[0].id, "FIFO order: first enqueued must be first sent");
    TEST_ASSERT_EQUAL_MESSAGE(3U, captured_send[0].length, "First frame length must be preserved");
    TEST_ASSERT_EQUAL_HEX8_ARRAY_MESSAGE(first.data, captured_send[0].data, CAN_COMMUNICATIONS_FRAME_DATA_SIZE, "First frame data must be preserved");
    TEST_ASSERT_EQUAL_MESSAGE(0x222U, captured_send[1].id, "Second enqueued must be second sent");
    TEST_ASSERT_EQUAL_MESSAGE(8U, captured_send[1].length, "Second frame length must be preserved");
    TEST_ASSERT_EQUAL_HEX8_ARRAY_MESSAGE(second.data, captured_send[1].data, CAN_COMMUNICATIONS_FRAME_DATA_SIZE, "Second frame data must be preserved");
}

void test_process_tx_surfaces_send_failure_but_keeps_draining(void) {
    struct CanCommunicationFrame frame = make_frame(0x10U, 1U);
    can_communications_api_add_to_tx_buffer(CAN_COMMUNICATION_NETWORK_PRIMARY, &frame);
    can_communications_api_add_to_tx_buffer(CAN_COMMUNICATION_NETWORK_PRIMARY, &frame);
    can_communications_api_add_to_tx_buffer(CAN_COMMUNICATION_NETWORK_PRIMARY, &frame);
    fake_send_fake.return_val = CAN_COMMUNICATION_RC_TRANSMISSION_ERROR;

    enum CanCommunicationReturnCode rc = can_communications_api_process_tx(CAN_COMMUNICATION_NETWORK_PRIMARY);

    TEST_ASSERT_EQUAL_MESSAGE(CAN_COMMUNICATION_RC_TRANSMISSION_ERROR, rc, "Failed send must surface as TRANSMISSION_ERROR");
    TEST_ASSERT_EQUAL_MESSAGE(3, fake_send_fake.call_count, "Drain must continue even if individual sends fail");
}

/*! \} */

/*!
 * \defgroup can_communications_api_process_rx Tests for can_communications_api_process_rx
 * \{
 */

void test_process_rx_invalid_network(void) {
    TEST_ASSERT_EQUAL(CAN_COMMUNICATION_RC_INVALID_NETWORK, can_communications_api_process_rx(CAN_COMMUNICATION_NETWORK_COUNT));
}

void test_process_rx_empty_queue_succeeds_without_dispatching(void) {
    enum CanCommunicationReturnCode rc = can_communications_api_process_rx(CAN_COMMUNICATION_NETWORK_PRIMARY);
    TEST_ASSERT_EQUAL_MESSAGE(CAN_COMMUNICATION_RC_OK, rc, "Empty drain must return OK");
    TEST_ASSERT_EQUAL_MESSAGE(0, fake_on_receive_fake.call_count, "on_receive must not be called when the queue was empty");
}

void test_process_rx_drains_a_single_frame(void) {
    struct CanCommunicationFrame frame = make_frame(0x321U, 7U);
    can_communications_api_add_to_rx_buffer(CAN_COMMUNICATION_NETWORK_PRIMARY, &frame);

    enum CanCommunicationReturnCode rc = can_communications_api_process_rx(CAN_COMMUNICATION_NETWORK_PRIMARY);

    TEST_ASSERT_EQUAL_MESSAGE(CAN_COMMUNICATION_RC_OK, rc, "Drain must return OK when dispatcher succeeded");
    TEST_ASSERT_EQUAL_MESSAGE(1, fake_on_receive_fake.call_count, "Dispatcher must be called once per queued frame");
}

void test_process_rx_drains_every_queued_frame_in_one_call(void) {
    handler.networks[CAN_COMMUNICATION_NETWORK_PRIMARY].on_receive = capturing_receive;
    for (uint32_t i = 0U; i < 7U; i++) {
        struct CanCommunicationFrame frame = make_frame(0x300U + i, (uint8_t)(i + 1U));
        can_communications_api_add_to_rx_buffer(CAN_COMMUNICATION_NETWORK_PRIMARY, &frame);
    }

    enum CanCommunicationReturnCode rc = can_communications_api_process_rx(CAN_COMMUNICATION_NETWORK_PRIMARY);

    TEST_ASSERT_EQUAL_MESSAGE(CAN_COMMUNICATION_RC_OK, rc, "Drain must succeed");
    TEST_ASSERT_EQUAL_MESSAGE(7U, captured_receive_count, "Every queued frame must be dispatched in one process_rx call");
}

void test_process_rx_preserves_frame_contents_and_order(void) {
    handler.networks[CAN_COMMUNICATION_NETWORK_PRIMARY].on_receive = capturing_receive;
    struct CanCommunicationFrame first = make_frame(0x711U, 2U);
    struct CanCommunicationFrame second = make_frame(0x722U, 5U);
    can_communications_api_add_to_rx_buffer(CAN_COMMUNICATION_NETWORK_PRIMARY, &first);
    can_communications_api_add_to_rx_buffer(CAN_COMMUNICATION_NETWORK_PRIMARY, &second);

    can_communications_api_process_rx(CAN_COMMUNICATION_NETWORK_PRIMARY);

    TEST_ASSERT_EQUAL_MESSAGE(2U, captured_receive_count, "Both queued frames must be dispatched");
    TEST_ASSERT_EQUAL_MESSAGE(0x711U, captured_receive[0].id, "FIFO order: first enqueued must be first dispatched");
    TEST_ASSERT_EQUAL_MESSAGE(2U, captured_receive[0].length, "First frame length must be preserved");
    TEST_ASSERT_EQUAL_HEX8_ARRAY_MESSAGE(first.data, captured_receive[0].data, CAN_COMMUNICATIONS_FRAME_DATA_SIZE, "First frame data must be preserved");
    TEST_ASSERT_EQUAL_MESSAGE(0x722U, captured_receive[1].id, "Second enqueued must be second dispatched");
    TEST_ASSERT_EQUAL_MESSAGE(5U, captured_receive[1].length, "Second frame length must be preserved");
    TEST_ASSERT_EQUAL_HEX8_ARRAY_MESSAGE(second.data, captured_receive[1].data, CAN_COMMUNICATIONS_FRAME_DATA_SIZE, "Second frame data must be preserved");
}

void test_process_rx_surfaces_dispatcher_failure_but_keeps_draining(void) {
    struct CanCommunicationFrame frame = make_frame(0x10U, 1U);
    can_communications_api_add_to_rx_buffer(CAN_COMMUNICATION_NETWORK_PRIMARY, &frame);
    can_communications_api_add_to_rx_buffer(CAN_COMMUNICATION_NETWORK_PRIMARY, &frame);
    can_communications_api_add_to_rx_buffer(CAN_COMMUNICATION_NETWORK_PRIMARY, &frame);
    fake_on_receive_fake.return_val = CAN_COMMUNICATION_RC_RECEIVE_HANDLER_ERROR;

    enum CanCommunicationReturnCode rc = can_communications_api_process_rx(CAN_COMMUNICATION_NETWORK_PRIMARY);

    TEST_ASSERT_EQUAL_MESSAGE(CAN_COMMUNICATION_RC_RECEIVE_HANDLER_ERROR, rc, "Failed dispatch must surface as RECEIVE_HANDLER_ERROR");
    TEST_ASSERT_EQUAL_MESSAGE(3, fake_on_receive_fake.call_count, "Drain must continue even if individual dispatches fail");
}

/*! \} */

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_init_success);
    RUN_TEST(test_init_null_configs);
    RUN_TEST(test_init_null_send_in_any_slot);
    RUN_TEST(test_init_null_on_receive_in_any_slot);
    RUN_TEST(test_init_accepts_null_critical_section_callbacks);
    RUN_TEST(test_init_resets_state_on_repeated_call);

    RUN_TEST(test_add_to_tx_success);
    RUN_TEST(test_add_to_tx_null_frame);
    RUN_TEST(test_add_to_tx_invalid_network);
    RUN_TEST(test_add_to_tx_invalid_length);
    RUN_TEST(test_add_to_tx_queue_full);

    RUN_TEST(test_add_to_rx_success);
    RUN_TEST(test_add_to_rx_null_frame);
    RUN_TEST(test_add_to_rx_invalid_network);
    RUN_TEST(test_add_to_rx_invalid_length);

    RUN_TEST(test_process_tx_invalid_network);
    RUN_TEST(test_process_tx_empty_queue_succeeds_without_calling_send);
    RUN_TEST(test_process_tx_drains_a_single_frame);
    RUN_TEST(test_process_tx_drains_every_queued_frame_in_one_call);
    RUN_TEST(test_process_tx_preserves_frame_contents_and_order);
    RUN_TEST(test_process_tx_surfaces_send_failure_but_keeps_draining);

    RUN_TEST(test_process_rx_invalid_network);
    RUN_TEST(test_process_rx_empty_queue_succeeds_without_dispatching);
    RUN_TEST(test_process_rx_drains_a_single_frame);
    RUN_TEST(test_process_rx_drains_every_queued_frame_in_one_call);
    RUN_TEST(test_process_rx_preserves_frame_contents_and_order);
    RUN_TEST(test_process_rx_surfaces_dispatcher_failure_but_keeps_draining);

    return UNITY_END();
}
