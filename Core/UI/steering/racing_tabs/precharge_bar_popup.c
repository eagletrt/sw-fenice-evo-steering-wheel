#include "precharge_bar_popup.h"

lv_style_t precharge_bar_style_precharge;
lv_style_t precharge_bar_style_back_indic;
lv_obj_t *precharge_bar_background;
lv_obj_t *precharge_bar_bar;

void precharge_bar_screen_create(lv_obj_t *screen) {
    lv_style_init(&precharge_bar_style_precharge);
    lv_style_set_radius(&precharge_bar_style_precharge, 0);
    lv_style_set_bg_opa(&precharge_bar_style_precharge, LV_OPA_COVER);
    lv_style_set_bg_color(&precharge_bar_style_precharge, lv_palette_main(LV_PALETTE_YELLOW));
    // Gradient makes performances much worse
    // lv_style_set_bg_grad_color(&precharge_bar_style_precharge, lv_palette_main(LV_PALETTE_RED));
    // lv_style_set_bg_grad_dir(&precharge_bar_style_precharge, LV_GRAD_DIR_HOR);

    lv_style_init(&precharge_bar_style_back_indic);
    lv_style_set_radius(&precharge_bar_style_back_indic, 0);
    lv_style_set_bg_opa(&precharge_bar_style_back_indic, LV_OPA_COVER);
    lv_style_set_bg_color(&precharge_bar_style_back_indic, lv_color_hex(COLOR_PRIMARY_HEX));

    precharge_bar_background = lv_obj_create(screen);
    lv_obj_set_size(precharge_bar_background, SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_obj_set_style_bg_color(precharge_bar_background, lv_color_hex(COLOR_PRIMARY_HEX), LV_PART_MAIN);

    precharge_bar_bar = lv_bar_create(screen);
    lv_obj_remove_style_all(precharge_bar_bar);
    lv_obj_add_style(precharge_bar_bar, &precharge_bar_style_precharge, LV_PART_INDICATOR);
    lv_obj_add_style(precharge_bar_bar, &precharge_bar_style_back_indic, LV_PART_MAIN);
    lv_obj_set_size(precharge_bar_bar, SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_bar_set_range(precharge_bar_bar, 0, HV_BUS_VOLTAGE_LIMIT);
    lv_bar_set_value(precharge_bar_bar, 0, LV_ANIM_OFF);
}

void precharge_bar_set_pack_voltage(float pack_voltage) {
    lv_bar_set_range(precharge_bar_bar, 0, pack_voltage);
}

void precharge_bar_set_bus_voltage(float bus_voltage) {
    lv_bar_set_value(precharge_bar_bar, bus_voltage, LV_ANIM_OFF);
}
