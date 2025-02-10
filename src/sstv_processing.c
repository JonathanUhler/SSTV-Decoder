#include "freq_processing.h"
#include "modes.h"
#include "sstv_processing.h"
#include "wav_file.h"
#include <math.h>
#include <stdint.h>
#include <stdlib.h>


size_t find_header_sample(const WavSamples *wav_samples, const SstvMode *mode) {
    // Extract information from `wav_samples` for easier/shorter access names.
    size_t num_samples = wav_samples->num_samples;
    uint32_t sample_rate = wav_samples->sample_rate;
    double *samples = wav_samples->samples;

    // We know that the header consists of four segments: a leader block, a break block, a
    // second leader, and a calibration bit before the VIS code. Graphically, this looks like the
    // following (see page 13 of http://www.sstv-handbook.com/download/sstv_03.pdf for a better
    // diagram):
    //
    //           300ms      300ms
    // 1900 Hz ---------++---------+
    //                  ||         |
    //           Lead1  ||  Lead2  |
    //                  ||         | VIS bit
    // 1200 Hz . . . . .++ . . . . +--.............
    //                 10ms        30ms
    //
    // To do this search, we define `_sample` variables for each block. These mark the relative
    // start (in samples) compared to `current_sample` (in the for-loop below) where we expect
    // the block to start. The search width is always `window_size`.

    double header_time_sec = 2 * mode->leader_time_sec + mode->break_time_sec + mode->bit_time_sec;
    size_t header_size = round(header_time_sec * sample_rate);
    size_t window_size = round(0.01 * sample_rate);  // 10ms search window for DFT

    size_t leader_1_sample = 0;
    size_t break_sample = round(mode->leader_time_sec * sample_rate);
    size_t leader_2_sample = round((mode->break_time_sec + mode->leader_time_sec) * sample_rate);
    double vis_start_time_sec = 2 * mode->leader_time_sec + mode->break_time_sec;
    size_t vis_start_sample = round(vis_start_time_sec * sample_rate);

    // With everything defined for the four blocks, we start the search.

    size_t jump_size = round(0.002 * sample_rate);  // Shift sliding window by 2ms every iteration

    // For each iteration through this loop, we get the list of samples starting at each block,
    // then check if the dominant frequency in the block is what we expect for the header. If
    // it is, then we return the sample we found plus the size of the header to get the sample
    // immediately after the header.
    for (size_t current_sample = 0;
         current_sample < num_samples - header_size;
         current_sample += jump_size)
    {
        double *search_area = &samples[current_sample];

        double *leader_1_area  = &search_area[leader_1_sample];
        double *break_area     = &search_area[break_sample];
        double *leader_2_area  = &search_area[leader_2_sample];
        double *vis_start_area = &search_area[vis_start_sample];

        bool leader_1_found  = is_frequency(leader_1_area,
                                            window_size,
                                            sample_rate,
                                            mode->leader_hz);
        bool break_found     = is_frequency(break_area,
                                            window_size,
                                            sample_rate,
                                            mode->break_hz);
        bool leader_2_found  = is_frequency(leader_2_area,
                                            window_size,
                                            sample_rate,
                                            mode->leader_hz);
        bool vis_start_found = is_frequency(vis_start_area,
                                            window_size,
                                            sample_rate,
                                            mode->sync_hz);

        if (leader_1_found && break_found && leader_2_found && vis_start_found) {
            return current_sample + header_size;
        }
    }

    // If nothing was found, we return a sentinel value.
    return SSTV_PROCESSING_NOT_FOUND;
}


/*
uint8_t decode_vis_code(const WavSamples *wav_samples, const SstvMode *mode, size_t vis_start) {
    return 0;
}


size_t find_sync_end(const WavSamples *wav_samples, const SstvMode *mode, size_t current_sample) {
    return 0;
}


uint8_t decode_image_data(const WavSamples *wav_samples, const SstvMode *mode, size_t image_start) {
    return 0;
}
*/
