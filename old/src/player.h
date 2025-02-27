#ifndef PLAYER_H
#define PLAYER_H

#include "types.h"

// Inicialización,
// movimiento,
// chequeo de colisión
//

enum player_dirs {
    UP,
    DOWN,
    LEFT,
    RIGHT,
    UPLEFT,
    UPRIGHT,
    DOWNLEFT,
    DOWNRIGHT,
};

void PlayerInit(Player *player);

void PlayerMove(Player *player, enum player_dirs mov_dir);

#endif
