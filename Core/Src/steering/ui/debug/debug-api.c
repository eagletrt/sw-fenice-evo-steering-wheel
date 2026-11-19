/*!
 * \file debug-api.c
 * \date 2026-11-19
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief Debug tab engine: turns a tab's elements into boxes (geometry from
 *     the Figma components) and gives the tabs helpers to fill their slots.
 *
 * \details Boxes of one element tile it, except QUAD, COLUMNS and STRIP,
 *     which first draw a panel box and then their texts over it: a value box
 *     repaints only its own rectangle, in the panel colour, so the panel never
 *     needs redrawing after the tab is mounted.
 */

#include "ui/debug/debug-api.h"

#include "box-api.h"
#include "eagletrt-api.h"
#include "font-api.h"
#include "label-api.h"
#include "raster-fonts.h"

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#define DEBUG_CAPTION_SIZE (16U)     /*!< Cell captions, panel titles */
#define DEBUG_SUBLABEL_SIZE (12U)    /*!< FL / FR / RL / RR */
#define DEBUG_CELL_CAPTION_H (36U)   /*!< Caption part of a cell (Figma: caption at y 12, value at y 39) */
#define DEBUG_ROW_STATUS_W (130U)    /*!< Status part of a STATUS_ROW */
#define DEBUG_ROW_STATUS_H (30U)     /*!< Status line height, the voltage below it */
#define DEBUG_LINE_VALUE_W (110U)    /*!< Value part of a LINE */
#define DEBUG_STRIP_CAPTION_W (104U) /*!< Caption part of a STRIP */
#define DEBUG_STRIP_VALUE_W (92U)    /*!< One value of a STRIP */
#define DEBUG_PAD (16)               /*!< Left / right text inset */
#define DEBUG_BIAS_MIN_BAR (1.0F)    /*!< Below this total brake pressure the bias means nothing */

const char *const debug_api_corner_labels[4] = { "FL", "FR", "RL", "RR" };
const char *const debug_api_ebs_labels[4] = { "TANK 1", "TANK 2", "LINE 1", "LINE 2" };

/*! \brief Table columns (SW tab): name, libcan, commit, state. */
EAGLETRT_STATIC const uint16_t debug_table_columns[] = { 0U, 329U, 479U, 600U, 798U };

/*!
 * \brief Text of one box.
 */
struct DebugText {
    const char *text; /*!< Static text, NULL for a value slot */
    int16_t x;        /*!< Label anchor inside the box */
    int16_t y;        /*!< Label top inside the box */
    uint16_t size;
    enum FontAlignment align;
    uint32_t argb;
};

/*!
 * \brief Append one box (and its label when \p text is not NULL).
 *
 * \param slot Value slot of the box, DEBUG_NO_SLOT for a static text or a plain fill.
 */
EAGLETRT_STATIC enum DebugReturnCode prv_debug_api_box(struct DebugHandler *handler, struct BoxRectangle rect, uint32_t background, const struct DebugText *text, uint8_t slot) {
    const uint16_t index = handler->box_count;
    if (index >= DEBUG_MAX_BOXES || (slot != DEBUG_NO_SLOT && slot >= DEBUG_MAX_SLOTS)) {
        return DEBUG_RC_ERROR;
    }
    struct Label *label = NULL;
    if (text != NULL) {
        (void)snprintf(handler->text[index], DEBUG_TEXT_SIZE, "%s", (slot != DEBUG_NO_SLOT) ? DEBUG_NO_VALUE : text->text);
        label = &handler->labels[index];
        if (label_api_init(label, handler->text[index], text->x, text->y, &font_inter, text->size, text->align, (struct Color){ .argb = text->argb }) != RASTER_RC_OK) {
            return DEBUG_RC_ERROR;
        }
    }
    if (box_api_init(&handler->boxes[index], index, rect, (struct Color){ .argb = background }, label) != RASTER_RC_OK) {
        return DEBUG_RC_ERROR;
    }
    handler->boxes[index].updated = true;
    if (slot != DEBUG_NO_SLOT) {
        handler->slot_box[slot] = (uint8_t)index;
    }
    handler->box_count++;
    return DEBUG_RC_OK;
}

