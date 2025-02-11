#include "logger.h"
#include "modes.h"
#include "png_file.h"
#include "sstv_processing.h"
#include "wav_file.h"
#include <fftw3.h>
#include <limits.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


void usage(const char *error) {
    if (error != NULL) {
        printf("error: %s\n", error);
    }

    printf("usage: sstv [-o path] [-v] path\n");
    printf("\n");
    printf("options:\n");
    printf("  -h       print this message and exit\n");
    printf("  -o path  specify the output path for the image file (default .)\n");
    printf("  -v       print verbose debug messages about program execution\n");
    printf("\n");
    printf("arguments:\n");
    printf("  path     the path to the wave audio file to decode\n");
    exit(error != NULL);
}


void sstv_decode_and_save(const char *input_path, const char *output_path) {
    WavFile *wav_file = wav_file_open(input_path);
    if (wav_file == NULL) {
        log_fatal("cannot open wave audio file '%s'", input_path);
    }
    if (logger_verbose) {
        log_debug("successfully opened wave audio file, header follow");
        wav_file_print_header(wav_file);
    }

    WavSamples *wav_samples = wav_file_get_mono_samples(wav_file);
    if (wav_samples == NULL) {
        log_fatal("cannot extract mono samples from wave audio file '%s'", input_path);
    }

    size_t vis_start = find_vis_start(wav_samples);
    uint8_t vis_code = decode_vis_code(wav_samples, vis_start);
    const SstvMode *sstv_mode = get_sstv_mode(vis_code);
    if (sstv_mode == NULL) {
        log_fatal("sstv mode with VIS code %d is not supported", vis_code);
    }
    log_debug("found VIS at sample %lu, mode is '%s' (%u)", vis_start, sstv_mode->name, vis_code);

    size_t sample_rate = wav_samples->sample_rate;
    size_t image_start = vis_start + round(SSTV_BIT_TIME_SEC * (CHAR_BIT + 1) * sample_rate);
    uint8_t *image_data = decode_image_data(wav_samples, sstv_mode, image_start);

    // FIXME: This always assumes a PD mode
    Pixel *pixels = png_file_y1crcby2_to_rgb(image_data, sstv_mode);
    png_file_save(pixels, sstv_mode->width, 2 * sstv_mode->height, output_path);

    free(pixels);
    free(image_data);
    fftw_cleanup();
    wav_file_free_samples(wav_samples);
    wav_file_close(wav_file);
}


int main(int argc, char **argv) {
    char *output_path = "./result.png";
    char *input_path = NULL;

    int flag;
    while ((flag = getopt(argc, argv, "ho:v")) != -1) {
        switch (flag) {
        case 'h':
            usage(NULL);
            break;
        case 'o':
            output_path = optarg;
            break;
        case 'v':
            logger_set_verbosity(true);
            break;
        default:
            usage("unknown option flag");
            break;
        }
    }

    if (optind >= argc || argv[optind] == NULL) {
        usage("missing required 'path' argument");
    }
    input_path = argv[optind];

    sstv_decode_and_save(input_path, output_path);

    return 0;
}
