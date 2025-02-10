#include "logger.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>


bool logger_verbose = false;


void logger_print(const char *severity, const char *message) {
    printf("%-5s  %s\n", severity, message);
}


void logger_set_verbosity(bool verbose) {
    logger_verbose = verbose;
}


void log_info(const char *message) {
    logger_print("INFO", message);
}


void log_debug(const char *message) {
    if (!logger_verbose) {
        return;
    }
    logger_print("DEBUG", message);
}


void log_warn(const char *message) {
    logger_print("WARN", message);
}


void log_error(const char *message) {
    logger_print("ERROR", message);
}


void log_fatal(const char *message) {
    logger_print("FATAL", message);
    exit(1);
}
