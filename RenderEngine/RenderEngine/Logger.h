#pragma once
//
// Logger.h -- minimal, dependency-free logging.
//
// The original code reported every failure (shader compile errors, texture
// load failures, file I/O errors) via std::cerr only. In a windowed Release
// build there is no console attached, so all of those failures were silent.
// This logger writes each line to:
//   1. RenderEngine_log.txt next to the executable (flushed immediately, so
//      the cause of a crash is on disk even if the process dies right after),
//   2. OutputDebugString on Windows (Visual Studio Output window / DebugView),
//   3. stderr for WARN/ERROR, for runs where a console is attached.
//
#include <string>
#include <fstream>
#include <mutex>
#include <chrono>
#include <ctime>
#include <iostream>
#include <sstream>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#endif

enum class LogLevel { Info, Warning, Error };

class Logger {
public:
    static Logger& instance() {
        static Logger logger;
        return logger;
    }

    void log(LogLevel level, const std::string& tag, const std::string& message) {
        std::lock_guard<std::mutex> lock(mutex_);
        const std::string line = formatLine(level, tag, message);

        if (file_.is_open()) {
            file_ << line << std::endl; // std::endl flushes
        }
#ifdef _WIN32
        OutputDebugStringA((line + "\n").c_str());
#endif
        if (level != LogLevel::Info) {
            std::cerr << line << std::endl;
        }
    }

private:
    Logger() {
        file_.open("RenderEngine_log.txt", std::ios::out | std::ios::app);
        if (file_.is_open()) file_ << "\n----- Session start -----\n";
    }
    ~Logger() {
        if (file_.is_open()) {
            file_ << "----- Session end -----\n";
            file_.close();
        }
    }
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    static const char* levelLabel(LogLevel level) {
        switch (level) {
            case LogLevel::Info:    return "INFO ";
            case LogLevel::Warning: return "WARN ";
            case LogLevel::Error:   return "ERROR";
        }
        return "?????";
    }

    static std::string timestamp() {
        auto now = std::chrono::system_clock::now();
        std::time_t t = std::chrono::system_clock::to_time_t(now);
        std::tm tmBuf{};
#ifdef _WIN32
        localtime_s(&tmBuf, &t);
#else
        localtime_r(&t, &tmBuf);
#endif
        char buf[32];
        std::strftime(buf, sizeof(buf), "%H:%M:%S", &tmBuf);
        return std::string(buf);
    }

    std::string formatLine(LogLevel level, const std::string& tag, const std::string& message) {
        std::ostringstream oss;
        oss << "[" << timestamp() << "] [" << levelLabel(level) << "] [" << tag << "] " << message;
        return oss.str();
    }

    std::ofstream file_;
    std::mutex mutex_;
};

#define LOG_INFO(tag, msg)  ::Logger::instance().log(LogLevel::Info,    tag, (msg))
#define LOG_WARN(tag, msg)  ::Logger::instance().log(LogLevel::Warning, tag, (msg))
#define LOG_ERROR(tag, msg) ::Logger::instance().log(LogLevel::Error,   tag, (msg))
