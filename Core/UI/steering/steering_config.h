#ifndef STEERING_CONFIG_H
#define STEERING_CONFIG_H

#include <stdint.h>

#define CANSNIFFER_ENABLED 1
#define SCREEN_ENABLED 1
#define CAN_LOG_ENABLED 0

#define STEER_TAB_CALIBRATION_ENABLED 0
#define STEER_TAB_SENSORS_ENABLED 1
#define STEER_TAB_TRACK_TEST_ENABLED 1
#define STEER_TAB_DEBUG_ENABLED 1

#define SDRAM_BASE_ADDRESS 0xC0000000
#define FRAMEBUFFER1_ADDR SDRAM_BASE_ADDRESS
#define FRAMEBUFFER2_ADDR 0xC0200000

#if CANSNIFFER_ENABLED == 1
#define PRIMARY_CANSNIFFER_MEMORY_POOL_ADDRESS 0xC0600000
#define SECONDARY_CANSNIFFER_MEMORY_POOL_ADDRESS                               \
  ((0xC0600000) + ((CANSNIFFER_ELEM_T_SIZE) * CAN_POSSIBLE_IDS))
#endif

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 480

uint32_t get_current_time_ms(void);
void openblt_reset(void);

#endif // STEERING_CONFIG_H