/*! \brief Centred text in a box \p w wide. */
#define DEBUG_CENTER(w, ty, sz, colour, str) ((struct DebugText){ (str), (int16_t)((w) / 2U), (int16_t)(ty), (sz), FONT_ALIGN_CENTER, (colour) })
/*! \brief Left-aligned text. */
#define DEBUG_LEFT(ty, sz, colour, str) ((struct DebugText){ (str), DEBUG_PAD, (int16_t)(ty), (sz), FONT_ALIGN_LEFT, (colour) })
/*! \brief Right-aligned text in a box \p w wide. */
#define DEBUG_RIGHT(w, ty, sz, colour, str) ((struct DebugText){ (str), (int16_t)((w) - (uint16_t)DEBUG_PAD), (int16_t)(ty), (sz), FONT_ALIGN_RIGHT, (colour) })
/*! \brief Rectangle shorthand. */
#define DEBUG_RECT(x, y, w, h) ((struct BoxRectangle){ (uint16_t)(x), (uint16_t)(y), (uint16_t)(w), (uint16_t)(h) })

/*!
 * \brief Boxes of a QUAD: panel, title, then FL FR / RL RR as sub-label + value.
 */
EAGLETRT_STATIC enum DebugReturnCode prv_debug_api_quad(struct DebugHandler *handler, const struct DebugElement *element) {
    const struct BoxRectangle r = element->rect;
    const uint16_t half = r.width / 2U;
    const uint16_t first_row = (uint16_t)(element->label_y - 4U); // the sub-label sits 4 px into its box
    enum DebugReturnCode rc = prv_debug_api_box(handler, r, DEBUG_COLOR_CELL, NULL, DEBUG_NO_SLOT);
    const struct DebugText title = DEBUG_CENTER(r.width, 8, DEBUG_CAPTION_SIZE, DEBUG_COLOR_LABEL, element->caption);
    rc = (rc == DEBUG_RC_OK) ? prv_debug_api_box(handler, DEBUG_RECT(r.x, r.y, r.width, first_row), DEBUG_COLOR_CELL, &title, DEBUG_NO_SLOT) : rc;
    for (uint8_t i = 0U; i < 4U && rc == DEBUG_RC_OK; i++) {
        const uint16_t x = (uint16_t)(r.x + ((i % 2U) * half));
        const uint16_t top = (uint16_t)(r.y + first_row + ((i / 2U) * element->pitch));
        const uint16_t value_h = (uint16_t)EAGLETRT_API_MIN(element->pitch - 16U, (r.y + r.height) - (top + 16U));
        const struct DebugText sub = DEBUG_CENTER(half, 4, DEBUG_SUBLABEL_SIZE, DEBUG_COLOR_LABEL, element->labels[i]);
        const struct DebugText value = DEBUG_CENTER(half, 4, element->value_size, DEBUG_COLOR_VALUE, "");
        rc = prv_debug_api_box(handler, DEBUG_RECT(x, top, half, 16U), DEBUG_COLOR_CELL, &sub, DEBUG_NO_SLOT);
        rc = (rc == DEBUG_RC_OK) ? prv_debug_api_box(handler, DEBUG_RECT(x, top + 16U, half, value_h), DEBUG_COLOR_CELL, &value, (uint8_t)(element->slot + i)) : rc;
    }
    return rc;
}

/*!
 * \brief Boxes of COLUMNS: panel, title, then four columns of sub-label + value.
 */
