/*!
 * \file menu-api.c
 * \date 2026-11-16
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief Implementation of the menu page.
 */

#include "menu-api.h"

#include "box-api.h"
#include "eagletrt-api.h"
#include "label-api.h"

#include <stddef.h>
#include <string.h>

/*!
 * \brief Text of every item, indexed by MenuItem.
 */
EAGLETRT_STATIC const char *const prv_menu_api_item_names[MENU_ITEM_COUNT] = {
    [MENU_ITEM_MISSION] = "MISSION",
    [MENU_ITEM_CALIBRATIONS] = "CALIBRATIONS",
    [MENU_ITEM_SCREEN_SAVER] = "SCREEN SAVER",
    [MENU_ITEM_DEBUG] = "DEBUG INFO",
    [MENU_ITEM_EXIT] = "EXIT",
};

/*!
 * \brief Whether an item can be selected now.
 *
 * \details DEBUG INFO and EXIT always; the rest only while unlocked (D17).
 */
EAGLETRT_STATIC bool prv_menu_api_selectable(const struct MenuHandler *handler, enum MenuItem item) {
    return handler->unlocked || item == MENU_ITEM_DEBUG || item == MENU_ITEM_EXIT;
}

/*!
 * \brief First item that can be selected (DEBUG INFO at worst).
 */
EAGLETRT_STATIC enum MenuItem prv_menu_api_first_selectable(const struct MenuHandler *handler) {
    uint8_t item = 0U;
    while (!prv_menu_api_selectable(handler, (enum MenuItem)item)) {
        item++;
    }
    return (enum MenuItem)item;
}

/*!
 * \brief Give a box a look, flagging it only on a real change.
 *
 * \param box The box (it has a label).
 * \param background Box colour.
 * \param text_color Label colour.
 * \param text Label text.
 */
EAGLETRT_STATIC void prv_menu_api_set_look(struct Box *box, uint32_t background, uint32_t text_color, const char *text) {
    struct Label *label = box->label;
    if (box->color.argb == background && label->color.argb == text_color && strcmp(label->text, text) == 0) {
        return;
    }
    box->color.argb = background;
    label->color.argb = text_color;
    label->text = text;
    box->updated = true;
}

/*!
 * \brief Restyle every row and the TS state after a cursor or lock change.
 */
EAGLETRT_STATIC void prv_menu_api_restyle(struct MenuHandler *handler) {
    for (uint8_t item = 0U; item < MENU_ITEM_COUNT; item++) {
        const bool selected = (item == handler->cursor);
        const bool locked = !prv_menu_api_selectable(handler, (enum MenuItem)item);
        const uint32_t background = selected ? MENU_COLOR_SELECTED : MENU_COLOR_BACKGROUND;
        const uint32_t name_color = selected ? MENU_COLOR_SELECTED_TEXT : (locked ? MENU_COLOR_LOCKED_NAME : MENU_COLOR_NAME);
        const uint32_t status_color = selected ? MENU_COLOR_SELECTED_TEXT : (locked ? MENU_COLOR_LOCKED_STATUS : MENU_COLOR_STATUS);
        // TODO(#11): MISSION shows the mission the ECU reports.
        prv_menu_api_set_look(&handler->boxes[MENU_BOX_NAME_FIRST + item], background, name_color, prv_menu_api_item_names[item]);
        prv_menu_api_set_look(&handler->boxes[MENU_BOX_STATUS_FIRST + item], background, status_color, locked ? MENU_LOCKED : "");
    }
    prv_menu_api_set_look(&handler->boxes[MENU_BOX_TS], MENU_COLOR_BAR, handler->unlocked ? MENU_COLOR_TS_OFF : MENU_COLOR_TS_ON, handler->unlocked ? MENU_TS_OFF : MENU_TS_ON);
}

/*!
 * \brief Build one box with its label.
 *
 * \retval true on success.
 */
EAGLETRT_STATIC bool prv_menu_api_box(struct MenuHandler *handler, uint8_t id, struct BoxRectangle rect, uint32_t background, int16_t text_x, int16_t text_y, const struct Font *font, uint16_t size, enum FontAlignment alignment) {
    return label_api_init(&handler->labels[id], "", text_x, text_y, font, size, alignment, (struct Color){ .argb = MENU_COLOR_NAME }) == RASTER_RC_OK &&
           box_api_init(&handler->boxes[id], id, rect, (struct Color){ .argb = background }, &handler->labels[id]) == RASTER_RC_OK;
}

enum MenuReturnCode menu_api_init(struct MenuHandler *handler, const struct Font *font) {
    if (handler == NULL || font == NULL) {
        return MENU_RC_NULL_POINTER;
    }
    memset(handler, 0, sizeof(*handler));

