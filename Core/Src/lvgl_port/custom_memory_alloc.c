#include "lvgl_port/custom_memory_alloc.h"

void *steering_malloc(size_t size) { return NULL; }

void *steering_realloc(void *ptr, size_t size) { return NULL; }

void steering_free(void *ptr) {}