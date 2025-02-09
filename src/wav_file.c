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

    WavFile *wav_file = (WavFile *) malloc(sizeof(WavFile));
    WavHeader *header = (WavHeader *) malloc(sizeof(WavHeader));
    if (wav_file == NULL || header == NULL) {
        return NULL;
    }

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
    fread(header->data_marker,      sizeof(header->data_marker),     1, file);
    fread(&header->data_size,       sizeof(header->data_size),       1, file);

    uint8_t *data = (uint8_t *) malloc(header->data_size);
    fread(data, sizeof(uint8_t), header->data_size, file);

    wav_file->header = header;
    wav_file->data = data;
    return wav_file;
}


void wav_file_print(const WavFile *wav_file) {
    printf("WavFile:\n");
    if (wav_file == NULL) {
        printf("[NULL]\n");
        return;
    }

    printf(" Header:\n");
    WavHeader *header = wav_file->header;
    if (wav_file->header == NULL) {
        printf("  [NULL]\n");
        return;
    }

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
    printf("WavFile:\n");
    if (wav_file == NULL) {
        printf("[NULL]\n");
        return;
    }

    printf(" Samples:\n");
    WavHeader *header = wav_file->header;
    uint8_t *data = wav_file->data;
    if (data == NULL) {
        printf("  [NULL]\n");
        return;
    }

    uint32_t sample_size = header->num_channels * header->bits_per_sample / CHAR_BIT;
    uint32_t data_size_per_channel = header->data_size / header->num_channels;
    if (sample_size > sizeof(uint32_t)) {
        printf("  [ERROR: Sample size %d B cannot be greater than 4 B]\n", sample_size);
    }

    for (size_t sample = 0; sample < data_size_per_channel; sample += sample_size) {
        printf("  Sample %8zu:", sample);
        for (size_t channel = 0; channel < header->num_channels; channel++) {
            uint32_t sample_data = 0;

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
