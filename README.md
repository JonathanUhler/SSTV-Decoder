# SSTV-Decoder
A C command-line program to decode slow-scan television (SSTV) images from wave audio files.

This repository is mostly an educational resource for learning about audio and visual file
encodings, SSTV protocols, and the C language. It may or may not be feature complete.


# About SSTV
This project mainly uses the following resources as a reference for learning about SSTV:

* [[1](http://www.barberdsp.com/downloads/Dayton%20Paper.pdf)] Proposal for SSTV Mode Specifications
* [[2](http://www.sstv-handbook.com/download/sstv-handbook.pdf)] Image Communication on Short Waves


# Usage
## Dependencies
This project only requires a C compiler and at least CMake 3.18.

## Installation
Clone this git repository. In the top-level directory, create build file with CMake:

```cmake
cmake -B build
```

And then build the project. The SSTV decoder uses the FFTW3 library for a Discrete Forier Transform
implementation, and libpng for creating image files, both of which will be fetched by CMake.

```cmake
cmake --build build
```

## Decoding Audio Files
The build will create an `sstv` binary in the build directory. The program can be run with the
following options:

| Option | Commentary                                                      |
|--------|-----------------------------------------------------------------|
| `-h`   | Print usage information and exit.                               |
| `-o`   | Specify the output file for the image, by default `result.png`. |
| `-v`   | Print verbose debug information about program execution.        |

Positional arguments for the program are specified after option flags:

| Positional Argument | Commentary                                    |
|---------------------|-----------------------------------------------|
| `path`              | The path to the wave audio file(s) to decode. |


# Programmer Concepts
The project consists of the following files:

- `freq_processing`: Generic analog signal processing with Discrete Fourier Tranforms.
- `logger`: Logging macros for the project.
- `modes`: Definitions of supported SSTV modes.
- `png_file`: Utilities to write a PNG image file from SSTV color data.
- `sstv`: The command line utility for the project.
- `sstv_processing`: Signal processing for SSTV format components.
- `wav_file`: Utilities to read an audio wave file and extract samples from it.

## Adding SSTV Modes
This project is currently intended to be used for decoding PD-120 SSTV signals from the ISS.
However, enough generality exists such that new modes should be relatively easy to add.

Each SSTV mode, regardless of its unique parsing atributes, is defined in `modes.c`. Add a new
`SstvMode` (a.k.a. `struct sstv_mode_s`) to the `sstv_modes` array. Each mode is defined and
identified by its VIS code (the `vis` member of the struct). This must be unique across all
supported modes.

There are generally two other attributes that may change between modes:

1. The color space. New color spaces (for the `color_space` member) are defined in the
   `ColorSpace` (a.k.a. `enum color_space_e`) enumerator in `modes.h`.
2. Pixel generation routines. The `png_file` functions work with `Pixel` (a.k.a. `struct pixel_s`)
   arrays. This abstracts the nuance of SSTV encoding when writing the image data. To convert
   from the `width * channels * height` structure returned by `decode_image_data` in
   `sstv_processing`, there should be a function in `png_file` that accepts the raw image data
   pointer and returns a `Pixel` array in RGB to be written to a PNG file. This function is
   also responsible for handling SSTV formats that encode multiple image lines in a single
   data line, like PD-120.

## Quality Variables
The quality of the decoded image is an optimzation problem on one variable, the number of audio
samples passed to the DFT to determine the value of one channel of one pixel. Two cases arise:

- Increasing the width of the sliding pixel window will allow the DFT to determine a more
  accurate peak frequency, but blurs the image with adjacent pixels if the window is too large.
- Decreasing the window width will sharpen the image by giving fewer samples to the DFT, but
  noise in the signal can more easily overwhelm the peak frequency computation.

The decoder provides a variable to control pixel granularity:

For each `SstvMode` struct, the `window_factor` member represents a scalar on the time (in
seconds) for a value of a single channel of a single pixel. A `window_factor` of 1.0 will
set the sliding window siz eo exactly the width of a pixel, and is the recommended minimum.
