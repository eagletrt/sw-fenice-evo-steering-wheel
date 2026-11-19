/*!
 * \file debug-tabs.h
 * \date 2026-11-19
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief Every debug tab. Each one lives in its own folder,
 *     Core/Src/steering/ui/debug/<tab>/debug-<tab>.c: its Figma layout as a
 *     table of elements, and the sync that fills it from the UI data.
 */

#ifndef DEBUG_TABS_H
#define DEBUG_TABS_H

#include "ui/debug/debug.h"

extern const struct DebugPage debug_hv_page;
extern const struct DebugPage debug_motors_page;
extern const struct DebugPage debug_cooling_page;
extern const struct DebugPage debug_inverters_page;
extern const struct DebugPage debug_pedals_page;
extern const struct DebugPage debug_brakes_page;
extern const struct DebugPage debug_shutdown_page;
extern const struct DebugPage debug_lv_page;
extern const struct DebugPage debug_pipo_page;
extern const struct DebugPage debug_as_page;
extern const struct DebugPage debug_dynamics_page;
extern const struct DebugPage debug_sw_page;
extern const struct DebugPage debug_wheel_page;

/*!
 * \brief The tabs in cycling order, indexed by DebugTab (for debug_api_init).
 */
extern const struct DebugPage *const debug_tabs[DEBUG_TAB_COUNT];

#endif // DEBUG_TABS_H
