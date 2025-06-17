#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>

// Log level enum
typedef enum {
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARN,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_FATAL
} LogLevel;

// Initialize the logger
void logger_init(LogLevel level);

// Log with specified level
void logger_log(LogLevel level, const char *file, int line, const char *format, ...);

// Convenience macros for logging
#define LOG_DEBUG(...) logger_log(LOG_LEVEL_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_INFO(...) logger_log(LOG_LEVEL_INFO, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_WARN(...) logger_log(LOG_LEVEL_WARN, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_ERROR(...) logger_log(LOG_LEVEL_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_FATAL(...) logger_log(LOG_LEVEL_FATAL, __FILE__, __LINE__, __VA_ARGS__)

// Set the minimum log level
void logger_set_level(LogLevel level);

// Close the logger and free resources
void logger_close(void);

#endif // LOGGER_H
