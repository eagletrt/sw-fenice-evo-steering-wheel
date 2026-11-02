/*!
 * \file input-events.h
 * \date 2026-10-31
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief Definitions for the input event layer: one stream of button, combo
 *     and knob events.
 *
 * \details Sits between the inputs module (edges, knob deltas) and the input
 *     router. Buttons and combos produce the same four events: press,
 *     release, tap (released before the long-press time) and long press.
 *
 *     A button that belongs to a combo active in the current state (the
 *     router says which) opens a combo window of INPUT_EVENTS_COMBO_WINDOW_MS:
 *     - if the other buttons of the combo go down inside it, the combo is
 *       pressed and its buttons produce no event of their own until released;
 *     - otherwise the button's own press is delivered when the window ends,
 *       or straight away if it is released first.
 *     A button in no active combo is delivered at once. Long presses are
 *     timed from the physical press (INPUT_EVENTS_LONG_PRESS_MS), a combo's
 *     from its recognition (INPUT_EVENTS_COMBO_LONG_PRESS_MS). Every time is
 *     a timeout of the wheel's pool (timeout.h). Knows nothing about modes:
 *     what an event does is the router's job.
 */

#ifndef INPUT_EVENTS_H
#define INPUT_EVENTS_H

#include "inputs-shared.h"
#include "timeout.h"

#include <limits.h>
#include <stdint.h>

/*!
 * \brief Bit of a button in a combo mask.
 */
#define INPUT_EVENTS_BUTTON_BIT(button_id) ((uint16_t)(1U << (uint16_t)(button_id)))

static_assert(INPUTS_SHARED_BUTTON_ID_COUNT <= sizeof(uint16_t) * CHAR_BIT, "every button needs a bit in a combo mask");

/*!
 * \brief Time the other buttons of a combo have to join the first one, in milliseconds.
 *
 * \details Also how late the own press of a button in an active combo is.
 */
#define INPUT_EVENTS_COMBO_WINDOW_MS (150U)

/*!
 * \brief Hold time of a button's long press, from the physical press, in milliseconds.
 */
#define INPUT_EVENTS_LONG_PRESS_MS (500U)

/*!
 * \brief Hold time of a combo's long press, from its recognition, in milliseconds.
 *
 * \details TODO(design): placeholder until the combos are defined (Q1).
 */
#define INPUT_EVENTS_COMBO_LONG_PRESS_MS (1000U)

static_assert(INPUT_EVENTS_LONG_PRESS_MS > INPUT_EVENTS_COMBO_WINDOW_MS, "a long press must not end inside the combo window");

/*!
 * \brief Buttons of the menu combo.
 *
 * \details TODO(design): placeholder (Q1).
 */
#define INPUT_EVENTS_COMBO_MENU_BUTTONS (INPUT_EVENTS_BUTTON_BIT(INPUTS_SHARED_BUTTON_ID_TOP_LEFT) | INPUT_EVENTS_BUTTON_BIT(INPUTS_SHARED_BUTTON_ID_TOP_RIGHT))

/*!
 * \brief Buttons of the screen-saver wake combo.
 *
 * \details TODO(design): placeholder (Q1).
 */
#define INPUT_EVENTS_COMBO_WAKE_BUTTONS (INPUT_EVENTS_BUTTON_BIT(INPUTS_SHARED_BUTTON_ID_PADDLE_BOTTOM_LEFT) | INPUT_EVENTS_BUTTON_BIT(INPUTS_SHARED_BUTTON_ID_PADDLE_BOTTOM_RIGHT))

/*!
 * \brief Return codes for input events operations.
 */
enum InputEventsReturnCode {
    INPUT_EVENTS_RC_OK,           /*!< Operation completed successfully */
    INPUT_EVENTS_RC_NULL_POINTER, /*!< A required pointer was NULL */
    INPUT_EVENTS_RC_ERROR,        /*!< Invalid identifier, timeout failure or the sink reported failure */
};

/*!
 * \brief Every combo the wheel knows. Which ones are recognised depends on the state.
 */
enum InputEventsComboId : uint8_t {
    INPUT_EVENTS_COMBO_MENU, /*!< Open the menu */
    INPUT_EVENTS_COMBO_WAKE, /*!< Leave the screen saver */
    INPUT_EVENTS_COMBO_COUNT,
};

