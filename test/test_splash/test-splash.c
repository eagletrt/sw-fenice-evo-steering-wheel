/*!
 * \file test-splash.c
 * \date 2026-10-29
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 *
 * \brief Unit tests for the splash page (logo + caption).
 *
 * \details The logo is drawn into a host coverage canvas through both paths
 *     (mask, rectangles); they must produce the same image.
 */

#include "unity.h"
#include "fff.h"
#include "splash-api.h"
#include "raster-api.h"
#include "raster-fonts.h"
#include <string.h>

DEFINE_FFF_GLOBALS;

FAKE_VALUE_FUNC(enum RasterReturnCode, fake_draw_rectangle, uint16_t, uint16_t, uint16_t, uint16_t, struct Color);
FAKE_VALUE_FUNC(enum RasterReturnCode, fake_draw_mask, uint16_t, uint16_t, uint16_t, uint16_t, const uint8_t *, struct Color);

/* Logo coverage drawn by the callbacks, indexed [y][x] in screen coordinates. */
static uint8_t prv_canvas[SPLASH_SCREEN_HEIGHT][SPLASH_SCREEN_WIDTH];
static uint8_t prv_reference[SPLASH_SCREEN_HEIGHT][SPLASH_SCREEN_WIDTH];
static uint8_t prv_scratch[SPLASH_LOGO_WIDTH * SPLASH_LOGO_HEIGHT];
static bool prv_out_of_bounds;

static struct SplashHandler prv_splash;
static struct RasterHandler prv_raster;

static bool prv_inside_logo(uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
    return x >= SPLASH_LOGO_X && y >= SPLASH_LOGO_Y &&
           x + width <= SPLASH_LOGO_X + SPLASH_LOGO_WIDTH && y + height <= SPLASH_LOGO_Y + SPLASH_LOGO_HEIGHT;
}

static enum RasterReturnCode prv_canvas_rectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, struct Color color) {
    if (!prv_inside_logo(x, y, width, height)) {
        prv_out_of_bounds = true;
        return RASTER_RC_OK;
    }
    for (uint16_t row = 0U; row < height; row++) {
        memset(&prv_canvas[y + row][x], color.a, width);
    }
    return RASTER_RC_OK;
}

static enum RasterReturnCode prv_canvas_mask(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint8_t *mask, struct Color color) {
    (void)color;
    if (!prv_inside_logo(x, y, width, height)) {
        prv_out_of_bounds = true;
        return RASTER_RC_OK;
    }
    for (uint16_t row = 0U; row < height; row++) {
        memcpy(&prv_canvas[y + row][x], &mask[(size_t)row * width], width);
    }
    return RASTER_RC_OK;
}

/* Raster with rectangles only, drawing into the canvas. */
static void prv_use_rectangles(void) {
    fake_draw_rectangle_fake.custom_fake = prv_canvas_rectangle;
    TEST_ASSERT_EQUAL(RASTER_RC_OK, raster_api_init(&prv_raster, prv_splash.boxes, SPLASH_BOX_COUNT, fake_draw_rectangle, NULL));
}

/* Raster with the mask path and a scratch of \p size bytes, drawing into the canvas. */
static void prv_use_mask(uint32_t size) {
    prv_use_rectangles();
    fake_draw_mask_fake.custom_fake = prv_canvas_mask;
    TEST_ASSERT_EQUAL(RASTER_RC_OK, raster_api_set_mask(&prv_raster, fake_draw_mask, prv_scratch, size));
}

void setUp(void) {
    RESET_FAKE(fake_draw_rectangle);
    RESET_FAKE(fake_draw_mask);
    FFF_RESET_HISTORY();
    memset(prv_canvas, 0, sizeof(prv_canvas));
    prv_out_of_bounds = false;
    TEST_ASSERT_EQUAL(SPLASH_RC_OK, splash_api_init(&prv_splash, &font_inter));
}

void tearDown(void) {}

/*!
 * \defgroup splash_api_init Tests for the splash boxes
 * \{
 */

void test_splash_init_rejects_null(void) {
    TEST_ASSERT_EQUAL(SPLASH_RC_NULL_POINTER, splash_api_init(NULL, &font_inter));
    TEST_ASSERT_EQUAL(SPLASH_RC_NULL_POINTER, splash_api_init(&prv_splash, NULL));
}

void test_splash_boxes_tile_the_screen(void) {
    uint32_t y = 0U;
    for (uint8_t i = 0U; i < SPLASH_BOX_COUNT; i++) {
        const struct BoxRectangle rect = prv_splash.boxes[i].rect;
        TEST_ASSERT_EQUAL_UINT16(0U, rect.x);
        TEST_ASSERT_EQUAL_UINT16(SPLASH_SCREEN_WIDTH, rect.width);
        TEST_ASSERT_EQUAL_UINT16_MESSAGE(y, rect.y, "Boxes must stack with no gap: a gap keeps the previous page's pixels");
        y += rect.height;
    }
    TEST_ASSERT_EQUAL_UINT32(SPLASH_SCREEN_HEIGHT, y);
}

