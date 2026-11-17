/*!
 * \file debug-wheel.h
 * \date 2026-11-17
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief Definitions for the WHEEL debug page: the steering wheel's own health.
 *
 * \details Layout from the Figma draft "Debug 13/13 - WHEEL" (page "6 Debug"):
 *     a black tab bar, then a 4x4 grid of DARK_GRAY cells on LIGHT_GRAY. Each
 *     cell is a caption box and a value box, so a value change redraws only
 *     its value. The data is a WheelHealth snapshot taken once per scheduler
 *     statistics window.
 */

#ifndef DEBUG_WHEEL_H
#define DEBUG_WHEEL_H

#include "box.h"
#include "label.h"
#include "style.h"

#include <stdint.h>

#define DEBUG_WHEEL_COLOR_SCREEN (STYLE_LIGHT_GRAY) /*!< Grid lines */
#define DEBUG_WHEEL_COLOR_BAR (STYLE_BLACK)         /*!< Tab bar */
#define DEBUG_WHEEL_COLOR_CELL (STYLE_DARK_GRAY)
#define DEBUG_WHEEL_COLOR_LABEL (STYLE_LIGHT_GRAY) /*!< Captions and the tab counter */
#define DEBUG_WHEEL_COLOR_VALUE (STYLE_WHITE)
#define DEBUG_WHEEL_COLOR_OK (STYLE_GREEN)
#define DEBUG_WHEEL_COLOR_WARNING (STYLE_YELLOW)
#define DEBUG_WHEEL_COLOR_ERROR (STYLE_RED)

#define DEBUG_WHEEL_BAR_HEIGHT (56U)
#define DEBUG_WHEEL_CELL_X (1U)  /*!< Left edge of the first column */
#define DEBUG_WHEEL_CELL_Y (57U) /*!< Top edge of the first row */
#define DEBUG_WHEEL_CELL_WIDTH (198U)
#define DEBUG_WHEEL_CELL_HEIGHT (104U)
#define DEBUG_WHEEL_CELL_PITCH_X (200U)
#define DEBUG_WHEEL_CELL_PITCH_Y (106U)
#define DEBUG_WHEEL_LABEL_HEIGHT (41U) /*!< Caption part of a cell, the value box is the rest */
#define DEBUG_WHEEL_COLUMNS (4U)
#define DEBUG_WHEEL_TITLE "WHEEL"
#define DEBUG_WHEEL_COUNTER "1/1" /*!< TODO(#13): tab position once the other pages exist */
#define DEBUG_WHEEL_NO_VALUE "--"

/*!
 * \brief Return codes for the WHEEL debug page.
 */
enum DebugWheelReturnCode {
    DEBUG_WHEEL_RC_OK,
    DEBUG_WHEEL_RC_NULL_POINTER,
    DEBUG_WHEEL_RC_ERROR, /*!< libraster rejected a box or a label */
};

/*!
 * \brief CAN controller state.
 */
enum WheelHealthBusState : uint8_t {
    WHEEL_HEALTH_BUS_ACTIVE,  /*!< Error active: sends and receives normally */
    WHEEL_HEALTH_BUS_PASSIVE, /*!< Error passive: a counter went past 127 */
    WHEEL_HEALTH_BUS_OFF,     /*!< Bus-off: TEC went past 255 */
};

/*!
 * \brief Why the MCU last reset (RCC_RSR at boot, most telling flag).
 */
enum WheelHealthResetCause : uint8_t {
    WHEEL_HEALTH_RESET_UNKNOWN,
    WHEEL_HEALTH_RESET_POR, /*!< Power-on */
    WHEEL_HEALTH_RESET_BOR, /*!< Brown-out: the supply dipped */
    WHEEL_HEALTH_RESET_PIN, /*!< NRST pin */
    WHEEL_HEALTH_RESET_SFT, /*!< Software: CAN reset request or Error_Handler */
    WHEEL_HEALTH_RESET_WDG, /*!< Watchdog */
    WHEEL_HEALTH_RESET_COUNT,
};

/*!
 * \brief Health snapshot of the steering wheel, one per statistics window.
 *
 * \details The scheduler fills the window, frame, task and uptime fields;
 *     the hardware hook (scheduler_read_health_callback) fills the CAN, DMA2D
 *     and reset-cause fields. The CAN fields describe the primary bus.
 */
