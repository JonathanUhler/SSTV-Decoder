#include "freq_processing.h"
#include "logger.h"
#include "modes.h"
#include "sstv_processing.h"
#include "wav_file.h"
#include <assert.h>
#include <limits.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>


size_t find_vis_start(const WavSamples *wav_samples) {
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

    double header_time_sec = 2 * SSTV_LEADER_TIME_SEC + SSTV_BREAK_TIME_SEC + SSTV_BIT_TIME_SEC;
    size_t header_size = round(header_time_sec * sample_rate);
    size_t window_size = round(0.01 * sample_rate);  // 10ms search window for DFT

    size_t leader_1_sample = 0;
    size_t break_sample = round(SSTV_LEADER_TIME_SEC * sample_rate);
    size_t leader_2_sample = round((SSTV_BREAK_TIME_SEC + SSTV_LEADER_TIME_SEC) * sample_rate);
    double vis_start_time_sec = 2 * SSTV_LEADER_TIME_SEC + SSTV_BREAK_TIME_SEC;
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
        if (current_sample % sample_rate == 0) {
            double current_time = (double) current_sample / (double) sample_rate;
            log_info("searching for SSTV header at time %5.1fs", current_time);
        }

        double *search_area = &samples[current_sample];

        double *leader_1_area  = &search_area[leader_1_sample];
        double *break_area     = &search_area[break_sample];
        double *leader_2_area  = &search_area[leader_2_sample];
        double *vis_start_area = &search_area[vis_start_sample];

        bool leader_1_found  = is_frequency(leader_1_area,
                                            window_size,
                                            sample_rate,
                                            SSTV_LEADER_HZ);
        bool break_found     = is_frequency(break_area,
                                            window_size,
                                            sample_rate,
                                            SSTV_BREAK_HZ);
        bool leader_2_found  = is_frequency(leader_2_area,
                                            window_size,
                                            sample_rate,
                                            SSTV_LEADER_HZ);
        bool vis_start_found = is_frequency(vis_start_area,
                                            window_size,
                                            sample_rate,
                                            SSTV_BREAK_HZ);

        if (leader_1_found && break_found && leader_2_found && vis_start_found) {
            log_info("found SSTV header!");
            return current_sample + header_size;
        }
    }

    // If nothing was found, we return a sentinel value.
    log_warn("did not find SSTV header");
    return SSTV_PROCESSING_NOT_FOUND;
}


uint8_t decode_vis_code(const WavSamples *wav_samples, size_t vis_start) {
    // Extract information to be used throughout this function.
    uint32_t sample_rate = wav_samples->sample_rate;
    double *samples = wav_samples->samples;

    size_t bit_size = round(SSTV_BIT_TIME_SEC * sample_rate);
    uint8_t vis_p_code = 0;  // The VIS code with parity bit that we will build bit-by-bit

    // For the number of bits in the VIS+P code, we loop through and figure out what sample
    // that bit contains. We then determine the sample area for that bit and find the peak
    // frequency. The frequency determines whether the bit is logical HI or LO, which we add
    // to `vis_p_code` (LSB is read first; lowest sample number).
    for (size_t i = 0; i < CHAR_BIT; i++) {
        size_t bit_sample = vis_start + i * bit_size;
        double *bit_area = &samples[bit_sample];
        double peak = peak_frequency(bit_area, bit_size, sample_rate);

        uint8_t bit_value = peak <= SSTV_BREAK_HZ;
        bit_value <<= i;
        vis_p_code |= bit_value;
    }

    // Check that the parity is correct for sanity.
    bool has_correct_parity = __builtin_parity(vis_p_code) == 0;
    log_debug("VIS+P code is %d", vis_p_code);
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


uint8_t *decode_image_data(const WavSamples *wav_samples, const SstvMode *mode, size_t image_start)
{
    // Extract information from the arguments into smaller symbol names for easy use.
    size_t num_samples = wav_samples->num_samples;
    uint32_t sample_rate = wav_samples->sample_rate;
    double *samples = wav_samples->samples;

    size_t width = mode->width;
    size_t height = mode->height;
    uint16_t num_channels = mode->num_channels;

    // We use calloc here so that we can return a zero'ed out list if we don't get all the image
    // data. Any missing data will be black in the final image.
    uint8_t *image_data = (uint8_t *) calloc(width * height * num_channels, sizeof(uint8_t));

    // Calculate some information about the number of samples per pixel to check with the DFT.
    // Also, calculate some time information from the mode description.
    double center_window_time = (mode->pixel_time_sec * mode->window_factor) / 2.0;
    size_t pixel_size = round(center_window_time * 2.0 * sample_rate);

    double channel_time_sec = mode->pixel_time_sec * width;
    double line_time_sec = channel_time_sec * num_channels;

    // We loop through the dimensions and depth of the image to get pixel values. The outer loop
    // goes through each scan row between sync pulses.
    size_t line_start = image_start;
    for (size_t line_num = 0; line_num < height; line_num++) {
        line_start = find_sync_end(wav_samples, mode, line_start);  // Skip sync pulse

        if (line_num % 10 == 0) {
            log_info("decoding image line %3lu / %lu...", line_num, height);
        }

        // The middle loop goes through each color channel per line. For some modes, like PD modes,
        // this contains channels for two lines at ones.
        for (size_t channel_num = 0; channel_num < num_channels; channel_num++) {
            // The inner loop goes through each pixel for each channel in a row.
            for (size_t pixel_num = 0; pixel_num < width; pixel_num++) {
                // We calculate the location of the pixel in terms of time and then sample number.
                // The pixel location is set to be the center of a window `pixel_size` samples
                // wide.
                double channel_offset_sec = mode->porch_time_sec + channel_time_sec * channel_num;
                double local_pixel_offset_sec = mode->pixel_time_sec * pixel_num;
                double pixel_offset_sec = channel_offset_sec + local_pixel_offset_sec;
                double centered_pixel_offset_sec = pixel_offset_sec - center_window_time;
                size_t pixel_sample = round(line_start + centered_pixel_offset_sec * sample_rate);

                // Get the pixel data and determine the peak frequency, then convert the frequency
                // to an integer on [0, 255] and add it to the image data.
                double *pixel_area = &samples[pixel_sample];
                double frequency = peak_frequency(pixel_area, pixel_size, sample_rate);

                size_t pixel_index =
                    line_num * num_channels * width + channel_num * width + pixel_num;
                image_data[pixel_index] = calculate_pixel_value(frequency, mode);
            }
        }
    }

    return image_data;
}


static uint8_t calculate_pixel_value(double frequency, const SstvMode *mode) {
    double pixel_range_hz = mode->pixel_max_hz - mode->pixel_min_hz;
    double pixel_value = (frequency - mode->pixel_min_hz) / pixel_range_hz * 256.0;
    pixel_value = fmin(fmax(pixel_value, 0.0), 255.0);
    return round(pixel_value);
}