    const uint16_t half = STYLE_SCREEN_WIDTH / 2U;
    const uint16_t status_width = MENU_ROW_WIDTH - MENU_NAME_WIDTH;
    bool ok = box_api_init(&handler->boxes[MENU_BOX_BACKGROUND], MENU_BOX_BACKGROUND, (struct BoxRectangle){ 0U, MENU_TITLE_HEIGHT, STYLE_SCREEN_WIDTH, MENU_HINTS_Y - MENU_TITLE_HEIGHT }, (struct Color){ .argb = MENU_COLOR_BACKGROUND }, NULL) == RASTER_RC_OK;
    ok = ok && prv_menu_api_box(handler, MENU_BOX_TITLE, (struct BoxRectangle){ 0U, 0U, half, MENU_TITLE_HEIGHT }, MENU_COLOR_BAR, MENU_TITLE_TEXT_X, MENU_TITLE_TEXT_Y, font, MENU_TITLE_FONT_SIZE, FONT_ALIGN_LEFT);
    ok = ok && prv_menu_api_box(handler, MENU_BOX_TS, (struct BoxRectangle){ half, 0U, half, MENU_TITLE_HEIGHT }, MENU_COLOR_BAR, (int16_t)(MENU_TS_TEXT_RIGHT - half), MENU_TS_TEXT_Y, font, MENU_TS_FONT_SIZE, FONT_ALIGN_RIGHT);
    ok = ok && prv_menu_api_box(handler, MENU_BOX_HINTS, (struct BoxRectangle){ 0U, MENU_HINTS_Y, STYLE_SCREEN_WIDTH, STYLE_SCREEN_HEIGHT - MENU_HINTS_Y }, MENU_COLOR_BAR, (int16_t)half, MENU_HINTS_TEXT_Y, font, MENU_HINTS_FONT_SIZE, FONT_ALIGN_CENTER);
    for (uint8_t item = 0U; ok && item < MENU_ITEM_COUNT; item++) {
        const uint16_t y = (uint16_t)(MENU_ROW_Y + (item * MENU_ROW_PITCH));
        ok = prv_menu_api_box(handler, (uint8_t)(MENU_BOX_NAME_FIRST + item), (struct BoxRectangle){ MENU_ROW_X, y, MENU_NAME_WIDTH, MENU_ROW_HEIGHT }, MENU_COLOR_BACKGROUND, MENU_ROW_PADDING, MENU_NAME_TEXT_Y, font, MENU_NAME_FONT_SIZE, FONT_ALIGN_LEFT) &&
             prv_menu_api_box(handler, (uint8_t)(MENU_BOX_STATUS_FIRST + item), (struct BoxRectangle){ MENU_ROW_X + MENU_NAME_WIDTH, y, status_width, MENU_ROW_HEIGHT }, MENU_COLOR_BACKGROUND, (int16_t)(status_width - MENU_ROW_PADDING), MENU_STATUS_TEXT_Y, font, MENU_STATUS_FONT_SIZE, FONT_ALIGN_RIGHT);
    }
    if (!ok) {
        return MENU_RC_ERROR;
    }

    handler->labels[MENU_BOX_TITLE].text = MENU_TITLE;
    handler->labels[MENU_BOX_TITLE].color.argb = MENU_COLOR_TITLE;
    handler->labels[MENU_BOX_HINTS].text = MENU_HINTS;
    handler->labels[MENU_BOX_HINTS].color.argb = MENU_COLOR_HINTS;
    menu_api_open(handler);
    for (uint8_t i = 0U; i < MENU_BOX_COUNT; i++) {
        handler->boxes[i].updated = true;
    }
    return MENU_RC_OK;
}

void menu_api_open(struct MenuHandler *handler) {
    if (handler == NULL) {
        return;
    }
    handler->mission_open = false;
    handler->cursor = prv_menu_api_first_selectable(handler);
    prv_menu_api_restyle(handler);
}

void menu_api_set_unlocked(struct MenuHandler *handler, bool unlocked) {
    if (handler == NULL || handler->unlocked == unlocked) {
        return;
    }
    handler->unlocked = unlocked;
    if (!prv_menu_api_selectable(handler, handler->cursor)) {
        handler->cursor = prv_menu_api_first_selectable(handler);
    }
    prv_menu_api_restyle(handler);
}

/*!
 * \brief Move the cursor, skipping locked items, stopping at the ends.
 */
EAGLETRT_STATIC void prv_menu_api_move(struct MenuHandler *handler, int8_t amount) {
    const int8_t direction = (amount > 0) ? 1 : -1;
    const int16_t steps = (int16_t)(direction * (int16_t)amount);
    for (int16_t step = 0; step < steps; step++) {
        int16_t item = (int16_t)((int16_t)handler->cursor + direction);
        while (item >= 0 && item < (int16_t)MENU_ITEM_COUNT && !prv_menu_api_selectable(handler, (enum MenuItem)item)) {
            item = (int16_t)(item + direction);
        }
        if (item < 0 || item >= (int16_t)MENU_ITEM_COUNT) {
            break;
        }
        handler->cursor = (enum MenuItem)item;
    }
    prv_menu_api_restyle(handler);
}

/*!
 * \brief Select the item under the cursor.
 */
EAGLETRT_STATIC enum MenuAction prv_menu_api_select(struct MenuHandler *handler) {
    switch (handler->cursor) {
        case MENU_ITEM_MISSION:
            handler->mission_open = true;
            return MENU_ACTION_NONE;
        case MENU_ITEM_CALIBRATIONS:
            return MENU_ACTION_CALIBRATIONS;
        case MENU_ITEM_SCREEN_SAVER:
            return MENU_ACTION_SCREEN_SAVER;
        case MENU_ITEM_DEBUG:
            return MENU_ACTION_DEBUG;
        case MENU_ITEM_EXIT:
        case MENU_ITEM_COUNT:
        default:
            return MENU_ACTION_EXIT;
    }
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
enum MenuAction menu_api_handle(struct MenuHandler *handler, enum MenuInput input, int8_t amount) {
    if (handler == NULL) {
        return MENU_ACTION_NONE;
    }
    if (handler->mission_open) {
        // TODO(#11): the mission list. Only BACK leaves the placeholder.
        if (input == MENU_INPUT_BACK) {
            handler->mission_open = false;
        }
        return MENU_ACTION_NONE;
    }
    switch (input) {
        case MENU_INPUT_MOVE:
            prv_menu_api_move(handler, amount);
            return MENU_ACTION_NONE;
        case MENU_INPUT_SELECT:
            // The cursor never rests on a locked item (set_unlocked moves it).
            return prv_menu_api_select(handler);
        case MENU_INPUT_BACK:
        default:
            return MENU_ACTION_EXIT;
    }
}