EAGLETRT_STATIC enum DebugReturnCode prv_debug_api_columns(struct DebugHandler *handler, const struct DebugElement *element) {
    const struct BoxRectangle r = element->rect;
    const uint16_t quarter = r.width / 4U;
    enum DebugReturnCode rc = prv_debug_api_box(handler, r, DEBUG_COLOR_CELL, NULL, DEBUG_NO_SLOT);
    const struct DebugText title = DEBUG_CENTER(r.width, 10, DEBUG_CAPTION_SIZE, DEBUG_COLOR_LABEL, element->caption);
    rc = (rc == DEBUG_RC_OK) ? prv_debug_api_box(handler, DEBUG_RECT(r.x, r.y, r.width, 36U), DEBUG_COLOR_CELL, &title, DEBUG_NO_SLOT) : rc;
    for (uint8_t i = 0U; i < 4U && rc == DEBUG_RC_OK; i++) {
        const uint16_t x = (uint16_t)(r.x + (i * quarter));
        const struct DebugText sub = DEBUG_CENTER(quarter, 4, DEBUG_SUBLABEL_SIZE, DEBUG_COLOR_LABEL, element->labels[i]);
        const struct DebugText value = DEBUG_CENTER(quarter, 2, element->value_size, DEBUG_COLOR_VALUE, "");
        rc = prv_debug_api_box(handler, DEBUG_RECT(x, r.y + 36U, quarter, 18U), DEBUG_COLOR_CELL, &sub, DEBUG_NO_SLOT);
        rc = (rc == DEBUG_RC_OK) ? prv_debug_api_box(handler, DEBUG_RECT(x, r.y + 54U, quarter, r.height - 54U), DEBUG_COLOR_CELL, &value, (uint8_t)(element->slot + i)) : rc;
    }
    return rc;
}

/*!
 * \brief Boxes of a STRIP: panel, caption on the left, values in a row.
 */
EAGLETRT_STATIC enum DebugReturnCode prv_debug_api_strip(struct DebugHandler *handler, const struct DebugElement *element) {
    const struct BoxRectangle r = element->rect;
    if (DEBUG_STRIP_CAPTION_W + (element->count * DEBUG_STRIP_VALUE_W) > r.width) {
        return DEBUG_RC_ERROR;
    }
    enum DebugReturnCode rc = prv_debug_api_box(handler, r, DEBUG_COLOR_CELL, NULL, DEBUG_NO_SLOT);
    const struct DebugText caption = DEBUG_LEFT(8, DEBUG_CAPTION_SIZE, DEBUG_COLOR_LABEL, element->caption);
    rc = (rc == DEBUG_RC_OK) ? prv_debug_api_box(handler, DEBUG_RECT(r.x, r.y, DEBUG_STRIP_CAPTION_W, r.height), DEBUG_COLOR_CELL, &caption, DEBUG_NO_SLOT) : rc;
    for (uint8_t i = 0U; i < element->count && rc == DEBUG_RC_OK; i++) {
        const struct DebugText value = DEBUG_CENTER(DEBUG_STRIP_VALUE_W, 30, element->value_size, DEBUG_COLOR_VALUE, "");
        rc = prv_debug_api_box(handler, DEBUG_RECT(r.x + DEBUG_STRIP_CAPTION_W + (i * DEBUG_STRIP_VALUE_W), r.y, DEBUG_STRIP_VALUE_W, r.height), DEBUG_COLOR_CELL, &value, (uint8_t)(element->slot + i));
    }
    return rc;
}

/*!
 * \brief Boxes of the SW table: header captions or a board row.
 */
EAGLETRT_STATIC enum DebugReturnCode prv_debug_api_table(struct DebugHandler *handler, const struct DebugElement *element) {
    const struct BoxRectangle r = element->rect;
    const bool header = element->kind == DEBUG_ELEMENT_TABLE_HEADER;
    const uint32_t background = header ? DEBUG_COLOR_BAR : DEBUG_COLOR_CELL;
    enum DebugReturnCode rc = DEBUG_RC_OK;
    for (uint8_t column = 0U; column < 4U && rc == DEBUG_RC_OK; column++) {
        const uint16_t w = (uint16_t)(debug_table_columns[column + 1U] - debug_table_columns[column]);
        const char *text = header ? element->labels[column] : ((column == 0U) ? element->caption : "");
        const uint16_t size = header ? DEBUG_CAPTION_SIZE : ((column == 3U) ? 18U : element->value_size);
        const uint32_t argb = header ? DEBUG_COLOR_LABEL : DEBUG_COLOR_VALUE;
        const int16_t y = (!header && column == 3U) ? 12 : 10;
        struct DebugText t = DEBUG_LEFT(y, size, argb, text);
        if (column == 3U) {
            t = DEBUG_RIGHT(w, y, size, argb, text);
        } else if (column > 0U) {
            t.x = 1; // the column starts 1 px left of the Figma text
        }
        const uint8_t slot = (header || column == 0U) ? DEBUG_NO_SLOT : (uint8_t)(element->slot + column - 1U);
        rc = prv_debug_api_box(handler, DEBUG_RECT(r.x + debug_table_columns[column], r.y, w, r.height), background, &t, slot);
    }
    return rc;
}

