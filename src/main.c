#include <SDL3/SDL_pixels.h>
#define SDL_MAIN_USE_CALLBACKS

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_keyboard.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_scancode.h>
#include <SDL3/SDL_surface.h>
#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_video.h>

#include <SDL3_image/SDL_image.h>

#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "game_map.h"
#include "game_math.h"
#include "game_player.h"

typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *draw_target;

    int win_width;
    int win_height;
    int logical_win_width;
    int logical_win_height;

    uint64_t start_time;
    uint64_t frame_time;
    double delta_time;
    bool app_is_active;
} AppContext;

typedef struct {
    struct Map map;
    struct Camera camera;

    SDL_Texture *spritesheet;
    SDL_Texture *skybox;
} Game;

typedef struct {
    AppContext ctx;
    Game scene;
} State;

SDL_AppResult LogAndDie(const char msg[]) {
    SDL_Log("[ERROR] %s - %s", msg, SDL_GetError());
    return SDL_APP_FAILURE;
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv) {
    /* global state */
    static State gs = {{
                           .win_width = 1280,
                           .win_height = 960,
                           .logical_win_width = 640,
                           .logical_win_height = 480,
                           .app_is_active = true,
                       },
                       {}};

    /* init */
    constexpr int init_flags = SDL_INIT_VIDEO;
    if (!SDL_Init(init_flags))
        return LogAndDie("Failure to initialize");

    /* window and renderer */
    // constexpr int win_init_flags = SDL_WINDOW_FULLSCREEN | SDL_WINDOW_VULKAN;
    constexpr int win_init_flags = 0;
    if (!SDL_CreateWindowAndRenderer("Casting Rays",
                                     gs.ctx.win_width,
                                     gs.ctx.win_height,
                                     win_init_flags,
                                     &gs.ctx.window,
                                     &gs.ctx.renderer))
        return LogAndDie("Failure to create window or renderer");

    // render settings
    if (!SDL_SetRenderVSync(gs.ctx.renderer, SDL_RENDERER_VSYNC_DISABLED))
        return LogAndDie("Failure to disable VSync");

    // stuff will render at 640x360 and then scale to whatever the window
    // resolution is
    if (!SDL_SetRenderLogicalPresentation(gs.ctx.renderer,
                                          gs.ctx.logical_win_width,
                                          gs.ctx.logical_win_height,
                                          SDL_LOGICAL_PRESENTATION_LETTERBOX))
        return LogAndDie("Failure to set logical resolution");

    // load sprites from disc to VRAM (or normal RAM in iGPUs, i guess)
    gs.scene.spritesheet =
        IMG_LoadTexture(gs.ctx.renderer, "./assets/sprites_walls.png");
    if (!gs.scene.spritesheet)
        return LogAndDie("Failure to load texture from image");

    if (!SDL_SetTextureScaleMode(gs.scene.spritesheet, SDL_SCALEMODE_NEAREST))
        return LogAndDie("Failure to change scaling mode on texture");

    gs.scene.skybox = IMG_LoadTexture(gs.ctx.renderer, "./assets/skybox.png");
    if (!gs.scene.skybox)
        return LogAndDie("Failure to load texture from image");

    if (!SDL_SetTextureScaleMode(gs.scene.skybox, SDL_SCALEMODE_NEAREST))
        return LogAndDie("Failure to change scaling mode on texture");

    /* game stuff */
    gs.scene.camera = Camera_init();

    const char *mapdata =
        "11111111111111111111" // string instead of int so the formatter
        "1  11  11  11  11  1" // wont mess the layout
        "1  11  11  11  11  1"
        "1                  1"
        "1                  1"
        "1                  1"
        "1                  1"
        "1                  1"
        "1                  1"
        "1                  1"
        "1                  1"
        "1                  1"
        "1                  1"
        "1                  1"
        "1                  1"
        "1                  1"
        "1                  1"
        "1                  1"
        "1                  1"
        "11111111111111111111";

    if (!LoadMap(&gs.scene.map, mapdata, 20, 20))
        LogAndDie("Failure to load map");

    /* assign everything to the global state */
    *appstate = &gs;

    return SDL_APP_CONTINUE;
}

