/*!
 * \file test-menu.c
 * \date 2026-11-16
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 *
 * \brief Unit tests for the menu page: cursor, locking, actions and layout.
 */

#include "unity.h"
#include "menu-api.h"
#include "raster-fonts.h"
#include <string.h>

static struct MenuHandler menu;

/* Clear every redraw flag, as a render does. */
static void prv_rendered(void) {
    for (uint8_t i = 0U; i < MENU_BOX_COUNT; i++) {
        menu.boxes[i].updated = false;
    }
}

static unsigned prv_flagged(void) {
    unsigned count = 0U;
    for (uint8_t i = 0U; i < MENU_BOX_COUNT; i++) {
        count += menu.boxes[i].updated ? 1U : 0U;
    }
    return count;
}

static const struct Box *prv_name(enum MenuItem item) {
    return &menu.boxes[MENU_BOX_NAME_FIRST + item];
}

static const struct Box *prv_status(enum MenuItem item) {
    return &menu.boxes[MENU_BOX_STATUS_FIRST + item];
}

/* Unlocked menu, cursor on MISSION. */
static void prv_unlocked(void) {
    menu_api_set_unlocked(&menu, true);
    menu_api_open(&menu);
}

void setUp(void) {
    TEST_ASSERT_EQUAL(MENU_RC_OK, menu_api_init(&menu, &font_inter));
}

void tearDown(void) {
}

/*!
 * \defgroup menu_init Tests for init and layout
 * \{
 */

void test_menu_init_rejects_null_pointers(void) {
    TEST_ASSERT_EQUAL(MENU_RC_NULL_POINTER, menu_api_init(NULL, &font_inter));
    TEST_ASSERT_EQUAL(MENU_RC_NULL_POINTER, menu_api_init(&menu, NULL));
}

void test_menu_init_starts_locked_on_debug_with_everything_to_draw(void) {
    TEST_ASSERT_FALSE(menu.unlocked);
    TEST_ASSERT_EQUAL(MENU_ITEM_DEBUG, menu.cursor);
    TEST_ASSERT_EQUAL(MENU_BOX_COUNT, prv_flagged());
    TEST_ASSERT_EQUAL_STRING(MENU_TS_ON, menu.boxes[MENU_BOX_TS].label->text);
    TEST_ASSERT_EQUAL_HEX32(MENU_COLOR_TS_ON, menu.boxes[MENU_BOX_TS].label->color.argb);
}

void test_menu_layout_matches_the_design(void) {
    TEST_ASSERT_EQUAL_MESSAGE(MENU_BOX_BACKGROUND, 0U, "The background is drawn first, under the rows");
    for (uint8_t item = 0U; item < MENU_ITEM_COUNT; item++) {
        const struct BoxRectangle name = prv_name((enum MenuItem)item)->rect;
        const struct BoxRectangle status = prv_status((enum MenuItem)item)->rect;
        TEST_ASSERT_EQUAL_UINT16(MENU_ROW_X, name.x);
        TEST_ASSERT_EQUAL_UINT16(MENU_ROW_Y + (item * MENU_ROW_PITCH), name.y);
        TEST_ASSERT_EQUAL_UINT16_MESSAGE(name.x + name.width, status.x, "Name and status make one row");
        TEST_ASSERT_EQUAL_UINT16(MENU_ROW_X + MENU_ROW_WIDTH, status.x + status.width);
        TEST_ASSERT_EQUAL_UINT16(name.y, status.y);
        TEST_ASSERT_TRUE_MESSAGE(name.y + name.height <= MENU_HINTS_Y, "Rows stay above the hint bar");
    }
    const struct BoxRectangle background = menu.boxes[MENU_BOX_BACKGROUND].rect;
    TEST_ASSERT_EQUAL_UINT16(MENU_TITLE_HEIGHT, background.y);
    TEST_ASSERT_EQUAL_UINT16_MESSAGE(MENU_HINTS_Y, background.y + background.height, "Title, background and hints tile the screen");
    TEST_ASSERT_EQUAL_UINT16(MENU_SCREEN_HEIGHT, menu.boxes[MENU_BOX_HINTS].rect.y + menu.boxes[MENU_BOX_HINTS].rect.height);
}

