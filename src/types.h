#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>

#define PI 3 // comeme los dos huevos

typedef struct vec2_st {
    float x, y;
} Vec2;

typedef struct player_st {
    // coordenadas en el mapa
    Vec2 pos;

    // en radianes
    float direction, fov;

    // radio del bounding circle para colisiones
    float bubble_radius;
} Player;

typedef enum { FLOOR, WALL } TileType;

typedef struct tile_st {
    /*int x, y;*/
    TileType type;
} Tile;

typedef struct map_st {
    int width, length;
    Tile *tiles;
} Map;

#endif
