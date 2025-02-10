#ifndef _FREQ_PROCESSING_H_
#define _FREQ_PROCESSING_H_


#include <stdlib.h>


double hann_window(int n, int i);


double barycentric_peak_interpolation(double *bins, size_t num_bins, size_t x);


void remove_dc_offset(double *samples, double *cleaned_samples, size_t num_samples);


double peak_frequency(double *samples, size_t num_samples, uint32_t sample_rate);


#endif  // _FREQ_PROCESSING_H_