void render(State *gs) {
    // it is, probably, "bad practice" to throw the entire global state to a
    // single function. i do not really care

    SDL_RenderClear(gs->ctx.renderer);

    // render skybox
    /*SDL_FRect dstrect = {0, 0, 640, 180};*/
    /*SDL_RenderTexture(gs->renderer, gs->skybox, nullptr, &dstrect);*/

    // sweep view and get raycasts using DDA
    struct raycast raycasts[gs->ctx.logical_win_width];

    for (int i = 0; i < gs->ctx.logical_win_width; ++i) {
        // dir in radians
        double angle =
            (atan2(gs->scene.camera.dir.y, gs->scene.camera.dir.x) -
             gs->scene.camera.fov / 2.) +
            (gs->scene.camera.fov * i) / (double)gs->ctx.logical_win_width;

        // position from where we shoot the ray
        struct FVec2 origin =
            (struct FVec2){gs->scene.camera.pos.x, gs->scene.camera.pos.y};

        raycasts[i] = Camera_raycast(origin, angle, &gs->scene.map);

        // fix fisheye
        double diff =
            atan2(gs->scene.camera.dir.y, gs->scene.camera.dir.x) - angle;

        raycasts[i].ray_length *= cos(diff);
    }

    // render "columns"
    for (int i = 0; i < gs->ctx.logical_win_width; ++i) {
        struct raycast *ray = &raycasts[i];
        int column_height = (gs->ctx.logical_win_height / ray->ray_length) >
                                    gs->ctx.logical_win_height
                                ? gs->ctx.logical_win_height
                                : gs->ctx.logical_win_height / ray->ray_length;

        int col_y = gs->ctx.logical_win_height / 2. - column_height / 2. +
                    gs->scene.camera.vertical_offset;

        // the texture begins in (0, 0). i want to offset the first value to
        // get the "slice" or "column" to render.
        // pretty obvious, but the 128 is there bc it's the width and height
        // of each sprite.
        // SDL_FRect wallsprite = {0, 32, 1, 32};
        SDL_FRect wallsprite = {0, 0, 1, 128};

        double offset_x = ray->hit_x - (int)ray->hit_x;
        double offset_y = ray->hit_y - (int)ray->hit_y;

        // int casts are needed to avoid some weird effects with sub-pixel
        // rendering
        if (ray->wall_align == 0) {
            wallsprite.x += (int)(offset_x * 128) % 128;
        } else {
            wallsprite.x += (int)(offset_y * 128) % 128;
        }

        SDL_FRect dstrect = {i, col_y, 1, column_height};

        /* rendering and scuffed lightning */
        struct color {
            float r, g, b;
        }; // texture colors

        struct color c_original;
        struct color c_shaded;
        SDL_GetTextureColorModFloat(
            gs->scene.spritesheet, &c_original.r, &c_original.g, &c_original.b);

        /* shading */
        c_shaded.r = c_original.r - ray->ray_length * 0.05;
        c_shaded.g = c_original.g - ray->ray_length * 0.05;
        c_shaded.b = c_original.b - ray->ray_length * 0.05;
        SDL_SetTextureColorModFloat(gs->scene.spritesheet,
                                    c_shaded.r - .1,
                                    c_shaded.g - .1,
                                    c_shaded.b - .1);

        SDL_RenderTexture(
            gs->ctx.renderer, gs->scene.spritesheet, &wallsprite, &dstrect);

        // set proper colors
        SDL_SetTextureColorModFloat(
            gs->scene.spritesheet, c_original.r, c_original.g, c_original.b);
    }

    // "crosshair"
    SDL_SetRenderDrawColor(gs->ctx.renderer, 255, 255, 255, 255);
    SDL_RenderPoint(gs->ctx.renderer,
                    gs->ctx.logical_win_width / 2.,
                    gs->ctx.logical_win_height / 2.);
    SDL_SetRenderDrawColor(gs->ctx.renderer, 0, 0, 0, 0);

    SDL_RenderPresent(gs->ctx.renderer);
}