/*!
 * \brief Boxes of one element.
 */
EAGLETRT_STATIC enum DebugReturnCode prv_debug_api_element(struct DebugHandler *handler, const struct DebugElement *element) {
    const struct BoxRectangle r = element->rect;
    enum DebugReturnCode rc = DEBUG_RC_OK;
    switch (element->kind) {
        case DEBUG_ELEMENT_CELL: {
            const struct DebugText caption = DEBUG_CENTER(r.width, 12, DEBUG_CAPTION_SIZE, DEBUG_COLOR_LABEL, element->caption);
            const struct DebugText value = DEBUG_CENTER(r.width, 3, element->value_size, DEBUG_COLOR_VALUE, "");
            rc = prv_debug_api_box(handler, DEBUG_RECT(r.x, r.y, r.width, DEBUG_CELL_CAPTION_H), DEBUG_COLOR_CELL, &caption, DEBUG_NO_SLOT);
            return (rc == DEBUG_RC_OK) ? prv_debug_api_box(handler, DEBUG_RECT(r.x, r.y + DEBUG_CELL_CAPTION_H, r.width, r.height - DEBUG_CELL_CAPTION_H), DEBUG_COLOR_CELL, &value, element->slot) : rc;
        }
        case DEBUG_ELEMENT_QUAD:
            return prv_debug_api_quad(handler, element);
        case DEBUG_ELEMENT_COLUMNS:
            return prv_debug_api_columns(handler, element);
        case DEBUG_ELEMENT_STRIP:
            return prv_debug_api_strip(handler, element);
        case DEBUG_ELEMENT_STATUS_ROW: {
            const uint16_t name_w = r.width - DEBUG_ROW_STATUS_W;
            const struct DebugText name = DEBUG_LEFT(13, 20U, DEBUG_COLOR_VALUE, element->caption);
            const struct DebugText status = DEBUG_RIGHT(DEBUG_ROW_STATUS_W, 8, DEBUG_CAPTION_SIZE, DEBUG_COLOR_VALUE, "");
            const struct DebugText voltage = DEBUG_RIGHT(DEBUG_ROW_STATUS_W, 1, 14U, DEBUG_COLOR_LABEL, "");
            rc = prv_debug_api_box(handler, DEBUG_RECT(r.x, r.y, name_w, r.height), DEBUG_COLOR_CELL, &name, DEBUG_NO_SLOT);
            rc = (rc == DEBUG_RC_OK) ? prv_debug_api_box(handler, DEBUG_RECT(r.x + name_w, r.y, DEBUG_ROW_STATUS_W, DEBUG_ROW_STATUS_H), DEBUG_COLOR_CELL, &status, element->slot) : rc;
            return (rc == DEBUG_RC_OK) ? prv_debug_api_box(handler, DEBUG_RECT(r.x + name_w, r.y + DEBUG_ROW_STATUS_H, DEBUG_ROW_STATUS_W, r.height - DEBUG_ROW_STATUS_H), DEBUG_COLOR_CELL, &voltage, (uint8_t)(element->slot + 1U)) : rc;
        }
        case DEBUG_ELEMENT_LINE: {
            const uint16_t left_w = r.width - DEBUG_LINE_VALUE_W;
            const struct DebugText name = DEBUG_LEFT(12, 20U, DEBUG_COLOR_VALUE, element->caption);
            const struct DebugText status = DEBUG_LEFT(6, DEBUG_CAPTION_SIZE, DEBUG_COLOR_LABEL, "");
            const struct DebugText value = DEBUG_RIGHT(DEBUG_LINE_VALUE_W, 20, element->value_size, DEBUG_COLOR_VALUE, "");
            rc = prv_debug_api_box(handler, DEBUG_RECT(r.x, r.y, left_w, 40U), DEBUG_COLOR_CELL, &name, DEBUG_NO_SLOT);
            rc = (rc == DEBUG_RC_OK) ? prv_debug_api_box(handler, DEBUG_RECT(r.x, r.y + 40U, left_w, r.height - 40U), DEBUG_COLOR_CELL, &status, element->slot) : rc;
            return (rc == DEBUG_RC_OK) ? prv_debug_api_box(handler, DEBUG_RECT(r.x + left_w, r.y, DEBUG_LINE_VALUE_W, r.height), DEBUG_COLOR_CELL, &value, (uint8_t)(element->slot + 1U)) : rc;
        }
        case DEBUG_ELEMENT_TABLE_HEADER:
        case DEBUG_ELEMENT_TABLE_ROW:
            return prv_debug_api_table(handler, element);
        default:
            return DEBUG_RC_ERROR;
    }
}

