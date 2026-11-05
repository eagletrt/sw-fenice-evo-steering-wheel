/*!
 * \file test-timeout.c
 * \date 2026-11-02
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 *
 * \brief Unit tests for the timeout pool, on a fake clock.
 */

#include "unity.h"
#include "fff.h"
#include "timeout-api.h"

DEFINE_FFF_GLOBALS;

FAKE_VOID_FUNC(fake_expired, void *);

static uint32_t prv_now;
static int prv_context_a;
static int prv_context_b;

static uint32_t prv_get_tick(void) {
    return prv_now;
}

/* Move the clock and let the pool expire whatever is due. */
static void prv_advance_to(uint32_t tick) {
    prv_now = tick;
    TEST_ASSERT_EQUAL(TIMEOUT_RC_OK, timeout_api_routine());
}

static timeout_id prv_create(uint32_t duration_ms, void *context) {
    timeout_id id = 0U;
    TEST_ASSERT_EQUAL(TIMEOUT_RC_OK, timeout_api_create(duration_ms, fake_expired, context, &id));
    return id;
}

/* Restarts the timeout whose id the context points to, from its own callback. */
static void prv_restart_self(void *context) {
    TEST_ASSERT_EQUAL(TIMEOUT_RC_OK, timeout_api_start(*(const timeout_id *)context));
}

void setUp(void) {
    RESET_FAKE(fake_expired);
    FFF_RESET_HISTORY();
    prv_now = 0U;
    TEST_ASSERT_EQUAL(TIMEOUT_RC_OK, timeout_api_init(prv_get_tick));
}

void tearDown(void) {}

/*!
 * \defgroup timeout_setup Tests for init and create
 * \{
 */

void test_timeout_init_rejects_null_clock(void) {
    TEST_ASSERT_EQUAL(TIMEOUT_RC_NULL_POINTER, timeout_api_init(NULL));
}

void test_timeout_create_rejects_null_pointers(void) {
    timeout_id id = 0U;
    TEST_ASSERT_EQUAL(TIMEOUT_RC_NULL_POINTER, timeout_api_create(10U, NULL, NULL, &id));
    TEST_ASSERT_EQUAL(TIMEOUT_RC_NULL_POINTER, timeout_api_create(10U, fake_expired, NULL, NULL));
}

void test_timeout_create_hands_out_distinct_ids_until_full(void) {
    timeout_id ids[TIMEOUT_MAX_COUNT];
    for (uint8_t i = 0U; i < TIMEOUT_MAX_COUNT; i++) {
        ids[i] = prv_create(10U, NULL);
        for (uint8_t j = 0U; j < i; j++) {
            TEST_ASSERT_NOT_EQUAL(ids[j], ids[i]);
        }
    }
    timeout_id id = 0U;
    TEST_ASSERT_EQUAL(TIMEOUT_RC_FULL, timeout_api_create(10U, fake_expired, NULL, &id));
}

void test_timeout_init_empties_the_pool(void) {
    const timeout_id id = prv_create(10U, NULL);
    TEST_ASSERT_EQUAL(TIMEOUT_RC_OK, timeout_api_init(prv_get_tick));
    TEST_ASSERT_EQUAL(TIMEOUT_RC_INVALID_ID, timeout_api_start(id));
}

void test_timeout_unknown_id_is_rejected(void) {
    TEST_ASSERT_EQUAL(TIMEOUT_RC_INVALID_ID, timeout_api_start(0U));
    TEST_ASSERT_EQUAL(TIMEOUT_RC_INVALID_ID, timeout_api_stop(0U));
    TEST_ASSERT_FALSE(timeout_api_is_running(0U));
    TEST_ASSERT_FALSE(timeout_api_has_expired(0U));
}

/*! \} */

/*!
 * \defgroup timeout_run Tests for start, stop and expiry
 * \{
 */

void test_timeout_created_stopped(void) {
    const timeout_id id = prv_create(10U, NULL);
    prv_advance_to(100U);
    TEST_ASSERT_FALSE(timeout_api_is_running(id));
    TEST_ASSERT_FALSE(timeout_api_has_expired(id));
    TEST_ASSERT_EQUAL(0, fake_expired_fake.call_count);
}

void test_timeout_expires_after_its_duration_with_its_context(void) {
    const timeout_id id = prv_create(50U, &prv_context_a);
    TEST_ASSERT_EQUAL(TIMEOUT_RC_OK, timeout_api_start(id));
    TEST_ASSERT_TRUE(timeout_api_is_running(id));

    prv_advance_to(49U);
    TEST_ASSERT_EQUAL(0, fake_expired_fake.call_count);
    prv_advance_to(50U);
    TEST_ASSERT_EQUAL(1, fake_expired_fake.call_count);
    TEST_ASSERT_EQUAL_PTR(&prv_context_a, fake_expired_fake.arg0_val);
    TEST_ASSERT_FALSE(timeout_api_is_running(id));
    TEST_ASSERT_TRUE(timeout_api_has_expired(id));
}

void test_timeout_expires_once_per_start(void) {
    const timeout_id id = prv_create(10U, NULL);
    TEST_ASSERT_EQUAL(TIMEOUT_RC_OK, timeout_api_start(id));
    for (uint32_t tick = 1U; tick <= 100U; tick++) {
        prv_advance_to(tick);
    }
    TEST_ASSERT_EQUAL(1, fake_expired_fake.call_count);

    TEST_ASSERT_EQUAL_MESSAGE(TIMEOUT_RC_OK, timeout_api_start(id), "An expired timeout can start again");
    prv_advance_to(110U);
    TEST_ASSERT_EQUAL(2, fake_expired_fake.call_count);
}

