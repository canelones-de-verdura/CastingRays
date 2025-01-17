#include "data/texture.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

uint32_t colors_in(const uint8_t r, const uint8_t g, const uint8_t b,
                   const uint8_t a) {
    return (r << 24) + (g << 16) + (b << 8) + a;
}

void generate_default_img(uint32_t *image, size_t width, size_t height) {
    // "techo"
    for (size_t i = 0; i < width; ++i) {
        for (size_t j = 0; j < height / 2; ++j) {
            image[i + j * width] = colors_in(0, 0, 0, 0);
        }
    }

    // "piso"
    for (size_t i = 0; i < width; ++i) {
        for (size_t j = height / 2; j < height; ++j) {
            image[i + j * width] = colors_in(0, 150, 150, 0);
        }
    }

    // pa probar
    for (size_t j = 0; j < ROCK_WALL_HEIGHT; ++j) {
        for (size_t i = 0; i < ROCK_WALL_WIDTH; ++i) {
            image[i + j * width] = rockwall[i + j * ROCK_WALL_WIDTH];
        }
    }
}

void colors_out(const uint32_t color, uint8_t *r, uint8_t *g, uint8_t *b,
                uint8_t *a) {
    *r = (color >> 24) & 255;
    *g = (color >> 16) & 255;
    *b = (color >> 8) & 255;
    *a = (color >> 0) & 255;
}

void write_ppm(const char *filename, const uint32_t *image, size_t img_size,
               size_t width, size_t height) {
    FILE *new_ppm = fopen(filename, "w");
    fprintf(new_ppm, "P6\n%zu %zu\n255\n", width, height);
    uint8_t r, g, b, a;
    for (size_t j = 0; j < height; ++j) {
        for (size_t i = 0; i < width; ++i) {
            colors_out(image[i + j * width], &r, &g, &b, &a);
            /*a = 255;*/
            fprintf(new_ppm, "%c%c%c%c", a, r, g, b);
        }
    }

    fclose(new_ppm);
}
