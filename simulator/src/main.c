/*!
 * \file main.c
 * \date 2026-10-21
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 *
 * \brief Host-side simulator for the steering-wheel firmware.
 *
 * \details Runs the real hardware-agnostic firmware (Core/Src/steering) on the
 *     PC: the same scheduler, task table, FSM, CAN router and screen as the
 *     wheel. Only the hardware hooks of SchedulerConfig are replaced: the
 *     millisecond tick, the input sampling (keyboard), the LTDC/DMA2D
 *     framebuffers (a Tigr window), CAN send (printed) and the UART logger
 *     (stdout). CAN frames can be injected from the keyboard and go through
 *     the router exactly like frames received by FDCAN.
 *
 *     Keyboard map:
 *       q/w  manettino FRONT_LEFT   (POWER)            -/+
 *       a/s  manettino FRONT_RIGHT  (REGEN)            -/+
 *       z/x  manettino FRONT_CENTER (TORQUE VECTORING) -/+
 *       e    button BOTTOM_LEFT  (TELEMETRY LOG toggle)
 *       d    button BOTTOM_RIGHT (LAUNCH CONTROL toggle)
 *       r/f  buttons TOP_LEFT_1 / TOP_RIGHT_1 (no function yet)
 *       c/v  paddles TOP_LEFT / TOP_RIGHT (PTT, hold)
 *       b/n  paddles BOTTOM_LEFT / BOTTOM_RIGHT (no function yet)
 *       t    TS-on button (hold)
 *       1    inject EcuFsm with the next vehicle status
 *       2    inject one step of telemetry (SoC, temperatures, laps)
 *       p    toggle a continuous telemetry feed
 *
 *     Options:
 *       --screenshot <file.ppm>  run for SIMULATOR_SCREENSHOT_DELAY_MS, save the
 *                                whole window (dashboard + status strip) as a
 *                                binary PPM and exit.
 *       --replay <from> <to>     for each step from \p from to \p to: one frame
 *                                with telemetry step s, then one frame with
 *                                only the ECU status changed (to s mod the
 *                                number of statuses), SIMULATOR_REPLAY_PERIOD_MS
 *                                apart. Consecutive frames change different
 *                                boxes, so with --screenshot replaying 0..N must
 *                                give the same image as N..N: any box missing
 *                                from a back buffer shows up.
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "arena-allocator-api.h"
#include "can-communications-api.h"
#include "can-communications-router-api.h"
#include "can-primary-api.h"
#include "can-primary.h"
#include "eagletrt-api.h"
#include "inputs-api.h"
#include "logger-api.h"
#include "pal-api.h"
#include "scheduler-api.h"
#include "screen.h"

#include "tigr.h"

#define SIMULATOR_DASHBOARD_WIDTH (SCREEN_WIDTH)
#define SIMULATOR_DASHBOARD_HEIGHT (SCREEN_HEIGHT)
#define SIMULATOR_STATUS_HEIGHT (44)
#define SIMULATOR_WIDTH (SIMULATOR_DASHBOARD_WIDTH)
#define SIMULATOR_HEIGHT (SIMULATOR_DASHBOARD_HEIGHT + SIMULATOR_STATUS_HEIGHT)

#define SIMULATOR_FRAME_PERIOD_MS (16U)        /*!< Window refresh, ~60 Hz like the LTDC scanout */
#define SIMULATOR_IDLE_SLEEP_NS (500000L)      /*!< Sleep between scheduler routines, keeps the 1 ms tasks on time */
#define SIMULATOR_FEED_PERIOD_MS (200U)        /*!< Continuous telemetry feed period */
#define SIMULATOR_NS_PER_US (1000U)            /*!< "Cycles" are nanoseconds on the PC */
#define SIMULATOR_CAN_ID_COUNT (2048U)         /*!< Standard 11-bit identifiers */
#define SIMULATOR_LOGGER_RX_CAPACITY (1U)      /*!< Same sizes as the wheel's logger */
#define SIMULATOR_LOGGER_TX_CAPACITY (10U)
#define SIMULATOR_LOGGER_MAX_MSG_SIZE (64U)
#define SIMULATOR_SCREENSHOT_DELAY_MS (1000U)
#define SIMULATOR_REPLAY_PERIOD_MS (30U)       /*!< Longer than the render period: one step per frame */
#define SIMULATOR_MASK_SIZE (128U * 1024U)     /*!< Same scratch size as the wheel (ltdc.c) */  /*!< Boot + first telemetry rendered well before this */

