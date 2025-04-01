#define SDL_MAIN_USE_CALLBACKS

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_scancode.h>
#include <SDL3/SDL_surface.h>
#include <SDL3/SDL_video.h>

#include <SDL3_image/SDL_image.h>

#include <math.h>
#include <stdlib.h>

#include "game_map.h"
#include "game_player.h"

struct state {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *sprites;
    SDL_Texture *skybox;

    struct Map map;
    struct Player player;
};

SDL_AppResult LogAndDie(const char msg[]) {
    SDL_Log("[ERROR] %s - %s", msg, SDL_GetError());
    return SDL_APP_FAILURE;
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv) {
    constexpr int window_width = 1280;
    constexpr int window_height = 720;
    constexpr int logical_width = 640;
    constexpr int logical_height = 360;
    constexpr int init_flags = SDL_INIT_VIDEO;

    /* init */
    if (!SDL_Init(init_flags))
        return LogAndDie("Failure to initialize");

    /* window and renderer */
    struct state *gs = malloc(sizeof(struct state));
    if (!SDL_CreateWindowAndRenderer("Casting Rays",
                                     window_width,
                                     window_height,
                                     0,
                                     &gs->window,
                                     &gs->renderer))
        return LogAndDie("Failure to create window or renderer");

    // render settings
    if (!SDL_SetRenderVSync(gs->renderer, SDL_RENDERER_VSYNC_DISABLED))
        return LogAndDie("Failure to disable VSync");

    // stuff will render at 640x360 and then scale to whatever the window
    // resolution is
    if (!SDL_SetRenderLogicalPresentation(gs->renderer,
                                          logical_width,
                                          logical_height,
                                          SDL_LOGICAL_PRESENTATION_LETTERBOX))
        return LogAndDie("Failure to set logical resolution");

    // load sprites from disc to VRAM (or normal RAM in iGPUs, i guess)
    gs->sprites = IMG_LoadTexture(gs->renderer,
                                  "./assets/32rogues-0.5.0/32rogues/tiles.png");
    if (!gs->sprites)
        return LogAndDie("Failure to load texture from image");

    if (!SDL_SetTextureScaleMode(gs->sprites, SDL_SCALEMODE_NEAREST))
        return LogAndDie("Failure to change scaling mode on texture");

    gs->skybox = IMG_LoadTexture(gs->renderer, "./assets/skybox.png");
    if (!gs->skybox)
        return LogAndDie("Failure to load texture from image");

    if (!SDL_SetTextureScaleMode(gs->skybox, SDL_SCALEMODE_NEAREST))
        return LogAndDie("Failure to change scaling mode on texture");

    /* game stuff */
    gs->player = InitPlayer();

    const char *mapdata = "11111111111111111111"
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
                          "11111111111111111111";

    if (!LoadMap(&gs->map, mapdata, 20, 20))
        LogAndDie("Failure to load map");

    /* assign everything to the global state */
    *appstate = gs;

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
    struct state *gs = appstate;

    SDL_RenderClear(gs->renderer);

    // render skybox
    SDL_FRect dstrect = {0, 0, 640, 180};
    SDL_RenderTexture(gs->renderer, gs->skybox, nullptr, &dstrect);

    // render view
    enum wall_align { NS, WE };
    struct ray {
        double ray_length, hit_x, hit_y;
        enum wall_align orientation;
    } raycasts[640];

    // sweep view and get raycasts using DDA
    for (int i = 0; i < 640; ++i) {
        struct ray *r = &raycasts[i];

        // dir in radians
        double angle = (gs->player.dir - gs->player.fov / 2.) +
                       (gs->player.fov * i) / (double)640;

        // dir as a vector
        struct doubleVec ray_dir = {
            .x = cos(angle),
            .y = sin(angle),
        };

        // position as int coords- that is, the position of the tile
        struct intVec map_pos = {
            .x = (int)gs->player.pos.x,
            .y = (int)gs->player.pos.y,
        };

        // "steps" needed to cross tiles. x & y can take either 1 or -1
        struct intVec map_step = {
            .x = (ray_dir.x > 0) ? 1 : -1,
            .y = (ray_dir.y > 0) ? 1 : -1,
        };

        // because every cell is the same size, we can calculate the length a
        // segment of the ray will have between two "borders" of the grid (on
        // the same axis)
        struct doubleVec ray_increment = {
            .x = sqrt(1 + (ray_dir.y / ray_dir.x) * (ray_dir.y / ray_dir.x)),
            .y = sqrt(1 + (ray_dir.x / ray_dir.y) * (ray_dir.x / ray_dir.y)),
        };

        // initial lengths from wherever the origin is inside the cell to the
        // first x-border or y-border
        double player_pos_dec_x = gs->player.pos.x - (int)gs->player.pos.x;
        double player_pos_dec_y = gs->player.pos.y - (int)gs->player.pos.y;

        struct doubleVec init_step = {
            .x = (map_step.x == 1) ? 1 - player_pos_dec_x : -player_pos_dec_x,
            .y = (map_step.y == 1) ? 1 - player_pos_dec_y : -player_pos_dec_y,
        };
        struct doubleVec ray_length = {
            .x = init_step.x / cos(angle),
            .y = init_step.y / sin(angle),
        };

        // the famous cast
        while (true) {
            if (ray_length.x < ray_length.y) {
                r->ray_length = ray_length.x;
                r->hit_x = gs->player.pos.x + r->ray_length * cos(angle);
                r->hit_y = gs->player.pos.y + r->ray_length * sin(angle);
                r->orientation = WE;

                map_pos.x += map_step.x;
                ray_length.x += ray_increment.x;
            } else {
                r->ray_length = ray_length.y;
                r->hit_x = gs->player.pos.x + r->ray_length * cos(angle);
                r->hit_y = gs->player.pos.y + r->ray_length * sin(angle);
                r->orientation = NS;

                map_pos.y += map_step.y;
                ray_length.y += ray_increment.y;
            }

            if (GetTileInMap(&gs->map, map_pos.x, map_pos.y).type != FLOOR)
                break;
        }
    }

    // render "columns"
    for (int i = 0; i < 640; ++i) {
        struct ray *r = &raycasts[i];
        int column_height =
            (360 / r->ray_length) > 360 ? 360 : 360 / r->ray_length;

        int col_y = 360 / 2 - column_height / 2;

        // the texture begins in (0, 32). i want to offset the first value to
        // get the "slice" or "column" to render.
        SDL_FRect wallsprite = {0, 32, 1, 32};

        double offset_x = r->hit_x - (int)r->hit_x;
        double offset_y = r->hit_y - (int)r->hit_y;

        if (r->orientation == NS) {
            wallsprite.x = (int)(offset_x * 32) % 32;
        } else {
            wallsprite.x = (int)(offset_y * 32) % 32;
        }

        SDL_FRect dstrect = {i, col_y, 1, column_height};

        SDL_RenderTexture(gs->renderer, gs->sprites, &wallsprite, &dstrect);
    }

    SDL_RenderPresent(gs->renderer);

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    struct state *gs = appstate;
    double xmouse = 0;

    if (event->type == SDL_EVENT_QUIT)
        return SDL_APP_SUCCESS;

    if (event->type == SDL_EVENT_KEY_DOWN) {
        // some really basic movement
        if (event->key.scancode == SDL_SCANCODE_UP) {
            gs->player.pos.x += cos(gs->player.dir) * .1;
            gs->player.pos.y += sin(gs->player.dir) * .1;
        }

        if (event->key.scancode == SDL_SCANCODE_DOWN) {
            gs->player.pos.x -= cos(gs->player.dir) * .1;
            gs->player.pos.y -= sin(gs->player.dir) * .1;
        }

        if (event->key.scancode == SDL_SCANCODE_RIGHT) {
            gs->player.dir += PI / 75.;
        }

        if (event->key.scancode == SDL_SCANCODE_LEFT) {
            gs->player.dir -= PI / 75.;
        }
    }

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    struct state *gs = appstate;
    if (result != SDL_APP_FAILURE) {
        SDL_DestroyTexture(gs->sprites);
        SDL_DestroyRenderer(gs->renderer);
        SDL_DestroyWindow(gs->window);
        gs->sprites = nullptr;
        gs->renderer = nullptr;
        gs->window = nullptr;
    }
    DestroyMap(&gs->map);

    free(gs);
    gs = nullptr;
    appstate = nullptr;

    SDL_Quit();
}
