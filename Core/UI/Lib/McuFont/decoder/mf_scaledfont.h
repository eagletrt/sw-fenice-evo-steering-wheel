/* Generate scaled (nearest-neighbor) fonts. This can be used for displaying
 * larger text without spending the memory required for including larger fonts.
 */

#ifndef _MF_SCALEDFONT_H_
#define _MF_SCALEDFONT_H_

#include "mf_font.h"
#include <math.h>

struct mf_scaledfont_s
{
    struct mf_font_s font;

    const struct mf_font_s *basefont;
    float x_scale;
    float y_scale;
};

struct scaled_renderstate
{
    mf_pixel_callback_t orig_callback;
    void *orig_state;
    float x_scale;
    float y_scale;
    int16_t x0;
    int16_t y0;
};

struct font_color_s {
    struct mf_scaledfont_s* font;
    uint32_t color;
};

MF_EXTERN void mf_scale_font(struct mf_scaledfont_s *newfont,
                             const struct mf_font_s *basefont,
                             float x_scale, float y_scale);

#endif
