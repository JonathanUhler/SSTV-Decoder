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
 * @param mode         The SSTV mode encoded in the samples.
 *
 * @return The number of the first sample in {@code wav_samples->samples} after the header. If
 *         no header is found, {@code SSTV_PROCESSING_NOT_FOUND} is returned.
 */
size_t find_header_sample(const WavSamples *wav_samples, const SstvMode *mode);


uint8_t decode_vis_code(const WavSamples *wav_samples, const SstvMode *mode, size_t vis_start);


size_t find_sync_end(const WavSamples *wav_samples, const SstvMode *mode, size_t current_sample);


uint8_t decode_image_data(const WavSamples *wav_samples, const SstvMode *mode, size_t image_start);


#endif  // _SSTV_PROCESSING_H_
