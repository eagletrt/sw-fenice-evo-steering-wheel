// C implementation of the Virtual Console (VC) for demos.
//
// # Usage
// ```c
// // demo.c
// // vc.c expectes render() to be defined and also supplies it's own entry point
// // if needed (some platforms like WASM_PLATFORM do not have the main()
// // entry point)
// #include "vc.c"
//
// #define WIDTH 800
// #define HEIGHT 600
// static uint32_t pixels[WIDTH*HEIGHT];
//
// static Olivec_Canvas vc_render(float dt)
// {
//     Olivec_Canvas oc = olivec_canvas(pixels, WIDTH, HEIGHT, WIDTH);
//     // ...
//     // ... render into oc ...
//     // ...
//     return oc;
// }
// ```
//
// # Build
// ```console
// $ clang -o demo.sdl -DVC_PLATFORM=VC_SDL_PLATFORM demo.c -lSDL2
// $ clang -o demo.term -DVC_PLATFORM=VC_TERM_PLATFORM demo.c
// $ clang -fno-builtin --target=wasm32 --no-standard-libraries -Wl,--no-entry -Wl,--export=render -Wl,--allow-undefined -o demo.wasm -DVC_PLATFORM=VC_WASM_PLATFORM demo.c
// ```

#include "queue.h"

#include "controls.h"
#include "graphics_manager.h"
#include "steering_config.h"
#include "steering_types.h"

#define WIDTH  800
#define HEIGHT 480
static uint32_t pixels[WIDTH * HEIGHT * 4];

Olivec_Canvas vc_render(float dt, UI_t *scr) {
    sw_update_graphics_from_can_messages(scr);
    sw_update_screen(dt, scr);
    return scr->oc;
}

#include <SDL2/SDL.h>
#include <stdio.h>
#include <time.h>

#define return_defer(value) \
    do {                    \
        result = (value);   \
        goto defer;         \
    } while (0)

static SDL_Texture *vc_sdl_texture = NULL;
static size_t vc_sdl_actual_width  = 0;
static size_t vc_sdl_actual_height = 0;

static bool vc_sdl_resize_texture(SDL_Renderer *renderer, size_t new_width, size_t new_height) {
    if (vc_sdl_texture != NULL)
        SDL_DestroyTexture(vc_sdl_texture);
    vc_sdl_actual_width  = new_width;
    vc_sdl_actual_height = new_height;
    vc_sdl_texture       = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, vc_sdl_actual_width, vc_sdl_actual_height);
    if (vc_sdl_texture == NULL)
        return false;
    return true;
}

uint32_t get_current_time_ms(void) {
    return SDL_GetTicks();
}

void openblt_reset() {
}

bool can_send(can_message_t *msg, bool to_primary_network) {
    // TODO: implement can send if needed
    return true;
}

typedef struct thread_data_t {
    can_t *can;
    int can_id;
} thread_data_t;

thread_data_t thread_data_1, thread_data_0;

can_t can_primary;
can_t can_secondary;

SDL_mutex *mtx;  // LOCK-> SDL_mutexP() , UNLOCK->SDL_mutexV()
queue_t queue;

const int NETWORK_PRIMARY   = 0;
const int NETWORK_SECONDARY = 1;

SDL_Thread *thread_id_0;
SDL_Thread *thread_id_1;

