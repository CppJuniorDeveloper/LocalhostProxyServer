#pragma once

#include <iostream>
#include <string_view>
#include <iomanip>
#include <chrono>
#include <format>

namespace utils::logger
{
    enum class LogLevel : uint8_t {DEBUG, INFO, WARNING, ERR, FATAL};

    inline LogLevel global_log_min_level = LogLevel::INFO;

    namespace color_constants
    {
        constexpr const char* RESET     = "\033[0m";
        constexpr const char* RED       = "\033[31m";
        constexpr const char* GREEN     = "\033[32m";
        constexpr const char* YELLOW    = "\033[33m";
        constexpr const char* BLUE      = "\033[34m";
        constexpr const char* MAGENTA   = "\033[35m";
        constexpr const char* GRAY      = "\033[90m";
        constexpr const char* BRIGHT_RED = "\033[91m";
    }

    template <typename... Args>
    void log(LogLevel level, std::string_view fmt, Args&&... args)
    {
        if (level < global_log_min_level)
        {
            return;
        }

        const char* color = color_constants::RESET;
        const char* prefix{};
        switch (level) {
            case LogLevel::DEBUG:   color = color_constants::GRAY;          prefix = "[DEBUG]"; break;
            case LogLevel::INFO:    color = color_constants::GREEN;         prefix = "[INFO]";  break;
            case LogLevel::WARNING: color = color_constants::YELLOW;        prefix = "[WARN]";  break;
            case LogLevel::ERR:     color = color_constants::RED;           prefix = "[ERROR]"; break;
            case LogLevel::FATAL:   color = color_constants::BRIGHT_RED;    prefix = "[FATAL]"; break;
            default:                color = color_constants::MAGENTA;       prefix = "[UNKNOWN]"; break;
        }

        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);

        std::string msg = std::vformat(fmt, std::make_format_args(args...));

        std::cerr << color_constants::GRAY << '[' << std::put_time(std::localtime(&time_t), "%H:%M:%S") << "] "
                  << color << prefix << ' ' << color_constants::RESET
                  << msg << '\n';
    }

    template <typename... Args>
    void log_debug(std::string_view fmt, Args&&... args)
    {
        log(LogLevel::DEBUG, fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void log_info(std::string_view fmt, Args&&... args)
    {
        log(LogLevel::INFO, fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void log_warning(std::string_view fmt, Args&&... args)
    {
        log(LogLevel::WARNING, fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void log_error(std::string_view fmt, Args&&... args)
    {
        log(LogLevel::ERR, fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void log_fatal(std::string_view fmt, Args&&... args)
    {
        log(LogLevel::FATAL, fmt, std::forward<Args>(args)...);
    }
}
