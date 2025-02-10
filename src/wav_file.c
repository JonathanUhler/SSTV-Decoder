#include "wav_file.h"


#include <limits.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>


WavFile *wav_file_open(const char *path) {
    FILE *file = fopen(path, "rb");
    if (file == NULL) {
        return NULL;
    }

    // Allocate memory for the wave file data. If the `malloc` calls fail, then we cannot recover,
    // so we free any allocated memory and exit.
    WavFile *wav_file = (WavFile *) malloc(sizeof(WavFile));
    WavHeader *header = (WavHeader *) malloc(sizeof(WavHeader));
    if (wav_file == NULL || header == NULL) {
        free(wav_file);
        free(header);
        fclose(file);
        return NULL;
    }

    // The basic canonical fields of the riff header are read directly into their struct members.
    fread(header->riff_marker,      sizeof(header->riff_marker),     1, file);
    fread(&header->size,            sizeof(header->size),            1, file);
    fread(header->wave_marker,      sizeof(header->wave_marker),     1, file);
    fread(header->fmt_marker,       sizeof(header->fmt_marker),      1, file);
    fread(&header->fmt_size,        sizeof(header->fmt_size),        1, file);
    fread(&header->fmt_type,        sizeof(header->fmt_type),        1, file);
    fread(&header->num_channels,    sizeof(header->num_channels),    1, file);
    fread(&header->sample_rate,     sizeof(header->sample_rate),     1, file);
    fread(&header->byte_rate,       sizeof(header->byte_rate),       1, file);
    fread(&header->block_align,     sizeof(header->block_align),     1, file);
    fread(&header->bits_per_sample, sizeof(header->bits_per_sample), 1, file);

    // For the data segment, we must deal with non-canonical riff data. Some files place additional
    // chunks (e.g. "LIST") immediately before the "data" segment. Each chunk will have a 4-byte
    // size following it that we can use to skip the chunk. We do this until we find "data"
    fread(header->data_marker, sizeof(header->data_marker), 1, file);
    while (strcmp(header->data_marker, "data") != 0) {
        uint32_t chunk_size = 0;
        fread(&chunk_size, sizeof(chunk_size), 1, file);
        fseek(file, chunk_size, SEEK_CUR);
        fread(header->data_marker, sizeof(header->data_marker), 1, file);
    }

    fread(&header->data_size, sizeof(header->data_size), 1, file);

    // The rest of the file data is allocated and the samples are read.
    uint8_t *data = (uint8_t *) malloc(header->data_size);
    fread(data, sizeof(uint8_t), header->data_size, file);

    wav_file->header = header;
    wav_file->data = data;
    return wav_file;
}


WavSamples *wav_file_get_mono_samples(const WavFile *wav_file) {
    if (wav_file == NULL || wav_file->header == NULL || wav_file->data == NULL) {
        return NULL;
    }

    // We get the information from the wave file for use throughout. This has been checked for
    // NULL above.
    WavHeader *header = wav_file->header;
    uint8_t *data = wav_file->data;

    // We compute some useful values for determining the size of a sample in bytes, the size of
    // a "row" (the samples in all channels for one time point), and the number of "rows". Because
    // this function compresses all channels down to one (mono), it contains the same number of
    // rows, just with only a single sample per row.
    uint32_t bytes_per_sample = header->bits_per_sample / CHAR_BIT;
    uint32_t bytes_per_row = bytes_per_sample * header->num_channels;
    uint32_t num_rows = header->data_size / bytes_per_row;

    double *samples = (double *) malloc(num_rows * sizeof(double));

    // The outer loop goes through each row (the samples in all channels for a time point in the
    // original audio, and a single sample for a single time point in the `samples` list).
    for (size_t row = 0; row < num_rows; row++) {
        double channel_aggregate = 0.0;  // We will take the average to make it mono later.

        // The channel loop goes through each channel for the current row/time point.
        for (size_t channel = 0; channel < header->num_channels; channel++) {
            uint32_t raw_sample = 0;

            // Finally, the inner most loop constructs the sample for the current channel, which
            // might occupy multiple bytes.
            for (size_t byte_in_sample = 0; byte_in_sample < bytes_per_sample; byte_in_sample++) {
                size_t index = row * bytes_per_row + channel * bytes_per_sample + byte_in_sample;
                uint32_t byte_data = data[index];
                byte_data <<= byte_in_sample * CHAR_BIT;
                raw_sample |= byte_data;
            }

            channel_aggregate += wav_file_normalize_sample(raw_sample, header->bits_per_sample);
        }

        // Take the average of all channels to get a single mono sample for this time point.
        double channel_average = channel_aggregate / header->num_channels;
        samples[row] = channel_average;
    }

    // With the samples determined and normalized, we can place them into a nice structure.
    WavSamples *wav_samples = (WavSamples *) malloc(sizeof(WavSamples));
    wav_samples->num_samples = num_rows;
    wav_samples->sample_rate = header->sample_rate;
    wav_samples->samples = samples;
    return wav_samples;
}


