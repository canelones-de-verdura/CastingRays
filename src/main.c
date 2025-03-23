#define SDL_MAIN_USE_CALLBACKS

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_surface.h>
#include <SDL3/SDL_video.h>

#include "../include/SDL3_image/SDL_image.h"

#include <stdint.h>
#include <stdlib.h>

struct state {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *sprites;
    SDL_Texture *skybox;
};

SDL_AppResult LogAndDie(const char msg[]) {
    SDL_Log("[ERROR] %s - %s", msg, SDL_GetError());
    return SDL_APP_FAILURE;
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv) {
    constexpr uint32_t window_width = 1280;
    constexpr uint32_t window_height = 720;
    constexpr uint32_t logical_width = 640;
    constexpr uint32_t logical_height = 360;
    constexpr uint32_t init_flags = SDL_INIT_VIDEO;

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
    gs->sprites = IMG_LoadTexture(
        gs->renderer, "./assets/32rogues-0.5.0/32rogues/tiles.png");
    if (!gs->sprites)
        return LogAndDie("Failure to load texture from image");

    if (!SDL_SetTextureScaleMode(gs->sprites, SDL_SCALEMODE_NEAREST))
        return LogAndDie("Failure to change scaling mode on texture");

    gs->skybox = IMG_LoadTexture(
        gs->renderer, "./assets/skybox.png");
    if (!gs->skybox)
        return LogAndDie("Failure to load texture from image");

    if (!SDL_SetTextureScaleMode(gs->skybox, SDL_SCALEMODE_NEAREST))
        return LogAndDie("Failure to change scaling mode on texture");

    /* assign everything to the global state */
    *appstate = gs;

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
    struct state *gs = appstate;
    SDL_RenderClear(gs->renderer);

    SDL_FRect dstrect = {0, 0, 640, 154};
    SDL_RenderTexture(gs->renderer, gs->skybox, nullptr, &dstrect);

    SDL_RenderPresent(gs->renderer);

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    if (event->type == SDL_EVENT_QUIT)
        return SDL_APP_SUCCESS;
    if (event->type == SDL_EVENT_KEY_DOWN)
        SDL_Log(":D");

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

    free(gs);
    gs = nullptr;
    appstate = nullptr;

    SDL_Quit();
}
