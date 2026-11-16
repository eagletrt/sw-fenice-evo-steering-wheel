/*!
 * \file menu-api.h
 * \date 2026-11-16
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief Public API of the menu page.
 */

#ifndef MENU_API_H
#define MENU_API_H

#include "font.h"
#include "menu.h"

/*!
 * \brief Build the menu boxes, locked, with the cursor on the first selectable item.
 *
 * \param handler The menu to build.
 * \param font Font of every text.
 *
 * \retval MENU_RC_OK on success.
 * \retval MENU_RC_NULL_POINTER if \p handler or \p font is NULL.
 * \retval MENU_RC_ERROR if libraster rejected a box or a label.
 */
enum MenuReturnCode menu_api_init(struct MenuHandler *handler, const struct Font *font);

/*!
 * \brief Back to the item list, cursor on the first selectable item.
 *
 * \param handler The menu.
 */
void menu_api_open(struct MenuHandler *handler);

/*!
 * \brief Lock or unlock MISSION, CALIBRATIONS and SCREEN SAVER.
 *
 * \details Only boxes whose look changes are flagged, so calling it every
 *     frame is cheap. A cursor left on a locked item moves to the first
 *     selectable one.
 *
 * \param handler The menu.
 * \param unlocked Whether they can be selected.
 */
void menu_api_set_unlocked(struct MenuHandler *handler, bool unlocked);

/*!
 * \brief Handle one menu input.
 *
 * \details MOVE skips locked items and stops at the first and last item.
 *     SELECT on MISSION opens the mission page, BACK there closes it; BACK
 *     on the list is the same as EXIT. Moving and selecting do nothing
 *     while the mission page is open.
 *
 * \param handler The menu.
 * \param input The input.
 * \param amount MOVE only: rows to move, positive = down.
 *
 * \return What the wheel should do.
 */
enum MenuAction menu_api_handle(struct MenuHandler *handler, enum MenuInput input, int8_t amount);

#endif // MENU_API_H
