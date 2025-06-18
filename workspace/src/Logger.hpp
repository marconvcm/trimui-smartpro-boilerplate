#pragma once

#include <string>
#include <fstream>
#include <memory>

namespace TG5040
{

    enum class LogLevel
    {
        DEBUG = 0,
        INFO,
        WARN,
        ERROR,
        FATAL
    };

    class Logger
    {
    public:
        static Logger &getInstance();

        void init(LogLevel level = LogLevel::DEBUG, const std::string &filename = "");
        void close();

        void log(LogLevel level, const char *file, int line, const char *format, ...);

        // Prevent copying
        Logger(const Logger &) = delete;
        Logger &operator=(const Logger &) = delete;

    private:
        Logger() = default;
        ~Logger() = default;

        LogLevel currentLevel_ = LogLevel::DEBUG;
        std::unique_ptr<std::ofstream> logFile_;
        bool initialized_ = false;

        const char *levelToString(LogLevel level) const;
        std::string getCurrentTime() const;
    };

} // namespace TG5040

// Convenience macros
#define LOG_DEBUG(...) TG5040::Logger::getInstance().log(TG5040::LogLevel::DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_INFO(...) TG5040::Logger::getInstance().log(TG5040::LogLevel::INFO, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_WARN(...) TG5040::Logger::getInstance().log(TG5040::LogLevel::WARN, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_ERROR(...) TG5040::Logger::getInstance().log(TG5040::LogLevel::ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_FATAL(...) TG5040::Logger::getInstance().log(TG5040::LogLevel::FATAL, __FILE__, __LINE__, __VA_ARGS__)
