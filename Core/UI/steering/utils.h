#ifndef UTILS_H
#define UTILS_H

#include "steering_config.h"

#if STEERING_WHEEL_MODE == STEERING_WHEEL_LVGL_MODE

#include "lvgl.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define NOTCH_HEIGHT (40u)
#define NOTCH_WIDTH  (498u)

#define NOTCH_BAR_WIDTH_S  (35)
#define NOTCH_BAR_HEIGHT_S (6)
#define NOTCH_BAR_WIDTH_L  (90)
#define NOTCH_BAR_HEIGHT_L (10)

#define COLOR_PRIMARY_HEX           (0x000000u)  // black background
#define COLOR_SECONDARY_HEX         (0xF2F3F5u)
#define COLOR_TERTIARY_HEX          (0xF2F3F5u)  // white
#define COLOR_YELLOW_STATUS_HEX     (0xFFFA14u)
#define COLOR_BLUE_STATUS_HEX       (0x0000FFu)
#define COLOR_BLACK_STATUS_HEX      (0x000000u)
#define COLOR_WHITE_STATUS_HEX      (0xFFFFFFu)
#define COLOR_GREEN_STATUS_HEX      (0x39D103u)
#define COLOR_DARK_GREEN_STATUS_HEX (0x084808)
#define COLOR_ORANGE_STATUS_HEX     (0xE78A00u)
#define COLOR_RED_STATUS_HEX        (0xCE0000u)
#define COLOR_CAR_HEX               (0X78797Au)

#define COLOR_BRIGHT_RED_HEX    (0xFF0000u)
#define COLOR_BRIGHT_GREEN_HEX  (0x00FF00u)
#define COLOR_BRIGHT_BLUE_HEX   (0x0000FFu)
#define COLOR_BRIGHT_YELLOW_HEX (0xFFFF00u)

LV_FONT_DECLARE(lv_font_inter_bold_14);
LV_FONT_DECLARE(lv_font_inter_bold_20);
LV_FONT_DECLARE(lv_font_inter_bold_22);
LV_FONT_DECLARE(lv_font_inter_bold_30);
LV_FONT_DECLARE(lv_font_inter_bold_38);
LV_FONT_DECLARE(lv_font_inter_bold_70);
LV_FONT_DECLARE(lv_font_inter_bold_90);
LV_FONT_DECLARE(lv_font_inter_bold_200);

extern lv_style_t grid_style;
extern lv_style_t label_style;
extern lv_style_t box_panels_style;

void init_custom_styles(void);

lv_obj_t *lv_vertical_pair_label(
    lv_obj_t *parent,
    lv_obj_t **main_l,
    lv_obj_t **aux_l,
    const char *up_text,
    const lv_font_t *up_text_font,
    const char *bottom_text,
    const lv_font_t *bottom_text_font);
lv_obj_t *lv_vertical_pair_two_labels(
    lv_obj_t *parent,
    lv_obj_t **main_l,
    const char *up_text,
    const lv_font_t *up_text_font,
    lv_obj_t **bottom_label,
    const char *bottom_text,
    const lv_font_t *bottom_text_font);
lv_obj_t *lv_horizontal_pair_label(
    lv_obj_t *parent,
    lv_obj_t **main_l,
    lv_obj_t **aux_l,
    const char *left_text,
    const lv_font_t *left_text_font,
    const char *right_text,
    const lv_font_t *right_text_font);
lv_obj_t *lv_triple_label(
    lv_obj_t *parent,
    lv_obj_t **main_l,
    const char *left_text,
    const lv_font_t *left_text_font,
    const char *right_text,
    const lv_font_t *right_text_font,
    const char *bottom_text,
    const lv_font_t *bottom_text_font);

lv_obj_t *create_notch(lv_obj_t *parent, racing_tab_t TabId);

#endif  // STEERING_WHEEL_MODE

#endif