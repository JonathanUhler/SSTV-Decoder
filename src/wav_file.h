#ifndef _WAV_FILE_H_
#define _WAV_FILE_H_


#include <stdint.h>
#include <stdlib.h>


typedef struct wav_header_s WavHeader;
typedef struct wav_file_s WavFile;
typedef struct wav_samples_s WavSamples;


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


/**
 * A structure describing a WAV file with sample data normalized to the range [-1.0, 1.0].
 *
 * @var header  A pointer to a {@code WavHeader} structure with the header data.
 * @var data    A pointer to the normalized audio data (with {@code header->data_size} entries).
 */
struct wav_samples_s {
    size_t num_samples;
    uint32_t sample_rate;
    double *samples;
};


/**
 * Opens a {@code .wav} file.
 *
 * @param path  The path to the {@code .wav} file. Relative paths are relative to the CWD.
 *
 * @return A pointer to a {@code WavFile} structure describing the data in the file. If the file
 *         cannot be opened {@code NULL} is returned.
 */
WavFile *wav_file_open(const char *path);


/**
 * Creates a list of audio samples from a wave file normalized on {@code [-1, 1]}.
 *
 * The sample data is always mono. If the original wave file contains more than one channel, the
 * normalized sample is the average of all channels in the wave file.
 *
 * @param wav_file  The wave file to get the normalized samples from.
 *
 * @return A pointer to a {@code WavSamples} structure with the list of samples, number of samples,
 *         and sample rate in Hertz.
 */
WavSamples *wav_file_get_mono_samples(const WavFile *wav_file);


/**
 * Normalizes a single sample in a wave file.
 *
 * @param raw_sample       The bits of the raw sample, which will be interpreted as signed.
 * @param bits_per_sample  The number of bits in the raw sample.
 *
 * @return The normalized value of the raw sample.
 */
double wav_file_normalize_sample(uint32_t raw_sample, uint32_t bits_per_sample);


/**
 * Frees a {@code WavSamples} structure returned by {@code wav_file_get_*_samples}.
 *
 * @param wav_samples  The {@code WavSamples} structure to free.
 */
void wav_file_free_samples(WavSamples *wav_samples);


/**
 * Pretty-prints the header metadata in the provided structure.
 *
 * Information is printed in a human-readable format to the standard output. If the wave file
 * pointer is {@code NULL}, the string {@code "[NULL]"} is printed.
 *
 * @param wav_file  The wave file structure to print.
 */
void wav_file_print_header(const WavFile *wav_file);


/**
 * Closes a {@code .wav} file opened with {@code wav_file_open}.
 *
 * @param wav_file  The wave file to close.
 *
 * @see wav_file_open
 */
void wav_file_close(WavFile *wav_file);


#endif  // _WAV_FILE_H_
