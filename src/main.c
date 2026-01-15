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

struct state {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *spritesheet;
    SDL_Texture *skybox;

    int win_width;
    int win_height;
    int logical_win_width;
    int logical_win_height;

    struct Map map;
    struct Camera camera;

    uint64_t start_time;
    uint64_t frame_time;
    double delta_time;
};

SDL_AppResult LogAndDie(const char msg[]) {
    SDL_Log("[ERROR] %s - %s", msg, SDL_GetError());
    return SDL_APP_FAILURE;
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv) {
    /* global state */
    static struct state gs = (struct state){
        .win_width = 1280,
        .win_height = 720,
        .logical_win_width = 640,
        .logical_win_height = 360,

        .start_time = 0,
        .frame_time = 0,
        .delta_time = 0,
    };

    /* init */
    constexpr int init_flags = SDL_INIT_VIDEO;
    if (!SDL_Init(init_flags))
        return LogAndDie("Failure to initialize");

    /* window and renderer */
    if (!SDL_CreateWindowAndRenderer("Casting Rays",
                                     gs.win_width,
                                     gs.win_height,
                                     0,
                                     &gs.window,
                                     &gs.renderer))
        return LogAndDie("Failure to create window or renderer");

    // render settings
    if (!SDL_SetRenderVSync(gs.renderer, SDL_RENDERER_VSYNC_DISABLED))
        return LogAndDie("Failure to disable VSync");

    // stuff will render at 640x360 and then scale to whatever the window
    // resolution is
    if (!SDL_SetRenderLogicalPresentation(gs.renderer,
                                          gs.logical_win_width,
                                          gs.logical_win_height,
                                          SDL_LOGICAL_PRESENTATION_LETTERBOX))
        return LogAndDie("Failure to set logical resolution");

    // load sprites from disc to VRAM (or normal RAM in iGPUs, i guess)
    gs.spritesheet = IMG_LoadTexture(gs.renderer, "./assets/sprites_walls.png");
    if (!gs.spritesheet)
        return LogAndDie("Failure to load texture from image");

    if (!SDL_SetTextureScaleMode(gs.spritesheet, SDL_SCALEMODE_NEAREST))
        return LogAndDie("Failure to change scaling mode on texture");

    gs.skybox = IMG_LoadTexture(gs.renderer, "./assets/skybox.png");
    if (!gs.skybox)
        return LogAndDie("Failure to load texture from image");

    if (!SDL_SetTextureScaleMode(gs.skybox, SDL_SCALEMODE_NEAREST))
        return LogAndDie("Failure to change scaling mode on texture");

    /* game stuff */
    gs.camera = Camera_init();

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

    if (!LoadMap(&gs.map, mapdata, 20, 20))
        LogAndDie("Failure to load map");

    /* assign everything to the global state */
    *appstate = &gs;

    return SDL_APP_CONTINUE;
}

struct raycast {
    double ray_length, hit_x, hit_y;
    int wall_align; // 0 or 1
};

struct raycast cast_ray(struct Map *map, struct FVec2 origin, double angle) {
    struct raycast r;

    // dir as a vector
    struct FVec2 ray_dir = {
        .x = cos(angle),
        .y = sin(angle),
    };

    // position as int coords- that is, the position of the tile
    struct Vec2 map_pos = {
        .x = (int)origin.x,
        .y = (int)origin.y,
    };

    // "steps" needed to cross tiles. x & y can take either 1 or -1
    struct Vec2 map_step = {
        .x = (ray_dir.x > 0) ? 1 : -1,
        .y = (ray_dir.y > 0) ? 1 : -1,
    };

    // because every cell is the same size, we can calculate the length a
    // segment of the ray will have between two "borders" of the grid (on
    // the same axis)
    struct FVec2 ray_increment = {
        .x = sqrt(1 + (ray_dir.y / ray_dir.x) * (ray_dir.y / ray_dir.x)),
        .y = sqrt(1 + (ray_dir.x / ray_dir.y) * (ray_dir.x / ray_dir.y)),
    };

    // initial lengths from wherever the origin is inside the cell to the
    // first x-border or y-border
    double origin_dec_x = origin.x - (int)origin.x;
    double origin_dec_y = origin.y - (int)origin.y;

    struct FVec2 init_step = {
        .x = (map_step.x == 1) ? 1 - origin_dec_x : -origin_dec_x,
        .y = (map_step.y == 1) ? 1 - origin_dec_y : -origin_dec_y,
    };

    struct FVec2 ray_length = {
        .x = init_step.x / cos(angle),
        .y = init_step.y / sin(angle),
    };

    // the famous cast
    while (true) {
        if (ray_length.x < ray_length.y) {
            r.ray_length = ray_length.x;
            r.hit_x = origin.x + r.ray_length * cos(angle);
            r.hit_y = origin.y + r.ray_length * sin(angle);
            r.wall_align = 1;

            map_pos.x += map_step.x;
            ray_length.x += ray_increment.x;
        } else {
            r.ray_length = ray_length.y;
            r.hit_x = origin.x + r.ray_length * cos(angle);
            r.hit_y = origin.y + r.ray_length * sin(angle);
            r.wall_align = 0;

            map_pos.y += map_step.y;
            ray_length.y += ray_increment.y;
        }

        if (GetTileInMap(map, map_pos.x, map_pos.y).type != TILE_FLOOR)
            break;
    }

    return r;
}

