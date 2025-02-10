#include "wav_file.h"


#include <limits.h>
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


void wav_file_print(const WavFile *wav_file) {
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


void wav_file_print_data(const WavFile *wav_file) {
    // Top-level message declaring this is a wave file structure.
    printf("WavFile:\n");
    if (wav_file == NULL) {
        printf("[NULL]\n");
        return;
    }

    // Top-level message for the list of raw audio samples.
    printf(" Samples:\n");
    WavHeader *header = wav_file->header;
    uint8_t *data = wav_file->data;
    if (data == NULL) {
        printf("  [NULL]\n");
        return;
    }

    // We assume a maximum fidelity of 32-bits per sample (`uint32_t`). If this is violated
    // by the sample size in the header, we cannot print the samples.
    uint32_t sample_size = header->num_channels * header->bits_per_sample / CHAR_BIT;
    uint32_t data_size_per_channel = header->data_size / header->num_channels;
    if (sample_size > sizeof(uint32_t)) {
        printf("  [ERROR: Sample size %d B cannot be greater than 4 B]\n", sample_size);
        return;
    }

    // For each sample, we loop through each channel (i.e. mono or stereo) and print the hex
    // information for that sample/channel. For audio data where a sample occupies more than one
    // byte, we must construct the multi-byte sample
    for (size_t sample = 0; sample < data_size_per_channel; sample += sample_size) {
        printf("  Sample %8zu:", sample);
        for (size_t channel = 0; channel < header->num_channels; channel++) {
            uint32_t sample_data = 0;

            // Construct the multi-byte sample
            for (size_t byte_in_sample = 0; byte_in_sample < sample_size; byte_in_sample++) {
                uint32_t byte_data = data[sample + byte_in_sample];
                byte_data <<= byte_in_sample * CHAR_BIT;
                sample_data |= byte_data;
            }

            printf(" CH%1zu = %0*x", channel, 2 * sample_size, sample_data);
        }
        printf("\n");
    }
}


void wav_file_close(WavFile *wav_file) {
    if (wav_file == NULL) {
        return;
    }

    free(wav_file->header);
    free(wav_file->data);
    free(wav_file);
}