void canread(thread_data_t *thread_data) {
    struct can_frame frame;
    can_message_t msg;
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

int main(void) {
    int result = 0;

    SDL_Window *window     = NULL;
    SDL_Renderer *renderer = NULL;

    UI_t sw_screen;
    sw_init_screen(&sw_screen);

    {
        if (SDL_Init(SDL_INIT_VIDEO) < 0)
            return_defer(1);

        window = SDL_CreateWindow("Olivec Simulator", 0, 0, 0, 0, 0);
        if (window == NULL)
            return_defer(1);

        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        if (renderer == NULL)
            return_defer(1);

        Uint32 prev = SDL_GetTicks();

        sw_set_canvas(&sw_screen, pixels, WIDTH, HEIGHT, WIDTH);
        sw_screen.oc.pixels = pixels;

        mtx = SDL_CreateMutex();
        queue_init(&queue);
        can_init("vcan0", &can_primary);
        can_init("vcan1", &can_secondary);

        // queue_element_t q_element;
        // uint16_t readMessage = 0;  // 0 = no message, 1 = message read

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

        thread_id_0 = SDL_CreateThread((void *)canread, "thread_0", &thread_data_0);
        thread_id_1 = SDL_CreateThread((void *)canread, "thread_1", &thread_data_1);

        time_t press_time;
        SDL_Keycode last_key = 0;
        bool once            = false;

        for (;;) {
            // Compute Delta Time
            Uint32 curr = SDL_GetTicks();
            float dt    = (curr - prev) / 1000.f;
            prev        = curr;

            // Flush the events
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                switch (event.type) {
                    case SDL_QUIT: {
                        return_defer(0);
                    } break;
                    case SDL_KEYDOWN: {
                        if (last_key != event.key.keysym.sym) {
                            last_key   = event.key.keysym.sym;
                            press_time = clock();
                            switch (event.key.keysym.sym) {
                                case SDLK_z:
                                    manettino_left_actions(1);
                                    break;
                                case SDLK_x:
                                    manettino_left_actions(-1);
                                    break;
                                case SDLK_c:
                                    manettino_center_actions(1);
                                    break;
                                case SDLK_v:
                                    manettino_center_actions(-1);
                                    break;
                                case SDLK_b:
                                    manettino_right_actions(1);
                                    break;
                                case SDLK_n:
                                    manettino_right_actions(-1);
                                    break;
                                case SDLK_w:
                                    buttons_pressed_actions(BUTTON_TOP_LEFT);
                                    break;
                                case SDLK_e:
                                    buttons_pressed_actions(BUTTON_TOP_RIGHT);
                                    break;
                                case SDLK_s:
                                    buttons_pressed_actions(BUTTON_BOTTOM_LEFT);
                                    break;
                                case SDLK_d:
                                    buttons_pressed_actions(BUTTON_BOTTOM_RIGHT);
                                    break;
                                case SDLK_q:
                                    buttons_pressed_actions(PADDLE_TOP_LEFT);
                                    break;
                                case SDLK_r:
                                    buttons_pressed_actions(PADDLE_TOP_RIGHT);
                                    break;
                                case SDLK_a:
                                    buttons_pressed_actions(PADDLE_BOTTOM_LEFT);
                                    break;
                                case SDLK_f:
                                    buttons_pressed_actions(PADDLE_BOTTOM_RIGHT);
                                    break;
                            }
                            break;
                        } else if (last_key == event.key.keysym.sym && (clock() - press_time) > 500 && !once) {
                            once = true;
                            switch (event.key.keysym.sym) {
                                case SDLK_w:
                                    buttons_long_pressed_actions(BUTTON_TOP_LEFT);
                                    break;
                                case SDLK_e:
                                    buttons_long_pressed_actions(BUTTON_TOP_RIGHT);
                                    break;
                                case SDLK_s:
                                    buttons_long_pressed_actions(BUTTON_BOTTOM_LEFT);
                                    break;
                                case SDLK_d:
                                    buttons_long_pressed_actions(BUTTON_BOTTOM_RIGHT);
                                    break;
                                case SDLK_a:
                                    buttons_long_pressed_actions(PADDLE_BOTTOM_LEFT);
                                    break;
                                case SDLK_f:
                                    buttons_long_pressed_actions(PADDLE_BOTTOM_RIGHT);
                                    break;
                            }
                            break;
                        }
                        break;
                    } break;
                    case SDL_KEYUP: {
                        last_key = 0;
                        once     = false;
                        switch (event.key.keysym.sym) {
                            case SDLK_w:
                                buttons_released_actions(BUTTON_TOP_LEFT);
                                break;
                            case SDLK_e:
                                buttons_released_actions(BUTTON_TOP_RIGHT);
                                break;
                            case SDLK_s:
                                buttons_released_actions(BUTTON_BOTTOM_LEFT);
                                break;
                            case SDLK_d:
                                buttons_released_actions(BUTTON_BOTTOM_RIGHT);
                                break;
                            case SDLK_q:
                                buttons_released_actions(PADDLE_TOP_LEFT);
                                break;
                            case SDLK_r:
                                buttons_released_actions(PADDLE_TOP_RIGHT);
                                break;
                            case SDLK_a:
                                buttons_released_actions(PADDLE_BOTTOM_LEFT);
                                break;
                            case SDLK_f:
                                buttons_released_actions(PADDLE_BOTTOM_RIGHT);
                                break;
                        }
                        break;
                    } break;
                }
            }

            SDL_Rect window_rect = {0, 0, vc_sdl_actual_width, vc_sdl_actual_height};

            // Render the texture
            Olivec_Canvas oc_src = vc_render(dt, &sw_screen);
            if (oc_src.width != vc_sdl_actual_width || oc_src.height != vc_sdl_actual_height) {
                if (!vc_sdl_resize_texture(renderer, oc_src.width, oc_src.height))
                    return_defer(1);
                SDL_SetWindowSize(window, vc_sdl_actual_width, vc_sdl_actual_height);
            }
            void *pixels_dst;
            int pitch;
            if (SDL_LockTexture(vc_sdl_texture, &window_rect, &pixels_dst, &pitch) < 0)
                return_defer(1);
            for (size_t y = 0; y < vc_sdl_actual_height; ++y) {
                // TODO: it would be cool if Olivec_Canvas supported pitch in bytes instead of pixels
                // It would be more flexible and we could draw on the locked texture memory directly
                memcpy((char *)pixels_dst + y * pitch, oc_src.pixels + y * vc_sdl_actual_width, vc_sdl_actual_width * sizeof(uint32_t));
            }
            SDL_UnlockTexture(vc_sdl_texture);

            // Display the texture
            if (SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0) < 0)
                return_defer(1);
            if (SDL_RenderClear(renderer) < 0)
                return_defer(1);
            if (SDL_RenderCopy(renderer, vc_sdl_texture, &window_rect, &window_rect) < 0)
                return_defer(1);
            SDL_RenderPresent(renderer);
        }
    }

defer:
    switch (result) {
        case 0:
            printf("OK\n");
            break;
        default:
            fprintf(stderr, "SDL ERROR: %s\n", SDL_GetError());
    }
    if (vc_sdl_texture)
        SDL_DestroyTexture(vc_sdl_texture);
    if (renderer)
        SDL_DestroyRenderer(renderer);
    if (window)
        SDL_DestroyWindow(window);
    SDL_Quit();
    return result;
}
