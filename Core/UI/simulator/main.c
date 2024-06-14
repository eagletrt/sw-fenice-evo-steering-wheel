
/**
 * @file main
 *
 */

/*********************
 *      INCLUDES
 *********************/
#define _DEFAULT_SOURCE /* needed for usleep() */
#include <stdlib.h>
#include <unistd.h>
#define SDL_MAIN_HANDLED /*To fix SDL's "undefined reference to WinMain"       \
                            issue*/
#include "graphics_manager.h"
#include "lv-drivers/display/monitor.h"
#include "lv-drivers/indev/keyboard.h"
#include "lv-drivers/indev/mouse.h"
#include "lv-drivers/indev/mousewheel.h"
#include "lvgl.h"
#include "steering.h"
#include "steering_config.h"
#include "tab_manager.h"
#include "watchdog.h"

#include <SDL2/SDL.h>

#define MAX(x, y) x > y ? x : y;

#ifdef SIMULATOR_CAN

/*canlib libraries*/
#define primary_NETWORK_IMPLEMENTATION
#define secondary_NETWORK_IMPLEMENTATION

#include "queue.h"

#include "../steering/can_messages.h"
#include "../steering/cansniffer.h"
#include "can.h"
#endif

#include "../steering/controls.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

#ifdef SIMULATOR_CAN
typedef struct thread_data_t {
    can_t *can;
    int can_id;
} thread_data_t;
#endif

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void hal_init(void);
static int tick_thread(void *data);

/**********************
 *  STATIC VARIABLES
 **********************/

#if CANSNIFFER_ENABLED == 1
extern cansniffer_elem_t primary_cansniffer_buffer[primary_MESSAGE_COUNT];
extern cansniffer_elem_t secondary_cansniffer_buffer[secondary_MESSAGE_COUNT];
#endif

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void keyboard_fn(lv_indev_drv_t *indev_drv, uint8_t e);
#if CANSNIFFER_ENABLED == 1
void save_cansniffer_data(lv_timer_t *timer);
#endif

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *      VARIABLES
 **********************/

#ifdef SIMULATOR_CAN
thread_data_t thread_data_1, thread_data_0;

can_t can_primary;
can_t can_secondary;

SDL_mutex *mtx;  // LOCK-> SDL_mutexP() , UNLOCK->SDL_mutexV()
queue_t queue;

const int NETWORK_PRIMARY   = 0;
const int NETWORK_SECONDARY = 1;

SDL_Thread *thread_id_0;
SDL_Thread *thread_id_1;
#endif

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

#ifdef SIMULATOR_CAN
/***
 * TODO: FIX: here every can messages is parsed both from primary and secondary network, please somebody fixes that
*/
void canread(thread_data_t *thread_data) {
    struct can_frame frame;
    can_message_t msg;
    int res;
    while (1) {
        can_receive(&frame, thread_data->can);
        msg.id   = frame.can_id;
        msg.size = frame.len;
        memcpy(msg.data, frame.data, frame.len);
        SDL_mutexP(mtx);
        if (thread_data->can_id == NETWORK_PRIMARY) {
            handle_primary(&msg);
        } else {
            handle_secondary(&msg);
        }
        SDL_mutexV(mtx);
    }
}
#endif

