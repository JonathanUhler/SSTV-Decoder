#include "wav_file.h"
#include "freq_processing.h"
#include "modes.h"
#include <stdio.h>


int main(void) {
    // Demo: opening a wav file
    WavFile *wav_file = wav_file_open("/Users/jonathan/Downloads/PD120 SSTV Test Recording.wav");
    wav_file_print_header(wav_file);

    // Demo: printing some normalized  samples
    WavSamples *wav_samples = wav_file_get_mono_samples(wav_file);
    size_t start_sample = 1269436;
    size_t num_samples_to_print = 10;
    for (size_t i = start_sample; i < start_sample + num_samples_to_print; i++) {
        printf("Normalized sample %4lu: %f\n", i, wav_samples->samples[i]);
    }

    // Demo: some random data for peak frequency, expecting 3802.082584... Hz
    size_t len = 10;
    double *data = (double *) malloc(sizeof(double) * len);
    data[0] = 0.27252197;
    data[1] = 0.44992065;
    data[2] = 0.60906982;
    data[3] = 0.74325562;
    data[4] = 0.84680176;
    data[5] = 0.91534424;
    data[6] = 0.94613647;
    data[7] = 0.93783569;
    data[8] = 0.89089966;
    data[9] = 0.80755615;
    double freq = peak_frequency(data, len, wav_file->header->sample_rate);
    printf("peak freq = %f Hz\n", freq);
    free(data);

    // Demo: get an sstv mode
    const SstvMode *pd120_mode = get_sstv_mode(95);
    printf("addr of pd120 mode (shouldn't be null) = %lu\n", (size_t) pd120_mode);
    printf("addr of '0' mode (should be null) = %lu\n", (size_t) get_sstv_mode(0));

    // Demo: cleaning up
    wav_file_free_samples(wav_samples);
    wav_file_close(wav_file);

    return 0;
}
