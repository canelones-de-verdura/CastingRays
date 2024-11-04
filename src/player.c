#include <math.h>

#include "types.h"
#include "player.h"

void PlayerInit(Player *player) {
    player->pos.x = 10;
    player->pos.y = 10;
    player->direction = 0.;
    player->fov = PI / 2.;
}

void PlayerProjectDir(Player *player, float angle) {
    float x = player->pos.x + cos(player->direction) * .1;
    float y = player->pos.y + sin(player->direction) * .1;
}

void PlayerMove(Player *player, enum player_dirs mov_dir) {
    switch (mov_dir) {
    case UP:
        player->pos.x += cos(player->direction) * .1;
        player->pos.y += sin(player->direction) * .1;
        break;
    case DOWN:
        player->pos.x -= cos(player->direction) * .1;
        player->pos.y -= sin(player->direction) * .1;
        break;
    case LEFT:
        player->pos.x -= cos(player->direction + PI/2.) * .1;
        player->pos.y -= sin(player->direction + PI/2.) * .1;
        break;
    case RIGHT:
        player->pos.x += cos(player->direction + PI/2.) * .1;
        player->pos.y += sin(player->direction + PI/2.) * .1;
        break;
    default:
        break;
    }
}
