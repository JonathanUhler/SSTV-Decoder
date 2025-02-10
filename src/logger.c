#include "logger.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>


bool logger_verbose = false;


void logger_set_verbosity(bool verbose) {
    logger_verbose = verbose;
}
