#include "logger.h"
#include "modes.h"
#include "sstv_processing.h"
#include "wav_file.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


void usage(const char *error) {
    if (error != NULL) {
        log_error(error);
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
        log_fatal("cannot open wave audio file");
    }
    if (logger_verbose) {
        log_debug("successfully opened wave audio file, header follow");
        wav_file_print_header(wav_file);
    }

    WavSamples *wav_samples = wav_file_get_mono_samples(wav_file);
    if (wav_samples == NULL) {
        log_fatal("cannot extract mono samples from wave audio file");
    }

    size_t vis_start = find_vis_start(wav_samples);
    uint8_t vis_code = decode_vis_code(wav_samples, vis_start);
    const SstvMode *sstv_mode = get_sstv_mode(vis_code);
    if (sstv_mode == NULL) {
        log_fatal("sstv mode is not supported");
    }
    printf("%lu, %u\n", vis_start, vis_code);

    printf("%s\n", output_path);

    wav_file_free_samples(wav_samples);
    wav_file_close(wav_file);
}


int main(int argc, char **argv) {
    char *output_path = NULL;
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


    /*
    // Demo: opening a wav file
    WavFile *wav_file = wav_file_open("/Users/jonathan/Downloads/PD120 SSTV Test Recording.wav");
    wav_file_print_header(wav_file);

    // Demo: printing some normalized  samples
    WavSamples *wav_samples = wav_file_get_mono_samples(wav_file);
    size_t start_sample = 1269436;
    size_t num_samples_to_print = 10;
    for (size_t i = start_sample; i < start_sample + num_samples_to_print; i++) {
        printf("Normalized sample %4lu: %f\n", i, wav_samples->samples[i]);
    }

    // Demo: some random data for peak frequency, expecting 3802.082584... Hz
    size_t len = 10;
    double *data = (double *) malloc(sizeof(double) * len);
    data[0] = 0.27252197;
    data[1] = 0.44992065;
    data[2] = 0.60906982;
    data[3] = 0.74325562;
    data[4] = 0.84680176;
    data[5] = 0.91534424;
    data[6] = 0.94613647;
    data[7] = 0.93783569;
    data[8] = 0.89089966;
    data[9] = 0.80755615;
    double freq = peak_frequency(data, len, wav_file->header->sample_rate);
    printf("peak freq = %f Hz\n", freq);
    free(data);

    // Demo: get an sstv mode
    const SstvMode *pd120_mode = get_sstv_mode(95);
    printf("addr of pd120 mode (shouldn't be null) = %lu\n", (size_t) pd120_mode);
    printf("addr of '0' mode (should be null) = %lu\n", (size_t) get_sstv_mode(0));

    const SstvMode *pd120_mode = get_sstv_mode(95);
    size_t header_sample = find_header_sample(wav_samples, pd120_mode);
    printf("Calibration header starts at sample %lu\n", header_sample);

    uint8_t vis_code = decode_vis_code(wav_samples, pd120_mode, header_sample);
    printf("VIS code is %u in decimal\n", vis_code);

    size_t first_sync_end = find_sync_end(wav_samples, pd120_mode, header_sample + round(pd120_mode->bit_time_sec * 9 * wav_samples->sample_rate));
    printf("Start of audio is at %lu\n", first_sync_end);

    // Demo: cleaning up
    wav_file_free_samples(wav_samples);
    wav_file_close(wav_file);

    return 0;
    */
}