void test_splash_logo_sits_inside_its_box(void) {
    TEST_ASSERT_TRUE(SPLASH_LOGO_Y + SPLASH_LOGO_HEIGHT <= SPLASH_LOGO_BOX_HEIGHT);
    TEST_ASSERT_TRUE(SPLASH_LOGO_X + SPLASH_LOGO_WIDTH <= SPLASH_SCREEN_WIDTH);
}

void test_splash_init_flags_every_box_with_empty_caption(void) {
    for (uint8_t i = 0U; i < SPLASH_BOX_COUNT; i++) {
        TEST_ASSERT_TRUE(prv_splash.boxes[i].updated);
    }
    TEST_ASSERT_EQUAL_STRING("", prv_splash.title.text);
    TEST_ASSERT_EQUAL_STRING("", prv_splash.name.text);
}

/*! \} */

/*!
 * \defgroup splash_api_set_caption Tests for the caption
 * \{
 */

void test_splash_caption_rejects_null_handler(void) {
    TEST_ASSERT_EQUAL(SPLASH_RC_NULL_POINTER, splash_api_set_caption(NULL, "A", "B"));
}

void test_splash_caption_flags_only_changed_lines(void) {
    for (uint8_t i = 0U; i < SPLASH_BOX_COUNT; i++) {
        prv_splash.boxes[i].updated = false;
    }

    TEST_ASSERT_EQUAL(SPLASH_RC_OK, splash_api_set_caption(&prv_splash, "FLASHING", "PEDALS"));
    TEST_ASSERT_TRUE(prv_splash.boxes[SPLASH_BOX_TITLE].updated);
    TEST_ASSERT_TRUE(prv_splash.boxes[SPLASH_BOX_NAME].updated);
    TEST_ASSERT_FALSE_MESSAGE(prv_splash.boxes[SPLASH_BOX_LOGO].updated, "The caption never touches the logo");

    prv_splash.boxes[SPLASH_BOX_TITLE].updated = false;
    prv_splash.boxes[SPLASH_BOX_NAME].updated = false;
    TEST_ASSERT_EQUAL(SPLASH_RC_OK, splash_api_set_caption(&prv_splash, "FLASHING", "COOLING"));
    TEST_ASSERT_FALSE_MESSAGE(prv_splash.boxes[SPLASH_BOX_TITLE].updated, "Same title: no redraw");
    TEST_ASSERT_TRUE(prv_splash.boxes[SPLASH_BOX_NAME].updated);
    TEST_ASSERT_EQUAL_STRING("COOLING", prv_splash.name.text);
}

void test_splash_caption_null_is_empty(void) {
    splash_api_set_caption(&prv_splash, "FLASHING", "PEDALS");
    prv_splash.boxes[SPLASH_BOX_TITLE].updated = false;

    TEST_ASSERT_EQUAL(SPLASH_RC_OK, splash_api_set_caption(&prv_splash, NULL, NULL));
    TEST_ASSERT_EQUAL_STRING("", prv_splash.title.text);
    TEST_ASSERT_EQUAL_STRING("", prv_splash.name.text);
    TEST_ASSERT_TRUE(prv_splash.boxes[SPLASH_BOX_TITLE].updated);
}

/*! \} */

/*!
 * \defgroup splash_api_draw_logo Tests for the logo
 * \{
 */

void test_splash_logo_rejects_null(void) {
    TEST_ASSERT_EQUAL(SPLASH_RC_NULL_POINTER, splash_api_draw_logo(NULL));
}

void test_splash_logo_fills_its_whole_rectangle(void) {
    prv_use_rectangles();
    TEST_ASSERT_EQUAL(SPLASH_RC_OK, splash_api_draw_logo(&prv_raster));
    TEST_ASSERT_FALSE(prv_out_of_bounds);

    // The logo touches every edge of its mask: a wrong width or height shifts or crops it.
    bool top = false, bottom = false, left = false, right = false;
    for (uint32_t x = 0U; x < SPLASH_LOGO_WIDTH; x++) {
        top = top || prv_canvas[SPLASH_LOGO_Y][SPLASH_LOGO_X + x] != 0U;
        bottom = bottom || prv_canvas[SPLASH_LOGO_Y + SPLASH_LOGO_HEIGHT - 1U][SPLASH_LOGO_X + x] != 0U;
    }
    for (uint32_t y = 0U; y < SPLASH_LOGO_HEIGHT; y++) {
        left = left || prv_canvas[SPLASH_LOGO_Y + y][SPLASH_LOGO_X] != 0U;
        right = right || prv_canvas[SPLASH_LOGO_Y + y][SPLASH_LOGO_X + SPLASH_LOGO_WIDTH - 1U] != 0U;
    }
    TEST_ASSERT_TRUE(top && bottom && left && right);
}

