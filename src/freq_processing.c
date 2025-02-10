#include "freq_processing.h"
#include <fftw3.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>


double hann_window(int n, int i) {
    return 0.5 * (1 - cos(2.0 * M_PI * i / (n - 1)));
}


double barycentric_peak_interpolation(double *bins, size_t num_bins, size_t x) {
    double y1 = (x == 0) ? bins[x] : bins[x - 1];
    double y3 = (x + 1 >= num_bins) ? bins[x] : bins[x + 1];
    double denominator = y1 + bins[x] + y3;
    return denominator == 0 ? 0 : (y3 - y1) / denominator + x;
}


void remove_dc_offset(double *samples, double *cleaned_samples, size_t num_samples) {
    double mean = 0.0;
    for (size_t i = 0; i < num_samples; i++) {
        mean += samples[i];
    }
    mean /= num_samples;

    for (size_t i = 0; i < num_samples; i++) {
        cleaned_samples[i] = samples[i] - mean;
    }
}


double peak_frequency(double *samples, size_t num_samples, uint32_t sample_rate) {
    double *cleaned_samples = (double *) malloc(num_samples * sizeof(double));
    remove_dc_offset(samples, cleaned_samples, num_samples);

    double *windowed_samples = (double *) malloc(num_samples * sizeof(double));
    for (size_t i = 0; i < num_samples; i++) {
        windowed_samples[i] = cleaned_samples[i] * hann_window(num_samples, i);
    }

    size_t num_fft_samples = num_samples / 2 + 1;
    fftw_complex *fft = (fftw_complex *) fftw_malloc(num_fft_samples * sizeof(fftw_complex));
    fftw_plan fft_plan = fftw_plan_dft_r2c_1d(num_samples, windowed_samples, fft, FFTW_ESTIMATE);
    fftw_execute(fft_plan);

    double *magnitudes = (double *) malloc(num_fft_samples * sizeof(double));
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

    free(cleaned_samples);
    free(windowed_samples);
    fftw_destroy_plan(fft_plan);
    fftw_free(fft);

    return peak_frequency;
}
