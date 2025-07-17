/*
    Lightweight header-only logger

    MIT License

    Copyright (c) 2025 Antonio Espinosa Garcia

*/
#pragma once

#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>

enum class LogLevel
{
    Info  = 0,
    Warn  = 1,
    Error = 2,
    None  = 3
};

class Logger
{
  public:
    static void init(LogLevel level = LogLevel::Info, const std::string& file = "") {
        instance().logLevel = level;
        if (!file.empty())
        {
            instance().logFile.open(file, std::ios::out | std::ios::app);
        }
    }

    static void shutdown() {
        if (instance().logFile.is_open())
        {
            instance().logFile.close();
        }
    }

    static void log_with_context(LogLevel level, const std::string& msg, const char* file, int line, const char* func) {
        Logger& logger = instance();
        if (level < logger.logLevel)
            return;

        std::lock_guard<std::mutex> lock(logger.mtx);

        std::string lineStr = line > 0 ? std::to_string(line) : "";
        std::ostringstream out;
        out << "[" << get_timestamp() << "] ";
        out << level_color(level) << "[" << level_to_string(level) << "]" << reset_color() << " ";
        out << "[" << file << ":" <<  lineStr << " (" << func << ")] ";
        out << msg << "\n";

        std::string formatted = out.str();
        std::cout << formatted;

        if (logger.logFile.is_open())
        {
            logger.logFile << formatted;
            logger.logFile.flush();
        }
    }
    static void log(LogLevel level, const std::string& msg) {
        Logger& logger = instance();
        if (level < logger.logLevel)
            return;

        std::lock_guard<std::mutex> lock(logger.mtx);

        std::ostringstream out;
        out << "[" << get_timestamp() << "] ";
        out << level_color(level) << "[" << level_to_string(level) << "]" << reset_color() << " ";
        out << msg << "\n";

        std::string formatted = out.str();
        std::cout << formatted;

        if (logger.logFile.is_open())
        {
            logger.logFile << formatted;
            logger.logFile.flush();
        }
    }

    static std::string format_with_tag(const std::string& tag, const char* color, const std::string& message) {
    std::ostringstream oss;
    oss << color << tag << reset_color() << " " << message;
    return oss.str();
}

  private:
    LogLevel      logLevel = LogLevel::Info;
    std::ofstream logFile;
    std::mutex    mtx;

    static Logger& instance() {
        static Logger instance;
        return instance;
    }

    static std::string get_timestamp() {
        auto               t  = std::time(nullptr);
        auto               tm = *std::localtime(&t);
        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
        return oss.str();
    }

    static const char* level_to_string(LogLevel level) {
        switch (level)
        {
        case LogLevel::Info:
            return "INFO";
        case LogLevel::Warn:
            return "WARN";
        case LogLevel::Error:
            return "ERROR";
        default:
            return "UNKNOWN";
        }
    }

    static const char* level_color(LogLevel level) {
        switch (level)
        {
        case LogLevel::Info:
            return "\033[36m"; // Cyan
        case LogLevel::Warn:
            return "\033[33m"; // Yellow
        case LogLevel::Error:
            return "\033[31m"; // Red
        default:
            return "\033[0m"; // Reset
        }
    }

    static const char* reset_color() {
        return "\033[0m";
    }

    
};

#define LOG_DEBUG(msg) Logger::log(LogLevel::Info, msg)
#define LOG_WARN(msg) Logger::log_with_context(LogLevel::Warn, msg, __FILE__, __LINE__, __func__)
#define LOG_ERROR(msg) Logger::log_with_context(LogLevel::Error, msg, __FILE__, __LINE__, __func__)