void test_splash_logo_rectangles_skip_transparent_runs(void) {
    prv_use_rectangles();
    splash_api_draw_logo(&prv_raster);

    TEST_ASSERT_TRUE(fake_draw_rectangle_fake.call_count > 0U);
    TEST_ASSERT_TRUE_MESSAGE(fake_draw_rectangle_fake.call_count < SPLASH_LOGO_WIDTH * SPLASH_LOGO_HEIGHT / 4U, "Runs, not pixels");
    for (unsigned i = 0U; i < FFF_ARG_HISTORY_LEN && i < fake_draw_rectangle_fake.call_count; i++) {
        TEST_ASSERT_NOT_EQUAL_MESSAGE(0U, fake_draw_rectangle_fake.arg4_history[i].a, "Transparent runs draw nothing");
    }
}

void test_splash_logo_mask_matches_rectangles(void) {
    prv_use_rectangles();
    splash_api_draw_logo(&prv_raster);
    memcpy(prv_reference, prv_canvas, sizeof(prv_canvas));

    memset(prv_canvas, 0, sizeof(prv_canvas));
    prv_use_mask(sizeof(prv_scratch));
    TEST_ASSERT_EQUAL(SPLASH_RC_OK, splash_api_draw_logo(&prv_raster));

    TEST_ASSERT_FALSE(prv_out_of_bounds);
    TEST_ASSERT_EQUAL_MESSAGE(1, fake_draw_mask_fake.call_count, "The whole logo fits: one blend");
    TEST_ASSERT_EQUAL_MEMORY(prv_reference, prv_canvas, sizeof(prv_canvas));
}

void test_splash_logo_mask_in_bands_matches_rectangles(void) {
    prv_use_rectangles();
    splash_api_draw_logo(&prv_raster);
    memcpy(prv_reference, prv_canvas, sizeof(prv_canvas));

    memset(prv_canvas, 0, sizeof(prv_canvas));
    const uint32_t band_rows = 7U;
    prv_use_mask(SPLASH_LOGO_WIDTH * band_rows + 13U);
    TEST_ASSERT_EQUAL(SPLASH_RC_OK, splash_api_draw_logo(&prv_raster));

    TEST_ASSERT_EQUAL((SPLASH_LOGO_HEIGHT + band_rows - 1U) / band_rows, fake_draw_mask_fake.call_count);
    TEST_ASSERT_EQUAL_MEMORY(prv_reference, prv_canvas, sizeof(prv_canvas));
}

void test_splash_logo_scratch_smaller_than_a_row_uses_rectangles(void) {
    prv_use_mask(SPLASH_LOGO_WIDTH - 1U);
    TEST_ASSERT_EQUAL(SPLASH_RC_OK, splash_api_draw_logo(&prv_raster));

    TEST_ASSERT_EQUAL(0, fake_draw_mask_fake.call_count);
    TEST_ASSERT_TRUE(fake_draw_rectangle_fake.call_count > 0U);
}

void test_splash_logo_callback_failure_is_reported(void) {
    prv_use_rectangles();
    fake_draw_rectangle_fake.custom_fake = NULL;
    fake_draw_rectangle_fake.return_val = RASTER_RC_ERROR;
    TEST_ASSERT_EQUAL(SPLASH_RC_ERROR, splash_api_draw_logo(&prv_raster));

    prv_use_mask(sizeof(prv_scratch));
    fake_draw_mask_fake.custom_fake = NULL;
    fake_draw_mask_fake.return_val = RASTER_RC_ERROR;
    TEST_ASSERT_EQUAL(SPLASH_RC_ERROR, splash_api_draw_logo(&prv_raster));
}

/*! \} */

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_splash_init_rejects_null);
    RUN_TEST(test_splash_boxes_tile_the_screen);
    RUN_TEST(test_splash_logo_sits_inside_its_box);
    RUN_TEST(test_splash_init_flags_every_box_with_empty_caption);

    RUN_TEST(test_splash_caption_rejects_null_handler);
    RUN_TEST(test_splash_caption_flags_only_changed_lines);
    RUN_TEST(test_splash_caption_null_is_empty);

    RUN_TEST(test_splash_logo_rejects_null);
    RUN_TEST(test_splash_logo_fills_its_whole_rectangle);
    RUN_TEST(test_splash_logo_rectangles_skip_transparent_runs);
    RUN_TEST(test_splash_logo_mask_matches_rectangles);
    RUN_TEST(test_splash_logo_mask_in_bands_matches_rectangles);
    RUN_TEST(test_splash_logo_scratch_smaller_than_a_row_uses_rectangles);
    RUN_TEST(test_splash_logo_callback_failure_is_reported);

    return UNITY_END();
}
