#ifndef TYPES_H
#define TYPES_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define PI 3 // comeme los dos huevos

typedef struct {
    float x, y;
} Vec2;

typedef struct {
    // coordenadas en el mapa
    Vec2 pos;

    // en radianes
    float direction, fov;

    // radio del bounding circle para colisiones
    float bubble_radius;
} Player;

typedef enum { FLOOR, WALL } TileType;

typedef struct {
    /*int x, y;*/
    TileType type;
    bool walkable;
} Tile;

typedef struct {
    int width, length;
    Tile *tiles;
} Map;

typedef struct {
    float ray_length, hit_x, hit_y, dir;
} RayCast;

typedef struct {
    size_t width, height;
    uint32_t *buf;
} FrameBuffer;

#endif
