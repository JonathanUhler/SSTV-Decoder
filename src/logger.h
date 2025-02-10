#ifndef _LOGGER_H_
#define _LOGGER_H_


#include <stdbool.h>


extern bool logger_verbose;


void logger_print(const char *severity, const char *message);
void logger_set_verbosity(bool verbose);
void log_info(const char *message);
void log_debug(const char *message);
void log_warn(const char *message);
void log_error(const char *message);
void log_fatal(const char *message);


#endif  // _LOGGER_H_
