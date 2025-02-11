#include "freq_processing.h"
#include <fftw3.h>
#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>


double hann_window(size_t num_samples, size_t sample_index) {
    return 0.5 * (1 - cos(2.0 * M_PI * sample_index / (num_samples - 1)));
}


double barycentric_peak_interpolation(double *bins, size_t num_bins, size_t index) {
    assert(bins && "barycentric_peak_interpolation got NULL bins");

    double left_neighbor = (index == 0) ? bins[index] : bins[index - 1];
    double right_neighbor = (index + 1 >= num_bins) ? bins[index] : bins[index + 1];
    double denominator = left_neighbor + bins[index] + right_neighbor;
    return denominator == 0 ? 0 : (right_neighbor - left_neighbor) / denominator + index;
}


void remove_dc_offset(double *samples, double *cleaned_samples, size_t num_samples) {
    assert(samples && "remove_dc_offset got NULL samples");
    assert(cleaned_samples && "remove_dc_offset got NULL cleaned_samples");

    // We first loop through all the samples and calculate the mean of the window.
    double mean = 0.0;
    for (size_t i = 0; i < num_samples; i++) {
        mean += samples[i];
    }
    mean /= num_samples;

    // With the mean, we then subtract that from all the samples. This eliminates the DC offset
    // that may exist in the sample data, which will cause `peak_frequency` to return 0 Hz.
    for (size_t i = 0; i < num_samples; i++) {
        cleaned_samples[i] = samples[i] - mean;
    }
}


double peak_frequency(double *samples, size_t num_samples, uint32_t sample_rate) {
    assert(samples && "peak_frequency got NULL samples");

    double *cleaned_samples = (double *) malloc(num_samples * sizeof(double));
    assert(cleaned_samples && "peak_frequency cannot malloc cleaned_samples");
    remove_dc_offset(samples, cleaned_samples, num_samples);

    double *windowed_samples = (double *) malloc(num_samples * sizeof(double));
    assert(windowed_samples && "peak_frequency cannot malloc windowed_samples");
    for (size_t i = 0; i < num_samples; i++) {
        windowed_samples[i] = cleaned_samples[i] * hann_window(num_samples, i);
    }

    size_t num_fft_samples = num_samples / 2 + 1;
    fftw_complex *fft = (fftw_complex *) fftw_malloc(num_fft_samples * sizeof(fftw_complex));
    assert(fft && "peak_frequency cannot malloc fft");

    fftw_plan fft_plan = fftw_plan_dft_r2c_1d(num_samples, windowed_samples, fft, FFTW_ESTIMATE);
    fftw_execute(fft_plan);

    double *magnitudes = (double *) malloc(num_fft_samples * sizeof(double));
    assert(magnitudes && "peak_frequency cannot malloc magnitudes");
    for (size_t i = 0; i < num_fft_samples; i++) {
        magnitudes[i] = sqrt(fft[i][0] * fft[i][0] + fft[i][1] * fft[i][1]);
    }

    size_t peak_index = 0;
    for (size_t i = 1; i < num_fft_samples; i++) {
        if (magnitudes[i] > magnitudes[peak_index]) {
            peak_index = i;
        }
    }

    double peak_magnitude = barycentric_peak_interpolation(magnitudes, num_fft_samples, peak_index);
    double peak_frequency = peak_magnitude * sample_rate / num_samples;

    free(magnitudes);
    fftw_destroy_plan(fft_plan);
    fftw_free(fft);
    free(windowed_samples);
    free(cleaned_samples);

    return peak_frequency;
}


bool is_frequency(double *samples, size_t num_samples, uint32_t sample_rate, double frequency) {
    assert(samples && "is_frequency got NULL samples");

    double peak = peak_frequency(samples, num_samples, sample_rate);
    double error = fabs(peak - frequency);
    return error < FREQ_PROCESSING_MARGIN_HZ;
}
