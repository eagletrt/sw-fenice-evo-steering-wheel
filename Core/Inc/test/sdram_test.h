#ifndef TEST_SDRAM_TEST_H
#define TEST_SDRAM_TEST_H

#include "main.h"
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

inline void write_buffer(uint32_t base_address, uint32_t offset,
                         uint8_t wdata[], uint32_t size) {
  memcpy((uint32_t *)base_address + offset, wdata, size);
}

/*
1. scrivere 4 bytes in 50 differenti regioni di memoria, leggere e verificare
che siano ancora uguali
2. scrivere TANTO a tanta velocita'
*/

#define TEST_MEMSIZE 2200000
#define TEST_SEGS 10
#define TEST_SEG_SIZE TEST_MEMSIZE / TEST_SEGS
#define TEST_BSIZE 4

void sdram_tests_init();
void sdram_test1();
void sdram_test2();
void sdram_test3();

#endif
