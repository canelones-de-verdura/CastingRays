#ifndef RAYS_H
#define RAYS_H

#include <stdint.h>
#include <stdio.h>

#include "types.h"

void load_texture();

void free_texture();

void draw_rect_on_buffer(uint32_t *framebuffer, const size_t fbw,
                         const size_t fbh, const size_t rect_x,
                         const size_t rect_y, const size_t rect_w,
                         const size_t rect_h, const uint32_t color);

RayCast cast_ray(const Map *map, Vec2 position, float direction);

void render_view(uint32_t *framebuffer, const size_t fbw, const size_t fbh,
                 const Map *map, Vec2 position, float direction, float fov);

#endif