/*!
 * \brief Keyboard key held for each physical button (0 = not mapped).
 */
EAGLETRT_STATIC const int simulator_button_keys[INPUTS_SHARED_BUTTON_ID_COUNT] = {
    [INPUTS_SHARED_BUTTON_ID_TS_ON] = 'T',
    [INPUTS_SHARED_BUTTON_ID_TOP_LEFT_1] = 'R',
    [INPUTS_SHARED_BUTTON_ID_BOTTOM_LEFT] = 'E',
    [INPUTS_SHARED_BUTTON_ID_TOP_RIGHT_1] = 'F',
    [INPUTS_SHARED_BUTTON_ID_BOTTOM_RIGHT] = 'D',
    [INPUTS_SHARED_BUTTON_ID_PADDLE_TOP_LEFT] = 'C',
    [INPUTS_SHARED_BUTTON_ID_PADDLE_BOTTOM_LEFT] = 'B',
    [INPUTS_SHARED_BUTTON_ID_PADDLE_TOP_RIGHT] = 'V',
    [INPUTS_SHARED_BUTTON_ID_PADDLE_BOTTOM_RIGHT] = 'N',
};

/*!
 * \brief Keys turning each manettino one step down / up.
 */
EAGLETRT_STATIC const int simulator_knob_keys[INPUTS_SHARED_KNOB_ID_COUNT][2] = {
    [INPUTS_SHARED_KNOB_ID_FRONT_LEFT] = { 'Q', 'W' },
    [INPUTS_SHARED_KNOB_ID_FRONT_RIGHT] = { 'A', 'S' },
    [INPUTS_SHARED_KNOB_ID_FRONT_CENTER] = { 'Z', 'X' },
};

/*!
 * \brief Names of the EcuFsm vehicle statuses, indexed by value.
 */
EAGLETRT_STATIC const char *const simulator_ecu_status_names[] = {
    "INIT", "IDLE", "PRECHARGE", "TSON", "DISCHARGE", "ASOFF", "AS_READY",
    "AS_DRIVING", "ERROR", "R2D", "AS_R2D", "AS_FINISHED", "AS_EMERGENCY",
};

/*!
 * \brief Last frame sent on each identifier, to print only what changes.
 */
struct SimulatorSentFrame {
    bool seen;
    uint8_t length;
    uint8_t data[CAN_COMMUNICATIONS_FRAME_DATA_SIZE];
};

EAGLETRT_STATIC Tigr *window;

/* Double-buffered framebuffers, same convention as Core/Src/ltdc.c: the
 * window shows display_framebuffer while the raster draws into
 * draw_framebuffer; a swap flips them (no copy, see simulator_swap_framebuffers). */
EAGLETRT_STATIC TPixel simulator_framebuffer_a[SIMULATOR_DASHBOARD_WIDTH * SIMULATOR_DASHBOARD_HEIGHT];
EAGLETRT_STATIC TPixel simulator_framebuffer_b[SIMULATOR_DASHBOARD_WIDTH * SIMULATOR_DASHBOARD_HEIGHT];
EAGLETRT_STATIC TPixel *simulator_display_framebuffer = simulator_framebuffer_a;
EAGLETRT_STATIC TPixel *simulator_draw_framebuffer = simulator_framebuffer_b;

/* Mask scratch (AXI SRAM on the wheel). */
EAGLETRT_STATIC uint8_t simulator_mask[SIMULATOR_MASK_SIZE];