/*!
 * \brief Screen background and tab bar: "< PREVIOUS", NAME, "n/N", "NEXT >".
 */
EAGLETRT_STATIC enum DebugReturnCode prv_debug_api_tab_bar(struct DebugHandler *handler) {
    static char names[3][DEBUG_TEXT_SIZE]; // the labels point here: one tab is built at a time
    const struct DebugPage *const *pages = handler->pages;
    const uint8_t tab = (uint8_t)handler->tab;
    (void)snprintf(names[0], DEBUG_TEXT_SIZE, "< %s", pages[(tab + DEBUG_TAB_COUNT - 1U) % DEBUG_TAB_COUNT]->name);
    (void)snprintf(names[1], DEBUG_TEXT_SIZE, "%s >", pages[(tab + 1U) % DEBUG_TAB_COUNT]->name);
    (void)snprintf(names[2], DEBUG_TEXT_SIZE, "%" PRIu8 "/%" PRIu8, (uint8_t)(tab + 1U), (uint8_t)DEBUG_TAB_COUNT);

    // The counter follows the centred title (Figma: 10 px right of it).
    struct FontBounds bounds = { 0 };
    if (font_api_bounds(0U, 0U, FONT_ALIGN_LEFT, &font_inter, pages[tab]->name, 36U, &bounds) != RASTER_RC_OK) {
        return DEBUG_RC_ERROR;
    }
    const uint16_t counter_x = (uint16_t)EAGLETRT_API_MIN(400U + (bounds.width / 2U) + 10U, 540U);

    const struct DebugText previous = DEBUG_LEFT(16, 20U, DEBUG_COLOR_LABEL, names[0]);
    const struct DebugText title = DEBUG_CENTER(300U, 6, 36U, DEBUG_COLOR_VALUE, pages[tab]->name);
    const struct DebugText counter = { names[2], 0, 20, DEBUG_CAPTION_SIZE, FONT_ALIGN_LEFT, DEBUG_COLOR_LABEL };
    const struct DebugText next = DEBUG_RIGHT(250U, 16, 20U, DEBUG_COLOR_LABEL, names[1]);
    enum DebugReturnCode rc = prv_debug_api_box(handler, DEBUG_RECT(0U, DEBUG_BAR_HEIGHT, STYLE_SCREEN_WIDTH, STYLE_SCREEN_HEIGHT - DEBUG_BAR_HEIGHT), DEBUG_COLOR_SCREEN, NULL, DEBUG_NO_SLOT);
    rc = (rc == DEBUG_RC_OK) ? prv_debug_api_box(handler, DEBUG_RECT(0U, 0U, 250U, DEBUG_BAR_HEIGHT), DEBUG_COLOR_BAR, &previous, DEBUG_NO_SLOT) : rc;
    rc = (rc == DEBUG_RC_OK) ? prv_debug_api_box(handler, DEBUG_RECT(250U, 0U, counter_x - 250U, DEBUG_BAR_HEIGHT), DEBUG_COLOR_BAR, &title, DEBUG_NO_SLOT) : rc;
    rc = (rc == DEBUG_RC_OK) ? prv_debug_api_box(handler, DEBUG_RECT(counter_x, 0U, 550U - counter_x, DEBUG_BAR_HEIGHT), DEBUG_COLOR_BAR, &counter, DEBUG_NO_SLOT) : rc;
    return (rc == DEBUG_RC_OK) ? prv_debug_api_box(handler, DEBUG_RECT(550U, 0U, 250U, DEBUG_BAR_HEIGHT), DEBUG_COLOR_BAR, &next, DEBUG_NO_SLOT) : rc;
}

