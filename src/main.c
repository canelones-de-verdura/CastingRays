#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_video.h>

#include "data/screen.h"
#include "img.h"
#include "player.h"
#include "rays.h"
#include "types.h"

#define MAP_SIDE 20

void load_map(Map *map, const char *map_data, size_t map_w, size_t map_l) {
    map->width = map_w;
    map->length = map_l;
    map->tiles = malloc(sizeof(Tile) * map_w * map_l);

    if (map->tiles == NULL)
        exit(1);

    Tile *current_tile;
    for (int i = 0; i < map_w; ++i) {
        for (int j = 0; j < map_l; ++j) {
            current_tile = &map->tiles[i + j * map_w];

            if (map_data[i + j * map_w] == ' ') {
                current_tile->type = FLOOR;
                current_tile->walkable = true;
            } else {
                current_tile->type = WALL;
                current_tile->walkable = false;
            }
        }
    }
}

void FramebufferInit(FrameBuffer *fb) {
    fb->width = VIEW_W;
    fb->height = VIEW_H;
    fb->buf = calloc(VIEW_W * VIEW_H, sizeof(uint32_t));
}

int main() {
    // 32 bits para rgba
    // en vez de [x][y] hacemos [x+y*vw]
    /*uint32_t framebuffer[VW * VH];*/
    /*generate_default_img(framebuffer, VW, VH);*/

    // mapa
    Map map;
    const char *mapdata = "11111111111111111111"
                          "1  1            1  1"
                          "1  1            1  1"
                          "1  1            1  1"
                          "1  11123    32111  1"
                          "1                  1"
                          "1                  1"
                          "1                  1"
                          "1                 11"
                          "1                111"
                          "1                 11"
                          "1                  1"
                          "1                  1"
                          "1            1     1"
                          "1111    1    1     1"
                          "11111111111111     1"
                          "1     1            1"
                          "1     1            1"
                          "1                  1"
                          "11111111111111111111";

    load_map(&map, mapdata, MAP_SIDE, MAP_SIDE);

    // jugador
    Player player;
    PlayerInit(&player);

    // framebuffer
    FrameBuffer fb;
    FramebufferInit(&fb);

    // sdl2
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
        return 1;

    int win_flags = SDL_WINDOW_OPENGL;
    SDL_Window *win;
    SDL_Renderer *ren;

    int res =
        SDL_CreateWindowAndRenderer(VIEW_W, VIEW_H, win_flags, &win, &ren);

    if (res < 0) // supuestamente no tendría que chequear win/ren
        return 1;

    SDL_Texture *txr =
        SDL_CreateTexture(ren, SDL_PIXELFORMAT_RGBA8888,
                          SDL_TEXTUREACCESS_STREAMING, VIEW_W, VIEW_H);
    if (!txr)
        return 1;

    SDL_Event event;
    int cont = 1;
    int xmouse = 0;

    // medir fps?
    int num_frames = 0;
    uint32_t start_time = SDL_GetTicks();
    uint32_t current_time;

    while (cont) {
        printf("\033[H\033[J"); // limpia terminal

        ++num_frames;
        current_time = SDL_GetTicks() - start_time;

        if (current_time) {
            double elapsedSeconds = current_time / 1000.0;
            double fps = num_frames / elapsedSeconds;
            printf("%f FPS\n", fps);
        }

        printf("POS %f X, %f Y\n", player.pos.x, player.pos.y);
        printf("DIR %f RAD\n", player.direction);

        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
                cont = 0;
                break;
            case SDL_KEYDOWN:
                /*const uint8_t *keyboard_state = SDL_GetKeyboardState(NULL);*/
                /*if (keyboard_state[SDL_SCANCODE_W]) {*/
                /*    if (keyboard_state[SDL_SCANCODE_D]) {*/
                /*        PlayerMove(&player, UPRIGHT);*/
                /*    }*/
                /*}*/

                // trucho mal
                switch (event.key.keysym.sym) {
                case SDLK_w:
                    PlayerMove(&player, UP);
                    break;
                case SDLK_a:
                    PlayerMove(&player, LEFT);
                    break;
                case SDLK_s:
                    PlayerMove(&player, DOWN);
                    break;
                case SDLK_d:
                    PlayerMove(&player, RIGHT);
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
        // tiene que haber una forma mejor de dibujar el buffer xq esto es
        // nefasto

        generate_default_img(fb.buf, fb.width, fb.height);

        render_view(&fb, &map, &player);

        /*render_view(fb.buf, fb.width, fb.height, &map, player.pos,
         * player.direction,*/
        /*            player.fov);*/

        // Copiamos el buffer a la textura que usa SDL
        for (int i = 0; i < fb.height * fb.width; ++i) {
            ((uint32_t *)pixels)[i] = fb.buf[i];
        }

        SDL_UnlockTexture(txr);

        SDL_RenderClear(ren);
        SDL_RenderCopy(ren, txr, NULL, NULL);
        SDL_RenderPresent(ren);
    }

    // limpiamos
    free(map.tiles);
    SDL_DestroyTexture(txr);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
