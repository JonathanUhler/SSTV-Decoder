#ifndef _PNG_FILE_H_
#define _PNG_FILE_H_


#include "modes.h"
#include <stdint.h>


typedef struct pixel_s Pixel;


/**
 * A structure representing a single RGB pixel in an image.
 *
 * @var red    The value of the red channel.
 * @var green  The value of the greeen channel.
 * @var blue   The value of the blue channel.
 */
struct pixel_s {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
};


/**
 * Saves a 2-dimensional array of pixels to a PNG file.
 *
 * @param pixels  The pixels of the image.
 * @param width   The number of columns in the image.
 * @param height  The number of rows in the image.
 * @param path    The path to the image file to save as.
 */
void png_file_save(const Pixel *pixels, size_t width, size_t height, const char *path);


/**
 * Converts raw SSTV image data that is in the Y1CRCBY2 color space to an array of RGB pixels.
 *
 * @param image_data  The raw image data decoded from a wave file in the Y1CRCBY2 color space.
 * @param mode        The SSTV mode that the image data corresponds to.
 *
 * @return A 2-dimensional array of RGB pixels representing the image data.
 */
Pixel *png_file_y1crcby2_to_rgb(const uint8_t *image_data, const SstvMode *mode);


/**
 * Converts a YCbCr color value to a single RGB pixel.
 *
 * @param y   The luminance channel.
 * @param cb  The blue chrominance channel.
 * @param cr  The red chrominance channel.
 *
 * @return An RGB pixel that represents the same color as the provided YCbCr data.
 */
Pixel png_file_ycbcr_pixel(uint8_t y, uint8_t cb, uint8_t cr);


#endif  // _PNG_FILE_H_
