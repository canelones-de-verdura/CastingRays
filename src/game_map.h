#include <stdlib.h>

#ifndef GAME_MAP_H
#define GAME_MAP_H

enum TileType { TILE_FLOOR, TILE_WALL };

enum WallTexture { TXT_CEMENT, TXT_WARN, TXT_SEWAGE, TXT_MOLDY };

struct Tile {
    enum TileType type;
    enum WallTexture skin;
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

            current_tile->type = TILE_WALL;
            current_tile->walkable = false;
            switch (map_data[i + j * map_w]) {
            case '1':
                current_tile->skin = TXT_CEMENT;
                break;
            case '2':
                current_tile->skin = TXT_WARN;
                break;
            case '3':
                current_tile->skin = TXT_SEWAGE;
                break;
            case '4':
                current_tile->skin = TXT_MOLDY;
                break;
            case ' ':
            default:
                current_tile->type = TILE_FLOOR;
                current_tile->walkable = true;
                break;
            }
        }
    }

    return true;
}

struct Tile GetTileInMap(struct Map *self, int x, int y) {
    // let's avoid reading garbage
    if ((x > self->width) || (x < 0) || (y > self->length) || (y < 0))
        return self->tiles[0];

    return self->tiles[x + y * self->width];
}

void DestroyMap(struct Map *self) { free(self->tiles); }

#endif