enum DebugReturnCode debug_api_set_tab(struct DebugHandler *handler, enum DebugTab tab) {
    if (handler == NULL) {
        return DEBUG_RC_NULL_POINTER;
    }
    if ((uint8_t)tab >= DEBUG_TAB_COUNT) {
        return DEBUG_RC_ERROR;
    }
    handler->tab = tab;
    handler->box_count = 0U;
    memset(handler->slot_box, DEBUG_NO_SLOT, sizeof(handler->slot_box));
    enum DebugReturnCode rc = prv_debug_api_tab_bar(handler);
    const struct DebugPage *page = handler->pages[tab];
    for (uint8_t i = 0U; i < page->element_count && rc == DEBUG_RC_OK; i++) {
        rc = prv_debug_api_element(handler, &page->elements[i]);
    }
    return rc;
}

enum DebugReturnCode debug_api_init(struct DebugHandler *handler, const struct DebugPage *const *pages) {
    if (handler == NULL || pages == NULL) {
        return DEBUG_RC_NULL_POINTER;
    }
    memset(handler, 0, sizeof(*handler));
    handler->pages = pages;
    return debug_api_set_tab(handler, DEBUG_TAB_HV);
}

enum DebugReturnCode debug_api_step(struct DebugHandler *handler, int8_t delta) {
    if (handler == NULL) {
        return DEBUG_RC_NULL_POINTER;
    }
    const int16_t next = (int16_t)(((int16_t)handler->tab + (delta % (int16_t)DEBUG_TAB_COUNT) + (int16_t)DEBUG_TAB_COUNT) % (int16_t)DEBUG_TAB_COUNT);
    return debug_api_set_tab(handler, (enum DebugTab)next);
}

