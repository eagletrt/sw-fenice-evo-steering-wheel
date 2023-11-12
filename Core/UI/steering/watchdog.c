#include "watchdog.h"

primary_watchdog m_primary_watchdog = {0};
secondary_watchdog m_secondary_watchdog = {0};
inverters_watchdog m_inverters_watchdog = {0};

lv_timer_t *watchdog_task;

can_id_t primary_watchdog_monitored_ids[primary_MESSAGE_COUNT];
can_id_t secondary_watchdog_monitored_ids[secondary_MESSAGE_COUNT];
can_id_t inverter_watchdog_monitored_ids[inverters_MESSAGE_COUNT];

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
  watchdog_task = lv_timer_create(watchdog_task_fn, 1000, NULL);
  lv_timer_set_repeat_count(watchdog_task, -1);
  lv_timer_reset(watchdog_task);
}

void watchdog_task_fn(lv_timer_t *tim) {
  uint32_t ctime = get_current_time_ms();
  primary_watchdog_timeout(&m_primary_watchdog, ctime);
  secondary_watchdog_timeout(&m_secondary_watchdog, ctime);
  inverters_watchdog_timeout(&m_inverters_watchdog, ctime);

  for (uint16_t iindex = 0; iindex < primary_MESSAGE_COUNT; ++iindex) {
    bool timed_out = CANLIB_BITTEST_ARRAY(m_primary_watchdog.timeout, iindex);
    if (timed_out) {
      can_id_t id = primary_id_from_index(iindex);
      primary_message_invalidate(id);
    }
  }

  for (uint16_t iindex = 0; iindex < secondary_MESSAGE_COUNT; ++iindex) {
    bool timed_out = CANLIB_BITTEST_ARRAY(m_secondary_watchdog.timeout, iindex);
    if (timed_out) {
      can_id_t id = secondary_id_from_index(iindex);
      secondary_message_invalidate(id);
    }
  }

  for (uint16_t iindex = 0; iindex < inverters_MESSAGE_COUNT; ++iindex) {
    bool timed_out = CANLIB_BITTEST_ARRAY(m_inverters_watchdog.timeout, iindex);
    if (timed_out) {
      can_id_t id = inverters_id_from_index(iindex);
      inverters_message_invalidate(id);
    }
  }
}