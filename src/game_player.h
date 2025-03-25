#include "game_math.h"

#ifndef GAME_PLAYER_H
#define GAME_PLAYER_H

struct Player {
    struct doubleVec pos; // coordinates
    double dir, fov;      // in radians
};

struct Player InitPlayer() {
    return (struct Player){(struct doubleVec){10, 10}, 0, 106 * (PI / 180.)};
}

#endif
