#include "graphics_manager.h"

lv_color_t *framebuffer_1 = (lv_color_t *)FRAMEBUFFER1_ADDR;
lv_color_t *framebuffer_2 = (lv_color_t *)FRAMEBUFFER2_ADDR;

// TODO
// lv_timer_t *shutdown_circuit_task =
// lv_timer_create(update_shutdown_circuit_ui, 100, NULL); lv_timer_t *ptt_tasks
// = lv_timer_create(ptt_tasks_fn, 1000, NULL);

void init_graphics_manager(void) {
  lv_init();
  screen_driver_init();
  tab_manager();
}

void refresh_graphics(void) { lv_tasks(); }

void update_graphics(void) {}
