#ifndef CJPROF_LOGGER_H
#define CJPROF_LOGGER_H

#include "spdlog/spdlog.h"
#include <cstring>

// Create logger aliases for backward compatibility
#define LOG_INFO(...) spdlog::info(__VA_ARGS__)
#define LOG_WARN(...) spdlog::warn(__VA_ARGS__)
#define LOG_ERROR(...) spdlog::error(__VA_ARGS__)
#define LOG_DEBUG(...) spdlog::debug(__VA_ARGS__)

namespace cjprof {

inline void initLogger() {
    // Set log pattern: [level] message
    spdlog::set_pattern("[%^%l%$] %v");

    // Set log level from environment variable or default to info
    auto logLevel = std::getenv("CJPROF_LOG_LEVEL");
    if (logLevel) {
        if (strcmp(logLevel, "debug") == 0) {
            spdlog::set_level(spdlog::level::debug);
        } else if (strcmp(logLevel, "warn") == 0) {
            spdlog::set_level(spdlog::level::warn);
        } else if (strcmp(logLevel, "error") == 0) {
            spdlog::set_level(spdlog::level::err);
        } else {
            spdlog::set_level(spdlog::level::info);
        }
    } else {
        spdlog::set_level(spdlog::level::info);
    }
}

} // namespace cjprof

#endif // CJPROF_LOGGER_H
