/*!
 * \file debug-api.h
 * \date 2026-11-19
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief Public API of the debug tabs: build, switch, sync, and the helpers
 *     the tabs' sync functions use to fill their slots.
 */

#ifndef DEBUG_API_H
#define DEBUG_API_H

#include "ui/debug/debug.h"

#include <stdbool.h>

/*!
 * \brief FL, FR, RL, RR: sub-labels of a four-wheel QUAD.
 */
extern const char *const debug_api_corner_labels[4];

/*!
 * \brief TANK 1, TANK 2, LINE 1, LINE 2: sub-labels of an EBS pressure QUAD.
 */
extern const char *const debug_api_ebs_labels[4];

/*!
 * \brief Build the first tab.
 *
 * \param[out] handler Storage.
 * \param[in] pages Every tab, DEBUG_TAB_COUNT entries in DebugTab order.
 *
 * \retval DEBUG_RC_OK on success.
 * \retval DEBUG_RC_NULL_POINTER if an argument is NULL.
 * \retval DEBUG_RC_ERROR if a tab doesn't fit or libraster rejected a box.
 */
enum DebugReturnCode debug_api_init(struct DebugHandler *handler, const struct DebugPage *const *pages);

/*!
 * \brief Show another tab: its boxes replace the current ones, all flagged.
 *
 * \retval DEBUG_RC_OK on success.
 * \retval DEBUG_RC_NULL_POINTER if \p handler is NULL.
 * \retval DEBUG_RC_ERROR if \p tab is out of range or doesn't fit.
 */
enum DebugReturnCode debug_api_set_tab(struct DebugHandler *handler, enum DebugTab tab);

/*!
 * \brief Move \p delta tabs forward (negative: back), wrapping around.
 *
 * \return What debug_api_set_tab returned.
 */
enum DebugReturnCode debug_api_step(struct DebugHandler *handler, int8_t delta);

/*!
 * \brief Fill the shown tab from the UI data (only changed boxes are flagged).
 *
 * \retval DEBUG_RC_OK on success.
 * \retval DEBUG_RC_NULL_POINTER if an argument is NULL.
 */
enum DebugReturnCode debug_api_sync(struct DebugHandler *handler, const struct UIData *ui_data);

/*!
 * \brief Set a slot's text and colour, flagging its box only on a real change.
 */
__attribute__((format(printf, 4, 5))) void debug_api_text(struct DebugHandler *handler, uint8_t slot, uint32_t argb, const char *fmt, ...);

/*!
 * \brief "--" in a slot (stale or never received).
 */
void debug_api_no_value(struct DebugHandler *handler, uint8_t slot);

/*!
 * \brief A number with \p decimals digits after the point and a unit, "--" when stale.
 */
void debug_api_number(struct DebugHandler *handler, uint8_t slot, bool stale, float value, uint8_t decimals, const char *unit);

/*!
 * \brief Four numbers in slots slot .. slot + 3 (FL, FR, RL, RR).
 */
void debug_api_corners(struct DebugHandler *handler, uint8_t slot, bool stale, const struct UIDataCorners *values, uint8_t decimals, const char *unit);

/*!
 * \brief Background of a STATUS_ROW (name, status and voltage together); \p slot is its status slot.
 */
void debug_api_row_background(struct DebugHandler *handler, uint8_t slot, uint32_t argb);

/*!
 * \brief Whether a CAN source is stale in the UI data.
 */
bool debug_api_stale(const struct UIData *ui_data, enum UIDataSource source);

/*!
 * \brief A shutdown-circuit voltage as a closed / open contact, "OK" or "OPEN" (red).
 */
void debug_api_contact(struct DebugHandler *handler, uint8_t slot, bool stale, float voltage);

/*!
 * \brief A shutdown-circuit switch (ASMS, TSMS): "ON" (green) or "OFF" (grey).
 */
void debug_api_switch(struct DebugHandler *handler, uint8_t slot, bool stale, float voltage);

/*!
 * \brief Brake over-travel switch (PedalsBrake), as debug_api_contact.
 */
void debug_api_bots(struct DebugHandler *handler, uint8_t slot, const struct UIData *ui_data);

/*!
 * \brief Measured and commanded brake bias, both as the pedals board sends them.
 */
void debug_api_bias(struct DebugHandler *handler, uint8_t slot, uint8_t set_slot, const struct UIData *ui_data);

/*!
 * \brief EBS tank 1, tank 2, line 1, line 2 pressures in slots slot .. slot + 3.
 */
void debug_api_ebs(struct DebugHandler *handler, uint8_t slot, const struct UIData *ui_data);

#endif // DEBUG_API_H
