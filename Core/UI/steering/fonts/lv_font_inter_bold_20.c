/*******************************************************************************
 * Size: 20 px
 * Bpp: 2
 * Opts:
 ******************************************************************************/

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

#ifndef LV_FONT_INTER_BOLD_20
#define LV_FONT_INTER_BOLD_20 1
#endif

#if LV_FONT_INTER_BOLD_20

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0020 " " */

    /* U+0021 "!" */
    0x7f, 0x7f, 0x7f, 0x3e, 0x3e, 0x3e, 0x3e, 0x3e, 0x3e, 0x3e, 0x14, 0x3e,
    0x7f, 0x3d,

    /* U+0025 "%" */
    0xf, 0xe0, 0x2, 0xc0, 0xff, 0xf0, 0x1d, 0x7, 0xc3, 0xc0, 0xe0, 0x1f, 0xf,
    0xf, 0x0, 0x3f, 0xf8, 0xb4, 0x0, 0x2f, 0x47, 0x80, 0x0, 0x0, 0x3c, 0x0, 0x0,
    0x2, 0xc7, 0xf4, 0x0, 0x1d, 0x7f, 0xf0, 0x0, 0xe3, 0xd2, 0xd0, 0xf, 0xf,
    0xb, 0x80, 0xb4, 0x3d, 0x2d, 0x7, 0x80, 0x7f, 0xf0, 0x3c, 0x0, 0x7f, 0x40,

    /* U+0030 "0" */
    0x2, 0xff, 0x40, 0xf, 0xff, 0xe0, 0x3f, 0xeb, 0xf8, 0x7f, 0x1, 0xfc, 0xbd,
    0x0, 0xbd, 0xfd, 0x0, 0xbe, 0xfc, 0x0, 0x7e, 0xfc, 0x0, 0x7e, 0xfd, 0x0,
    0xbe, 0xbd, 0x0, 0xbd, 0x7f, 0x1, 0xfc, 0x3f, 0xeb, 0xf8, 0xf, 0xff, 0xf0,
    0x2, 0xff, 0x40,

    /* U+0031 "1" */
    0x7, 0xf8, 0xbf, 0xef, 0xff, 0xbd, 0xbe, 0x82, 0xf8, 0xb, 0xe0, 0x2f, 0x80,
    0xbe, 0x2, 0xf8, 0xb, 0xe0, 0x2f, 0x80, 0xbe, 0x2, 0xf8, 0xb, 0xe0,

    /* U+0032 "2" */
    0x7, 0xfe, 0x0, 0xbf, 0xff, 0x47, 0xf9, 0xff, 0x3f, 0x0, 0xfd, 0x68, 0x2,
    0xf4, 0x0, 0xf, 0xc0, 0x0, 0xbd, 0x0, 0xf, 0xe0, 0x0, 0xfe, 0x0, 0xf, 0xe0,
    0x1, 0xfd, 0x0, 0x1f, 0xfa, 0xa9, 0xff, 0xff, 0xfb, 0xff, 0xff, 0xe0,

    /* U+0033 "3" */
    0x6, 0xfe, 0x40, 0x2f, 0xff, 0xe0, 0x7f, 0x9b, 0xf4, 0x69, 0x1, 0xf8, 0x0,
    0x3, 0xf4, 0x0, 0xff, 0xd0, 0x0, 0xff, 0x90, 0x0, 0x57, 0xf4, 0x0, 0x1,
    0xfc, 0x54, 0x0, 0xfc, 0xfe, 0x1, 0xfc, 0x7f, 0xab, 0xf8, 0x2f, 0xff, 0xe0,
    0x6, 0xfe, 0x40,

    /* U+0034 "4" */
    0x0, 0x7, 0xfc, 0x0, 0x3, 0xff, 0x0, 0x2, 0xff, 0xc0, 0x1, 0xfb, 0xf0, 0x0,
    0xfc, 0xfc, 0x0, 0xbd, 0x3f, 0x0, 0x7e, 0xf, 0xc0, 0x3f, 0x3, 0xf0, 0x2f,
    0x40, 0xfc, 0xf, 0xff, 0xff, 0xe3, 0xff, 0xff, 0xf8, 0x55, 0x57, 0xf5, 0x0,
    0x0, 0xfc, 0x0, 0x0, 0x3f, 0x0,

    /* U+0035 "5" */
    0x3f, 0xff, 0xf0, 0xff, 0xff, 0xc3, 0xfa, 0xaa, 0xf, 0x80, 0x0, 0x3e, 0xbf,
    0x41, 0xff, 0xff, 0x87, 0xf5, 0xbf, 0x40, 0x0, 0x7e, 0x0, 0x0, 0xfd, 0x50,
    0x3, 0xfb, 0xe0, 0x2f, 0x9f, 0xe6, 0xfc, 0x1f, 0xff, 0xd0, 0xb, 0xf8, 0x0,

    /* U+0036 "6" */
    0x0, 0x7f, 0xd0, 0x0, 0xff, 0xff, 0x0, 0xff, 0xaf, 0xe0, 0x7f, 0x0, 0x64,
    0x2f, 0x5f, 0xe0, 0xf, 0xef, 0xff, 0x3, 0xfe, 0x5f, 0xe0, 0xfe, 0x0, 0xfc,
    0x3f, 0x40, 0x3f, 0xf, 0xd0, 0xf, 0xc2, 0xfc, 0x7, 0xf0, 0x3f, 0xdb, 0xf4,
    0x7, 0xff, 0xf8, 0x0, 0x2f, 0xe4, 0x0,

    /* U+0037 "7" */
    0x3f, 0xff, 0xfc, 0xff, 0xff, 0xf2, 0xaa, 0xaf, 0xc0, 0x0, 0xbd, 0x0, 0x3,
    0xf0, 0x0, 0x3f, 0x40, 0x1, 0xfc, 0x0, 0xf, 0xd0, 0x0, 0x7f, 0x0, 0x3, 0xf4,
    0x0, 0x1f, 0xc0, 0x0, 0xfc, 0x0, 0x7, 0xe0, 0x0, 0x3f, 0x0, 0x0,

    /* U+0038 "8" */
    0x1, 0xbf, 0xd0, 0x2, 0xff, 0xff, 0x1, 0xfe, 0x5f, 0xe0, 0x7e, 0x1, 0xf8,
    0xf, 0xc0, 0xbd, 0x0, 0xff, 0xfd, 0x0, 0x3f, 0xff, 0x40, 0x7f, 0x57, 0xf4,
    0x3f, 0x40, 0x3f, 0xf, 0xc0, 0xf, 0xc3, 0xf4, 0x3, 0xf0, 0xbf, 0x97, 0xf8,
    0xb, 0xff, 0xfc, 0x0, 0x6f, 0xf4, 0x0,

    /* U+0039 "9" */
    0x6, 0xfe, 0x0, 0x2f, 0xff, 0xd0, 0x7f, 0x9b, 0xf0, 0xfd, 0x1, 0xf8, 0xfc,
    0x0, 0xfc, 0xfd, 0x1, 0xfc, 0xbf, 0x57, 0xfc, 0x3f, 0xfe, 0xfc, 0xb, 0xf4,
    0xfc, 0x0, 0x0, 0xfc, 0xbe, 0x2, 0xf8, 0x7f, 0xef, 0xf0, 0x2f, 0xff, 0xc0,
    0x6, 0xfe, 0x0,

    /* U+003D "=" */
    0x3f, 0xff, 0xf8, 0xff, 0xff, 0xe2, 0xaa, 0xaa, 0x40, 0x0, 0x0, 0x2a, 0xaa,
    0xa4, 0xff, 0xff, 0xe3, 0xff, 0xff, 0x80,

    /* U+003F "?" */
    0x2, 0xfe, 0x40, 0x7f, 0xff, 0x43, 0xf9, 0xbf, 0xf, 0xc0, 0xfd, 0x0, 0x3,
    0xf0, 0x0, 0x3f, 0x80, 0x7, 0xf8, 0x0, 0x3f, 0x40, 0x0, 0xf8, 0x0, 0x2,
    0x90, 0x0, 0x1, 0x0, 0x0, 0x3e, 0x0, 0x1, 0xfc, 0x0, 0x3, 0xd0, 0x0,

    /* U+0041 "A" */
    0x0, 0x2f, 0xe0, 0x0, 0x0, 0xff, 0xc0, 0x0, 0x7, 0xff, 0x40, 0x0, 0x3f,
    0xbe, 0x0, 0x0, 0xfc, 0xfc, 0x0, 0x7, 0xe2, 0xf4, 0x0, 0x3f, 0x47, 0xf0,
    0x0, 0xfc, 0xf, 0xc0, 0xb, 0xf5, 0x7f, 0x40, 0x3f, 0xff, 0xff, 0x1, 0xff,
    0xff, 0xfc, 0xb, 0xe0, 0x2, 0xf8, 0x3f, 0x40, 0x7, 0xf1, 0xfc, 0x0, 0xf,
    0xd0,

    /* U+0042 "B" */
    0xbf, 0xff, 0x80, 0xbf, 0xff, 0xf0, 0xbe, 0x5b, 0xf8, 0xbd, 0x1, 0xfc, 0xbd,
    0x1, 0xfc, 0xbd, 0x3, 0xf4, 0xbf, 0xff, 0xc0, 0xbf, 0xff, 0xf0, 0xbd, 0x1,
    0xfc, 0xbd, 0x0, 0xfd, 0xbd, 0x0, 0xfd, 0xbe, 0xab, 0xfc, 0xbf, 0xff, 0xf8,
    0xbf, 0xff, 0x90,

    /* U+0043 "C" */
    0x0, 0xbf, 0xd0, 0x0, 0xbf, 0xff, 0xd0, 0x2f, 0xff, 0xff, 0x3, 0xf8, 0x3,
    0xf8, 0xbe, 0x0, 0xf, 0xcf, 0xd0, 0x0, 0x0, 0xfc, 0x0, 0x0, 0xf, 0xc0, 0x0,
    0x0, 0xfd, 0x0, 0x0, 0xb, 0xe0, 0x0, 0xfc, 0x3f, 0x80, 0x3f, 0x82, 0xff,
    0xbf, 0xf0, 0xb, 0xff, 0xfc, 0x0, 0xb, 0xfd, 0x0,

    /* U+0044 "D" */
    0xbf, 0xff, 0x40, 0x2f, 0xff, 0xfc, 0xb, 0xfa, 0xff, 0xc2, 0xf4, 0x3, 0xfc,
    0xbd, 0x0, 0x3f, 0x2f, 0x40, 0xb, 0xdb, 0xd0, 0x2, 0xf6, 0xf4, 0x0, 0xbd,
    0xbd, 0x0, 0x2f, 0x6f, 0x40, 0xf, 0xcb, 0xd0, 0xf, 0xf2, 0xfe, 0xbf, 0xf0,
    0xbf, 0xff, 0xf4, 0x2f, 0xff, 0xd0, 0x0,

    /* U+0045 "E" */
    0xbf, 0xff, 0xf2, 0xff, 0xff, 0xcb, 0xea, 0xaa, 0x2f, 0x40, 0x0, 0xbd, 0x0,
    0x2, 0xfa, 0xaa, 0x4b, 0xff, 0xfe, 0x2f, 0xff, 0xf8, 0xbd, 0x0, 0x2, 0xf4,
    0x0, 0xb, 0xd0, 0x0, 0x2f, 0xaa, 0xa8, 0xbf, 0xff, 0xf2, 0xff, 0xff, 0xc0,

    /* U+0046 "F" */
    0xbf, 0xff, 0xfb, 0xff, 0xff, 0xbe, 0xaa, 0x9b, 0xd0, 0x0, 0xbd, 0x0, 0xb,
    0xea, 0xa8, 0xbf, 0xff, 0xdb, 0xff, 0xfd, 0xbd, 0x0, 0xb, 0xd0, 0x0, 0xbd,
    0x0, 0xb, 0xd0, 0x0, 0xbd, 0x0, 0xb, 0xd0, 0x0,

    /* U+0047 "G" */
    0x0, 0xbf, 0xe0, 0x0, 0xbf, 0xff, 0xd0, 0x2f, 0xff, 0xff, 0x3, 0xf8, 0x2,
    0xfc, 0xbe, 0x0, 0x5, 0x4f, 0xd0, 0x0, 0x0, 0xfc, 0x3, 0xff, 0xcf, 0xc0,
    0x3f, 0xfc, 0xfd, 0x1, 0x5f, 0xcb, 0xe0, 0x0, 0xfc, 0x3f, 0x80, 0x3f, 0x82,
    0xff, 0xff, 0xf0, 0xb, 0xff, 0xfc, 0x0, 0xb, 0xfd, 0x0,

    /* U+0048 "H" */
    0xbd, 0x0, 0x1f, 0xaf, 0x40, 0x7, 0xeb, 0xd0, 0x1, 0xfa, 0xf4, 0x0, 0x7e,
    0xbd, 0x0, 0x1f, 0xaf, 0xaa, 0xab, 0xeb, 0xff, 0xff, 0xfa, 0xff, 0xff, 0xfe,
    0xbd, 0x0, 0x1f, 0xaf, 0x40, 0x7, 0xeb, 0xd0, 0x1, 0xfa, 0xf4, 0x0, 0x7e,
    0xbd, 0x0, 0x1f, 0xaf, 0x40, 0x7, 0xe0,

    /* U+0049 "I" */
    0xbd, 0xbd, 0xbd, 0xbd, 0xbd, 0xbd, 0xbd, 0xbd, 0xbd, 0xbd, 0xbd, 0xbd,
    0xbd, 0xbd,

    /* U+004A "J" */
    0x0, 0x3, 0xf0, 0x0, 0xf, 0xc0, 0x0, 0x3f, 0x0, 0x0, 0xfc, 0x0, 0x3, 0xf0,
    0x0, 0xf, 0xc0, 0x0, 0x3f, 0x0, 0x0, 0xfc, 0x0, 0x3, 0xf0, 0x0, 0xf, 0xc7,
    0xe0, 0x3f, 0xf, 0xe7, 0xf8, 0x2f, 0xff, 0xc0, 0x1f, 0xf8, 0x0,

    /* U+004B "K" */
    0xbd, 0x0, 0xbf, 0x2f, 0x40, 0xbf, 0xb, 0xd0, 0x7f, 0x42, 0xf4, 0x3f, 0x40,
    0xbd, 0x3f, 0x80, 0x2f, 0x7f, 0x80, 0xb, 0xff, 0xd0, 0x2, 0xff, 0xfc, 0x0,
    0xbf, 0xbf, 0x80, 0x2f, 0xc7, 0xf4, 0xb, 0xd0, 0xbf, 0x2, 0xf4, 0xf, 0xe0,
    0xbd, 0x0, 0xfd, 0x2f, 0x40, 0x2f, 0xc0,

    /* U+004C "L" */
    0xbd, 0x0, 0xb, 0xd0, 0x0, 0xbd, 0x0, 0xb, 0xd0, 0x0, 0xbd, 0x0, 0xb, 0xd0,
    0x0, 0xbd, 0x0, 0xb, 0xd0, 0x0, 0xbd, 0x0, 0xb, 0xd0, 0x0, 0xbd, 0x0, 0xb,
    0xea, 0xa9, 0xbf, 0xff, 0xeb, 0xff, 0xfe,

    /* U+004D "M" */
    0xbf, 0x40, 0x0, 0xff, 0x2f, 0xe0, 0x0, 0x7f, 0xcb, 0xfc, 0x0, 0x3f, 0xf2,
    0xff, 0x80, 0x1f, 0xfc, 0xbf, 0xf0, 0xb, 0xff, 0x2f, 0xbd, 0x3, 0xef, 0xcb,
    0xdf, 0xc2, 0xf7, 0xf2, 0xf6, 0xf4, 0xfc, 0xfc, 0xbd, 0x3e, 0x7d, 0x3f,
    0x2f, 0x4b, 0xff, 0xf, 0xcb, 0xd1, 0xff, 0x83, 0xf2, 0xf4, 0x3f, 0xc0, 0xfc,
    0xbd, 0x7, 0xf0, 0x3f, 0x2f, 0x40, 0xf4, 0xf, 0xc0,

    /* U+004E "N" */
    0xbd, 0x0, 0x2f, 0x6f, 0xc0, 0xb, 0xdb, 0xf8, 0x2, 0xf6, 0xff, 0x40, 0xbd,
    0xbf, 0xf0, 0x2f, 0x6f, 0xbf, 0xb, 0xdb, 0xdb, 0xe2, 0xf6, 0xf4, 0xfd, 0xbd,
    0xbd, 0xf, 0xef, 0x6f, 0x41, 0xff, 0xdb, 0xd0, 0x2f, 0xf6, 0xf4, 0x3, 0xfd,
    0xbd, 0x0, 0x7f, 0x6f, 0x40, 0xb, 0xd0,

    /* U+004F "O" */
    0x0, 0xbf, 0xd0, 0x0, 0xbf, 0xff, 0xd0, 0x2f, 0xfb, 0xff, 0x3, 0xf8, 0x3,
    0xfc, 0xbe, 0x0, 0xf, 0xcf, 0xd0, 0x0, 0xbd, 0xfc, 0x0, 0xb, 0xef, 0xc0,
    0x0, 0xbe, 0xfd, 0x0, 0xb, 0xdb, 0xe0, 0x0, 0xfc, 0x3f, 0x80, 0x3f, 0xc2,
    0xff, 0xbf, 0xf0, 0xb, 0xff, 0xfd, 0x0, 0xb, 0xfd, 0x0,

    /* U+0050 "P" */
    0xbf, 0xff, 0x40, 0xbf, 0xff, 0xf0, 0xbe, 0xab, 0xf8, 0xbd, 0x1, 0xfc, 0xbd,
    0x0, 0xfc, 0xbd, 0x0, 0xfc, 0xbd, 0x3, 0xfc, 0xbf, 0xff, 0xf4, 0xbf, 0xff,
    0xd0, 0xbe, 0x55, 0x0, 0xbd, 0x0, 0x0, 0xbd, 0x0, 0x0, 0xbd, 0x0, 0x0, 0xbd,
    0x0, 0x0,

    /* U+0051 "Q" */
    0x0, 0xbf, 0xd0, 0x0, 0xbf, 0xff, 0xd0, 0x2f, 0xfb, 0xff, 0x3, 0xf8, 0x3,
    0xfc, 0xbe, 0x0, 0xf, 0xcf, 0xd0, 0x0, 0xbd, 0xfc, 0x0, 0xb, 0xef, 0xc0,
    0x0, 0xbe, 0xfd, 0x0, 0xb, 0xdb, 0xe0, 0xfc, 0xfc, 0x3f, 0x83, 0xff, 0xc2,
    0xff, 0xbf, 0xf0, 0xb, 0xff, 0xfe, 0x0, 0xb, 0xff, 0xf0, 0x0, 0x0, 0xf,
    0xc0,

    /* U+0052 "R" */
    0xbf, 0xff, 0x80, 0xbf, 0xff, 0xf0, 0xbe, 0xab, 0xfc, 0xbd, 0x1, 0xfc, 0xbd,
    0x0, 0xfc, 0xbd, 0x0, 0xfc, 0xbe, 0x5b, 0xfc, 0xbf, 0xff, 0xf0, 0xbf, 0xff,
    0xc0, 0xbd, 0xf, 0xc0, 0xbd, 0xb, 0xf0, 0xbd, 0x3, 0xf4, 0xbd, 0x1, 0xfc,
    0xbd, 0x0, 0xfd,

    /* U+0053 "S" */
    0x1, 0xbf, 0xd0, 0x2, 0xff, 0xff, 0x1, 0xff, 0xaf, 0xe0, 0xbe, 0x0, 0xfc,
    0x3f, 0x40, 0x0, 0x7, 0xf9, 0x0, 0x0, 0xbf, 0xfe, 0x0, 0x2, 0xff, 0xf0, 0x0,
    0x1, 0xff, 0xa, 0x40, 0xf, 0xc3, 0xf0, 0x3, 0xf0, 0xbf, 0xab, 0xfc, 0xf,
    0xff, 0xfd, 0x0, 0x7f, 0xf8, 0x0,

    /* U+0054 "T" */
    0x7f, 0xff, 0xff, 0x9f, 0xff, 0xff, 0xe2, 0xab, 0xfa, 0xa4, 0x0, 0xfc, 0x0,
    0x0, 0x3f, 0x0, 0x0, 0xf, 0xc0, 0x0, 0x3, 0xf0, 0x0, 0x0, 0xfc, 0x0, 0x0,
    0x3f, 0x0, 0x0, 0xf, 0xc0, 0x0, 0x3, 0xf0, 0x0, 0x0, 0xfc, 0x0, 0x0, 0x3f,
    0x0, 0x0, 0xf, 0xc0, 0x0,

    /* U+0055 "U" */
    0xbd, 0x0, 0x3f, 0x6f, 0x40, 0xf, 0xdb, 0xd0, 0x3, 0xf6, 0xf4, 0x0, 0xfd,
    0xbd, 0x0, 0x3f, 0x6f, 0x40, 0xf, 0xdb, 0xd0, 0x3, 0xf6, 0xf4, 0x0, 0xfd,
    0xbd, 0x0, 0x3f, 0x6f, 0x80, 0xf, 0xd7, 0xf0, 0xb, 0xf0, 0xff, 0xef, 0xf8,
    0xf, 0xff, 0xf8, 0x0, 0x7f, 0xe4, 0x0,

    /* U+0056 "V" */
    0x7f, 0x0, 0x3, 0xf4, 0xfd, 0x0, 0x1f, 0xc2, 0xf8, 0x0, 0xfe, 0x7, 0xf0,
    0x3, 0xf0, 0xf, 0xd0, 0x1f, 0xc0, 0x2f, 0x80, 0xfd, 0x0, 0x3f, 0x3, 0xf0,
    0x0, 0xfd, 0x1f, 0xc0, 0x1, 0xf8, 0xbd, 0x0, 0x3, 0xf3, 0xf0, 0x0, 0xf,
    0xff, 0x80, 0x0, 0x1f, 0xfd, 0x0, 0x0, 0x3f, 0xf0, 0x0, 0x0, 0xbf, 0x80,
    0x0,

    /* U+0057 "W" */
    0x7f, 0x0, 0x3f, 0x0, 0x3f, 0xf, 0xc0, 0x1f, 0xc0, 0x1f, 0xc3, 0xf4, 0xf,
    0xf4, 0xb, 0xe0, 0xbe, 0x3, 0xfe, 0x3, 0xf4, 0x1f, 0xc1, 0xff, 0xc0, 0xfc,
    0x3, 0xf0, 0xbd, 0xf4, 0x7e, 0x0, 0xbd, 0x3f, 0x3e, 0x2f, 0x40, 0x1f, 0x8f,
    0x4f, 0xcf, 0xc0, 0x3, 0xf7, 0xc1, 0xf3, 0xf0, 0x0, 0xff, 0xf0, 0x3f, 0xf8,
    0x0, 0x2f, 0xf8, 0xf, 0xfc, 0x0, 0x3, 0xfd, 0x2, 0xff, 0x0, 0x0, 0xff, 0x0,
    0x7f, 0x80, 0x0, 0x2f, 0x80, 0xf, 0xd0, 0x0,

    /* U+0058 "X" */
    0x3f, 0x40, 0xf, 0xd1, 0xfc, 0x3, 0xf8, 0xb, 0xe0, 0x7f, 0x0, 0x3f, 0x4f,
    0xd0, 0x1, 0xfe, 0xfc, 0x0, 0xf, 0xff, 0x0, 0x0, 0x3f, 0xd0, 0x0, 0x3, 0xfd,
    0x0, 0x0, 0xff, 0xf0, 0x0, 0x2f, 0xef, 0xc0, 0x3, 0xf4, 0xfd, 0x0, 0xfe,
    0x7, 0xf0, 0x1f, 0xc0, 0x3f, 0x83, 0xf4, 0x0, 0xfd,

    /* U+0059 "Y" */
    0x7f, 0x0, 0xb, 0xf0, 0xbe, 0x0, 0x3f, 0x0, 0xfc, 0x2, 0xf8, 0x2, 0xfc,
    0x1f, 0xc0, 0x3, 0xf4, 0xfd, 0x0, 0x7, 0xf7, 0xf0, 0x0, 0xf, 0xff, 0x40,
    0x0, 0xf, 0xf8, 0x0, 0x0, 0x2f, 0xc0, 0x0, 0x0, 0x7f, 0x0, 0x0, 0x1, 0xfc,
    0x0, 0x0, 0x7, 0xf0, 0x0, 0x0, 0x1f, 0xc0, 0x0, 0x0, 0x7f, 0x0, 0x0,

    /* U+005A "Z" */
    0xff, 0xff, 0xfc, 0xff, 0xff, 0xfc, 0x6a, 0xab, 0xf8, 0x0, 0x7, 0xf0, 0x0,
    0xf, 0xc0, 0x0, 0x3f, 0x40, 0x0, 0xbe, 0x0, 0x1, 0xfc, 0x0, 0x3, 0xf0, 0x0,
    0xf, 0xd0, 0x0, 0x2f, 0x80, 0x0, 0x7f, 0xaa, 0xa8, 0xff, 0xff, 0xfd, 0xff,
    0xff, 0xfd};

