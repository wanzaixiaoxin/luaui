#pragma once

#include <string>
#include <memory>
#include <vector>
#include <fstream>
#include <mutex>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <iostream>

namespace luaui {
namespace utils {

// Log levels
enum class LogLevel {
    Debug = 0,
    Info = 1,
    Warning = 2,
    Error = 3,
    Fatal = 4
};

// Convert log level to string
inline const char* LogLevelToString(LogLevel level) {
    switch (level) {
        case LogLevel::Debug:   return "DEBUG";
        case LogLevel::Info:    return "INFO";
        case LogLevel::Warning: return "WARN";
        case LogLevel::Error:   return "ERROR";
        case LogLevel::Fatal:   return "FATAL";
        default:                return "UNKNOWN";
    }
}

// Base logger interface
class ILogger {
public:
    virtual ~ILogger() = default;
    
    // Core logging method
    virtual void Log(LogLevel level, const std::string& message) = 0;
    
    // Set minimum log level
    virtual void SetLevel(LogLevel level) = 0;
    virtual LogLevel GetLevel() const = 0;
    
    // Convenience methods
    void Debug(const std::string& msg) { Log(LogLevel::Debug, msg); }
    void Info(const std::string& msg) { Log(LogLevel::Info, msg); }
    void Warning(const std::string& msg) { Log(LogLevel::Warning, msg); }
    void Error(const std::string& msg) { Log(LogLevel::Error, msg); }
    void Fatal(const std::string& msg) { Log(LogLevel::Fatal, msg); }
    
    // Formatted logging (variadic template)
    template<typename... Args>
    void DebugF(const char* format, Args... args) {
        LogF(LogLevel::Debug, format, args...);
    }
    
    template<typename... Args>
    void InfoF(const char* format, Args... args) {
        LogF(LogLevel::Info, format, args...);
    }
    
    template<typename... Args>
    void WarningF(const char* format, Args... args) {
        LogF(LogLevel::Warning, format, args...);
    }
    
    template<typename... Args>
    void ErrorF(const char* format, Args... args) {
        LogF(LogLevel::Error, format, args...);
    }
    
    template<typename... Args>
    void FatalF(const char* format, Args... args) {
        LogF(LogLevel::Fatal, format, args...);
    }
    
protected:
    template<typename... Args>
    void LogF(LogLevel level, const char* format, Args... args) {
        char buffer[4096];
        snprintf(buffer, sizeof(buffer), format, args...);
        Log(level, std::string(buffer));
    }
};

using ILoggerPtr = std::shared_ptr<ILogger>;

// File logger implementation
class FileLogger : public ILogger {
public:
    explicit FileLogger(const std::string& filename);
    ~FileLogger() override;
    
    void Log(LogLevel level, const std::string& message) override;
    void SetLevel(LogLevel level) override { m_minLevel = level; }
    LogLevel GetLevel() const override { return m_minLevel; }
    
    // Set max file size in MB (will rotate)
    void SetMaxFileSize(size_t mb) { m_maxFileSize = mb * 1024 * 1024; }
    
    // Flush to disk
    void Flush();
    
private:
    void CheckRotation();
    std::string GetTimestamp();
    
    std::ofstream m_file;
    std::string m_filename;
    LogLevel m_minLevel = LogLevel::Debug;
    std::mutex m_mutex;
    size_t m_maxFileSize = 10 * 1024 * 1024; // 10MB default
};

// Console logger implementation
class ConsoleLogger : public ILogger {
public:
    ConsoleLogger(bool useStderr = false);
    
    void Log(LogLevel level, const std::string& message) override;
    void SetLevel(LogLevel level) override { m_minLevel = level; }
    LogLevel GetLevel() const override { return m_minLevel; }
    
    // Enable colored output (Windows only)
    void SetColored(bool colored) { m_colored = colored; }
    
private:
    std::string GetTimestamp();
    void SetConsoleColor(LogLevel level);
    void ResetConsoleColor();
    
    bool m_useStderr = false;
    LogLevel m_minLevel = LogLevel::Debug;
    std::mutex m_mutex;
    bool m_colored = true;
};

// Multi-logger (sends to multiple outputs)
class MultiLogger : public ILogger {
public:
    void AddLogger(ILoggerPtr logger);
    void RemoveLogger(ILoggerPtr logger);
    void ClearLoggers();
    
    void Log(LogLevel level, const std::string& message) override;
    void SetLevel(LogLevel level) override;
    LogLevel GetLevel() const override { return m_minLevel; }
    
private:
    std::vector<ILoggerPtr> m_loggers;
    LogLevel m_minLevel = LogLevel::Debug;
    std::mutex m_mutex;
};

// Global logger accessor
class Logger {
public:
    // Initialize with default console logger
    static void Initialize();
    
    // Initialize with file logger
    static void Initialize(const std::string& logFile);
    
    // Initialize with custom logger
    static void Initialize(ILoggerPtr logger);
    
    // Shutdown
    static void Shutdown();
    
    // Get global instance
    static ILoggerPtr Get();
    
    // Check if initialized
    static bool IsInitialized();
    
    // Convenience methods (redirect to global instance)
    static void Debug(const std::string& msg);
    static void Info(const std::string& msg);
    static void Warning(const std::string& msg);
    static void Error(const std::string& msg);
    static void Fatal(const std::string& msg);
    
    template<typename... Args>
    static void DebugF(const char* format, Args... args) {
        auto logger = Get();
        if (logger) logger->DebugF(format, args...);
    }
    
    template<typename... Args>
    static void InfoF(const char* format, Args... args) {
        auto logger = Get();
        if (logger) logger->InfoF(format, args...);
    }
    
    template<typename... Args>
    static void WarningF(const char* format, Args... args) {
        auto logger = Get();
        if (logger) logger->WarningF(format, args...);
    }
    
    template<typename... Args>
    static void ErrorF(const char* format, Args... args) {
        auto logger = Get();
        if (logger) logger->ErrorF(format, args...);
    }
    
    template<typename... Args>
    static void FatalF(const char* format, Args... args) {
        auto logger = Get();
        if (logger) logger->FatalF(format, args...);
    }
    
private:
    static ILoggerPtr s_instance;
    static std::mutex s_mutex;
};

// RAII log initializer for scope-based logging
class LogScope {
public:
    LogScope(ILoggerPtr logger, LogLevel level, const std::string& enterMsg, const std::string& exitMsg = "");
    ~LogScope();
    
private:
    ILoggerPtr m_logger;
    LogLevel m_level;
    std::string m_exitMsg;
};

} // namespace utils
} // namespace luaui