static_assert(INPUT_EVENTS_COMBO_COUNT <= sizeof(uint8_t) * CHAR_BIT, "every combo needs a bit in the active-combos mask");

/*!
 * \brief Bit of a combo in an active-combos mask.
 */
#define INPUT_EVENTS_COMBO_BIT(combo_id) ((uint8_t)(1U << (uint8_t)(combo_id)))

/*!
 * \brief What produced an event.
 */
enum InputEventSource : uint8_t {
    INPUT_EVENT_SOURCE_BUTTON, /*!< A single button */
    INPUT_EVENT_SOURCE_COMBO,  /*!< A combo */
    INPUT_EVENT_SOURCE_KNOB,   /*!< A knob */
};

/*!
 * \brief What happened.
 */
enum InputEventType : uint8_t {
    INPUT_EVENT_TYPE_PRESS,      /*!< Went down (a combo: was recognised) */
    INPUT_EVENT_TYPE_RELEASE,    /*!< Went up (a combo: its first button went up) */
    INPUT_EVENT_TYPE_TAP,        /*!< Released before its long-press time, right after RELEASE */
    INPUT_EVENT_TYPE_LONG_PRESS, /*!< Still down after its long-press time, once per press */
    INPUT_EVENT_TYPE_ROTATION,   /*!< A knob moved by delta steps */
};

/*!
 * \brief One input event.
 */
struct InputEvent {
    enum InputEventSource source; /*!< Which member of the union is meaningful */
    enum InputEventType type;     /*!< What happened */
    union {
        enum InputsSharedButtonID button; /*!< INPUT_EVENT_SOURCE_BUTTON */
        enum InputEventsComboId combo;    /*!< INPUT_EVENT_SOURCE_COMBO */
        enum InputsSharedKnobID knob;     /*!< INPUT_EVENT_SOURCE_KNOB */
    };
    int8_t delta; /*!< Rotation only: signed steps since the last event */
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
 * \brief Combos recognised right now (the router: the ones the current state binds).
 *
 * \return Mask of INPUT_EVENTS_COMBO_BIT.
 */
typedef uint8_t (*input_events_active_combos_callback)(void);

/*!
 * \brief Where a held button is in its gesture.
 */
enum InputEventsButtonPhase : uint8_t {
    INPUT_EVENTS_BUTTON_RELEASED, /*!< Up */
    INPUT_EVENTS_BUTTON_PENDING,  /*!< Down inside a combo window, press not delivered yet */
    INPUT_EVENTS_BUTTON_SINGLE,   /*!< Down, press delivered as a single button */
    INPUT_EVENTS_BUTTON_COMBO,    /*!< Down, part of a recognised combo: no event of its own */
};

/*!
 * \brief State of one button.
 */
struct InputEventsButton {
    enum InputsSharedButtonID id;      /*!< Which button (the long-press callback's context) */
    enum InputEventsButtonPhase phase; /*!< Where it is in its gesture */
    timeout_id long_press;             /*!< Started at the physical press */
};

/*!
 * \brief Timeouts the layer takes from the pool: one per button, the combo window, the combo long press.
 */
#define INPUT_EVENTS_TIMEOUT_COUNT ((uint8_t)INPUTS_SHARED_BUTTON_ID_COUNT + 2U)

/*!
 * \brief No combo is pressed.
 */
#define INPUT_EVENTS_NO_COMBO (INPUT_EVENTS_COMBO_COUNT)

/*!
 * \brief File-static state of the input events layer.
 */
struct InputEventsHandler {
    input_events_sink_callback sink;                                  /*!< Receiver of every event */
    input_events_active_combos_callback active_combos;                /*!< Combos recognised right now, NULL = none */
    struct InputEventsButton buttons[INPUTS_SHARED_BUTTON_ID_COUNT];  /*!< Indexed by InputsSharedButtonID */
    enum InputsSharedButtonID pending[INPUTS_SHARED_BUTTON_ID_COUNT]; /*!< Buttons in the combo window, in press order */
    uint8_t pending_count;                                            /*!< Entries of pending */
    timeout_id window;                                                /*!< Combo window, started by the first pending button */
    enum InputEventsComboId combo;                                    /*!< Pressed combo, INPUT_EVENTS_NO_COMBO if none */
    timeout_id combo_long_press;                                      /*!< Started when the combo is recognised */
};

#endif // INPUT_EVENTS_H
