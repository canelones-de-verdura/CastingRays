#include "game_math.h"

#ifndef GAME_PLAYER_H
#define GAME_PLAYER_H

struct Camera {
    struct FVec3 pos; // coordinates
    struct FVec2 dir;
    struct FVec2 vel; // velocity

    double fov; // in radians
    bool sway;
    double sway_amount;
};

struct Camera Camera_init() {
    return (struct Camera){
        .pos = (struct FVec3){10, 10, 0},
        .dir = {cos(270 * PI / 180.), sin(270 * PI / 180.)},
        .vel = {0, 0},
        .fov = 106 * (PI / 180.),
    };
}

void Camera_velUpdate(struct Camera *self, struct FVec2 vel) {
    self->vel = vel;
}

void Camera_physUpdate(struct Camera *self, double dt) {
    // ...
    self->pos.x += self->vel.x * dt;
    self->pos.y += self->vel.y * dt;
}

#endif
