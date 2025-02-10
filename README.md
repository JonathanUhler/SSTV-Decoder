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
implementation, which will be fetched from [this archive](https://www.fftw.org/download.html) by
CMake.

```cmake
cmake --build build
```

## Decoding Audio Files
The build will create an `sstv` binary in the build directory. The program can be run with the
following options:

| Option | Commentary                                                  |
|--------|-------------------------------------------------------------|
| `-v`   | Print verbose debug information about program execution.    |
| `-o`   | Specify the output path for the image file, by default `.`. |

Positional arguments for the program are specified after option flags:

| Positional Argument | Commentary                                    |
|---------------------|-----------------------------------------------|
| `path`              | The path to the wave audio file(s) to decode. |
