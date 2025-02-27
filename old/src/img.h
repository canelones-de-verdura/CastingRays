#ifndef IMG_H
#define IMG_H

#include <stdint.h>
#include <stdio.h>

uint32_t colors_in(const uint8_t r, const uint8_t g, const uint8_t b,
                   const uint8_t a);

void generate_default_img(uint32_t *image, size_t width, size_t height);

void colors_out(const uint32_t color, uint8_t *r, uint8_t *g, uint8_t *b,
                uint8_t *a);

void write_ppm(const char *filename, const uint32_t *image, size_t img_size,
               size_t width, size_t height);

#endif
