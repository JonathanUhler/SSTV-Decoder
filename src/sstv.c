#include "wav_file.h"
#include <stdio.h>


int main(void) {
    WavFile *wav_file = wav_file_open("/Users/jonathan/Downloads/PD120 SSTV Test Recording.wav");
    wav_file_print(wav_file);
    wav_file_print_data(wav_file);
    wav_file_close(wav_file);
    return 0;
}
