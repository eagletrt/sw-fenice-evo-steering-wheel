/*!
 * \file debug.h
 * \date 2026-11-19
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief Definitions for the debug tabs (Figma page "6 Debug").
 *
 * \details Every tab is a black tab bar plus a list of elements on a
 *     LIGHT_GRAY screen (the gaps are the grid lines). An element is one
 *     Figma shape (a cell, a four-wheel panel, a shutdown row, ...); the
 *     engine (debug-api.c) turns it into libraster boxes, one per text, and
 *     numbers its values as slots. A tab's sync function fills the slots
 *     from the UI data. Only the shown tab has boxes: switching tab rebuilds
 *     them in the same storage.
 */

#ifndef DEBUG_H
#define DEBUG_H

#include "box.h"
#include "label.h"
#include "ui/data/ui-data.h"
#include "ui/style.h"

#include <stdint.h>

#define DEBUG_COLOR_SCREEN (STYLE_LIGHT_GRAY) /*!< Grid lines */
#define DEBUG_COLOR_BAR (STYLE_BLACK)         /*!< Tab bar, table header */
#define DEBUG_COLOR_CELL (STYLE_DARK_GRAY)
#define DEBUG_COLOR_LABEL (STYLE_LIGHT_GRAY) /*!< Captions, tab names, values not to look at */
#define DEBUG_COLOR_VALUE (STYLE_WHITE)
#define DEBUG_COLOR_OK (STYLE_GREEN)
#define DEBUG_COLOR_WARNING (STYLE_YELLOW)
#define DEBUG_COLOR_ERROR (STYLE_RED)
#define DEBUG_COLOR_COLD (STYLE_CYAN)
#define DEBUG_COLOR_AS (STYLE_BLUE) /*!< ASSI blue (finished, emergency) */

#define DEBUG_BAR_HEIGHT (56U)
#define DEBUG_MAX_BOXES (72U)     /*!< Boxes of the biggest tab (MOTORS: 65) */
#define DEBUG_MAX_SLOTS (64U)     /*!< Values of the biggest tab */
#define DEBUG_TEXT_SIZE (32U)     /*!< Per-box text, terminator included */
#define DEBUG_NO_SLOT (UINT8_MAX) /*!< DebugHandler.slot_box entry of an unused slot */
#define DEBUG_NO_VALUE "--"       /*!< Stale or never received */

/*!
 * \brief Return codes for the debug tabs.
 */
enum DebugReturnCode {
    DEBUG_RC_OK,
    DEBUG_RC_NULL_POINTER,
    DEBUG_RC_ERROR, /*!< A tab doesn't fit the storage, or libraster rejected a box */
};

/*!
 * \brief The tabs, in cycling order (Figma numbering, WHEEL last).
 */
enum DebugTab : uint8_t {
    DEBUG_TAB_HV,
    DEBUG_TAB_MOTORS,
    DEBUG_TAB_COOLING,
    DEBUG_TAB_INVERTERS,
    DEBUG_TAB_PEDALS,
    DEBUG_TAB_BRAKES,
    DEBUG_TAB_SHUTDOWN,
    DEBUG_TAB_LV,
    DEBUG_TAB_PIPO,
    DEBUG_TAB_AS,
    DEBUG_TAB_DYNAMICS,
    DEBUG_TAB_SW,
    DEBUG_TAB_WHEEL,
    DEBUG_TAB_COUNT,
};

/*!
 * \brief Shapes of the Figma debug pages. Slots are numbered from DebugElement.slot.
 */
enum DebugElementKind : uint8_t {
    DEBUG_ELEMENT_CELL,         /*!< Caption over a value: 1 slot */
    DEBUG_ELEMENT_QUAD,         /*!< Title, then 2 x 2 sub-label + value (FL FR / RL RR): 4 slots */
    DEBUG_ELEMENT_COLUMNS,      /*!< Title, then 4 columns of sub-label + value: 4 slots */
    DEBUG_ELEMENT_STRIP,        /*!< Caption on the left, \c count values in a row: \c count slots */
    DEBUG_ELEMENT_STATUS_ROW,   /*!< Name on the left, status over a voltage on the right: 2 slots (status, voltage; the row background follows the status) */
    DEBUG_ELEMENT_LINE,         /*!< Name over a status line on the left, big value on the right: 2 slots (status, value) */
    DEBUG_ELEMENT_TABLE_HEADER, /*!< Four column captions on black: no slot */
    DEBUG_ELEMENT_TABLE_ROW,    /*!< Name, then three columns: 3 slots */
};

/*!
 * \brief One Figma shape of a tab.
 */
