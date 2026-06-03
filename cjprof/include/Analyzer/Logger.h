// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

#ifndef CJPROF_LOGGER_H
#define CJPROF_LOGGER_H

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>

namespace cjprof {

enum class LogLevel { Debug = 0, Info = 1, Warn = 2, Error = 3 };

inline LogLevel g_logLevel = LogLevel::Info;
inline std::mutex g_logMutex;

inline void replaceFirstPlaceholder(std::string& s, const std::string& val)
{
    size_t pos = s.find("{}");
    if (pos != std::string::npos) {
        s.replace(pos, 2, val);
    }
}

inline std::string formatString(const std::string& fmt)
{
    return fmt;
}

template <typename T, typename... Args>
std::string formatString(const std::string& fmt, T&& val, Args&&... args) {
    std::string result = fmt;
    std::ostringstream oss;
    oss << std::forward<T>(val);
    replaceFirstPlaceholder(result, oss.str());
    if constexpr (sizeof...(args) > 0) {
        return formatString(result, std::forward<Args>(args)...);
    }
    return result;
}

inline const char* levelString(LogLevel level)
{
    switch (level) {
        case LogLevel::Error: return "error";
        case LogLevel::Warn:  return "warn";
        case LogLevel::Info:  return "info";
        case LogLevel::Debug: return "debug";
    }
    return "";
}

inline void logMessage(LogLevel level, const std::string& msg)
{
    if (static_cast<int>(level) < static_cast<int>(g_logLevel)) {
        return;
    }
    std::lock_guard<std::mutex> lock(g_logMutex);
    std::cout << "[" << levelString(level) << "] " << msg << std::endl;
}

inline void initLogger()
{
    auto logLevel = std::getenv("CJPROF_LOG_LEVEL");
    if (logLevel) {
        if (strcmp(logLevel, "debug") == 0) {
            g_logLevel = LogLevel::Debug;
        } else if (strcmp(logLevel, "warn") == 0) {
            g_logLevel = LogLevel::Warn;
        } else if (strcmp(logLevel, "error") == 0) {
            g_logLevel = LogLevel::Error;
        } else {
            g_logLevel = LogLevel::Info;
        }
    } else {
        g_logLevel = LogLevel::Info;
    }
}

} // namespace cjprof

#define LOG_INFO(...)  cjprof::logMessage(cjprof::LogLevel::Info,  cjprof::formatString(__VA_ARGS__))
#define LOG_WARN(...)  cjprof::logMessage(cjprof::LogLevel::Warn,  cjprof::formatString(__VA_ARGS__))
#define LOG_ERROR(...) cjprof::logMessage(cjprof::LogLevel::Error, cjprof::formatString(__VA_ARGS__))
#define LOG_DEBUG(...) cjprof::logMessage(cjprof::LogLevel::Debug, cjprof::formatString(__VA_ARGS__))

#endif // CJPROF_LOGGER_H