/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0,
     .adv_w = 0,
     .box_w = 0,
     .box_h = 0,
     .ofs_x = 0,
     .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0,
     .adv_w = 74,
     .box_w = 0,
     .box_h = 0,
     .ofs_x = 0,
     .ofs_y = 0},
    {.bitmap_index = 0,
     .adv_w = 104,
     .box_w = 4,
     .box_h = 14,
     .ofs_x = 1,
     .ofs_y = 0},
    {.bitmap_index = 14,
     .adv_w = 275,
     .box_w = 15,
     .box_h = 14,
     .ofs_x = 1,
     .ofs_y = 0},
    {.bitmap_index = 67,
     .adv_w = 220,
     .box_w = 12,
     .box_h = 14,
     .ofs_x = 1,
     .ofs_y = 0},
    {.bitmap_index = 109,
     .adv_w = 157,
     .box_w = 7,
     .box_h = 14,
     .ofs_x = 1,
     .ofs_y = 0},
    {.bitmap_index = 134,
     .adv_w = 202,
     .box_w = 11,
     .box_h = 14,
     .ofs_x = 1,
     .ofs_y = 0},
    {.bitmap_index = 173,
     .adv_w = 211,
     .box_w = 12,
     .box_h = 14,
     .ofs_x = 1,
     .ofs_y = 0},
    {.bitmap_index = 215,
     .adv_w = 217,
     .box_w = 13,
     .box_h = 14,
     .ofs_x = 0,
     .ofs_y = 0},
    {.bitmap_index = 261,
     .adv_w = 206,
     .box_w = 11,
     .box_h = 14,
     .ofs_x = 1,
     .ofs_y = 0},
    {.bitmap_index = 300,
     .adv_w = 211,
     .box_w = 13,
     .box_h = 14,
     .ofs_x = 0,
     .ofs_y = 0},
    {.bitmap_index = 346,
     .adv_w = 190,
     .box_w = 11,
     .box_h = 14,
     .ofs_x = 0,
     .ofs_y = 0},
    {.bitmap_index = 385,
     .adv_w = 211,
     .box_w = 13,
     .box_h = 14,
     .ofs_x = 0,
     .ofs_y = 0},
    {.bitmap_index = 431,
     .adv_w = 211,
     .box_w = 12,
     .box_h = 14,
     .ofs_x = 1,
     .ofs_y = 0},
    {.bitmap_index = 473,
     .adv_w = 217,
     .box_w = 11,
     .box_h = 7,
     .ofs_x = 1,
     .ofs_y = 2},
    {.bitmap_index = 493,
     .adv_w = 180,
     .box_w = 11,
     .box_h = 14,
     .ofs_x = 0,
     .ofs_y = 0},
    {.bitmap_index = 532,
     .adv_w = 239,
     .box_w = 15,
     .box_h = 14,
     .ofs_x = 0,
     .ofs_y = 0},
    {.bitmap_index = 585,
     .adv_w = 211,
     .box_w = 12,
     .box_h = 14,
     .ofs_x = 1,
     .ofs_y = 0},
    {.bitmap_index = 627,
     .adv_w = 241,
     .box_w = 14,
     .box_h = 14,
     .ofs_x = 1,
     .ofs_y = 0},
    {.bitmap_index = 676,
     .adv_w = 233,
     .box_w = 13,
     .box_h = 14,
     .ofs_x = 1,
     .ofs_y = 0},
    {.bitmap_index = 722,
     .adv_w = 196,
     .box_w = 11,
     .box_h = 14,
     .ofs_x = 1,
     .ofs_y = 0},
    {.bitmap_index = 761,
     .adv_w = 187,
     .box_w = 10,
     .box_h = 14,
     .ofs_x = 1,
     .ofs_y = 0},
    {.bitmap_index = 796,
     .adv_w = 243,
     .box_w = 14,
     .box_h = 14,
     .ofs_x = 1,
     .ofs_y = 0},
    {.bitmap_index = 845,
     .adv_w = 239,
     .box_w = 13,
     .box_h = 14,
     .ofs_x = 1,
     .ofs_y = 0},
    {.bitmap_index = 891,
     .adv_w = 90,
     .box_w = 4,
     .box_h = 14,
     .ofs_x = 1,
     .ofs_y = 0},
    {.bitmap_index = 905,
     .adv_w = 182,
     .box_w = 11,
     .box_h = 14,
     .ofs_x = 0,
     .ofs_y = 0},
    {.bitmap_index = 944,
     .adv_w = 221,
     .box_w = 13,
     .box_h = 14,
     .ofs_x = 1,
     .ofs_y = 0},
    {.bitmap_index = 990,
     .adv_w = 182,
     .box_w = 10,
     .box_h = 14,
     .ofs_x = 1,
     .ofs_y = 0},
    {.bitmap_index = 1025,
     .adv_w = 293,
     .box_w = 17,
     .box_h = 14,
     .ofs_x = 1,
     .ofs_y = 0},
    {.bitmap_index = 1085,
     .adv_w = 235,
     .box_w = 13,
     .box_h = 14,
     .ofs_x = 1,
     .ofs_y = 0},
    {.bitmap_index = 1131,
     .adv_w = 250,
     .box_w = 14,
     .box_h = 14,
     .ofs_x = 1,
     .ofs_y = 0},
    {.bitmap_index = 1180,
     .adv_w = 207,
     .box_w = 12,
     .box_h = 14,
     .ofs_x = 1,
     .ofs_y = 0},
    {.bitmap_index = 1222,
     .adv_w = 250,
     .box_w = 14,
     .box_h = 15,
     .ofs_x = 1,
     .ofs_y = -1},
    {.bitmap_index = 1275,
     .adv_w = 210,
     .box_w = 12,
     .box_h = 14,
     .ofs_x = 1,
     .ofs_y = 0},
    {.bitmap_index = 1317,
     .adv_w = 210,
     .box_w = 13,
     .box_h = 14,
     .ofs_x = 0,
     .ofs_y = 0},
    {.bitmap_index = 1363,
     .adv_w = 214,
     .box_w = 13,
     .box_h = 14,
     .ofs_x = 0,
     .ofs_y = 0},
    {.bitmap_index = 1409,
     .adv_w = 233,
     .box_w = 13,
     .box_h = 14,
     .ofs_x = 1,
     .ofs_y = 0},
    {.bitmap_index = 1455,
     .adv_w = 239,
     .box_w = 15,
     .box_h = 14,
     .ofs_x = 0,
     .ofs_y = 0},
    {.bitmap_index = 1508,
     .adv_w = 331,
     .box_w = 21,
     .box_h = 14,
     .ofs_x = 0,
     .ofs_y = 0},
    {.bitmap_index = 1582,
     .adv_w = 229,
     .box_w = 14,
     .box_h = 14,
     .ofs_x = 0,
     .ofs_y = 0},
    {.bitmap_index = 1631,
     .adv_w = 232,
     .box_w = 15,
     .box_h = 14,
     .ofs_x = 0,
     .ofs_y = 0},
    {.bitmap_index = 1684,
     .adv_w = 213,
     .box_w = 12,
     .box_h = 14,
     .ofs_x = 1,
     .ofs_y = 0}};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/

