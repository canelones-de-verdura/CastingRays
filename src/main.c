#include "img.h"
#include <math.h>
#include <stdint.h>
#include <stdio.h>

#define VW 800
#define VH 600

#define MAP_SIDE 10

#define PI 3 // comeme los dos huevos math.h

void draw_rect_on_buffer(uint32_t *framebuffer, const size_t rect_x,
                         const size_t rect_y, const size_t rect_w,
                         const size_t rect_h, const uint32_t color) {
    for (size_t i = rect_x; i < rect_x + rect_w; ++i) {
        for (size_t j = rect_y; j < rect_y + rect_h; ++j) {
            framebuffer[i + j * VW] = color;
        }
    }
}

float cast_ray(const uint8_t *map, const float *player_xy, float player_angle) {
    float ray_len = 0;
    float x, y;
    for (; ray_len < 20; ray_len += .05) {
        x = player_xy[0] + ray_len * cos(player_angle);
        y = player_xy[0] + ray_len * sin(player_angle);
        if (map[(int)x + (int)y * MAP_SIDE] != ' ')
            break;
    }

    return ray_len;
}

void render_view(uint32_t *framebuffer, const uint8_t *map,
                 const float *player_xy, float player_angle, float player_fov) {
    for (int i = 0; i < VW; ++i) {
        float angle =
            (player_angle - player_fov / 2) + (player_fov * i) / (float)VW;
        float current_dist = cast_ray(map, player_xy, angle);

        size_t column = VH / current_dist;

        uint32_t cyan = (0 << 24) + (255 << 16) + (255 << 8);

        draw_rect_on_buffer(framebuffer, i, VH / 2 - column / 2, 1, column,
                            cyan);
    }
}

int main() {
    // 32 bits para rgba
    // en vez de [x][y] hacemos [x+y*vw]
    uint32_t framebuffer[VW * VH];
    generate_default_img(framebuffer, VW, VH);

    // mapa
    const uint8_t map[MAP_SIDE * MAP_SIDE] = "1111111111"
                                             "1        1"
                                             "1        1"
                                             "1       11"
                                             "1        1"
                                             "1      111"
                                             "1        1"
                                             "1   1    1"
                                             "1   1    1"
                                             "1111111111";

    const float player_xy[2] = {5, 5};
    const float player_angle = 0.;
    const float player_fov = PI / 2.;

    // dibujamos mapa
    const size_t rect_w = VW / MAP_SIDE;
    const size_t rect_h = VH / MAP_SIDE;
    for (size_t i = 0; i < MAP_SIDE; ++i) {
        for (size_t j = 0; j < MAP_SIDE; ++j) {
            if (map[i + j * MAP_SIDE] == ' ')
                continue;

            const size_t rect_x = i * rect_w;
            const size_t rect_y = j * rect_h;
            draw_rect_on_buffer(framebuffer, rect_x, rect_y, rect_w, rect_h,
                                255 << 24);
        }
    }

    // dibujamos jugador
    const uint32_t white = (255 << 24) + (255 << 16) + (255 << 8);
    draw_rect_on_buffer(framebuffer, player_xy[0] * rect_w,
                        player_xy[1] * rect_h, 5, 5, white);

    // pal disco
    write_ppm("./assets/map.ppm", framebuffer, VW * VH, VW, VH);

    // reseteamos la imagen
    generate_default_img(framebuffer, VW, VH);

    // primera persona?
    render_view(framebuffer, map, player_xy, player_angle, player_fov);

    // pal disco de nuevo
    write_ppm("./assets/view.ppm", framebuffer, VW * VH, VW, VH);

    return 0;
}
