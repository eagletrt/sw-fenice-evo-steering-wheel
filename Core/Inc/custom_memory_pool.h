#ifndef CUSTOM_MEMORY_POOL_H
#define CUSTOM_MEMORY_POOL_H

#include <stdint.h>
#include <stdlib.h>

void *steering_malloc(size_t size);
void steering_free(void *_Nullable ptr);
void *steering_realloc(void *_Nullable ptr, size_t size);

#endif  // CUSTOM_MEMORY_POOL_H
