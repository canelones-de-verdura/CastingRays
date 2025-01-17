#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "data/screen.h"
#include "data/texture.h"
#include "types.h"

static RayCast rays[VIEW_W];

void draw_rect_on_buffer(FrameBuffer *fb, const size_t rect_x,
                         const size_t rect_y, const size_t rect_w,
                         const size_t rect_h, const uint32_t color) {
    for (size_t i = rect_x; i < rect_x + rect_w; ++i) {
        for (size_t j = rect_y; j < rect_y + rect_h; ++j) {
            fb->buf[i + j * fb->width] = color;
        }
    }
}

RayCast cast_ray(const Map *map, Vec2 position, float direction) {
    RayCast ray = {0, 0, 0};

    for (; ray.ray_length < 20; ray.ray_length += .05) {
        ray.hit_x = position.x + ray.ray_length * cos(direction);
        ray.hit_y = position.y + ray.ray_length * sin(direction);
        if (map->tiles[(int)ray.hit_x + (int)ray.hit_y * map->width].type !=
            FLOOR)
            break;
    }

    return ray;
}

void sweep(const Map *map, const Player *player) {
    for (int i = 0; i < VIEW_W; ++i) {
        float angle = (player->direction - player->fov / 2) +
                      (player->fov * i) / (float)VIEW_W;
        rays[i] = cast_ray(map, player->pos, angle);
    }
}

void render_column(FrameBuffer *fb, RayCast *ray, const size_t col_x) {
    size_t column_height = (fb->height / ray->ray_length) > fb->height
                               ? fb->height
                               : fb->height / ray->ray_length;

    size_t col_y = fb->height / 2 - column_height / 2;

    uint32_t column[column_height];
    int mul = column_height / ROCK_WALL_HEIGHT;
    /*for (int i = 0; i < ROCK_WALL_HEIGHT*ROCK_WALL_WIDTH; ++i) {*/
    /*    column[i] = rockwall[i];*/
    /*}*/

    for (int i = col_y; i < col_y + column_height; ++i) {
        fb->buf[col_x + i*fb->width] = rockwall[((i - col_y) % ROCK_WALL_WIDTH) * ROCK_WALL_WIDTH];
    }

    /*
    // jodemos con los colores para que las paredes "más lejos" se vean más
    // oscuras
    uint8_t g = (255 - (int)ray->ray_length * 10) < 0
                    ? 0
                    : 255 - (int)ray->ray_length * 10;
    uint8_t b = (255 - (int)ray->ray_length * 10) < 0
                    ? 0
                    : 255 - (int)ray->ray_length * 10;
    uint32_t cyan = (0 << 24) + (g << 16) + (b << 8);

    draw_rect_on_buffer(fb, col_x, col_y, 1, column_height, cyan);
    */
}

void render_view(FrameBuffer *fb, const Map *map, const Player *player) {
    sweep(map, player);
    for (int i = 0; i < fb->width; ++i) {
        render_column(fb, &rays[i], i);
    }
}