struct WheelHealth {
    enum WheelHealthBusState can_bus_state;
    uint8_t can_last_error; /*!< FDCAN PSR.LEC: 0 none, 1 stuff, 2 form, 3 ack, 4 bit1, 5 bit0, 6 crc, 7 unchanged */
    enum WheelHealthResetCause reset_cause;
    uint32_t can_transmit_errors; /*!< TEC */
    uint32_t can_receive_errors;  /*!< REC */
    uint32_t can_bus_offs;        /*!< Bus-off restarts since boot */
    uint32_t can_tx_fifo_free;    /*!< Free Tx FIFO elements */
    uint32_t can_tx_fifo_size;    /*!< Tx FIFO elements */
    uint32_t can_tx_refused;      /*!< Frames refused by a full Tx FIFO since boot */
    uint32_t can_tx_unjams;       /*!< Times every pending Tx request was cancelled */
    uint32_t can_restricted;      /*!< Times restricted operation mode was left */
    uint32_t frame_max_us;        /*!< Longest frame (first DMA2D transfer to the swap) in the window */
    uint32_t dma2d_errors;        /*!< DMA2D transfer errors since boot */

    uint32_t window_ms;     /*!< Length of the statistics window */
    uint32_t frames;        /*!< Frames presented in the window */
    uint32_t render_max_us; /*!< Longest render task run in the window (CPU) */
    uint32_t inputs_max_us; /*!< Longest inputs task run in the window */
    uint32_t dropped_runs;  /*!< libtimebase task runs dropped in the window */
    uint32_t uptime_s;
};

/*!
 * \brief The 16 cells, row by row.
 */
enum DebugWheelCell : uint8_t {
    DEBUG_WHEEL_CELL_CAN_BUS,
    DEBUG_WHEEL_CELL_TEC_REC,
    DEBUG_WHEEL_CELL_LAST_ERROR,
    DEBUG_WHEEL_CELL_BUS_OFFS,
    DEBUG_WHEEL_CELL_TX_FIFO_FREE,
    DEBUG_WHEEL_CELL_TX_REFUSED,
    DEBUG_WHEEL_CELL_TX_UNJAMS,
    DEBUG_WHEEL_CELL_RESTRICTED,
    DEBUG_WHEEL_CELL_FPS,
    DEBUG_WHEEL_CELL_FRAME_MAX,
    DEBUG_WHEEL_CELL_RENDER_CPU,
    DEBUG_WHEEL_CELL_DMA2D_ERRORS,
    DEBUG_WHEEL_CELL_DROPPED_RUNS,
    DEBUG_WHEEL_CELL_INPUTS_MAX,
    DEBUG_WHEEL_CELL_UPTIME,
    DEBUG_WHEEL_CELL_LAST_RESET,
    DEBUG_WHEEL_CELL_COUNT,
};

/*!
 * \brief Boxes in drawing order: background, tab bar, then caption + value per cell.
 */
enum DebugWheelBox : uint8_t {
    DEBUG_WHEEL_BOX_BACKGROUND, /*!< LIGHT_GRAY under the grid */
    DEBUG_WHEEL_BOX_TITLE,      /*!< Tab bar, left part: page name */
    DEBUG_WHEEL_BOX_COUNTER,    /*!< Tab bar, right part: tab position */
    DEBUG_WHEEL_BOX_CELLS,      /*!< First cell caption */
};

#define DEBUG_WHEEL_BOX_LABEL(cell) ((uint8_t)(DEBUG_WHEEL_BOX_CELLS + (2U * (cell)))) /*!< Caption box of a cell */
#define DEBUG_WHEEL_BOX_VALUE(cell) ((uint8_t)(DEBUG_WHEEL_BOX_LABEL(cell) + 1U))      /*!< Value box of a cell */
#define DEBUG_WHEEL_BOX_COUNT (DEBUG_WHEEL_BOX_LABEL(DEBUG_WHEEL_CELL_COUNT))
#define DEBUG_WHEEL_TEXT_BUFFER_SIZE (16U)

/*!
 * \brief Storage of the page: one text and label per box (the background has none).
 */
struct DebugWheelHandler {
    char text[DEBUG_WHEEL_BOX_COUNT][DEBUG_WHEEL_TEXT_BUFFER_SIZE];
    struct Label labels[DEBUG_WHEEL_BOX_COUNT];
    struct Box boxes[DEBUG_WHEEL_BOX_COUNT];
};

#endif // DEBUG_WHEEL_H