/* Input state written by the window loop, read by the scheduler inputs task. */
EAGLETRT_STATIC bool simulator_button_held[INPUTS_SHARED_BUTTON_ID_COUNT];
EAGLETRT_STATIC bool simulator_button_tapped[INPUTS_SHARED_BUTTON_ID_COUNT];
EAGLETRT_STATIC int16_t simulator_knob_positions[INPUTS_SHARED_KNOB_ID_COUNT];

EAGLETRT_STATIC struct SimulatorSentFrame simulator_sent_frames[CAN_COMMUNICATION_NETWORK_COUNT][SIMULATOR_CAN_ID_COUNT];
EAGLETRT_STATIC uint8_t simulator_ecu_status = CAN_PRIMARY_ECUFSM_VEHICLESTATUS_IDLE;
EAGLETRT_STATIC uint32_t simulator_feed_step;
EAGLETRT_STATIC bool simulator_feed_enabled;

EAGLETRT_STATIC struct ArenaAllocatorHandler simulator_arena;
EAGLETRT_STATIC struct PalHandler simulator_logger_pal;

/*!
 * \brief Monotonic nanoseconds since an arbitrary origin.
 */
EAGLETRT_STATIC uint64_t simulator_now_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ((uint64_t)ts.tv_sec * 1000000000ULL) + (uint64_t)ts.tv_nsec;
}

EAGLETRT_STATIC uint64_t simulator_start_ns;

/*!
 * \brief Millisecond tick since the simulator started (HAL_GetTick on the wheel).
 *
 * \details CLOCK_MONOTONIC keeps ticking while the process sleeps, which is
 *     what the popup and the tasks expect (clock() is CPU time).
 */
EAGLETRT_STATIC uint32_t simulator_get_tick(void) {
    return (uint32_t)((simulator_now_ns() - simulator_start_ns) / 1000000ULL);
}

/*!
 * \brief Free-running "cycle counter" for the task statistics: nanoseconds.
 */
EAGLETRT_STATIC uint32_t simulator_get_cycles(void) {
    return (uint32_t)simulator_now_ns();
}

/*!
 * \brief Raster rectangle callback, stand-in for ltdc_draw_rectangle + DMA2D.
 *
 * \details Alpha-blends \p color over the draw framebuffer, like the DMA2D
 *     blend used for the anti-aliased fonts.
 */
EAGLETRT_STATIC enum RasterReturnCode simulator_draw_rectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, struct Color color) {
    if (x >= SIMULATOR_DASHBOARD_WIDTH || y >= SIMULATOR_DASHBOARD_HEIGHT) {
        return RASTER_RC_OK;
    }
    const uint16_t x_end = ((uint32_t)x + w > SIMULATOR_DASHBOARD_WIDTH) ? SIMULATOR_DASHBOARD_WIDTH : (uint16_t)(x + w);
    const uint16_t y_end = ((uint32_t)y + h > SIMULATOR_DASHBOARD_HEIGHT) ? SIMULATOR_DASHBOARD_HEIGHT : (uint16_t)(y + h);
    for (uint16_t row = y; row < y_end; row++) {
        for (uint16_t col = x; col < x_end; col++) {
            TPixel *dst = &simulator_draw_framebuffer[(row * SIMULATOR_DASHBOARD_WIDTH) + col];
            const uint8_t a = color.a;
            dst->r = (uint8_t)(((color.r * a) + (dst->r * (255U - a))) / 255U);
            dst->g = (uint8_t)(((color.g * a) + (dst->g * (255U - a))) / 255U);
            dst->b = (uint8_t)(((color.b * a) + (dst->b * (255U - a))) / 255U);
            dst->a = 255;
        }
    }
    return RASTER_RC_OK;
}

/*!
 * \brief Mask callback, stand-in for ltdc_draw_mask + the DMA2D A8 blend.
 *
 * \details Pixel alpha = coverage * color alpha / 255, blended over the draw
 *     framebuffer; clipped to the dashboard like the wheel.
 */
