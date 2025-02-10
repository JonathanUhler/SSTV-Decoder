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
 * Searches for the end of the current/next sync signal in the provided samples.
 *
 * @param wav_samples  The samples to search for the VIS code in.
 * @param mode         The SSTV mode encoded in the samples.
 * @param align_start  The sample to start searching from, which should ideally be in a sync pulse.
 *
 * @return The index of the first sample that is not in the sync pulse that was found.
 */
size_t find_sync_end(const WavSamples *wav_samples, const SstvMode *mode, size_t align_start);


uint8_t decode_image_data(const WavSamples *wav_samples, const SstvMode *mode, size_t image_start);


#endif  // _SSTV_PROCESSING_H_
