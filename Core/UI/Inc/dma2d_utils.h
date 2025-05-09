#ifndef DMA2D_UTILS
#define DMA2D_UTILS

#include <stdint.h>

void dma2d_m2m(uint32_t src, uint32_t dest, uint32_t width, uint32_t height);
void dma2d_r2m(uint32_t dest, uint32_t color, uint32_t width, uint32_t height);

#endif // DMA2D_UTILS
