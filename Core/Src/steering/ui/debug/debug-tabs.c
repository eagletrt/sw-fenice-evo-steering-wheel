/*!
 * \file debug-tabs.c
 * \date 2026-11-19
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief Order of the debug tabs (the bottom paddles cycle through it).
 */

#include "ui/debug/debug-tabs.h"

const struct DebugPage *const debug_tabs[DEBUG_TAB_COUNT] = {
    [DEBUG_TAB_HV] = &debug_hv_page,
    [DEBUG_TAB_MOTORS] = &debug_motors_page,
    [DEBUG_TAB_COOLING] = &debug_cooling_page,
    [DEBUG_TAB_INVERTERS] = &debug_inverters_page,
    [DEBUG_TAB_PEDALS] = &debug_pedals_page,
    [DEBUG_TAB_BRAKES] = &debug_brakes_page,
    [DEBUG_TAB_SHUTDOWN] = &debug_shutdown_page,
    [DEBUG_TAB_LV] = &debug_lv_page,
    [DEBUG_TAB_PIPO] = &debug_pipo_page,
    [DEBUG_TAB_AS] = &debug_as_page,
    [DEBUG_TAB_DYNAMICS] = &debug_dynamics_page,
    [DEBUG_TAB_SW] = &debug_sw_page,
    [DEBUG_TAB_WHEEL] = &debug_wheel_page,
};
