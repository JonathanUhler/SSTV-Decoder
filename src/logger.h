#ifndef _LOGGER_H_
#define _LOGGER_H_


#include <stdbool.h>
#include <stdio.h>


extern bool logger_verbose;


/**
 * Enable or disable DEBUG level logging.
 *
 * @param verbose  Whether to enable DEBUG logging.
 */
void logger_set_verbosity(bool verbose);


#define LOG_PRINT_HELPER(severity, format, ...)                 \
    printf("%-5s  " format "\n%s", severity, __VA_ARGS__)

#define LOG_PRINT(severity, ...)                \
    LOG_PRINT_HELPER(severity, __VA_ARGS__, "")

#define log_debug(...)                          \
    if (logger_verbose) {                       \
        LOG_PRINT("DEBUG", __VA_ARGS__);        \
    }

#define log_info(...)                           \
    {                                           \
        LOG_PRINT("INFO", __VA_ARGS__);         \
    }

#define log_warn(...)                           \
    {                                           \
        LOG_PRINT("WARN", __VA_ARGS__);         \
    }

#define log_error(...)                           \
    {                                            \
        LOG_PRINT("ERROR", __VA_ARGS__);         \
    }

#define log_fatal(...)                          \
    {                                           \
        LOG_PRINT("FATAL", __VA_ARGS__);        \
        exit(1);                                \
    }


#endif  // _LOGGER_H_
