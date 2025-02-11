#ifndef _MODES_H_
#define _MODES_H_


#include <stdint.h>
#include <stdlib.h>


#define SSTV_LEADER_TIME_SEC 0.300000
#define SSTV_BREAK_TIME_SEC  0.010000
#define SSTV_BIT_TIME_SEC    0.030000

#define SSTV_LEADER_HZ 1900
#define SSTV_BREAK_HZ  1200


typedef struct sstv_mode_s SstvMode;


/**
 * An enumerator of color space encodings for lines in an SSTV mode.
 *
 * @var Y1_CR_CB_Y2  A YUV color space encoding with two luminance lines (e.g. in PD modes).
 */
enum color_space_e {
    Y1_CR_CB_Y2
};
typedef enum color_space_e ColorSpace;


/**
 * A structure defining the characteristics of an SSTV mode.
 *
 * @var name            A human-readable name or abbreviation for this mode.
 * @var vis             The VIS code for this mode.
 * @var width           The number of pixels in each line transmitted.
 * @var height          The number of lines transmitted (does not include double lines as in PD).
 * @var num_channels    The number of data channels in a line.
 * @var sync_time_sec   The time for a sync pulse between each line ins econds.
 * @var pixel_time_sec  The time for one channel of one pixel in seconds.
 * @var porch_time_sec  The time for the "porch" signal that comes after the "sync" on each line.
 * @var window_factor   An arbitrary value to set the size of the pixel Fourier transform window.
 * @var color_space     The color space of this mode.
 * @var sync_hz         The frequency of the "sync" pulse at the start of each line.
 * @var porch_hz        The frequency of the "porch" signal after each sync pulse.
 * @var pixel_min_hz    The minimum frequency for a channel of a pixel.
 * @var pixel_max_hz    The maximum frequency for a channel of a pixel.
 */
struct sstv_mode_s {
    // Identifier information
    char name[16];
    uint8_t vis;
    // Dimensions of the image
    size_t width;
    size_t height;
    uint16_t num_channels;
    // Timing and parsing information
    double sync_time_sec;
    double porch_time_sec;
    double pixel_time_sec;
    double window_factor;
    ColorSpace color_space;
    // Signal frequency information
    double sync_hz;
    double porch_hz;
    double pixel_min_hz;
    double pixel_max_hz;
};


/** List of all supported modes. */
extern const SstvMode sstv_modes[];


/**
 * Gets an {@code SstvMode} structure for the provided VIS code.
 *
 * @param vis  The VIS code expected in the returned mode structure.
 *
 * @return The SSTV mode structure with the VIS code {@code vis}.
 */
const SstvMode *get_sstv_mode(uint8_t vis);


#endif  // _MODES_H_