void render(struct state *gs) {
    // it is, probably, "bad practice" to throw the entire global state to a
    // single function. i do not really care

    SDL_RenderClear(gs->renderer);

    // render skybox
    /*SDL_FRect dstrect = {0, 0, 640, 180};*/
    /*SDL_RenderTexture(gs->renderer, gs->skybox, nullptr, &dstrect);*/

    // sweep view and get raycasts using DDA
    struct raycast raycasts[gs->logical_win_width];

    for (int i = 0; i < gs->logical_win_width; ++i) {
        // dir in radians
        double angle =
            (atan2(gs->camera.dir.y, gs->camera.dir.x) - gs->camera.fov / 2.) +
            (gs->camera.fov * i) / (double)gs->logical_win_width;

        // position from where we shoot the ray
        struct FVec2 origin =
            (struct FVec2){gs->camera.pos.x, gs->camera.pos.y};

        raycasts[i] = cast_ray(&gs->map, origin, angle);
    }

    // render "columns"
    for (int i = 0; i < gs->logical_win_width; ++i) {
        struct raycast *ray = &raycasts[i];
        int column_height =
            (gs->logical_win_height / ray->ray_length) > gs->logical_win_height
                ? gs->logical_win_height
                : gs->logical_win_height / ray->ray_length;

        int col_y =
            gs->logical_win_height / 2. - column_height / 2. + gs->camera.pos.z;

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
            gs->spritesheet, &c_original.r, &c_original.g, &c_original.b);

        /* shading */
        c_shaded.r = c_original.r - ray->ray_length * 0.05;
        c_shaded.g = c_original.g - ray->ray_length * 0.05;
        c_shaded.b = c_original.b - ray->ray_length * 0.05;
        SDL_SetTextureColorModFloat(
            gs->spritesheet, c_shaded.r - .1, c_shaded.g - .1, c_shaded.b - .1);

        SDL_RenderTexture(gs->renderer, gs->spritesheet, &wallsprite, &dstrect);

        // set proper colors
        SDL_SetTextureColorModFloat(
            gs->spritesheet, c_original.r, c_original.g, c_original.b);
    }

    SDL_RenderPresent(gs->renderer);
}

SDL_AppResult SDL_AppIterate(void *appstate) {
    struct state *gs = appstate;
    const bool *keyboard = SDL_GetKeyboardState(nullptr);

    /* timing */
    gs->start_time = SDL_GetTicks();

    /* input */
    auto move = (struct FVec2){0, 0};
    if (keyboard[SDL_SCANCODE_W]) {
        double dir_angle = atan2(gs->camera.dir.y, gs->camera.dir.x);
        move.x += cos(dir_angle) * 5;
        move.y += sin(dir_angle) * 5;
    }

    if (keyboard[SDL_SCANCODE_A]) {
        double dir_angle = atan2(gs->camera.dir.y, gs->camera.dir.x);
        move.x -= cos(dir_angle + 90 * PI / 180) * 5;
        move.y -= sin(dir_angle + 90 * PI / 180) * 5;
    }

    if (keyboard[SDL_SCANCODE_S]) {
        double dir_angle = atan2(gs->camera.dir.y, gs->camera.dir.x);
        move.x -= cos(dir_angle) * 5;
        move.y -= sin(dir_angle) * 5;
    }

    if (keyboard[SDL_SCANCODE_D]) {
        double dir_angle = atan2(gs->camera.dir.y, gs->camera.dir.x);
        move.x += cos(dir_angle + 90 * PI / 180) * 5;
        move.y += sin(dir_angle + 90 * PI / 180) * 5;
    }

    Camera_velUpdate(&gs->camera, move);
    Camera_physUpdate(&gs->camera, gs->delta_time);

    render(gs);

    /* timing, again */
    SDL_Delay(gs->start_time + 16.67 -
              SDL_GetTicks()); // Locks the frame rate at 60

    gs->frame_time = SDL_GetTicks() - gs->start_time;
    gs->delta_time = gs->frame_time / 1000.;
    if (gs->frame_time) {
        double fps = 1000. / gs->frame_time;
        SDL_Log("%f FPS", fps);
        SDL_Log("%f delta", gs->delta_time);
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    struct state *gs = appstate;
    double xmouse = 0;

    if (event->type == SDL_EVENT_QUIT)
        return SDL_APP_SUCCESS;

    if (event->type == SDL_EVENT_KEY_DOWN) {
        // camera
        if (event->key.scancode == SDL_SCANCODE_RIGHT) {
            gs->camera.dir.x =
                gs->camera.dir.x * cos(.1) - gs->camera.dir.y * sin(.1);
            gs->camera.dir.y =
                gs->camera.dir.x * sin(.1) + gs->camera.dir.y * cos(.1);
        }

        if (event->key.scancode == SDL_SCANCODE_LEFT) {
            gs->camera.dir.x =
                gs->camera.dir.x * cos(-.1) - gs->camera.dir.y * sin(-.1);
            gs->camera.dir.y =
                gs->camera.dir.x * sin(-.1) + gs->camera.dir.y * cos(-.1);
        }

        // movement
        if (event->key.scancode == SDL_SCANCODE_UP) {
            double dir_angle = atan2(gs->camera.dir.y, gs->camera.dir.x);
            gs->camera.pos.x += cos(dir_angle) * .1;
            gs->camera.pos.y += sin(dir_angle) * .1;
        }
    }

    if (event->type == SDL_EVENT_KEY_UP) {
    }

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
    struct state *gs = appstate;
    if (result != SDL_APP_FAILURE) {
        SDL_DestroyTexture(gs->spritesheet);
        SDL_DestroyRenderer(gs->renderer);
        SDL_DestroyWindow(gs->window);
        gs->spritesheet = nullptr;
        gs->renderer = nullptr;
        gs->window = nullptr;
    }
    DestroyMap(&gs->map);

    appstate = nullptr;
    SDL_Quit();
}