EAGLETRT_STATIC enum RasterReturnCode simulator_draw_mask(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint8_t *mask, struct Color color) {
    for (uint32_t row = 0U; row < h; row++) {
        const uint32_t screen_y = (uint32_t)y + row;
        if (screen_y >= SIMULATOR_DASHBOARD_HEIGHT) {
            break;
        }
        for (uint32_t col = 0U; col < w; col++) {
            const uint32_t screen_x = (uint32_t)x + col;
            if (screen_x >= SIMULATOR_DASHBOARD_WIDTH) {
                break;
            }
            const uint32_t a = ((uint32_t)mask[(row * w) + col] * color.a) / 255U;
            if (a == 0U) {
                continue;
            }
            TPixel *dst = &simulator_draw_framebuffer[(screen_y * SIMULATOR_DASHBOARD_WIDTH) + screen_x];
            dst->r = (uint8_t)(((color.r * a) + (dst->r * (255U - a))) / 255U);
            dst->g = (uint8_t)(((color.g * a) + (dst->g * (255U - a))) / 255U);
            dst->b = (uint8_t)(((color.b * a) + (dst->b * (255U - a))) / 255U);
            dst->a = 255;
        }
    }
    return RASTER_RC_OK;
}

/*!
 * \brief Mirror of ltdc_swap_framebuffers: flip, no copy.
 *
 * \details Like the wheel, the new draw buffer is one frame behind and the
 *     screen module redraws into it what it drew last time: any mistake in
 *     that bookkeeping shows up here as stale boxes flickering between frames.
 */
EAGLETRT_STATIC void simulator_swap_framebuffers(void) {
    TPixel *tmp = simulator_display_framebuffer;
    simulator_display_framebuffer = simulator_draw_framebuffer;
    simulator_draw_framebuffer = tmp;
}

/*!
 * \brief Scheduler inputs task hook: report the keyboard as buttons and manettini.
 *
 * \details Same observation model as i2c_inputs_poll/gpio_inputs_poll: every
 *     poll reports the current level of each button and the accumulated
 *     position of each manettino, the inputs module does edge detection. A
 *     tap shorter than one poll is reported as pressed for one poll.
 */
EAGLETRT_STATIC void simulator_poll_inputs(uint32_t tick_ms) {
    for (uint8_t id = 0U; id < INPUTS_SHARED_BUTTON_ID_COUNT; id++) {
        if (simulator_button_keys[id] == 0) {
            continue;
        }
        const bool pressed = simulator_button_held[id] || simulator_button_tapped[id];
        simulator_button_tapped[id] = false;
        EAGLETRT_API_UNUSED(inputs_api_update_button((enum InputsSharedButtonID)id, pressed, tick_ms));
    }
    for (uint8_t id = 0U; id < INPUTS_SHARED_KNOB_ID_COUNT; id++) {
        EAGLETRT_API_UNUSED(inputs_api_update_knob((enum InputsSharedKnobID)id, simulator_knob_positions[id]));
    }
}

/*!
 * \brief Reset request from the CAN router (bootloader IDs). Nothing to reset on the PC.
 */
EAGLETRT_STATIC void simulator_reset(void) {
    static bool reported = false;
    if (!reported) {
        reported = true;
        printf("[SIM] reset into the bootloader requested over CAN (ignored)\n");
    }
}

/*!
 * \brief Print a frame the firmware sends, only when its content changes.
 */