enum DebugReturnCode debug_api_sync(struct DebugHandler *handler, const struct UIData *ui_data) {
    if (handler == NULL || ui_data == NULL) {
        return DEBUG_RC_NULL_POINTER;
    }
    handler->pages[handler->tab]->sync(handler, ui_data);
    return DEBUG_RC_OK;
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
void debug_api_text(struct DebugHandler *handler, uint8_t slot, uint32_t argb, const char *fmt, ...) {
    if (slot >= DEBUG_MAX_SLOTS || handler->slot_box[slot] == DEBUG_NO_SLOT) {
        return;
    }
    const uint8_t index = handler->slot_box[slot];
    char scratch[DEBUG_TEXT_SIZE];
    va_list args;
    va_start(args, fmt);
    const int written = vsnprintf(scratch, sizeof(scratch), fmt, args);
    va_end(args);
    struct Label *label = &handler->labels[index];
    if (written < 0) {
        return;
    }
    if (strncmp(scratch, handler->text[index], DEBUG_TEXT_SIZE) != 0) {
        memcpy(handler->text[index], scratch, sizeof(scratch));
        handler->boxes[index].updated = true;
    }
    if (label->color.argb != argb) {
        label->color.argb = argb;
        handler->boxes[index].updated = true;
    }
}

void debug_api_no_value(struct DebugHandler *handler, uint8_t slot) {
    debug_api_text(handler, slot, DEBUG_COLOR_VALUE, "%s", DEBUG_NO_VALUE);
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
void debug_api_number(struct DebugHandler *handler, uint8_t slot, bool stale, float value, uint8_t decimals, const char *unit) {
    if (stale) {
        debug_api_no_value(handler, slot);
        return;
    }
    debug_api_text(handler, slot, DEBUG_COLOR_VALUE, "%.*f%s", (int)decimals, (double)value, unit);
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
void debug_api_corners(struct DebugHandler *handler, uint8_t slot, bool stale, const struct UIDataCorners *values, uint8_t decimals, const char *unit) {
    debug_api_number(handler, slot, stale, values->fl, decimals, unit);
    debug_api_number(handler, (uint8_t)(slot + 1U), stale, values->fr, decimals, unit);
    debug_api_number(handler, (uint8_t)(slot + 2U), stale, values->rl, decimals, unit);
    debug_api_number(handler, (uint8_t)(slot + 3U), stale, values->rr, decimals, unit);
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
void debug_api_row_background(struct DebugHandler *handler, uint8_t slot, uint32_t argb) {
    if (slot >= DEBUG_MAX_SLOTS || handler->slot_box[slot] == DEBUG_NO_SLOT || handler->slot_box[slot] == 0U) {
        return;
    }
    // A STATUS_ROW's boxes: name, status (this slot), voltage.
    const uint8_t index = handler->slot_box[slot];
    for (uint8_t i = (uint8_t)(index - 1U); i <= (uint8_t)(index + 1U); i++) {
        if (handler->boxes[i].color.argb != argb) {
            handler->boxes[i].color.argb = argb;
            handler->boxes[i].updated = true;
        }
    }
}

bool debug_api_stale(const struct UIData *ui_data, enum UIDataSource source) {
    return (ui_data->stale & UI_DATA_STALE_BIT(source)) != 0U;
}

void debug_api_contact(struct DebugHandler *handler, uint8_t slot, bool stale, float voltage) {
    if (stale) {
        debug_api_no_value(handler, slot);
        return;
    }
    const bool closed = voltage >= SHUTDOWN_CLOSED_V;
    debug_api_text(handler, slot, closed ? DEBUG_COLOR_OK : DEBUG_COLOR_ERROR, "%s", closed ? "OK" : "OPEN");
}

void debug_api_switch(struct DebugHandler *handler, uint8_t slot, bool stale, float voltage) {
    if (stale) {
        debug_api_no_value(handler, slot);
        return;
    }
    const bool on = voltage >= SHUTDOWN_CLOSED_V;
    debug_api_text(handler, slot, on ? DEBUG_COLOR_OK : DEBUG_COLOR_LABEL, "%s", on ? "ON" : "OFF");
}

void debug_api_bots(struct DebugHandler *handler, uint8_t slot, const struct UIData *ui_data) {
    debug_api_contact(handler, slot, debug_api_stale(ui_data, UI_DATA_SOURCE_BRAKE), ui_data->pedals.bots_v);
}

void debug_api_bias(struct DebugHandler *handler, uint8_t slot, uint8_t set_slot, const struct UIData *ui_data) {
    const struct UIDataCorners *pressure = &ui_data->pedals.pressure_bar;
    const float front = pressure->fl + pressure->fr;
    const float total = front + pressure->rl + pressure->rr;
    const bool stale = debug_api_stale(ui_data, UI_DATA_SOURCE_BRAKE) || total < DEBUG_BIAS_MIN_BAR;
    debug_api_number(handler, slot, stale, stale ? 0.0F : (front * 100.0F) / total, 1U, "%");
    debug_api_number(handler, set_slot, debug_api_stale(ui_data, UI_DATA_SOURCE_BRAKE_BIAS_SET), ui_data->pedals.bias_set, 1U, "%");
}

void debug_api_ebs(struct DebugHandler *handler, uint8_t slot, const struct UIData *ui_data) {
    const struct UIDataCorners ebs = { ui_data->pedals.ebs_tank_bar[0], ui_data->pedals.ebs_tank_bar[1], ui_data->pedals.ebs_line_bar[0], ui_data->pedals.ebs_line_bar[1] };
    debug_api_corners(handler, slot, debug_api_stale(ui_data, UI_DATA_SOURCE_EBS), &ebs, 1U, " bar");
}
