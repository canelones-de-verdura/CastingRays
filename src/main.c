#include <SDL3/SDL_error.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_video.h>

#include <math.h>

#define VW 800
#define VH 600
#define MAPSIDE 10

typedef struct double2 {
    double x, y;
} double2;

double2 rotateVector(double2 v, double a) {
    return (double2) {
        .x = (v.x * cos(a)) + (v.y * (-sin(a))),
        .y = (v.x * sin(a)) + (v.y * cos(a)),
    };
}

typedef struct Camera {
    double2 pos, dir;
} Camera;

typedef struct Map {
    int width, height, tiles[MAPSIDE*MAPSIDE];
} Map;

Camera initCamera() {
    return (Camera) {
        .pos = {0,0},
        .dir = {0,1},
    };
}

Map initMap() {
    return (Map) {
        .height = MAPSIDE,
        .width = MAPSIDE,
        .tiles = {
            1,1,1,1,1,1,1,1,1,1,
            1,0,0,0,0,0,0,0,0,1,
            1,0,0,0,0,0,0,0,0,1,
            1,0,0,0,0,0,0,0,0,1,
            1,0,0,0,0,0,0,0,0,1,
            1,0,0,0,0,0,0,0,0,1,
            1,0,0,0,0,0,0,0,0,1,
            1,0,0,0,0,0,0,0,0,1,
            1,0,0,0,0,0,0,0,0,1,
            1,1,1,1,1,1,1,1,1,1,
        }
    };
}

double castRay(double2 pos, double2 dir, Map *m) {
    double a = tan((dir.y - pos.y) / (dir.x - pos.x));
    for (double ray_len = 0; ray_len < 20; ray_len+=0.05) {
        double2 hit = {
            .x = pos.x + ray_len * cos(a),
            .y = pos.y + ray_len * sin(a),
        };

        int tile = m->tiles[(int)hit.x + (int)hit.y * m->width];
        if (tile != 0)
            return ray_len;
    }

    return 0;
}

void sweepRays(Camera c, Map m) {
    // get the camera plane
    
    for (int i = 0; i < VH; ++i) {

    }
}

int main(int argc, char *argv[]) {
    // init
    SDL_SetAppMetadata("CastingRays", "0.0.0", "eze.eze.eze");

    int init_flags = SDL_INIT_VIDEO;
    bool init = SDL_Init(init_flags);
    if (!init) {
        SDL_Log("ERROR: Couldn't init.\nSDL Error: %s\n", SDL_GetError());
        return 1;
    }

    // window
    SDL_Window *window;
    SDL_Renderer *renderer;
    int win_flags = SDL_WINDOW_OPENGL; // ?
    bool check = SDL_CreateWindowAndRenderer("CastingRays", VW, VH, win_flags,
                                             &window, &renderer);
    if (!check) {
        SDL_Log("ERROR: Couldn't create window or renderer.\nSDL Error: %s\n",
                SDL_GetError());
        return 1;
    }

    // main loop
    bool running = true;
    while (running) {
        // events
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_EVENT_QUIT:
                running = false;
                break;
            }
        }

        // logic
    }

    // cleanup
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
