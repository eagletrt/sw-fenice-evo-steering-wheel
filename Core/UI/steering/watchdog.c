#include "watchdog.h"

primary_watchdog m_primary_watchdog     = {0};
secondary_watchdog m_secondary_watchdog = {0};
inverters_watchdog m_inverters_watchdog = {0};

// TODO: find solution with olivec
#if STEERING_WHEEL_MODE == STEERING_WHEEL_LVGL_MODE
lv_timer_t *watchdog_task;
#endif

void init_watchdog(void) {
    for (uint16_t iindex = 0; iindex < primary_MESSAGE_COUNT; ++iindex) {
        CANLIB_BITSET_ARRAY(m_primary_watchdog.activated, iindex);
    }
    for (uint16_t iindex = 0; iindex < secondary_MESSAGE_COUNT; ++iindex) {
        CANLIB_BITSET_ARRAY(m_secondary_watchdog.activated, iindex);
    }
    for (uint16_t iindex = 0; iindex < inverters_MESSAGE_COUNT; ++iindex) {
        CANLIB_BITSET_ARRAY(m_inverters_watchdog.activated, iindex);
    }
// TODO: find solution with olivec
#if STEERING_WHEEL_MODE == STEERING_WHEEL_LVGL_MODE
    watchdog_task = lv_timer_create(watchdog_task_fn, 1000, NULL);
    lv_timer_set_repeat_count(watchdog_task, -1);
    lv_timer_reset(watchdog_task);
#endif
}

void watchdog_task_fn(void *unused) {
    return;
    (void)unused;
    uint32_t ctime = get_current_time_ms();
    primary_watchdog_timeout(&m_primary_watchdog, ctime);
    secondary_watchdog_timeout(&m_secondary_watchdog, ctime);
    inverters_watchdog_timeout(&m_inverters_watchdog, ctime);

    for (uint16_t iindex = 0; iindex < primary_MESSAGE_COUNT; ++iindex) {
        bool timed_out = CANLIB_BITTEST_ARRAY(m_primary_watchdog.timeout, iindex);
        if (timed_out) {
            is_pmsg_new[iindex]   = true;
            is_pmsg_valid[iindex] = false;
        }
    }

    for (uint16_t iindex = 0; iindex < secondary_MESSAGE_COUNT; ++iindex) {
        bool timed_out = CANLIB_BITTEST_ARRAY(m_secondary_watchdog.timeout, iindex);
        if (timed_out) {
            is_smsg_new[iindex]   = true;
            is_smsg_valid[iindex] = false;
        }
    }

    for (uint16_t iindex = 0; iindex < inverters_MESSAGE_COUNT; ++iindex) {
        bool timed_out = CANLIB_BITTEST_ARRAY(m_inverters_watchdog.timeout, iindex);
        if (timed_out) {
            is_imsg_new[iindex]   = true;
            is_imsg_valid[iindex] = false;
        }
    }
}