EAGLETRT_STATIC enum CanCommunicationReturnCode simulator_can_send(enum CanCommunicationNetwork network, const struct CanCommunicationFrame *frame) {
    if (frame->id >= SIMULATOR_CAN_ID_COUNT) {
        return CAN_COMMUNICATION_RC_OK;
    }
    struct SimulatorSentFrame *last = &simulator_sent_frames[network][frame->id];
    if (last->seen && last->length == frame->length && memcmp(last->data, frame->data, frame->length) == 0) {
        return CAN_COMMUNICATION_RC_OK;
    }
    last->seen = true;
    last->length = frame->length;
    memcpy(last->data, frame->data, frame->length);

    printf("[CAN TX] %s 0x%03X [%u]", (network == CAN_COMMUNICATION_NETWORK_PRIMARY) ? "primary  " : "secondary", (unsigned)frame->id, (unsigned)frame->length);
    for (uint8_t i = 0U; i < frame->length; i++) {
        printf(" %02X", frame->data[i]);
    }
    printf("\n");
    return CAN_COMMUNICATION_RC_OK;
}

EAGLETRT_STATIC enum CanCommunicationReturnCode simulator_can_send_primary(const struct CanCommunicationFrame *frame) {
    return simulator_can_send(CAN_COMMUNICATION_NETWORK_PRIMARY, frame);
}

EAGLETRT_STATIC enum CanCommunicationReturnCode simulator_can_send_secondary(const struct CanCommunicationFrame *frame) {
    return simulator_can_send(CAN_COMMUNICATION_NETWORK_SECONDARY, frame);
}

/*!
 * \brief UART logger stand-in: print the formatted record.
 */
EAGLETRT_STATIC enum PalReturnCode simulator_logger_transmit(const struct PalMessage *message) {
    if (message == NULL) {
        return PAL_RC_NULL_POINTER;
    }
    fwrite(message->payload, 1U, strnlen((const char *)message->payload, message->size), stdout);
    return PAL_RC_OK;
}

/*!
 * \brief Serialize a libcan message and queue it as if FDCAN had received it.
 */
EAGLETRT_STATIC void simulator_inject(enum CanPrimaryMessageFrameId id, union CanPrimaryMessages *message, uint8_t length) {
    struct CanCommunicationFrame frame = { .id = (uint32_t)id, .length = length };
    if (can_primary_api_serialize_from_id(id, message, frame.data) == -1) {
        printf("[SIM] cannot serialize 0x%03X\n", (unsigned)id);
        return;
    }
    if (can_communications_api_add_to_rx_buffer(CAN_COMMUNICATION_NETWORK_PRIMARY, &frame) != CAN_COMMUNICATION_RC_OK) {
        printf("[SIM] RX queue full, 0x%03X dropped\n", (unsigned)id);
    }
}

/*!
 * \brief Inject EcuFsm with the current simulated vehicle status.
 */
EAGLETRT_STATIC void simulator_inject_ecu_status(void) {
    union CanPrimaryMessages message = { 0 };
    message.ecufsm.vehiclestatus = simulator_ecu_status;
    simulator_inject(CAN_PRIMARY_MESSAGE_FRAME_ID_ECUFSM, &message, can_primary_byte_size_ecufsm);
}

/*!
 * \brief Inject one step of every telemetry message the dashboard renders.
 *
 * \details Values sweep with \p step so every field visibly changes and the
 *     warning/error colours show up over a full sweep.
 */