int main(int argc, char **argv) {
    (void)argc; /*Unused*/
    (void)argv; /*Unused*/

    /*Initialize LVGL*/
    lv_init();

    /*Initialize the HAL (display, input devices, tick) for LVGL*/
    hal_init();

    // steering_values_init();

    tab_manager();

    // lv_timer_create((lv_timer_cb_t) test_value_update_incremental, 70, NULL);

    /*----init structures and values to read from can----*/
#ifdef SIMULATOR_CAN
    mtx = SDL_CreateMutex();
    queue_init(&queue);
    can_init("vcan0", &can_primary);
    can_init("vcan1", &can_secondary);

    queue_element_t q_element;
    uint16_t readMessage = 0;  // 0 = no message, 1 = message read

    if (can_open_socket(&can_primary) < 0) {
        printf("[ERR] could not open can_primary\n");
    }

    if (can_open_socket(&can_secondary) < 0) {
        printf("[ERR] could not open can_secondary\n");
    }

    thread_data_0.can    = &can_primary;
    thread_data_0.can_id = NETWORK_PRIMARY;
    thread_data_1.can    = &can_secondary;
    thread_data_1.can_id = NETWORK_SECONDARY;

    thread_id_0 = SDL_CreateThread(canread, "thread_0", &thread_data_0);
    thread_id_1 = SDL_CreateThread(canread, "thread_1", &thread_data_1);
#endif

    lv_timer_t *ugt = lv_timer_create(update_graphics, 100, NULL);
    lv_timer_set_repeat_count(ugt, -1);
    lv_timer_reset(ugt);

    lv_timer_t *ptt_checker_task = lv_timer_create(ptt_tasks_fn, 100, NULL);
    lv_timer_set_repeat_count(ptt_checker_task, -1);
    lv_timer_reset(ptt_checker_task);

    // init_watchdog();

    while (1) {
#if SIMULATOR_CAN
        SDL_mutexP(mtx);
#endif
        /* Periodically call the lv_task handler.
     * It could be done in a timer interrupt or an OS task too.*/
        lv_timer_handler();

#if SIMULATOR_CAN
        SDL_mutexV(mtx);
#endif
        usleep(5 * 1000);
    }

    return 0;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

#if CANSNIFFER_ENABLED == 1
void save_cansniffer_data(lv_timer_t *timer) {
    for (int i = 0; i < primary_MESSAGE_COUNT; i++) {
        if (primary_cansniffer_buffer[i].timestamp != 0) {
            printf("primary_cansniffer_buffer[%d].timestamp = %d\n", i, primary_cansniffer_buffer[i].timestamp);
        }
    }
    for (int i = 0; i < secondary_MESSAGE_COUNT; i++) {
        if (secondary_cansniffer_buffer[i].timestamp != 0) {
            printf("secondary_cansniffer_buffer[%d].timestamp = %d\n", i, secondary_cansniffer_buffer[i].timestamp);
        }
    }
}
#endif

/**
 * Initialize the Hardware Abstraction Layer (HAL) for the LVGL graphics
 * library
 */
static void hal_init(void) {
    /* Use the 'monitor' driver which creates window on PC's monitor to simulate a
   * display*/
    monitor_init();
    /* Tick init.
   * You have to call 'lv_tick_inc()' in periodically to inform LittelvGL about
   * how much time were elapsed Create an SDL thread to do this*/
    SDL_CreateThread(tick_thread, "tick", NULL);

    /*Create a display buffer*/
    static lv_disp_draw_buf_t disp_buf1;
    static lv_color_t buf1_1[MONITOR_HOR_RES * 100];
    static lv_color_t buf1_2[MONITOR_HOR_RES * 100];
    lv_disp_draw_buf_init(&disp_buf1, buf1_1, buf1_2, MONITOR_HOR_RES * 100);

    /*Create a display*/
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv); /*Basic initialization*/
    disp_drv.draw_buf     = &disp_buf1;
    disp_drv.flush_cb     = monitor_flush;
    disp_drv.hor_res      = MONITOR_HOR_RES;
    disp_drv.ver_res      = MONITOR_VER_RES;
    disp_drv.antialiasing = 1;
    disp_drv.dpi          = 10;

    lv_disp_t *disp = lv_disp_drv_register(&disp_drv);

    lv_theme_t *th = lv_theme_default_init(disp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED), LV_THEME_DEFAULT_DARK, LV_FONT_DEFAULT);
    lv_disp_set_theme(disp, th);

    g = lv_group_create();
    lv_group_set_default(g);

    keyboard_init();
    static lv_indev_drv_t indev_drv_2;
    lv_indev_drv_init(&indev_drv_2); /*Basic initialization*/
    indev_drv_2.type        = LV_INDEV_TYPE_KEYPAD;
    indev_drv_2.read_cb     = keyboard_read;
    indev_drv_2.feedback_cb = keyboard_fn;
    lv_indev_t *kb_indev    = lv_indev_drv_register(&indev_drv_2);
    lv_indev_set_group(kb_indev, g);
}

/**
 * A task to measure the elapsed time for LVGL
 * @param data unused
 * @return never return
 */
static int tick_thread(void *data) {
    (void)data;

    while (1) {
        SDL_Delay(5);
        lv_tick_inc(5); /*Tell LittelvGL that 5 milliseconds were elapsed*/
    }

    return 0;
}

void keyboard_fn(lv_indev_drv_t *indev_drv, uint8_t e) {
    lv_indev_data_t data;
    keyboard_read(indev_drv, &data);

    switch (data.key) {
        case 'a':
            buttons_pressed_actions(PADDLE_TOP_LEFT);
            break;
        case 'g':
            buttons_pressed_actions(PADDLE_TOP_RIGHT);
            break;
        case 'z':
            buttons_pressed_actions(PADDLE_BOTTOM_LEFT);
            break;
        case 'v':
            buttons_pressed_actions(PADDLE_BOTTOM_RIGHT);
            break;
        case 's':
            buttons_pressed_actions(BUTTON_TOP_LEFT);
            break;
        case 'f':
            buttons_pressed_actions(BUTTON_TOP_RIGHT);
            break;
        case 'x':
            buttons_pressed_actions(BUTTON_BOTTOM_LEFT);
            break;
        case 'c':
            buttons_pressed_actions(BUTTON_BOTTOM_RIGHT);
            break;
        case 'm':
            switch_mode();
            break;
        case '1':  // manettino sinistro verso sinistra
            manettino_left_actions(-1);
            break;
        case '2':  // manettino sinistro verso destra
            manettino_left_actions(1);
            break;
        case '3':  // manettino centrale verso sinistra
            manettino_center_actions(-1);
            break;
        case '4':  // manettino centrale verso destra
            manettino_center_actions(1);
            break;
        case '5':  // manettino destro verso sinistra
            manettino_right_actions(-1);
            break;
        case '6':  // manettino destro verso destra
            manettino_right_actions(1);
            break;
        default:
            break;
    }
}

bool can_send(can_message_t *msg, bool to_primary_network) {
}

void openblt_reset(void) {
}

uint32_t get_current_time_ms(void) {
    return SDL_GetTicks();
}

void all_leds_green(void) {
}

void all_leds_red(void) {
}