static const uint16_t unicode_list_0[] = {0x0,  0x1,  0x5,  0x10, 0x11,
                                          0x12, 0x13, 0x14, 0x15, 0x16,
                                          0x17, 0x18, 0x19, 0x1d, 0x1f};

/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] = {
    {.range_start = 32,
     .range_length = 32,
     .glyph_id_start = 1,
     .unicode_list = unicode_list_0,
     .glyph_id_ofs_list = NULL,
     .list_length = 15,
     .type = LV_FONT_FMT_TXT_CMAP_SPARSE_TINY},
    {.range_start = 65,
     .range_length = 26,
     .glyph_id_start = 16,
     .unicode_list = NULL,
     .glyph_id_ofs_list = NULL,
     .list_length = 0,
     .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY}};

/*-----------------
 *    KERNING
 *----------------*/

/*Pair left and right glyphs for kerning*/
static const uint8_t kern_pair_glyph_ids[] = {
    4,  11, 4,  40, 6,  8,  8,  5,  11, 4,  11, 7,  11, 8,  11, 9,  11, 10,
    11, 11, 11, 12, 11, 13, 11, 16, 11, 25, 13, 11, 13, 40, 14, 16, 14, 37,
    14, 38, 14, 39, 14, 40, 15, 16, 16, 5,  16, 14, 16, 15, 16, 18, 16, 22,
    16, 30, 16, 32, 16, 34, 16, 35, 16, 37, 16, 38, 16, 40, 18, 16, 18, 39,
    19, 16, 19, 35, 19, 37, 19, 38, 19, 39, 19, 40, 21, 16, 21, 25, 22, 16,
    22, 35, 22, 37, 22, 39, 22, 40, 26, 14, 26, 18, 26, 22, 26, 30, 26, 32,
    26, 34, 27, 5,  27, 16, 27, 18, 27, 22, 27, 30, 27, 32, 27, 35, 27, 37,
    27, 38, 27, 40, 30, 16, 30, 35, 30, 37, 30, 38, 30, 39, 30, 40, 31, 16,
    31, 25, 32, 16, 32, 35, 32, 37, 32, 38, 32, 39, 32, 40, 33, 37, 33, 40,
    34, 16, 35, 8,  35, 16, 35, 18, 35, 22, 35, 25, 35, 30, 35, 32, 37, 8,
    37, 14, 37, 16, 37, 18, 37, 22, 37, 25, 37, 30, 37, 32, 38, 8,  38, 14,
    38, 16, 38, 18, 38, 22, 38, 25, 38, 30, 38, 32, 38, 34, 39, 14, 39, 18,
    39, 22, 39, 30, 39, 32, 40, 8,  40, 14, 40, 16, 40, 18, 40, 22, 40, 25,
    40, 30, 40, 32, 40, 35, 41, 18, 41, 22, 41, 30, 41, 32};

