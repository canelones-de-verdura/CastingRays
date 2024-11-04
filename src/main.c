#include <SDL2/SDL_timer.h>
#include <stdint.h>
#include <stdio.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_video.h>

#include <string.h>

#include "img.h"
#include "player.h"
#include "render.h"
#include "types.h"

#define VW 800
#define VH 600

#define MAP_SIDE 20

int main() {
    // 32 bits para rgba
    // en vez de [x][y] hacemos [x+y*vw]
    /*uint32_t framebuffer[VW * VH];*/
    /*generate_default_img(framebuffer, VW, VH);*/

    // mapa
    const Map map = {.width = MAP_SIDE,
                     .length = MAP_SIDE,
                     .data = "11111111111111111111"
                             "1  1            1  1"
                             "1  1            1  1"
                             "1  1            1  1"
                             "1  11111    11111  1"
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
                             "11111111111111111111"};

    // jugador
    Player player;
    PlayerInit(&player);

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

    // medir fps?
    int num_frames = 0;
    uint32_t start_time = SDL_GetTicks();
    uint32_t current_time;

    while (cont) {
        ++num_frames;
        current_time = SDL_GetTicks() - start_time;

        if (current_time) {
            double elapsedSeconds = current_time / 1000.0;
            double fps = num_frames / elapsedSeconds;
            printf("%f\n", fps);
        }

        /*printf("%f, %f\n", player.pos.x, player.pos.y);*/
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
                cont = 0;
                break;
            case SDL_KEYDOWN:
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

        generate_default_img(pixels, VW, VH);

        // to' negro
        /*for (int i = 0; i < VW; ++i)*/
        /*    for (int j = 0; j < VH; ++j)*/
        /*        ((int *)pixels)[i + j * VW] = colors_in(0, 0, 0, 0);*/

        render_view(pixels, VW, VH, &map, player.pos, player.direction,
                    player.fov);

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
