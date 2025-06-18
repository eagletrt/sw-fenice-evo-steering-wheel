#ifndef GRAPHIC_CALLBACKS
#define GRAPHIC_CALLBACKS

#include <stdint.h>

void draw_rectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint32_t color);
void draw_line(uint16_t x, uint16_t y, uint16_t lenght, uint32_t color);
void copy_batch(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint32_t color, uint8_t *alphas);
void clear_screen();
void fix_print_interface();
void swap_framebuffers();

#endif // GRAPHIC_CALLBACKS
