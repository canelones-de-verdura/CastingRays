#include <stdint.h>
#include <stdio.h>

uint32_t colors_in(const uint8_t r, const uint8_t g, const uint8_t b,
                   const uint8_t a) {
    return (r << 24) + (g << 16) + (b << 8) + a;
}

void generate_default_img(uint32_t *image, size_t width, size_t height) {
    uint8_t r, g, b, a = 255;
    for (size_t i = 0; i < width; ++i) {
        for (size_t j = 0; j < height; ++j) {
            r = 255 * i / (float)width;
            g = 255 * j / (float)height;
            b = 255 * (i + j) / (float)width * height;
            image[i + j * width] = colors_in(r, g, b, a);
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
    for (size_t _ = 0; _ < img_size; ++_) {
        colors_out(image[_], &r, &g, &b, &a);
        a = 255;
        fprintf(new_ppm, "%c%c%c", r, g, b);
    }

    fclose(new_ppm);
}
