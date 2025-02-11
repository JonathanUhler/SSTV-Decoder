#include "logger.h"
#include "modes.h"
#include "png_file.h"
#include <png.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>


void png_file_save(const Pixel *pixels, size_t width, size_t height, const char *path) {
    FILE *file = fopen(path, "wb");
    if (file == NULL) {
        log_fatal("cannot open output file '%s'", path);
    }

    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (png == NULL) {
        fclose(file);
        log_fatal("could not allocate png file");
    }

    png_infop info = png_create_info_struct(png);
    if (info == NULL) {
        png_destroy_write_struct(&png, NULL);
        fclose(file);
        log_fatal("could not allocate png info");
    }

    if (setjmp(png_jmpbuf(png))) {
        png_destroy_write_struct(&png, &info);
        fclose(file);
        log_fatal("error during png file creation");
    }

    png_init_io(png, file);
    png_set_IHDR(png, info,
                 width, height, 8,
                 PNG_COLOR_TYPE_RGB,
                 PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT,
                 PNG_FILTER_TYPE_DEFAULT);
    png_write_info(png, info);

    png_bytep row = (png_bytep) malloc(3 * width * sizeof(png_byte));
    for (size_t y = 0; y < height; y++) {
        for (size_t x = 0; x < width; x++) {
            row[x * 3 + 0] = pixels[y * width + x].red;
            row[x * 3 + 1] = pixels[y * width + x].green;
            row[x * 3 + 2] = pixels[y * width + x].blue;
        }
        png_write_row(png, row);
    }

    png_write_end(png, NULL);

    free(row);
    png_destroy_write_struct(&png, &info);
    fclose(file);
}


Pixel *png_file_y1crcby2_to_rgb(const uint8_t *image_data, const SstvMode *mode) {
    assert(mode->color_space == Y1_CR_CB_Y2 && "Expected Y1CRCBY2 color space");

    size_t width = mode->width;
    size_t data_height = mode->height;
    size_t image_height = 2 * data_height;
    uint16_t num_channels = mode->num_channels;

    Pixel *pixels = (Pixel *) malloc(width * image_height * sizeof(Pixel));

    for (size_t r = 0; r < data_height; r++) {
        size_t line_offset = r * num_channels * width;
        const uint8_t *y1_data = &image_data[line_offset + 0 * width];
        const uint8_t *cr_data = &image_data[line_offset + 1 * width];
        const uint8_t *cb_data = &image_data[line_offset + 2 * width];
        const uint8_t *y2_data = &image_data[line_offset + 3 * width];

        for (size_t c = 0; c < width; c++) {
            uint8_t y1_value = y1_data[c];
            uint8_t cr_value = cr_data[c];
            uint8_t cb_value = cb_data[c];
            uint8_t y2_value = y2_data[c];

            Pixel pixel1 = png_file_ycbcr_pixel(y1_value, cb_value, cr_value);
            Pixel pixel2 = png_file_ycbcr_pixel(y2_value, cb_value, cr_value);
            pixels[    (2 * r) * width + c] = pixel1;
            pixels[(2 * r + 1) * width + c] = pixel2;
        }
    }

    return pixels;
}


Pixel png_file_ycbcr_pixel(uint8_t y, uint8_t cb, uint8_t cr) {
    Pixel pixel = {
        .red   = fmin(fmax(round(y + 1.40200 * (cr - 128.0)),                          0.0), 255.0),
        .green = fmin(fmax(round(y - 0.34414 * (cb - 128.0) - 0.71414 * (cr - 128.0)), 0.0), 255.0),
        .blue  = fmin(fmax(round(y + 1.77200 * (cb - 128.0)),                          0.0), 255.0),
    };
    return pixel;
}