void test_timeout_restart_counts_from_now(void) {
    const timeout_id id = prv_create(50U, NULL);
    TEST_ASSERT_EQUAL(TIMEOUT_RC_OK, timeout_api_start(id));
    prv_advance_to(40U);
    TEST_ASSERT_EQUAL(TIMEOUT_RC_OK, timeout_api_start(id));
    prv_advance_to(89U);
    TEST_ASSERT_EQUAL(0, fake_expired_fake.call_count);
    prv_advance_to(90U);
    TEST_ASSERT_EQUAL(1, fake_expired_fake.call_count);
}

void test_timeout_stop_cancels_and_clears_expiry(void) {
    const timeout_id id = prv_create(10U, NULL);
    TEST_ASSERT_EQUAL(TIMEOUT_RC_OK, timeout_api_start(id));
    TEST_ASSERT_EQUAL(TIMEOUT_RC_OK, timeout_api_stop(id));
    prv_advance_to(100U);
    TEST_ASSERT_EQUAL(0, fake_expired_fake.call_count);

    TEST_ASSERT_EQUAL(TIMEOUT_RC_OK, timeout_api_start(id));
    prv_advance_to(110U);
    TEST_ASSERT_TRUE(timeout_api_has_expired(id));
    TEST_ASSERT_EQUAL_MESSAGE(TIMEOUT_RC_OK, timeout_api_stop(id), "Stopping an expired timeout is fine");
    TEST_ASSERT_FALSE(timeout_api_has_expired(id));
    TEST_ASSERT_EQUAL_MESSAGE(TIMEOUT_RC_OK, timeout_api_stop(id), "Stopping a stopped timeout is fine");
}

void test_timeout_each_expiry_reaches_its_own_callback(void) {
    const timeout_id a = prv_create(10U, &prv_context_a);
    const timeout_id b = prv_create(20U, &prv_context_b);
    TEST_ASSERT_EQUAL(TIMEOUT_RC_OK, timeout_api_start(a));
    TEST_ASSERT_EQUAL(TIMEOUT_RC_OK, timeout_api_start(b));

    prv_advance_to(10U);
    TEST_ASSERT_EQUAL(1, fake_expired_fake.call_count);
    TEST_ASSERT_EQUAL_PTR(&prv_context_a, fake_expired_fake.arg0_history[0]);
    prv_advance_to(20U);
    TEST_ASSERT_EQUAL(2, fake_expired_fake.call_count);
    TEST_ASSERT_EQUAL_PTR(&prv_context_b, fake_expired_fake.arg0_history[1]);
}

void test_timeout_simultaneous_expiries_all_reported(void) {
    const timeout_id a = prv_create(10U, &prv_context_a);
    const timeout_id b = prv_create(10U, &prv_context_b);
    TEST_ASSERT_EQUAL(TIMEOUT_RC_OK, timeout_api_start(a));
    TEST_ASSERT_EQUAL(TIMEOUT_RC_OK, timeout_api_start(b));

    prv_advance_to(25U);

    TEST_ASSERT_EQUAL(2, fake_expired_fake.call_count);
}

void test_timeout_callback_may_restart_itself(void) {
    static timeout_id self;
    TEST_ASSERT_EQUAL(TIMEOUT_RC_OK, timeout_api_create(10U, prv_restart_self, &self, &self));
    TEST_ASSERT_EQUAL(TIMEOUT_RC_OK, timeout_api_start(self));

    prv_advance_to(10U);
    TEST_ASSERT_TRUE_MESSAGE(timeout_api_is_running(self), "Restarted from its own callback");
    prv_advance_to(19U);
    TEST_ASSERT_TRUE(timeout_api_is_running(self));
    prv_advance_to(20U);
    TEST_ASSERT_TRUE(timeout_api_is_running(self));
}

void test_timeout_routine_rejects_time_going_backwards(void) {
    prv_advance_to(100U);
    prv_now = 50U;
    TEST_ASSERT_EQUAL(TIMEOUT_RC_ERROR, timeout_api_routine());
}

/*! \} */

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_timeout_init_rejects_null_clock);
    RUN_TEST(test_timeout_create_rejects_null_pointers);
    RUN_TEST(test_timeout_create_hands_out_distinct_ids_until_full);
    RUN_TEST(test_timeout_init_empties_the_pool);
    RUN_TEST(test_timeout_unknown_id_is_rejected);

    RUN_TEST(test_timeout_created_stopped);
    RUN_TEST(test_timeout_expires_after_its_duration_with_its_context);
    RUN_TEST(test_timeout_expires_once_per_start);
    RUN_TEST(test_timeout_restart_counts_from_now);
    RUN_TEST(test_timeout_stop_cancels_and_clears_expiry);
    RUN_TEST(test_timeout_each_expiry_reaches_its_own_callback);
    RUN_TEST(test_timeout_simultaneous_expiries_all_reported);
    RUN_TEST(test_timeout_callback_may_restart_itself);
    RUN_TEST(test_timeout_routine_rejects_time_going_backwards);

    return UNITY_END();
}
