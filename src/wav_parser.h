#ifndef _WAV_PARSER_H_
#define _WAV_PARSER_H_


#include <stdint.h>


typedef struct wav_header_s WavHeader;
typedef struct wav_file_s WavFile;


/**
 * A structure describing the header information of a WAV file.
 *
 * The field descriptors come from: http://soundfile.sapp.org/doc/WaveFormat/
 *
 * @var riff_marker      The string literal "RIFF" with no trailing NUL byte, in big-endian order.
 * @var size             The number of bytes in the file minus 8.
 * @var wave_marker      The string literal "WAVE" with no trailing NUL byte, in big-endian order.
 * @var fmt_marker       The string literal "fmt " with no trailing NUL byte, in big-endian order.
 * @var fmt_size         The number of bytes in the format section.
 * @var fmt_type         The type of the WAV format (uncompressed PCM is 0x0001).
 * @var num_channels     The number of channels in the audio signal (1 = mono, 2 = stereo, etc).
 * @var sample_rate      The sample rate in Hertz (blocks per second).
 * @var byte_rate        Equal to {@code sample_rate * num_chanells * bits_per_sample / 8}.
 * @var block_align      Equal to {@code num_channels * bits_per_sample / 8}.
 * @var bits_per_sample  The number of bits per sample in the data section.
 * @var data_marker      The string literal "data" with no trailing NUL byte, in big-endian order.
 * @var data_size        The number of bytes in the data section.
 */
struct wav_header_s {
    // Riff header
    char riff_marker[4];
    uint32_t size;
    char wave_marker[4];
    // Format descriptors
    char fmt_marker[4];
    uint32_t fmt_size;
    uint16_t fmt_type;
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bits_per_sample;
    // Data section descriptors
    char data_marker[4];
    uint32_t data_size;
};


/**
 * A structure describing a WAV file.
 *
 * @var header  A pointer to a {@code WavHeader} structure with the header data.
 * @var data    A pointer to the raw data. The size of the data is {@code header->data_size}.
 */
struct wav_file_s {
    WavHeader *header;
    uint8_t *data;
};


WavFile *wav_file_open(const char *path);
void wav_file_print(const WavFile *wav_file);
void wav_file_print_data(const WavFile *wav_file);
void wav_file_close(WavFile *wav_file);


#endif  // _WAV_PARSER_H_
