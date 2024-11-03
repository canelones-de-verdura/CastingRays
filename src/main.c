#include "img.h"
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_render.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>
#include <string.h>

#define VW 800
#define VH 600

#define MAP_SIDE 10

#define PI 3 // comeme los dos huevos

void draw_rect_on_buffer(uint32_t *framebuffer, const size_t rect_x,
                         const size_t rect_y, const size_t rect_w,
                         const size_t rect_h, const uint32_t color) {
    for (size_t i = rect_x; i < rect_x + rect_w; ++i) {
        for (size_t j = rect_y; j < rect_y + rect_h; ++j) {
            framebuffer[i + j * VW] = color;
        }
    }
}

float cast_ray(const uint8_t *map, const float *player_xy,
               float player_direction) {
    float ray_len = 0;
    float x, y;
    for (; ray_len < 20; ray_len += .05) {
        x = player_xy[0] + ray_len * cos(player_direction);
        y = player_xy[1] + ray_len * sin(player_direction);
        if (map[(int)x + (int)y * MAP_SIDE] != ' ')
            break;
    }

    return ray_len;
}

void render_view(uint32_t *framebuffer, const uint8_t *map,
                 const float *player_xy, float player_direction,
                 float player_fov) {
    for (int i = 0; i < VW; ++i) {
        float angle =
            (player_direction - player_fov / 2) + (player_fov * i) / (float)VW;
        float current_dist = cast_ray(map, player_xy, angle);

        size_t column = VH / current_dist;

        uint32_t cyan = (0 << 24) + (255 << 16) + (255 << 8);

        draw_rect_on_buffer(framebuffer, i, VH / 2 - column / 2, 1, column,
                            cyan);
    }
}

typedef struct player_st {
    float xy[2];
    float direction;
    float fov;
} player_t;

int main() {
    // 32 bits para rgba
    // en vez de [x][y] hacemos [x+y*vw]
    uint32_t framebuffer[VW * VH];
    generate_default_img(framebuffer, VW, VH);

    // mapa
    const uint8_t map[MAP_SIDE * MAP_SIDE] = "1111111111"
                                             "1        1"
                                             "1        1"
                                             "1       11"
                                             "1        1"
                                             "1      111"
                                             "1        1"
                                             "1   1    1"
                                             "1   1    1"
                                             "1111111111";

    // jugador
    player_t player = {
        .xy = {5, 5},
        .direction = 0.,
        .fov = PI / 2.,
    };

    // sdl2
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
        return 1;

    int win_flags = SDL_WINDOW_OPENGL;
    SDL_Window *win;
    SDL_Renderer *ren;

    int res = SDL_CreateWindowAndRenderer(VW, VH, win_flags, &win, &ren);

    if (res < 0) // supuestamente no tendría que chequear win/ren
        return 1;

    SDL_Texture *txr = SDL_CreateTexture(ren, SDL_PIXELFORMAT_RGBA8888,
                                         SDL_TEXTUREACCESS_STREAMING, VW, VH);
    if (!txr)
        return 1;

    SDL_Event event;
    int cont = 1;
    int xmouse = 0;
    while (cont) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
                cont = 0;
                break;
            case SDL_KEYDOWN:
                // trucho mal
                switch (event.key.keysym.sym) {
                case SDLK_w:
                    player.xy[0] += .1;
                    break;
                case SDLK_a:
                    player.xy[1] -= .1;
                    break;
                case SDLK_s:
                    player.xy[0] -= .1;
                    break;
                case SDLK_d:
                    player.xy[1] += .1;
                    break;
                }
                break;
            case SDL_MOUSEMOTION:
                if (event.motion.x > xmouse)
                    player.direction += PI / 100.;
                else
                    player.direction -= PI / 100.;

                xmouse = event.motion.x;
            }
        }

        void *pixels;
        int pitch;

        SDL_LockTexture(txr, NULL, &pixels, &pitch);
        // tiene que haber una forma mejor de dibujar el buffer xq esto es nefasto


        generate_default_img(pixels, VW, VH);

        // to' negro
        /*for (int i = 0; i < VW; ++i)*/
        /*    for (int j = 0; j < VH; ++j)*/
        /*        ((int *)pixels)[i + j * VW] = colors_in(0, 0, 0, 0);*/

        render_view(pixels, map, player.xy, player.direction, player.fov);

        SDL_UnlockTexture(txr);

        SDL_RenderClear(ren);
        SDL_RenderCopy(ren, txr, NULL, NULL);
        SDL_RenderPresent(ren);
    }

    SDL_DestroyTexture(txr);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
