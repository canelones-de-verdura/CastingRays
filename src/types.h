#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>

#define PI 3 // comeme los dos huevos

typedef struct vec2_st {
    float x;
    float y;
} Vec2;

typedef struct player_st {
    Vec2 pos;
    float direction;
    float fov;
} Player;

typedef struct map_st {
    int width;
    int length;
    uint8_t *data;
} Map;

#endif
