#include "Logger.hpp"
#include <iostream>
#include <cstdarg>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <cstring>

namespace TG5040
{
    Logger &Logger::getInstance()
    {
        static Logger instance;
        return instance;
    }

    void Logger::init(LogLevel level, const std::string &filename)
    {
        currentLevel_ = level;

        if (!filename.empty())
        {
            logFile_ = std::make_unique<std::ofstream>(filename, std::ios::app);
            if (!logFile_->is_open())
            {
                std::cerr << "Warning: Could not open log file: " << filename << std::endl;
                logFile_.reset();
            }
        }

        initialized_ = true;
        log(LogLevel::INFO, __FILE__, __LINE__, "Logger initialized with level: %s", levelToString(level));
    }

    void Logger::close()
    {
        if (initialized_)
        {
            log(LogLevel::INFO, __FILE__, __LINE__, "Logger shutting down");
            if (logFile_)
            {
                logFile_->close();
                logFile_.reset();
            }
            initialized_ = false;
        }
    }

    void Logger::log(LogLevel level, const char *file, int line, const char *format, ...)
    {
        if (!initialized_ || level < currentLevel_)
        {
            return;
        }

        // Format the message
        char buffer[1024];
        va_list args;
        va_start(args, format);
        vsnprintf(buffer, sizeof(buffer), format, args);
        va_end(args);

        // Get timestamp
        std::string timestamp = getCurrentTime();

        // Extract filename from path
        const char *filename = strrchr(file, '/');
        if (!filename)
            filename = strrchr(file, '\\');
        if (!filename)
            filename = file;
        else
            filename++;

        // Format the complete log line
        std::ostringstream logLine;
        logLine << "[" << timestamp << "] [" << levelToString(level) << "] "
                << filename << ":" << line << " - " << buffer;

        // Output to console
        std::cout << logLine.str() << std::endl;

        // Output to file if available
        if (logFile_ && logFile_->is_open())
        {
            *logFile_ << logLine.str() << std::endl;
            logFile_->flush();
        }
    }

    const char *Logger::levelToString(LogLevel level) const
    {
        switch (level)
        {
        case LogLevel::DEBUG:
            return "DEBUG";
        case LogLevel::INFO:
            return "INFO ";
        case LogLevel::WARN:
            return "WARN ";
        case LogLevel::ERROR:
            return "ERROR";
        case LogLevel::FATAL:
            return "FATAL";
        default:
            return "UNKNOWN";
        }
    }

    std::string Logger::getCurrentTime() const
    {
        auto now = std::time(nullptr);
        auto tm = *std::localtime(&now);

        std::ostringstream oss;
        oss << std::put_time(&tm, "%H:%M:%S");
        return oss.str();
    }

} // namespace TG5040
