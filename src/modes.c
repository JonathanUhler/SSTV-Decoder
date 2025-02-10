#include "modes.h"
#include <stdint.h>
#include <stdlib.h>


const SstvMode sstv_modes[] = {
    {
        .name = "PD 120",
            .vis             = 95,
            .width           = 640,
            .height          = 248,
            .num_channels    = 4,
            .sync_time_sec   = 0.020000,
            .porch_time_sec  = 0.002080,
            .pixel_time_sec  = 0.000190,
            .color_space     = Y1_CR_CB_Y2,
            .sync_hz         = 1200,
            .porch_hz        = 1500,
            .pixel_min_hz    = 1500,
            .pixel_max_hz    = 2300,
    }
};


const SstvMode *get_sstv_mode(uint8_t vis) {
    size_t num_modes = sizeof(sstv_modes) / sizeof(SstvMode);

    for (size_t i = 0; i < num_modes; i++) {
        if (sstv_modes[i].vis == vis) {
            return &sstv_modes[i];
        }
    }

    return NULL;
}
