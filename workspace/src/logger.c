#include "logger.h"
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Current log level
static LogLevel current_log_level = LOG_LEVEL_INFO;

// Log file (NULL means use stdout)
static FILE *log_file = NULL;

// Log level strings
static const char *level_strings[] = {
    "DEBUG",
    "INFO ",
    "WARN ",
    "ERROR",
    "FATAL"
};

// ANSI color codes
static const char *level_colors[] = {
    "\x1b[36m", // Cyan for DEBUG
    "\x1b[32m", // Green for INFO
    "\x1b[33m", // Yellow for WARN
    "\x1b[31m", // Red for ERROR
    "\x1b[35m"  // Magenta for FATAL
};
static const char *color_reset = "\x1b[0m";

void logger_init(LogLevel level) {
    current_log_level = level;
    log_file = stdout; // Default to stdout
    
    // Initial log message
    logger_log(LOG_LEVEL_INFO, __FILE__, __LINE__, "Logger initialized with level %s", level_strings[level]);
}

void logger_set_level(LogLevel level) {
    logger_log(LOG_LEVEL_INFO, __FILE__, __LINE__, "Changing log level from %s to %s", 
               level_strings[current_log_level], level_strings[level]);
    current_log_level = level;
}

void logger_log(LogLevel level, const char *file, int line, const char *format, ...) {
    // Only log if the level is greater than or equal to the current log level
    if (level < current_log_level) {
        return;
    }
    
    // Get the current time
    time_t now;
    time(&now);
    struct tm *timeinfo = localtime(&now);
    
    char time_str[9];
    strftime(time_str, sizeof(time_str), "%H:%M:%S", timeinfo);
    
    // Get the filename from the path
    const char *filename = strrchr(file, '/');
    filename = filename ? filename + 1 : file;
    
    // Print log header with color
    fprintf(log_file, "%s[%s][%s] %s:%d: ", 
            level_colors[level], time_str, level_strings[level], filename, line);
    
    // Print the message with color reset
    va_list args;
    va_start(args, format);
    vfprintf(log_file, format, args);
    va_end(args);
    
    fprintf(log_file, "%s\n", color_reset);
    
    // Flush output immediately
    fflush(log_file);
    
    // If fatal, exit the program
    if (level == LOG_LEVEL_FATAL) {
        fprintf(log_file, "%s[%s][FATAL] Exiting program.%s\n", 
                level_colors[level], time_str, color_reset);
        exit(EXIT_FAILURE);
    }
}

void logger_close(void) {
    if (log_file != NULL && log_file != stdout && log_file != stderr) {
        fclose(log_file);
        log_file = NULL;
    }
    
    // Reset to defaults
    current_log_level = LOG_LEVEL_INFO;
}
