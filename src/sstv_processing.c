#include "freq_processing.h"
#include "modes.h"
#include "sstv_processing.h"
#include "wav_file.h"
#include <assert.h>
#include <limits.h>
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


uint8_t decode_vis_code(const WavSamples *wav_samples, const SstvMode *mode, size_t vis_start) {
    // Extract information to be used throughout this function.
    uint32_t sample_rate = wav_samples->sample_rate;
    double *samples = wav_samples->samples;

    size_t bit_size = round(mode->bit_time_sec * sample_rate);
    uint8_t vis_p_code = 0;  // The VIS code with parity bit that we will build bit-by-bit

    // For the number of bits in the VIS+P code, we loop through and figure out what sample
    // that bit contains. We then determine the sample area for that bit and find the peak
    // frequency. The frequency determines whether the bit is logical HI or LO, which we add
    // to `vis_p_code` (LSB is read first; lowest sample number).
    for (size_t i = 0; i < CHAR_BIT; i++) {
        size_t bit_sample = vis_start + i * bit_size;
        double *bit_area = &samples[bit_sample];
        double peak = peak_frequency(bit_area, bit_size, sample_rate);

        uint8_t bit_value = peak <= mode->sync_hz;
        bit_value <<= i;
        vis_p_code |= bit_value;
    }

    // Check that the parity is correct for sanity.
    bool has_correct_parity = __builtin_parity(vis_p_code) == 0;
    assert(has_correct_parity && "Incorrect parity!");

    // Remove the parity bit (MSB) after it has been checked and return the VIS code.
    uint8_t vis_code = vis_p_code & 0x7F;
    return vis_code;
}


size_t find_sync_end(const WavSamples *wav_samples, const SstvMode *mode, size_t align_start) {
    // Extract information to be used throughout.
    size_t num_samples = wav_samples->num_samples;
    uint32_t sample_rate = wav_samples->sample_rate;
    double *samples = wav_samples->samples;

    // Define a size for the sync window, with some margin-of-error factor from the sync time.
    // Then, determine when the sync alignment stop should be.
    size_t sync_window = round(mode->sync_time_sec * 1.4 * sample_rate);
    size_t align_stop = num_samples - sync_window;
    if (align_stop <= align_start) {
        return SSTV_PROCESSING_NOT_FOUND;
    }

    // Search for end of the sync signal.
    size_t current_sample;
    for (current_sample = align_start; current_sample < align_stop; current_sample++) {
        double *sync_window_area = &samples[current_sample];
        if (!is_frequency(sync_window_area, sync_window, sample_rate, mode->sync_hz)) {
            break;
        }
    }

    // Return the first sample that is not in the sync signal.
    return current_sample + (sync_window / 2);
}


/*
uint8_t decode_image_data(const WavSamples *wav_samples, const SstvMode *mode, size_t image_start) {
    return 0;
}
*/
