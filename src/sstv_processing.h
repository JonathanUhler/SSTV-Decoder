#ifndef _SSTV_PROCESSING_H_
#define _SSTV_PROCESSING_H_


#include "modes.h"
#include "wav_file.h"
#include <stdlib.h>


size_t find_header_sample(const WavSamples *wav_samples, const SstvMode *mode);

uint8_t find_vis_code(const WavSamples *wav_samples, const SstvMode *mode);

size_t find_sync_end(const WavSamples *wav_samples, const SstvMode *mode, size_t current_sample);


#endif  // _SSTV_PROCESSING_H_