EAGLETRT_STATIC void simulator_inject_telemetry(uint32_t step) {
    union CanPrimaryMessages message;
    const float sweep = (float)(step % 50U) / 50.0F; /* 0..1 */

    memset(&message, 0, sizeof(message));
    message.tsacmainboardestimatedsoc.soc = 1.0F - sweep;
    simulator_inject(CAN_PRIMARY_MESSAGE_FRAME_ID_TSACMAINBOARDESTIMATEDSOC, &message, can_primary_byte_size_tsacmainboardestimatedsoc);

    memset(&message, 0, sizeof(message));
    message.tsacmainboardtemperatureinfo.max = 25.0F + (sweep * 40.0F);
    simulator_inject(CAN_PRIMARY_MESSAGE_FRAME_ID_TSACMAINBOARDTEMPERATUREINFO, &message, can_primary_byte_size_tsacmainboardtemperatureinfo);

    memset(&message, 0, sizeof(message));
    message.invertertemperature.temperaturefl = 30.0F + (sweep * 50.0F);
    message.invertertemperature.temperaturefr = 28.0F + (sweep * 50.0F);
    message.invertertemperature.temperaturerl = 26.0F + (sweep * 50.0F);
    message.invertertemperature.temperaturerr = 24.0F + (sweep * 50.0F);
    simulator_inject(CAN_PRIMARY_MESSAGE_FRAME_ID_INVERTERTEMPERATURE, &message, can_primary_byte_size_invertertemperature);

    memset(&message, 0, sizeof(message));
    message.motortemperature.temperaturefl = 40.0F + (sweep * 80.0F);
    message.motortemperature.temperaturefr = 42.0F + (sweep * 80.0F);
    message.motortemperature.temperaturerl = 44.0F + (sweep * 80.0F);
    message.motortemperature.temperaturerr = 46.0F + (sweep * 80.0F);
    simulator_inject(CAN_PRIMARY_MESSAGE_FRAME_ID_MOTORTEMPERATURE, &message, can_primary_byte_size_motortemperature);

    memset(&message, 0, sizeof(message));
    message.tyretemperaturefront.frontleft = (uint8_t)(40U + (step % 50U));
    message.tyretemperaturefront.frontright = (uint8_t)(45U + (step % 50U));
    simulator_inject(CAN_PRIMARY_MESSAGE_FRAME_ID_TYRETEMPERATUREFRONT, &message, can_primary_byte_size_tyretemperaturefront);

    memset(&message, 0, sizeof(message));
    message.tyretemperaturerear.rearleft = (uint8_t)(50U + (step % 50U));
    message.tyretemperaturerear.rearright = (uint8_t)(55U + (step % 50U));
    simulator_inject(CAN_PRIMARY_MESSAGE_FRAME_ID_TYRETEMPERATUREREAR, &message, can_primary_byte_size_tyretemperaturerear);

    memset(&message, 0, sizeof(message));
    message.currentlap.number = (uint8_t)(1U + ((step / 10U) % 22U));
    message.currentlap.deltabestlap = (sweep * 4.0F) - 2.0F;
    simulator_inject(CAN_PRIMARY_MESSAGE_FRAME_ID_CURRENTLAP, &message, can_primary_byte_size_currentlap);

    memset(&message, 0, sizeof(message));
    message.laps.totalnumber = 22U;
    simulator_inject(CAN_PRIMARY_MESSAGE_FRAME_ID_LAPS, &message, can_primary_byte_size_laps);
}

/*!
 * \brief Read one window frame of keyboard input into the simulated inputs and CAN injections.
 */
EAGLETRT_STATIC void simulator_handle_keyboard(void) {
    for (uint8_t id = 0U; id < INPUTS_SHARED_BUTTON_ID_COUNT; id++) {
        const int key = simulator_button_keys[id];
        if (key == 0) {
            continue;
        }
        simulator_button_held[id] = tigrKeyHeld(window, key) != 0;
        if (tigrKeyDown(window, key) != 0) {
            simulator_button_tapped[id] = true;
        }
    }
    for (uint8_t id = 0U; id < INPUTS_SHARED_KNOB_ID_COUNT; id++) {
        if (tigrKeyDown(window, simulator_knob_keys[id][0]) != 0) {
            simulator_knob_positions[id]--;
        }
        if (tigrKeyDown(window, simulator_knob_keys[id][1]) != 0) {
            simulator_knob_positions[id]++;
        }
    }

    if (tigrKeyDown(window, '1') != 0) {
        simulator_ecu_status = (uint8_t)((simulator_ecu_status + 1U) % (sizeof(simulator_ecu_status_names) / sizeof(simulator_ecu_status_names[0])));
        simulator_inject_ecu_status();
    }
    if (tigrKeyDown(window, '2') != 0) {
        simulator_inject_telemetry(simulator_feed_step++);
    }
    if (tigrKeyDown(window, 'P') != 0) {
        simulator_feed_enabled = !simulator_feed_enabled;
    }
}

