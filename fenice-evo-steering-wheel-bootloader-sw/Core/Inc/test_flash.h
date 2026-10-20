#ifndef TEST_FLASH_H
#define TEST_FLASH_H

#if EXTERNAL_FLASH_ENABLED == 1

#include "main.h"
#include <stdint.h>
#include <string.h>

/*APS1604M-3SQR PSRAM APmemory*/
#define FAST_READ_QUAD 0xEB
#define QUAD_WRITE 0x38
#define FAST_READ_QUAD_DUMMY_CYCLES 6
#define WRITE_QUAD_DUMMY_CYCLES 0
#define ENTER_QUAD_DUMMY_CYCLES 0
#define ENTER_QUAD_MODE 0x35
#define EXIT_QUAD_MODE 0xF5
/* Exported macro -----------------------------------------------------*/
#define BUFFERSIZE (COUNTOF(aTxBuffer) - 1)
#define COUNTOF(__BUFFER__) (sizeof(__BUFFER__) / sizeof(*(__BUFFER__)))
#define EXTENDEDBUFFERSIZE (1048576)


inline void write_buffer(uint32_t base_address, uint32_t offset,
                         uint8_t wdata[], uint32_t size) {
  memcpy((uint32_t *)base_address + offset, wdata, size);
}

void Flash_Init();
void Flash_Test();

/*
* @brief  This function writes and reads data from flash memory
* @param  mem_addr: memory address to write to
* @param  BUFFERSIZE: size of buffer to write
* @param  aTxBuffer: buffer to write to memory
* @retval 0 if successful, 1 if not
*/
int manual_write_and_read(__IO uint8_t * mem_addr, int BUFFERSIZE, uint8_t aTxBuffer[]); 

int string_write_and_read(__IO uint8_t * mem_addr, int BUFFERSIZE, uint8_t aTxBuffer[]); 

#endif // EXTERNAL_FLASH_ENABLED

#endif // TEST_FLASH_H