SDL_AppResult SDL_AppIterate(void *appstate) {
    State *gs = appstate;
    if (!gs->ctx.app_is_active)
        return SDL_APP_CONTINUE;

    const bool *keyboard = SDL_GetKeyboardState(nullptr);

    /* timing */
    gs->ctx.start_time = SDL_GetTicks();

    /* input */
    auto move = (struct FVec2){0, 0};
    if (keyboard[SDL_SCANCODE_W]) {
        double dir_angle =
            atan2(gs->scene.camera.dir.y, gs->scene.camera.dir.x);
        move.x += cos(dir_angle) * 5;
        move.y += sin(dir_angle) * 5;
    }

    if (keyboard[SDL_SCANCODE_A]) {
        double dir_angle =
            atan2(gs->scene.camera.dir.y, gs->scene.camera.dir.x);
        move.x -= cos(dir_angle + 90 * PI / 180) * 5;
        move.y -= sin(dir_angle + 90 * PI / 180) * 5;
    }

    if (keyboard[SDL_SCANCODE_S]) {
        double dir_angle =
            atan2(gs->scene.camera.dir.y, gs->scene.camera.dir.x);
        move.x -= cos(dir_angle) * 5;
        move.y -= sin(dir_angle) * 5;
    }

    if (keyboard[SDL_SCANCODE_D]) {
        double dir_angle =
            atan2(gs->scene.camera.dir.y, gs->scene.camera.dir.x);
        move.x += cos(dir_angle + 90 * PI / 180) * 5;
        move.y += sin(dir_angle + 90 * PI / 180) * 5;
    }

    Camera_velUpdate(&gs->scene.camera, move);
    Camera_physUpdate(&gs->scene.camera, gs->ctx.delta_time);

    render(gs);

    /* timing, again */
    SDL_Delay(gs->ctx.start_time + 16.67 -
              SDL_GetTicks()); // Locks the frame rate at 60

    gs->ctx.frame_time = SDL_GetTicks() - gs->ctx.start_time;
    gs->ctx.delta_time = gs->ctx.frame_time / 1000.;
    if (gs->ctx.frame_time) {
        double fps = 1000. / gs->ctx.frame_time;
        SDL_Log("%f FPS", fps);
        SDL_Log("%f delta", gs->ctx.delta_time);
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    State *gs = appstate;
    double xmouse = 0;

    if (event->type == SDL_EVENT_QUIT)
        return SDL_APP_SUCCESS;

    if (event->type == SDL_EVENT_KEY_DOWN) {
        // camera
        if (event->key.scancode == SDL_SCANCODE_RIGHT) {
            gs->scene.camera.dir.x = gs->scene.camera.dir.x * cos(.1) -
                                     gs->scene.camera.dir.y * sin(.1);
            gs->scene.camera.dir.y = gs->scene.camera.dir.x * sin(.1) +
                                     gs->scene.camera.dir.y * cos(.1);
        }

        if (event->key.scancode == SDL_SCANCODE_LEFT) {
            gs->scene.camera.dir.x = gs->scene.camera.dir.x * cos(-.1) -
                                     gs->scene.camera.dir.y * sin(-.1);
            gs->scene.camera.dir.y = gs->scene.camera.dir.x * sin(-.1) +
                                     gs->scene.camera.dir.y * cos(-.1);
        }

        // movement
        if (event->key.scancode == SDL_SCANCODE_UP) {
            double dir_angle =
                atan2(gs->scene.camera.dir.y, gs->scene.camera.dir.x);
            gs->scene.camera.pos.x += cos(dir_angle) * .1;
            gs->scene.camera.pos.y += sin(dir_angle) * .1;
        }
    }

    if (event->type == SDL_EVENT_KEY_UP) {
        switch (event->key.scancode) {
        case SDL_SCANCODE_ESCAPE:
            return SDL_APP_SUCCESS;
        }
    }

    if (event->type == SDL_EVENT_WINDOW_FOCUS_LOST)
        gs->ctx.app_is_active = false;

    if (event->type == SDL_EVENT_WINDOW_FOCUS_GAINED)
        gs->ctx.app_is_active = true;

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    State *gs = appstate;
    if (result != SDL_APP_FAILURE) {
        SDL_DestroyTexture(gs->scene.spritesheet);
        SDL_DestroyRenderer(gs->ctx.renderer);
        SDL_DestroyWindow(gs->ctx.window);
        gs->scene.spritesheet = nullptr;
        gs->ctx.renderer = nullptr;
        gs->ctx.window = nullptr;
    }
    DestroyMap(&gs->scene.map);

    appstate = nullptr;
    SDL_Quit();
}