/*!
 * \brief Copy the displayed framebuffer to the window (the LTDC scanout).
 */
EAGLETRT_STATIC void simulator_blit_display_framebuffer(void) {
    for (int y = 0; y < SIMULATOR_DASHBOARD_HEIGHT; y++) {
        memcpy(&window->pix[y * window->w],
               &simulator_display_framebuffer[y * SIMULATOR_DASHBOARD_WIDTH],
               (size_t)SIMULATOR_DASHBOARD_WIDTH * sizeof(TPixel));
    }
}

/*!
 * \brief Status strip under the dashboard: key help and simulated state.
 */
EAGLETRT_STATIC void simulator_draw_status(void) {
    const TPixel background = tigrRGB(16, 16, 16);
    const TPixel text = tigrRGB(200, 200, 200);
    tigrFill(window, 0, SIMULATOR_DASHBOARD_HEIGHT, SIMULATOR_WIDTH, SIMULATOR_STATUS_HEIGHT, background);
    tigrPrint(window, tfont, 6, SIMULATOR_DASHBOARD_HEIGHT + 6, text,
              "knobs q/w a/s z/x | buttons e d r f | PTT c v | b n | TSon t | 1 ECU state | 2 telemetry | p feed");
    const fsm_state_t state = scheduler_api_get_fsm_state();
    tigrPrint(window, tfont, 6, SIMULATOR_DASHBOARD_HEIGHT + 24, text,
              "FSM %s   ECU vehicleStatus %s   telemetry feed %s",
              (state < FSM_NUM_STATES) ? fsm_state_names[state] : "?",
              simulator_ecu_status_names[simulator_ecu_status],
              simulator_feed_enabled ? "ON" : "off");
}

/*!
 * \brief Save the window content as a binary PPM (P6).
 *
 * \retval 0 on success, 1 if the file cannot be written.
 */
EAGLETRT_STATIC int simulator_save_screenshot(const char *path) {
    FILE *file = fopen(path, "wb");
    if (file == NULL) {
        fprintf(stderr, "cannot write %s\n", path);
        return 1;
    }
    fprintf(file, "P6\n%d %d\n255\n", window->w, window->h);
    for (int i = 0; i < window->w * window->h; i++) {
        const unsigned char rgb[3] = { window->pix[i].r, window->pix[i].g, window->pix[i].b };
        fwrite(rgb, 1U, sizeof(rgb), file);
    }
    fclose(file);
    printf("[SIM] screenshot saved to %s\n", path);
    return 0;
}

