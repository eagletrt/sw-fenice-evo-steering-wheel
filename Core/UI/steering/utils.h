#ifndef UTILS_H
#define UTILS_H

#include "lvgl.h"

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 480

#define NOTCH_HEIGHT 40
#define NOTCH_WIDTH 498

#define NOTCH_BAR_WIDTH_S 35 // smaller items
#define NOTCH_BAR_HEIGHT_S 6
#define NOTCH_BAR_WIDTH_L 90 // large item
#define NOTCH_BAR_HEIGHT_L 10

typedef enum {
  NOT_SCREEN = -1,
  TAB_RACING,
  TAB_CALIBRATION,
  TAB_DEBUG,
  TAB_TRACK_TEST,
  TAB_SENSORS,
  NUM_TABS
} tab_t;

#define COLOR_PRIMARY_HEX 0x000000 // black background
#define COLOR_SECONDARY_HEX                                                    \
  0xF2F3F5                          // 0XD3D3D3 //light grey -> changed to
                                    // white
#define COLOR_TERTIARY_HEX 0xF2F3F5 // white
#define COLOR_YELLOW_STATUS_HEX 0xFFFA14
#define COLOR_BLUE_STATUS_HEX 0x0000FF
#define COLOR_GREEN_STATUS_HEX 0x39D103
#define COLOR_ORANGE_STATUS_HEX 0xE78A00
#define COLOR_RED_STATUS_HEX 0xCE0000
#define COLOR_CAR_HEX 0X78797A

LV_FONT_DECLARE(lv_font_inter_bold_14);
LV_FONT_DECLARE(lv_font_inter_bold_20);
LV_FONT_DECLARE(lv_font_inter_bold_22);
LV_FONT_DECLARE(lv_font_inter_bold_30);
LV_FONT_DECLARE(lv_font_inter_bold_38);

extern lv_style_t grid_style;
extern lv_style_t label_style;
extern lv_style_t box_panels_style;

void init_custom_styles(void);

lv_obj_t *lv_vertical_pair_label(lv_obj_t *parent, lv_obj_t **main_l,
                                 const char *up_text,
                                 const lv_font_t *up_text_font,
                                 const char *bottom_text,
                                 const lv_font_t *bottom_text_font);
lv_obj_t *lv_vertical_pair_two_labels(lv_obj_t *parent, lv_obj_t **main_l,
                                      const char *up_text,
                                      const lv_font_t *up_text_font,
                                      lv_obj_t **bottom_label,
                                      const char *bottom_text,
                                      const lv_font_t *bottom_text_font);
lv_obj_t *lv_horizontal_pair_label(lv_obj_t *parent, lv_obj_t **main_l,
                                   const char *left_text,
                                   const lv_font_t *left_text_font,
                                   const char *right_text,
                                   const lv_font_t *right_text_font);
lv_obj_t *lv_triple_label(lv_obj_t *parent, lv_obj_t **main_l, const char *left_text,
                const lv_font_t *left_text_font, const char *right_text,
                const lv_font_t *right_text_font, const char *bottom_text,
                const lv_font_t *bottom_text_font);

lv_obj_t *create_notch(lv_obj_t *parent, tab_t TabId);

#endif