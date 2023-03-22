#include "test/sdram_test.h"

void sdram_tests_init() { srand(time(NULL)); }

/*
Test One:
    Write TEST_BSIZE bytes of data in TEST_SEGS different segments in memory,
and then read BSIZE bytes from the same address and check that they are the same
*/
void sdram_test1() {
  uint32_t correct = 0;
  uint32_t wrong = 0;
  for (uint32_t iseg = 0; iseg < TEST_SEGS; ++iseg) {
    uint32_t offset = TEST_SEG_SIZE * iseg + (rand() % TEST_SEG_SIZE);
    uint8_t wdata_buf[TEST_BSIZE] = {0x01, 0x02, 0x03, 0x04};
    memcpy((uint32_t *)SDRAM_BASE_ADDRESS + offset, wdata_buf, TEST_BSIZE);
    HAL_Delay(1);
    uint8_t rdata_buf[TEST_BSIZE] = {0x00};
    memcpy(rdata_buf, (uint32_t *)SDRAM_BASE_ADDRESS + offset, TEST_BSIZE);
    for (uint32_t iin = 0; iin < TEST_BSIZE; ++iin) {
      if (wdata_buf[iin] != rdata_buf[iin]) {
        wrong++;
      } else {
        correct++;
      }
    }
    HAL_Delay(1);
  }
  wrong++;
}

/*
Test Two:
    Write a long array into the memory, then read it and check it
*/
void sdram_test2() {
  const uint32_t data_buf_size = 50000;
  uint8_t wdata_buf[data_buf_size];
  for (uint32_t iin = 0; iin < data_buf_size; ++iin) {
    wdata_buf[iin] = (data_buf_size + iin) % 256;
  }
  uint32_t offset = 150;
  memcpy((uint32_t *)SDRAM_BASE_ADDRESS + offset, wdata_buf, TEST_BSIZE);
  uint8_t rdata_buf[data_buf_size];
  HAL_Delay(1);
  memcpy(rdata_buf, (uint32_t *)SDRAM_BASE_ADDRESS + offset, TEST_BSIZE);
  for (uint32_t iin = 0; iin < data_buf_size; ++iin) {
    if (wdata_buf[iin] != rdata_buf[iin]) {
      Error_Handler();
    }
  }
  HAL_Delay(1);
}

void sdram_test3() {
  uint8_t wdata_buf[TEST_BSIZE] = {0x01, 0x02, 0x03, 0x04};
  memcpy((void *)SDRAM_BASE_ADDRESS, wdata_buf, TEST_BSIZE);
  HAL_Delay(1);
  uint8_t rdata_buf[TEST_BSIZE];
  memcpy(rdata_buf, (void *)SDRAM_BASE_ADDRESS, TEST_BSIZE);
  for (uint32_t iin = 0; iin < TEST_BSIZE; ++iin) {
    if (wdata_buf[iin] != rdata_buf[iin]) {
      Error_Handler();
    }
  }
}