#include "mf_scaledfont.h"

static void scaled_pixel_callback(int16_t x, int16_t y, uint8_t count,
                                  uint8_t alpha, void *state)
{
    struct scaled_renderstate *rstate = state;
    uint8_t dy;

    count = round((float)count * rstate->x_scale);
    x = rstate->x0 + round((float)x * rstate->x_scale);
    y = rstate->y0 + round((float)y * rstate->y_scale);

    for (dy = 0; dy < rstate->y_scale; dy++)
    {
        rstate->orig_callback(x, y + dy, count, alpha, rstate->orig_state);
    }
}

static uint8_t scaled_character_width(const struct mf_font_s *font,
                                      mf_char character)
{
    struct mf_scaledfont_s *sfont = (struct mf_scaledfont_s*)font;
    uint8_t basewidth;

    basewidth = sfont->basefont->character_width(sfont->basefont, character);

    return sfont->x_scale * basewidth;
}

static uint8_t scaled_render_character(const struct mf_font_s *font,
                                       int16_t x0, int16_t y0,
                                       mf_char character,
                                       mf_pixel_callback_t callback,
                                       void *state)
{
    struct mf_scaledfont_s *sfont = (struct mf_scaledfont_s*)font;
    struct scaled_renderstate rstate;
    uint8_t basewidth;

    rstate.orig_callback = callback;
    rstate.orig_state = state;
    rstate.x_scale = sfont->x_scale;
    rstate.y_scale = sfont->y_scale;
    rstate.x0 = x0;
    rstate.y0 = y0;

    basewidth = sfont->basefont->render_character(sfont->basefont, 0, 0,
                            character, scaled_pixel_callback, &rstate);

    return sfont->x_scale * basewidth;
}

void mf_scale_font(struct mf_scaledfont_s *newfont,
                   const struct mf_font_s *basefont,
                   float x_scale, float y_scale)
{
    newfont->font = *basefont;
    newfont->basefont = basefont;

    newfont->font.width = round((float)newfont->font.width * x_scale);
    newfont->font.height = round((float)newfont->font.height * y_scale);
    newfont->font.baseline_x = round((float)newfont->font.baseline_x * x_scale);
    newfont->font.baseline_y = round((float)newfont->font.baseline_y * y_scale);
    newfont->font.min_x_advance = round((float)newfont->font.min_x_advance * x_scale);
    newfont->font.max_x_advance = round((float)newfont->font.max_x_advance * x_scale);
    newfont->font.line_height = round((float)newfont->font.line_height * y_scale);
    newfont->font.character_width = &scaled_character_width;
    newfont->font.render_character = &scaled_render_character;

    newfont->x_scale = x_scale;
    newfont->y_scale = y_scale;
}

