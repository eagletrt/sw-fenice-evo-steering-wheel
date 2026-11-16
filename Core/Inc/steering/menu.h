/*!
 * \file menu.h
 * \date 2026-11-16
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief Definitions for the menu page: items, cursor and layout.
 *
 * \details Layout from the Figma "Menu - TS OFF / TS ON" frames: a 72 px
 *     black title bar, five 720x64 rows from y 88 with a 68 px pitch, a 48 px
 *     black hint bar. A row is two boxes (name, status on the right) because a
 *     libraster box holds one label. The background box comes first so the
 *     gaps between rows are drawn before the rows on a full redraw.
 */

#ifndef MENU_H
#define MENU_H

#include "box.h"
#include "label.h"
#include "style.h"

#include <stdbool.h>
#include <stdint.h>

#define MENU_SCREEN_WIDTH (800U)  /*!< Width of the display, in pixels */
#define MENU_SCREEN_HEIGHT (480U) /*!< Height of the display, in pixels */

#define MENU_TITLE_HEIGHT (72U)     /*!< Title bar, from the top */
#define MENU_TITLE_TEXT_X (24U)     /*!< "MENU" left edge */
#define MENU_TITLE_TEXT_Y (14U)     /*!< "MENU" top */
#define MENU_TITLE_FONT_SIZE (36U)  /*!< "MENU" */
#define MENU_TS_TEXT_RIGHT (776U)   /*!< TS state right edge */
#define MENU_TS_TEXT_Y (22U)        /*!< TS state top */
#define MENU_TS_FONT_SIZE (24U)     /*!< TS state */
#define MENU_HINTS_Y (432U)         /*!< Hint bar, to the bottom */
#define MENU_HINTS_TEXT_Y (14U)     /*!< Hint text top */
#define MENU_HINTS_FONT_SIZE (18U)  /*!< Hint text */
#define MENU_ROW_X (40U)            /*!< Left edge of every row */
#define MENU_ROW_WIDTH (720U)       /*!< Row width */
#define MENU_ROW_HEIGHT (64U)       /*!< Row height */
#define MENU_ROW_Y (88U)            /*!< Top of the first row */
#define MENU_ROW_PITCH (68U)        /*!< Distance between two rows */
#define MENU_ROW_PADDING (24U)      /*!< Text inset from the row edges */
#define MENU_NAME_WIDTH (480U)      /*!< Name box; the status box takes the rest of the row */
#define MENU_NAME_TEXT_Y (12U)      /*!< Item name top, inside the row */
#define MENU_NAME_FONT_SIZE (32U)   /*!< Item name */
#define MENU_STATUS_TEXT_Y (20U)    /*!< Item status top, inside the row */
#define MENU_STATUS_FONT_SIZE (20U) /*!< Item status */

#define MENU_TITLE "MENU"
#define MENU_TS_OFF "TS OFF"                                             /*!< Title bar while the items are unlocked */
#define MENU_TS_ON "TS ON"                                               /*!< Title bar while some items are locked */
#define MENU_LOCKED "LOCKED  TS ON"                                      /*!< Status of a locked item */
#define MENU_HINTS "ROTATE  MOVE        PRESS  SELECT        BACK  EXIT" /*!< TODO(design): Figma text, written for push encoders */

#define MENU_COLOR_BACKGROUND (STYLE_DARK_GRAY) /*!< Page and unselected rows */
#define MENU_COLOR_BAR (STYLE_BLACK)            /*!< Title and hint bars */
#define MENU_COLOR_TITLE (STYLE_WHITE)          /*!< "MENU" */
#define MENU_COLOR_TS_OFF (STYLE_LIGHT_GRAY)    /*!< "TS OFF" */
#define MENU_COLOR_TS_ON (STYLE_RED)            /*!< "TS ON" */
#define MENU_COLOR_HINTS (STYLE_LIGHT_GRAY)     /*!< Hint text */
#define MENU_COLOR_NAME (STYLE_WHITE)           /*!< Selectable item */
#define MENU_COLOR_STATUS (STYLE_LIGHT_GRAY)    /*!< Status of a selectable item */
#define MENU_COLOR_LOCKED_NAME (STYLE_LIGHT_GRAY)
#define MENU_COLOR_LOCKED_STATUS (STYLE_YELLOW)
#define MENU_COLOR_SELECTED (STYLE_WHITE)      /*!< Row under the cursor */
#define MENU_COLOR_SELECTED_TEXT (STYLE_BLACK) /*!< Text of the row under the cursor */

/*!
 * \brief Return codes for menu operations.
 */
enum MenuReturnCode {
    MENU_RC_OK,           /*!< Operation completed successfully */
    MENU_RC_NULL_POINTER, /*!< A required pointer was NULL */
    MENU_RC_ERROR,        /*!< libraster rejected a box or a label */
};

/*!
 * \brief Items of the menu, top to bottom.
 */
enum MenuItem : uint8_t {
    MENU_ITEM_MISSION,      /*!< Mission page (TODO(#11): placeholder) */
    MENU_ITEM_CALIBRATIONS, /*!< Calibration mode */
    MENU_ITEM_SCREEN_SAVER, /*!< Screen saver mode */
    MENU_ITEM_DEBUG,        /*!< Debug mode, never locked */
    MENU_ITEM_EXIT,         /*!< Back to driving, never locked */
    MENU_ITEM_COUNT,
};

/*!
 * \brief What the wheel should do after a menu input.
 */
enum MenuAction : uint8_t {
    MENU_ACTION_NONE,         /*!< Stay in the menu */
    MENU_ACTION_EXIT,         /*!< Back to driving */
    MENU_ACTION_CALIBRATIONS, /*!< Enter the calibration mode */
    MENU_ACTION_SCREEN_SAVER, /*!< Enter the screen saver */
    MENU_ACTION_DEBUG,        /*!< Enter the debug mode */
};

/*!
 * \brief Menu inputs (D49: center knob, top right, top left).
 */
enum MenuInput : uint8_t {
    MENU_INPUT_MOVE,   /*!< Move the cursor by the amount, positive = down */
    MENU_INPUT_SELECT, /*!< Select the item under the cursor */
    MENU_INPUT_BACK,   /*!< Leave the page */
};

/*!
 * \brief Boxes of the menu page, in drawing order.
 */
enum MenuBoxId : uint8_t {
    MENU_BOX_BACKGROUND,                                           /*!< Whole page, under the rows */
    MENU_BOX_TITLE,                                                /*!< "MENU", left part of the title bar */
    MENU_BOX_TS,                                                   /*!< TS state, right part of the title bar */
    MENU_BOX_HINTS,                                                /*!< Hint bar */
    MENU_BOX_NAME_FIRST,                                           /*!< Name of the first item */
    MENU_BOX_STATUS_FIRST = MENU_BOX_NAME_FIRST + MENU_ITEM_COUNT, /*!< Status of the first item */
    MENU_BOX_COUNT = MENU_BOX_STATUS_FIRST + MENU_ITEM_COUNT,
};

/*!
 * \brief State and boxes of the menu page, mounted as a libraster interface.
 */
struct MenuHandler {
    enum MenuItem cursor;                /*!< Item under the cursor, never a locked one */
    bool unlocked;                       /*!< MISSION, CALIBRATIONS and SCREEN SAVER can be selected (D17, D22) */
    bool mission_open;                   /*!< The mission page replaces the list (TODO(#11)) */
    struct Label labels[MENU_BOX_COUNT]; /*!< Label of every box but the background */
    struct Box boxes[MENU_BOX_COUNT];    /*!< Interface, indexed by MenuBoxId */
};

#endif // MENU_H