int main(int argc, char **argv) {
    const char *screenshot_path = NULL;
    bool replay = false;
    uint32_t replay_step = 0U;
    uint32_t replay_to = 0U;
    bool replay_ecu_phase = false;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--screenshot") == 0 && (i + 1) < argc) {
            screenshot_path = argv[++i];
        } else if (strcmp(argv[i], "--replay") == 0 && (i + 2) < argc) {
            replay = true;
            replay_step = (uint32_t)strtoul(argv[++i], NULL, 10);
            replay_to = (uint32_t)strtoul(argv[++i], NULL, 10);
        }
    }

    /* Line-buffered even when piped to a file, so logs survive a kill. */
    setvbuf(stdout, NULL, _IOLBF, 0);
    simulator_start_ns = simulator_now_ns();

    arena_allocator_api_init(&simulator_arena);
    EAGLETRT_API_UNUSED(pal_api_init(&simulator_logger_pal,
                                     SIMULATOR_LOGGER_RX_CAPACITY,
                                     SIMULATOR_LOGGER_TX_CAPACITY,
                                     SIMULATOR_LOGGER_MAX_MSG_SIZE,
                                     NULL,
                                     simulator_logger_transmit,
                                     NULL,
                                     NULL,
                                     &simulator_arena));
    EAGLETRT_API_UNUSED(logger_api_init(&simulator_logger_pal, true));

    window = tigrWindow(SIMULATOR_WIDTH, SIMULATOR_HEIGHT, "Fenice Evo steering wheel simulator", 0);
    if (window == NULL) {
        fprintf(stderr, "cannot open the simulator window\n");
        return 1;
    }

    const struct SchedulerConfig config = {
        .post = {
            .can_network_configs = {
                [CAN_COMMUNICATION_NETWORK_PRIMARY] = {
                    .cs_enter = NULL,
                    .cs_exit = NULL,
                    .on_receive = can_communications_router_api_receive_primary,
                    .send = simulator_can_send_primary,
                },
                [CAN_COMMUNICATION_NETWORK_SECONDARY] = {
                    .cs_enter = NULL,
                    .cs_exit = NULL,
                    .on_receive = can_communications_router_api_receive_secondary,
                    .send = simulator_can_send_secondary,
                },
            },
            .draw_rectangle = simulator_draw_rectangle,
            .screen_accelerators = {
                .draw_mask = simulator_draw_mask,
                .mask_buffer = simulator_mask,
                .mask_size = SIMULATOR_MASK_SIZE,
            },
        },
        .get_tick = simulator_get_tick,
        .poll_inputs = simulator_poll_inputs,
        .swap_framebuffers = simulator_swap_framebuffers,
        .reset = simulator_reset,
        .get_cycles = simulator_get_cycles,
        .cycles_per_us = SIMULATOR_NS_PER_US,
    };
    if (scheduler_api_init(&config) != SCHEDULER_RC_OK) {
        fprintf(stderr, "scheduler init failed\n");
        tigrFree(window);
        return 1;
    }
    if (scheduler_api_get_fsm_state() == FSM_STATE_ERROR) {
        fprintf(stderr, "POST failed, the FSM is in ERROR\n");
    }

    /* Start from a populated dashboard instead of dashes. */
    simulator_inject_ecu_status();
    simulator_inject_telemetry(simulator_feed_step++);

    uint32_t last_frame_ms = 0U;
    uint32_t last_feed_ms = 0U;
    uint32_t last_replay_ms = simulator_get_tick();
    while (tigrClosed(window) == 0) {
        if (scheduler_api_routine() != SCHEDULER_RC_OK) {
            fprintf(stderr, "scheduler routine failed\n");
        }

        const uint32_t now = simulator_get_tick();
        if (replay && replay_step <= replay_to && (now - last_replay_ms) >= SIMULATOR_REPLAY_PERIOD_MS) {
            last_replay_ms = now;
            if (!replay_ecu_phase) {
                simulator_inject_telemetry(replay_step);
            } else {
                simulator_ecu_status = (uint8_t)(replay_step % (sizeof(simulator_ecu_status_names) / sizeof(simulator_ecu_status_names[0])));
                simulator_inject_ecu_status();
                replay_step++;
            }
            replay_ecu_phase = !replay_ecu_phase;
        }
        if (simulator_feed_enabled && (now - last_feed_ms) >= SIMULATOR_FEED_PERIOD_MS) {
            last_feed_ms = now;
            simulator_inject_telemetry(simulator_feed_step++);
        }

        if ((now - last_frame_ms) >= SIMULATOR_FRAME_PERIOD_MS) {
            last_frame_ms = now;
            simulator_blit_display_framebuffer();
            simulator_draw_status();
            tigrUpdate(window);
            simulator_handle_keyboard();
            if (screenshot_path != NULL && now >= SIMULATOR_SCREENSHOT_DELAY_MS) {
                const int rc = simulator_save_screenshot(screenshot_path);
                tigrFree(window);
                return rc;
            }
        } else {
            const struct timespec pause = { .tv_sec = 0, .tv_nsec = SIMULATOR_IDLE_SLEEP_NS };
            nanosleep(&pause, NULL);
        }
    }

    tigrFree(window);
    return 0;
}
