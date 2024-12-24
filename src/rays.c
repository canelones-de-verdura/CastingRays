#include <math.h>
#include <stdint.h>
#include <stdio.h>

#include "types.h"

void draw_rect_on_buffer(uint32_t *framebuffer, const size_t fbw,
                         const size_t fbh, const size_t rect_x,
                         const size_t rect_y, const size_t rect_w,
                         const size_t rect_h, const uint32_t color) {
    for (size_t i = rect_x; i < rect_x + rect_w; ++i) {
        for (size_t j = rect_y; j < rect_y + rect_h; ++j) {
            framebuffer[i + j * fbw] = color;
        }
    }
}

RayCast cast_ray(const Map *map, Vec2 position, float direction) {
    RayCast ray = {
        .ray_length = 0,
        .hit_x = 0,
        .hit_y = 0
    };

    for (; ray.ray_length < 20; ray.ray_length += .05) {
        ray.hit_x = position.x + ray.ray_length * cos(direction);
        ray.hit_y = position.y + ray.ray_length * sin(direction);
        if (map->tiles[(int)ray.hit_x + (int)ray.hit_y * map->width].type != FLOOR)
            break;
    }

    return ray;
}

void render_view(uint32_t *framebuffer, const size_t fbw, const size_t fbh,
                 const Map *map, Vec2 position, float direction, float fov) {
    for (int i = 0; i < fbw; ++i) {
        float angle = (direction - fov / 2) + (fov * i) / (float)fbw;
        RayCast ray = cast_ray(map, position, angle);

        size_t column = (fbh / ray.ray_length) > fbh ? fbh : fbh / ray.ray_length;

        uint8_t g = (255 - (int)ray.ray_length * 10) < 0 ? 0 : 255 - (int)ray.ray_length * 10;
        uint8_t b = (255 - (int)ray.ray_length * 10) < 0 ? 0 : 255 - (int)ray.ray_length * 10;
        uint32_t cyan = (0 << 24) + (g << 16) + (b << 8);

        draw_rect_on_buffer(framebuffer, fbw, fbh, i, fbh / 2 - column / 2, 1,
                            column, cyan);
    }
}
