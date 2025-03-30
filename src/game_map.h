#include <stdlib.h>

#ifndef GAME_MAP_H
#define GAME_MAP_H

enum TileType { FLOOR, WALL };

struct Tile {
    enum TileType type;
    bool walkable;
};

struct Map {
    int width, length;
    struct Tile *tiles;
};

bool LoadMap(struct Map *self, const char *map_data, int map_w, int map_l) {
    self->width = map_w;
    self->length = map_l;
    self->tiles = malloc(sizeof(struct Tile) * map_w * map_l);

    if (!self->tiles)
        return false;

    struct Tile *current_tile;
    for (int i = 0; i < map_w; ++i) {
        for (int j = 0; j < map_l; ++j) {
            current_tile = &self->tiles[i + j * map_w];

            if (map_data[i + j * map_w] == ' ') {
                current_tile->type = FLOOR;
                current_tile->walkable = true;
            } else {
                current_tile->type = WALL;
                current_tile->walkable = false;
            }
        }
    }

    return true;
}

struct Tile GetTileInMap(struct Map *self, int x, int y) {
    return self->tiles[x + y * self->width];
}

void DestroyMap(struct Map *self) { free(self->tiles); }

#endif