/*! \} */

/*!
 * \defgroup menu_cursor Tests for the cursor and the locked items
 * \{
 */

void test_menu_open_unlocked_starts_on_mission(void) {
    prv_unlocked();
    TEST_ASSERT_EQUAL(MENU_ITEM_MISSION, menu.cursor);
    TEST_ASSERT_EQUAL_STRING(MENU_TS_OFF, menu.boxes[MENU_BOX_TS].label->text);
}

void test_menu_move_skips_locked_items_and_stops_at_the_ends(void) {
    TEST_ASSERT_EQUAL(MENU_ACTION_NONE, menu_api_handle(&menu, MENU_INPUT_MOVE, -1));
    TEST_ASSERT_EQUAL_MESSAGE(MENU_ITEM_DEBUG, menu.cursor, "Everything above DEBUG INFO is locked");
    menu_api_handle(&menu, MENU_INPUT_MOVE, 1);
    TEST_ASSERT_EQUAL(MENU_ITEM_EXIT, menu.cursor);
    menu_api_handle(&menu, MENU_INPUT_MOVE, 5);
    TEST_ASSERT_EQUAL(MENU_ITEM_EXIT, menu.cursor);

    prv_unlocked();
    menu_api_handle(&menu, MENU_INPUT_MOVE, 2);
    TEST_ASSERT_EQUAL(MENU_ITEM_SCREEN_SAVER, menu.cursor);
    menu_api_handle(&menu, MENU_INPUT_MOVE, -100);
    TEST_ASSERT_EQUAL(MENU_ITEM_MISSION, menu.cursor);
}

void test_menu_locking_moves_the_cursor_off_a_locked_item(void) {
    prv_unlocked();
    menu_api_handle(&menu, MENU_INPUT_MOVE, 1);
    menu_api_set_unlocked(&menu, false);
    TEST_ASSERT_EQUAL(MENU_ITEM_DEBUG, menu.cursor);
    TEST_ASSERT_EQUAL_STRING(MENU_LOCKED, prv_status(MENU_ITEM_CALIBRATIONS)->label->text);
    TEST_ASSERT_EQUAL_HEX32(MENU_COLOR_LOCKED_NAME, prv_name(MENU_ITEM_CALIBRATIONS)->label->color.argb);
    TEST_ASSERT_EQUAL_HEX32(MENU_COLOR_LOCKED_STATUS, prv_status(MENU_ITEM_CALIBRATIONS)->label->color.argb);
}

void test_menu_selected_row_is_inverted(void) {
    prv_unlocked();
    TEST_ASSERT_EQUAL_HEX32(MENU_COLOR_SELECTED, prv_name(MENU_ITEM_MISSION)->color.argb);
    TEST_ASSERT_EQUAL_HEX32(MENU_COLOR_SELECTED, prv_status(MENU_ITEM_MISSION)->color.argb);
    TEST_ASSERT_EQUAL_HEX32(MENU_COLOR_SELECTED_TEXT, prv_name(MENU_ITEM_MISSION)->label->color.argb);
    TEST_ASSERT_EQUAL_HEX32(MENU_COLOR_BACKGROUND, prv_name(MENU_ITEM_EXIT)->color.argb);
    TEST_ASSERT_EQUAL_HEX32(MENU_COLOR_NAME, prv_name(MENU_ITEM_EXIT)->label->color.argb);
}

void test_menu_a_move_redraws_only_the_two_rows(void) {
    prv_unlocked();
    prv_rendered();
    menu_api_handle(&menu, MENU_INPUT_MOVE, 1);
    TEST_ASSERT_EQUAL_MESSAGE(4U, prv_flagged(), "Old and new row, name and status each");
    TEST_ASSERT_TRUE(prv_name(MENU_ITEM_MISSION)->updated);
    TEST_ASSERT_TRUE(prv_name(MENU_ITEM_CALIBRATIONS)->updated);
}

