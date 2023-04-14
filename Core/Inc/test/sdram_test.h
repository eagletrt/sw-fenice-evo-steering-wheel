#ifndef TEST_SDRAM_TEST_H
#define TEST_SDRAM_TEST_H

#include "main.h"
#include <stdint.h>
#include <string.h>

inline void write_buffer(uint32_t base_address, uint32_t offset,
                         uint8_t wdata[], uint32_t size) {
  memcpy((uint32_t *)base_address + offset, wdata, size);
}

#define MEMSIZE 8 * 1000000
#define SEGS 20
#define SEG_SIZE MEMSIZE / SEGS
#define BSIZE 4

void sdram_test_write_all();
void sdram_test_segments();
void sdram_test_long_arrays();
void sdram_test_end_of_memory();
void sdram_test_simple_write();

#endif
