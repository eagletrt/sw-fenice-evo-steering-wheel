#ifndef CUSTOM_MEMORY_ALLOC_H
#define CUSTOM_MEMORY_ALLOC_H

#include <stdint.h>
#include <string.h>

#define MEM_ADDRESS_START 0xC02EF400
#define MEM_ADDRESS_SIZE 4096

void *steering_malloc(size_t size);
void *steering_realloc(void *ptr, size_t size);
void steering_free(void *ptr);

#endif
