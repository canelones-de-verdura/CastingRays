#include "game_map.h"
#include "game_math.h"
#include <math.h>
#include <stddef.h>

#ifndef GAME_PLAYER_H
#define GAME_PLAYER_H

struct Camera {
    struct FVec2 pos; // coordinates
    struct FVec2 dir;

    struct FVec2 vel; // velocity

    double fov; // in radians

    double sway_speed;
    double sway_time;
    double vertical_offset; // zero is the center of the screen
    double offset_max;

    double tilt_angle;
    double tilt_max;
};

struct Camera Camera_init() {
    return (struct Camera){
        .pos = (struct FVec2){10, 10},
        .dir = {cos(270 * PI / 180.), sin(270 * PI / 180.)},
        .vel = {0, 0},
        // .fov = 106 * (PI / 180.),
        .fov = 75 * (PI / 180.),
        // .fov = 90 * (PI / 180.),

        .sway_speed = 15,
        .offset_max = 5,
        .tilt_max = 50,
    };
}

void Camera_velUpdate(struct Camera *self, struct FVec2 vel) {
    self->vel = vel;
}

void Camera_physUpdate(struct Camera *self, double dt) {
    self->pos.x += self->vel.x * dt;
    self->pos.y += self->vel.y * dt;

    bool moving = !vec_eq(self->vel, ((struct FVec2){0, 0}));

    if (moving) {
        self->sway_time += self->sway_speed * dt;
        self->vertical_offset = self->offset_max * sin(self->sway_time);
    } else {
        if (self->vertical_offset > 0)
            self->vertical_offset -= self->sway_speed * dt * 5;
        if (self->vertical_offset < 0)
            self->vertical_offset += self->sway_speed * dt * 5;

        // ACORDATE NUNCA COMPARAR FLOATS
        if (fabs(self->vertical_offset) < .01 ||
            fabs(self->vertical_offset) > .01) {
            self->vertical_offset = 0;
            self->sway_time = 0;
        }
    }
}

struct raycast {
    double ray_length, hit_x, hit_y;
    int wall_align; // 0 or 1
};

struct raycast Camera_raycast(struct FVec2 origin, double angle,
                              struct Map *map) {

    struct raycast r;

    // dir as a vector
    struct FVec2 dir = {
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
        .x = (dir.x > 0) ? 1 : -1,
        .y = (dir.y > 0) ? 1 : -1,
    };

    // because every cell is the same size, we can calculate the length a
    // segment of the ray will have between two "borders" of the grid (on
    // the same axis)
    struct FVec2 ray_increment = {
        .x = sqrt(1 + (dir.y / dir.x) * (dir.y / dir.x)),
        .y = sqrt(1 + (dir.x / dir.y) * (dir.x / dir.y)),
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

// it would be nice to move rendering the famebuffer here and have main
// handle the SDL specific stuff
// void Camera_render(...) {
// }

#endif