struct DebugElement {
    const char *caption;       /*!< Cell caption, panel title, row name */
    const char *const *labels; /*!< Sub-labels (QUAD, COLUMNS: 4) or column captions (TABLE_HEADER: 4) */
    struct BoxRectangle rect;  /*!< Where the shape sits on the 800x480 screen */
    enum DebugElementKind kind;
    uint8_t slot;       /*!< First value slot */
    uint8_t value_size; /*!< Pixel height of the values */
    uint8_t label_y;    /*!< QUAD: sub-label top in the first row, from the panel top */
    uint8_t pitch;      /*!< QUAD: distance between the two rows */
    uint8_t count;      /*!< STRIP: number of values */
};

/*! \brief Left edge of a column of the 4 x 4 grid (Figma cells 198 x 104, 2 px gaps). */
#define DEBUG_GRID_X(column) ((uint16_t)(1U + (200U * (column))))
/*! \brief Top edge of a row of the 4 x 4 grid. */
#define DEBUG_GRID_Y(row) ((uint16_t)(57U + (106U * (row))))
#define DEBUG_GRID_W (198U)
#define DEBUG_GRID_H (104U)

/*! \brief A CELL (Figma Cell/Debug). */
#define DEBUG_CELL(X, Y, W, H, CAPTION, SLOT, SIZE) { .kind = DEBUG_ELEMENT_CELL, .rect = { (X), (Y), (W), (H) }, .caption = (CAPTION), .slot = (SLOT), .value_size = (SIZE) }
/*! \brief A CELL of the 4 x 4 grid with a 32 px value. */
#define DEBUG_GRID_CELL(COLUMN, ROW, CAPTION, SLOT) DEBUG_CELL(DEBUG_GRID_X(COLUMN), DEBUG_GRID_Y(ROW), DEBUG_GRID_W, DEBUG_GRID_H, CAPTION, SLOT, 32U)
/*! \brief A QUAD: LABEL_Y and PITCH from the Figma panel (52 / 89 for 28 px values, 55 / 89 for 24 px, 46 / 71 in 174 px panels). */
#define DEBUG_QUAD(X, Y, W, H, CAPTION, LABELS, SLOT, SIZE, LABEL_Y, PITCH) { .kind = DEBUG_ELEMENT_QUAD, .rect = { (X), (Y), (W), (H) }, .caption = (CAPTION), .labels = (LABELS), .slot = (SLOT), .value_size = (SIZE), .label_y = (LABEL_Y), .pitch = (PITCH) }
/*! \brief A STATUS_ROW. */
#define DEBUG_STATUS_ROW(X, Y, CAPTION, SLOT) { .kind = DEBUG_ELEMENT_STATUS_ROW, .rect = { (X), (Y), 398U, 51U }, .caption = (CAPTION), .slot = (SLOT) }
/*! \brief A LINE (PIPO output). */
#define DEBUG_LINE(X, Y, W, CAPTION, SLOT) { .kind = DEBUG_ELEMENT_LINE, .rect = { (X), (Y), (W), 80U }, .caption = (CAPTION), .slot = (SLOT), .value_size = 32U }
/*! \brief A TABLE_ROW (SW tab). */
#define DEBUG_TABLE_ROW(Y, CAPTION, SLOT) { .kind = DEBUG_ELEMENT_TABLE_ROW, .rect = { 1U, (Y), 798U, 46U }, .caption = (CAPTION), .slot = (SLOT), .value_size = 22U }

struct DebugHandler;

/*!
 * \brief Fill a tab's value slots from the UI data.
 */
typedef void (*debug_sync_callback)(struct DebugHandler *handler, const struct UIData *ui_data);

/*!
 * \brief A tab: its name in the tab bar, its shapes and how to fill them.
 */
struct DebugPage {
    const char *name;
    const struct DebugElement *elements;
    uint8_t element_count;
    debug_sync_callback sync;
};

/*!
 * \brief Storage of the shown tab.
 */
struct DebugHandler {
    const struct DebugPage *const *pages; /*!< Every tab, in DebugTab order */
    enum DebugTab tab;                    /*!< Shown tab */
    uint16_t box_count;                   /*!< Boxes of the shown tab */
    uint8_t slot_box[DEBUG_MAX_SLOTS];    /*!< Box of each value slot, DEBUG_NO_SLOT if unused */
    char text[DEBUG_MAX_BOXES][DEBUG_TEXT_SIZE];
    struct Label labels[DEBUG_MAX_BOXES];
    struct Box boxes[DEBUG_MAX_BOXES];
};

#endif // DEBUG_H
