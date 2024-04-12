#include "test/sdram_test.h"

void sdram_test_write_all() {
    const uint32_t data_buf_size = MEMSIZE;
    uint8_t *array               = (uint8_t *)SDRAM_BASE_ADDRESS;
    for (uint64_t ic = 0; ic < data_buf_size; ++ic) {
        array[ic] = 69;
    }
    for (uint64_t ic = 0; ic < data_buf_size; ++ic) {
        if (array[ic] != 69) {
            Error_Handler();
        }
    }
}

/*
Write five long array into the memory, then read it and check it
*/
void sdram_test_long_arrays() {
    const uint32_t data_buf_size = 1000;
    for (int itime = 0; itime < 5; itime++) {
        uint8_t wdata_buf[data_buf_size];
        for (uint32_t iin = 0; iin < data_buf_size; ++iin) {
            wdata_buf[iin] = (uint8_t)(iin % 256);
        }
        uint32_t offset   = 0 + itime * 1048576;
        void *destination = memcpy((void *)SDRAM_BASE_ADDRESS + offset, wdata_buf, data_buf_size);
        uint8_t rdata_buf[data_buf_size];
        memcpy(rdata_buf, destination, data_buf_size);
        for (uint32_t iin = 0; iin < data_buf_size; ++iin) {
            if (wdata_buf[iin] != rdata_buf[iin]) {
                Error_Handler();
            }
        }
    }
    HAL_Delay(1);
}

/*
Go at the end of the memory and write 100 bytes, then read them and expect ok
*/
void sdram_test_end_of_memory() {
    const uint32_t buf_len = 100;
    uint8_t wdata_buf[buf_len];
    for (uint32_t iin = 0; iin < buf_len; ++iin) {
        wdata_buf[iin] = iin;
    }
    uint32_t write_address = SDRAM_BASE_ADDRESS + MEMSIZE - buf_len;
    memcpy((void *)write_address, wdata_buf, buf_len);
    uint8_t rdata_buf[buf_len];
    memcpy(rdata_buf, (void *)write_address, buf_len);
    for (uint32_t iin = 0; iin < buf_len; ++iin) {
        if (wdata_buf[iin] != rdata_buf[iin]) {
            Error_Handler();
        }
    }
    HAL_Delay(1);
}

/*
Simple write. Set directly the memory region without functions like memset or
memcpy.
*/
void sdram_test_simple_write() {
    const uint32_t buf_len = 100;
    uint8_t *data          = (uint8_t *)SDRAM_BASE_ADDRESS;
    uint8_t content        = 0xA1;
    for (uint32_t iin = 0; iin < buf_len; ++iin) {
        data[iin] = content;
    }
    for (uint32_t iin = 0; iin < buf_len; ++iin) {
        if (data[iin] != content) {
            Error_Handler();
        }
    }
    HAL_Delay(1);
}
