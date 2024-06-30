#include "custom_memory_pool.h"

#define MEMORY_POOL_SIZE (2097152U)
#define N_MAX_ALLOCATIONS_ALLOWED

uint8_t *memory_pool   = (uint8_t *)0xC0400000;
size_t current_mem_idx = 0U;

void *steering_malloc(size_t size) {
    if (MEMORY_POOL_SIZE - current_mem_idx < size) {
        return NULL;
    }
    uint8_t *ptr = memory_pool + (current_mem_idx * sizeof(uint8_t));
    current_mem_idx += size;
    return ptr;
}

void steering_free(void *_Nullable ptr) {
    return;
}

void *steering_realloc(void *_Nullable ptr, size_t size) {
    return steering_malloc(size);
}
