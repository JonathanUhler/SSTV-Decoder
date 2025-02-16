#ifndef _SSTV_PROCESSING_H_
#define _SSTV_PROCESSING_H_


#define SSTV_PROCESSING_NOT_FOUND -1


#include "modes.h"
#include "wav_file.h"
#include <stdint.h>
#include <stdlib.h>


/**
 * Searches for the SSTV calibration header in a set of audio samples, returning the first sample
 * after the header if one is found.
 *
 * The search will begin at the first sample in the provided list. The DFT is run with a 10ms
 * window that slides 2ms for each check.
 *
 * @param wav_samples  The samples to search for an SSTV calibration header.
 *
 * @return The number of the first sample in {@code wav_samples->samples} after the header. If
 *         no header is found, {@code SSTV_PROCESSING_NOT_FOUND} is returned.
 */
size_t find_vis_start(const WavSamples *wav_samples);


/**
 * Searches for and decodes the VIS code in the SSTV header.
 *
 * If {@code assert} is enabled, an internal parity check will be performed. The returned code
 * does not contain the parity bit.
 *
 * @param wav_samples  The samples to search for the VIS code in.
 * @param vis_start    The start of the VIS code portion, returned by {@code find_header_sample}.
 *
 * @return The VIS code decoded from the samples starting at {@code vis_start}.
 */
uint8_t decode_vis_code(const WavSamples *wav_samples, size_t vis_start);


/**
 * Searches for a sample within a sync pulse.
 *
 * The search will begin at the {@code align_start} sample. The DFT to find a sync pulse is run
 * with a window that is one third the width of the sync pulse itself. The window slides 2ms
 * for each check.
 *
 * This function is not primarily used to align to a sync pulse after each scan line (see
 * {@code find_sync_end} for that behavior). This function is a substitute to {@code find_vis_start}
 * for very noisy signals in which a header cannot be recovered. The {@code align_start} sample
 * and the SSTV mode can be specified to attempt parsing without headers.
 *
 * @param wav_samples  The samples to search for a sync pulse.
 * @param mode         The SSTV mode encoded in the samples.
 * @param align_start  The sample to start searching from.
 *
 * @return The index of a sample in the first sync pulse found.
 */
size_t find_sync_start(const WavSamples *wav_samples, const SstvMode *mode, size_t align_start);


/**
 * Searches for the end of the current/next sync signal in the provided samples.
 *
 * @param wav_samples  The samples to search for the VIS code in.
 * @param mode         The SSTV mode encoded in the samples.
 * @param align_start  The sample to start searching from, which should ideally be in a sync pulse.
 *
 * @return The index of the first sample that is not in the sync pulse that was found.
 */
size_t find_sync_end(const WavSamples *wav_samples, const SstvMode *mode, size_t align_start);


/**
 * Decodes pixel data from the list of provided samples.
 *
 * The data is returned as a 3-dimensional array with the shape {@code [width, channels, height]}.
 * Values in the array are converted to luminance on the interval {@code [0, 255]} and ready
 * to be parsed into/written to an image file.
 *
 * @param wav_samples  The samples to search for the VIS code in.
 * @param mode         The SSTV mode encoded in the samples.
 * @param image_start  The index of the first sample with image data, possibly including a sync
 *                     pulse that will be automatically skipped.
 *
 * @return The pixel data.
 */
uint8_t *decode_image_data(const WavSamples *wav_samples, const SstvMode *mode, size_t image_start);


/**
 * Converts a frequency value in the pixel range for the provided mode to a luminance value.
 *
 * @param frequency  The frequency to convert.
 * @param mode       The SSTV mode being used, which defines the minimu and maximum frequencies
 *                   for the values of a channel in a pixel.
 *
 * @return The value of the channel of the pixel on the interval {@code [0, 255]}.
 */
static uint8_t calculate_pixel_value(double frequency, const SstvMode *mode);


#endif  // _SSTV_PROCESSING_H_
