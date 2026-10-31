/*!
 * \file input-events.h
 * \date 2026-10-31
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief Definitions for the input event layer: one stream of button, knob
 *     and combo events.
 *
 * \details Sits between the inputs module (edges, knob deltas) and whoever
 *     consumes them (the input router). On top of what the inputs module
 *     reports it tracks the held buttons and times the holds:
 *     - long press: one button held for INPUT_EVENTS_LONG_PRESS_MS;
 *     - combos: a set of buttons held together for a given time.
 *     Both fire once per hold, and again only after a release. Every hold
 *     time is a libtimebase watchdog (one per button, one per combo). Knows
 *     nothing about modes: deciding what an event does is the router's job.
 */

#ifndef INPUT_EVENTS_H
#define INPUT_EVENTS_H

#include "inputs-shared.h"
#include "watchdogs.h"

#include <limits.h>
#include <stdint.h>

/*!
 * \brief Bit of a button in a combo mask.
 */
#define INPUT_EVENTS_BUTTON_BIT(button_id) ((uint16_t)(1U << (uint16_t)(button_id)))

static_assert(INPUTS_SHARED_BUTTON_ID_COUNT <= sizeof(uint16_t) * CHAR_BIT, "every button needs a bit in a combo mask");

/*!
 * \brief Hold time of a long press, in milliseconds.
 */
#define INPUT_EVENTS_LONG_PRESS_MS (500U)

/*!
 * \brief Hold time of every combo, in milliseconds.
 *
 * \details TODO(design): placeholder until the combos are defined (Q1).
 */
#define INPUT_EVENTS_COMBO_HOLD_MS (1000U)

/*!
 * \brief Buttons of the menu combo.
 *
 * \details TODO(design): placeholder (Q1). Both buttons do nothing on their
 *     own while driving, so holding them changes no parameter.
 */
#define INPUT_EVENTS_COMBO_MENU_BUTTONS (INPUT_EVENTS_BUTTON_BIT(INPUTS_SHARED_BUTTON_ID_TOP_LEFT) | INPUT_EVENTS_BUTTON_BIT(INPUTS_SHARED_BUTTON_ID_TOP_RIGHT))

/*!
 * \brief Buttons of the screen-saver wake combo.
 *
 * \details TODO(design): placeholder (Q1). The bottom paddles do nothing on
 *     their own.
 */
#define INPUT_EVENTS_COMBO_WAKE_BUTTONS (INPUT_EVENTS_BUTTON_BIT(INPUTS_SHARED_BUTTON_ID_PADDLE_BOTTOM_LEFT) | INPUT_EVENTS_BUTTON_BIT(INPUTS_SHARED_BUTTON_ID_PADDLE_BOTTOM_RIGHT))

/*!
 * \brief Return codes for input events operations.
 */
enum InputEventsReturnCode {
    INPUT_EVENTS_RC_OK,           /*!< Operation completed successfully */
    INPUT_EVENTS_RC_NULL_POINTER, /*!< A required pointer was NULL */
    INPUT_EVENTS_RC_ERROR,        /*!< Invalid identifier, libtimebase failure or the sink reported failure */
};

/*!
 * \brief Every combo the wheel recognises.
 */
enum InputEventsComboId : uint8_t {
    INPUT_EVENTS_COMBO_MENU, /*!< Open the menu */
    INPUT_EVENTS_COMBO_WAKE, /*!< Leave the screen saver */
    INPUT_EVENTS_COMBO_COUNT,
};

/*!
 * \brief Kind of an input event.
 */
enum InputEventType : uint8_t {
    INPUT_EVENT_TYPE_BUTTON_PRESS,      /*!< A button went down */
    INPUT_EVENT_TYPE_BUTTON_RELEASE,    /*!< A button went up */
    INPUT_EVENT_TYPE_BUTTON_LONG_PRESS, /*!< A button has been held past the long-press threshold */
    INPUT_EVENT_TYPE_KNOB_ROTATION,     /*!< A knob moved by delta steps */
    INPUT_EVENT_TYPE_COMBO,             /*!< Every button of a combo has been held for its hold time */
};

/*!
 * \brief One input event.
 */
struct InputEvent {
    enum InputEventType type; /*!< Which of the fields below is meaningful */
    union {
        enum InputsSharedButtonID button; /*!< Button events */
        enum InputsSharedKnobID knob;     /*!< Knob rotation */
        enum InputEventsComboId combo;    /*!< Combo */
    };
    int8_t delta; /*!< Knob rotation only: signed steps since the last event */
};

/*!
 * \brief Receiver of every input event.
 *
 * \param event The event, valid only during the call.
 *
 * \retval INPUT_EVENTS_RC_OK if the event was handled (or deliberately dropped).
 * \retval INPUT_EVENTS_RC_ERROR if handling it failed.
 */
typedef enum InputEventsReturnCode (*input_events_sink_callback)(const struct InputEvent *event);

/*!
 * \brief Buttons and hold time of a combo.
 */
struct InputEventsCombo {
    uint16_t buttons; /*!< Mask of INPUT_EVENTS_BUTTON_BIT */
    uint16_t hold_ms; /*!< Time every button must stay held */
};

/*!
 * \brief Watchdogs the layer takes from the pool: one per button (long press), one per combo.
 */
#define INPUT_EVENTS_WATCHDOG_COUNT ((uint8_t)INPUTS_SHARED_BUTTON_ID_COUNT + (uint8_t)INPUT_EVENTS_COMBO_COUNT)

/*!
 * \brief File-static state of the input events layer.
 */
struct InputEventsHandler {
    input_events_sink_callback sink;                                     /*!< Receiver of every event */
    struct WatchdogHandler *watchdogs;                                   /*!< Pool running the hold times */
    struct Watchdog long_press_watchdogs[INPUTS_SHARED_BUTTON_ID_COUNT]; /*!< Running: button held. Timed out: long press fired */
    struct Watchdog combo_watchdogs[INPUT_EVENTS_COMBO_COUNT];           /*!< Running: combo held, waiting. Timed out: fired */
    uint16_t held;                                                       /*!< Mask of the buttons currently held */
};

#endif // INPUT_EVENTS_H