/* Kerning between the respective left and right glyphs
 * 4.4 format which needs to scaled with `kern_scale`*/
static const int8_t kern_pair_values[] = {
    -6,  -11, -5,  -3,  -5,  -5,  -19, -3,  -5,  6,   -5,  -3,  -35, -31,
    -6,  -11, -16, -22, -20, -16, -31, -20, -11, -16, -20, -11, -11, -11,
    -11, -5,  -29, -29, -24, -24, -4,  -5,  -13, -10, -13, -9,  -11, -14,
    -29, -22, -7,  -10, -7,  -5,  -9,  -11, -13, -13, -13, -13, -2,  -15,
    6,   -9,  -9,  -9,  -9,  -29, -25, -13, -30, -13, -10, -13, -9,  -11,
    -14, -27, -31, -13, -10, -13, -9,  -11, -14, -7,  -10, -5,  -22, -29,
    -10, -10, -29, -10, -10, -11, -20, -31, -13, -13, -31, -13, -13, -11,
    -20, -24, -13, -13, -20, -13, -13, -2,  -16, -11, -11, -11, -11, -22,
    -31, -24, -14, -14, -11, -14, -14, 15,  -11, -11, -11, -11};

/*Collect the kern pair's data in one place*/
static const lv_font_fmt_txt_kern_pair_t kern_pairs = {
    .glyph_ids = kern_pair_glyph_ids,
    .values = kern_pair_values,
    .pair_cnt = 124,
    .glyph_ids_size = 0};