void test_menu_same_lock_state_every_frame_draws_nothing(void) {
    prv_unlocked();
    prv_rendered();
    menu_api_set_unlocked(&menu, true);
    menu_api_handle(&menu, MENU_INPUT_MOVE, -1);
    TEST_ASSERT_EQUAL(0U, prv_flagged());
}

/*! \} */

/*!
 * \defgroup menu_actions Tests for SELECT and BACK
 * \{
 */

void test_menu_select_gives_the_item_action(void) {
    const enum MenuAction expected[MENU_ITEM_COUNT] = {
        [MENU_ITEM_MISSION] = MENU_ACTION_NONE,
        [MENU_ITEM_CALIBRATIONS] = MENU_ACTION_CALIBRATIONS,
        [MENU_ITEM_SCREEN_SAVER] = MENU_ACTION_SCREEN_SAVER,
        [MENU_ITEM_DEBUG] = MENU_ACTION_DEBUG,
        [MENU_ITEM_EXIT] = MENU_ACTION_EXIT,
    };
    for (uint8_t item = 0U; item < MENU_ITEM_COUNT; item++) {
        prv_unlocked();
        menu_api_handle(&menu, MENU_INPUT_MOVE, (int8_t)item);
        TEST_ASSERT_EQUAL(expected[item], menu_api_handle(&menu, MENU_INPUT_SELECT, 1));
    }
}

void test_menu_back_on_the_list_exits(void) {
    TEST_ASSERT_EQUAL(MENU_ACTION_EXIT, menu_api_handle(&menu, MENU_INPUT_BACK, 1));
}

void test_menu_mission_page_ignores_everything_but_back(void) {
    prv_unlocked();
    TEST_ASSERT_EQUAL(MENU_ACTION_NONE, menu_api_handle(&menu, MENU_INPUT_SELECT, 1));
    TEST_ASSERT_TRUE(menu.mission_open);

    TEST_ASSERT_EQUAL(MENU_ACTION_NONE, menu_api_handle(&menu, MENU_INPUT_MOVE, 3));
    TEST_ASSERT_EQUAL(MENU_ACTION_NONE, menu_api_handle(&menu, MENU_INPUT_SELECT, 1));
    TEST_ASSERT_EQUAL(MENU_ITEM_MISSION, menu.cursor);

    TEST_ASSERT_EQUAL_MESSAGE(MENU_ACTION_NONE, menu_api_handle(&menu, MENU_INPUT_BACK, 1), "BACK closes the page, it doesn't leave the menu");
    TEST_ASSERT_FALSE(menu.mission_open);
}

void test_menu_open_closes_the_mission_page(void) {
    prv_unlocked();
    menu_api_handle(&menu, MENU_INPUT_SELECT, 1);
    menu_api_open(&menu);
    TEST_ASSERT_FALSE(menu.mission_open);
}

void test_menu_null_handler_is_harmless(void) {
    menu_api_open(NULL);
    menu_api_set_unlocked(NULL, true);
    TEST_ASSERT_EQUAL(MENU_ACTION_NONE, menu_api_handle(NULL, MENU_INPUT_SELECT, 1));
}

/*! \} */

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_menu_init_rejects_null_pointers);
    RUN_TEST(test_menu_init_starts_locked_on_debug_with_everything_to_draw);
    RUN_TEST(test_menu_layout_matches_the_design);

    RUN_TEST(test_menu_open_unlocked_starts_on_mission);
    RUN_TEST(test_menu_move_skips_locked_items_and_stops_at_the_ends);
    RUN_TEST(test_menu_locking_moves_the_cursor_off_a_locked_item);
    RUN_TEST(test_menu_selected_row_is_inverted);
    RUN_TEST(test_menu_a_move_redraws_only_the_two_rows);
    RUN_TEST(test_menu_same_lock_state_every_frame_draws_nothing);

    RUN_TEST(test_menu_select_gives_the_item_action);
    RUN_TEST(test_menu_back_on_the_list_exits);
    RUN_TEST(test_menu_mission_page_ignores_everything_but_back);
    RUN_TEST(test_menu_open_closes_the_mission_page);
    RUN_TEST(test_menu_null_handler_is_harmless);

    return UNITY_END();
}