double wav_file_normalize_sample(uint32_t raw_sample, uint32_t bits_per_sample) {
    // We will use two masks to interpret the bits of the raw sample as a signed integer and
    // sign-extend to an INT32 if needed. The `value_mask` masks the bits of the raw sample
    // (assuming it has `bits_per_sample` bits). Thus, the bitwise inverse of this will be used
    // to sign extend. The `sign_mask` is just for the sign bit.
    uint32_t value_mask = (1U << bits_per_sample) - 1;
    uint32_t sign_mask = 1U << (bits_per_sample - 1);
    raw_sample &= value_mask;

    // If the sign bit in the raw sample's bits is set, then we sign extend to an INT32.
    if (raw_sample & sign_mask) {
        raw_sample |= ~value_mask;
    }

    // After sign extension, we can interpret the value as an INT32. Finally, we can normalize
    // it from the range of an integer with `bits_per_sample` bits to a float on [-1, 1].
    int32_t signed_raw_sample = (int32_t) raw_sample;
    return (double) signed_raw_sample / pow(2.0, bits_per_sample - 1);
}


void wav_file_free_samples(WavSamples *wav_samples) {
    if (wav_samples == NULL) {
        return;
    }

    free(wav_samples->samples);
    free(wav_samples);
}


void wav_file_print_header(const WavFile *wav_file) {
    // Top-level message declaring this is a wave file structure.
    printf("WavFile:\n");
    if (wav_file == NULL) {
        printf("[NULL]\n");
        return;
    }

    // Top-level message for the header data.
    printf(" Header:\n");
    WavHeader *header = wav_file->header;
    if (wav_file->header == NULL) {
        printf("  [NULL]\n");
        return;
    }

    // Header data printed out in a human-readable format. For the 'string' types (e.g. char[]),
    // there are no trailing NUL bytes, so we tell `printf` exactly how many characters to print.
    printf("  RIFF marker: %.4s\n",  header->riff_marker);
    printf("  Size:        %d B\n",  header->size);
    printf("  WAVE marker: %.4s\n",  header->wave_marker);
    printf("  fmt  marker: %.4s\n",  header->fmt_marker);
    printf("  Format size: %d B\n",  header->fmt_size);
    printf("  Format type: %d\n",    header->fmt_type);
    printf("  Channels:    %d\n",    header->num_channels);
    printf("  Sample rate: %d Hz\n", header->sample_rate);
    printf("  Byte rate:   %d B\n",  header->byte_rate);
    printf("  Block align: %d B\n",  header->block_align);
    printf("  Bits/sample: %d b\n",  header->bits_per_sample);
    printf("  data marker: %.4s\n",  header->data_marker);
    printf("  Data size:   %d B\n",  header->data_size);
}


void wav_file_close(WavFile *wav_file) {
    if (wav_file == NULL) {
        return;
    }

    free(wav_file->header);
    free(wav_file->data);
    free(wav_file);
}
