#include "wav_parser.h"
#include <stdio.h>


int main(void) {
    WavFile *wav_file = wav_file_open("examples_iss-20201225-002100-short.wav");
    wav_file_print(wav_file);
    wav_file_print_data(wav_file);
    wav_file_close(wav_file);
    return 0;
}