/*--------------------
 *  ALL CUSTOM DATA
 *--------------------*/

#if LV_VERSION_CHECK(8, 0, 0)
/*Store all the custom data of the font*/
static lv_font_fmt_txt_glyph_cache_t cache;
static const lv_font_fmt_txt_dsc_t font_dsc = {
#else
static lv_font_fmt_txt_dsc_t font_dsc = {
#endif
    .glyph_bitmap = glyph_bitmap,
    .glyph_dsc = glyph_dsc,
    .cmaps = cmaps,
    .kern_dsc = &kern_pairs,
    .kern_scale = 16,
    .cmap_num = 2,
    .bpp = 2,
    .kern_classes = 0,
    .bitmap_format = 0,
#if LV_VERSION_CHECK(8, 0, 0)
    .cache = &cache
#endif
};

/*-----------------
 *  PUBLIC FONT
 *----------------*/

/*Initialize a public general font descriptor*/
#if LV_VERSION_CHECK(8, 0, 0)
const lv_font_t lv_font_inter_bold_20 = {
#else
lv_font_t lv_font_inter_bold_20 = {
#endif
    .get_glyph_dsc =
        lv_font_get_glyph_dsc_fmt_txt, /*Function pointer to get glyph's data*/
    .get_glyph_bitmap =
        lv_font_get_bitmap_fmt_txt, /*Function pointer to get glyph's bitmap*/
    .line_height = 15, /*The maximum line height required by the font*/
    .base_line = 1,    /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = -3,
    .underline_thickness = 2,
#endif
    .dsc = &font_dsc /*The custom font data. Will be accessed by
                        `get_glyph_bitmap/dsc` */
};

#endif /*#if LV_FONT_INTER_BOLD_20*/
