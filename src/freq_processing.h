#ifndef _FREQ_PROCESSING_H_
#define _FREQ_PROCESSING_H_


#define FREQ_PROCESSING_MARGIN_HZ 50


#include <stdbool.h>
#include <stdlib.h>


/**
 * Calculates the Hann window coefficient for a given number of samples and sample index.
 *
 * @param num_samples   The number of samples in the window.
 * @param sample_index  The index of the sample of interest.
 */
double hann_window(size_t num_samples, size_t sample_index);


/**
 * Interpolates over a provided set of data bins to find the peak in the specified bin.
 *
 * @param bins      The sample bins from a Fourier transform.
 * @param num_bins  The number of sample bins.
 * @param index     The bin index to perform interpolation within.
 *
 * @return The interpolated magnitude of the peak in `bins[index]` based on the neighbor bins.
 */
double barycentric_peak_interpolation(double *bins, size_t num_bins, size_t index);


/**
 * Removes DC offset from a set of signal samples.
 *
 * @param samples          A pointer with samples to remove DC offset from.
 * @param cleaned_samples  A pointer to place samples with DC offset removed.
 * @param num_samples      The size of the two sample pointers.
 */
void remove_dc_offset(double *samples, double *cleaned_samples, size_t num_samples);


/**
 * Determines the maximum frequency magnitude in a set of samples.
 *
 * @param samples       The set of samples to find the peak frequency within.
 * @param num_samples   The number of samples in `samples`.
 * @param samples_rate  The sample rate in Hertz.
 *
 * @return The maximum frequency in the provided samples, in Hertz.
 */
double peak_frequency(double *samples, size_t num_samples, uint32_t sample_rate);


/**
 * Determines whether the peak frequency in a set of samples is approximately equal to a target
 * frequency.
 *
 * The error threshold allowable by this function is defined as {@code FREQ_PROCESSING_MARGIN_HZ}.
 *
 * @param samples       The set of samples to find the peak frequency within.
 * @param num_samples   The number of samples in `samples`.
 * @param samples_rate  The sample rate in Hertz.
 * @param frequency     The target frequency to compare against.
 *
 * @return Whether the peak frequency in the samples is close to the target frequency.
 */
bool is_frequency(double *samples, size_t num_samples, uint32_t sample_rate, double frequency);


#endif  // _FREQ_PROCESSING_H_
