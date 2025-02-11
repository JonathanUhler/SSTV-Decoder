#ifndef _PNG_FILE_H_
#define _PNG_FILE_H_


#include "modes.h"
#include <stdint.h>


typedef struct pixel_s Pixel;


struct pixel_s {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
};


void png_file_save(const Pixel *pixels, size_t width, size_t height, const char *path);


Pixel *png_file_y1crcby2_to_rgb(const uint8_t *image_data, const SstvMode *mode);


Pixel png_file_ycbcr_pixel(uint8_t y, uint8_t cb, uint8_t cr);


#endif  // _PNG_FILE_H_
