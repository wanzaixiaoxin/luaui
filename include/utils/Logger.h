/**
 * @file Logger.h
 * @brief 日志模块
 * @details 提供线程安全、异步日志功能，支持控制台和文件输出
 * @version 1.0.0
 */

#ifndef LUAUI_LOGGER_H
#define LUAUI_LOGGER_H

#include <string>
#include <queue>
#include <mutex>
#include <thread>
#include <atomic>
#include <memory>
#include <fstream>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace LuaUI {
namespace Utils {

/**
 * @brief 日志级别
 */
enum class LogLevel {
    LevelDebug,   ///< 调试信息
    LevelInfo,    ///< 普通信息
    LevelWarn,    ///< 警告
    LevelError,   ///< 错误
    LevelFatal    ///< 致命错误
};

/**
 * @brief 日志消息结构
 */
struct LogMessage {
    std::string formatted;   ///< 格式化后的日志字符串
    LogLevel level;           ///< 日志级别
    std::string category;     ///< 日志分类

    LogMessage() : level(LogLevel::LevelInfo) {}
    LogMessage(const std::string& f, LogLevel l, const std::string& c)
        : formatted(f), level(l), category(c) {}
};

/**
 * @brief 日志类
 * @details 提供异步线程安全日志功能
 */
class Logger {
public:
    static Logger& getInstance();

    void initialize();
    void shutdown();

    void setLevel(LogLevel level);
    LogLevel getLevel() const;

    void setCategory(const std::string& category);
    std::string getCategory() const;

    void enableConsole(bool enable);
    bool isConsoleEnabled() const;

    bool enableFile(const std::string& filepath, size_t maxSize = 1024 * 1024, int maxFiles = 3);
    void disableFile();
    bool isFileEnabled() const;

    void debug(const std::string& message);
    void info(const std::string& message);
    void warn(const std::string& message);
    void error(const std::string& message);
    void fatal(const std::string& message);

    void log(LogLevel level, const std::string& message);

    static std::string levelToString(LogLevel level);
    static LogLevel stringToLevel(const std::string& str);

private:
    Logger();
    ~Logger();

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    Logger(Logger&&) = delete;
    Logger& operator=(Logger&&) = delete;

    void workerThread();
    void processMessage(const LogMessage& msg);
    void writeToConsole(const LogMessage& msg);
    void writeToFile(const LogMessage& msg);
    void rotateFile();
    std::string formatMessage(LogLevel level, const std::string& category, const std::string& message);

    std::atomic<bool> m_initialized{false};
    std::atomic<bool> m_running{false};
    std::atomic<LogLevel> m_level{LogLevel::LevelInfo};
    std::string m_category{"App"};

    std::atomic<bool> m_consoleEnabled{true};

    std::atomic<bool> m_fileEnabled{false};
    std::string m_filepath;
    size_t m_maxSize{1024 * 1024};
    int m_maxFiles{3};
    std::ofstream m_file;

    std::queue<LogMessage> m_queue;
    std::mutex m_queueMutex;
    std::condition_variable m_queueCV;
    std::thread m_workerThread;

    static constexpr size_t MAX_QUEUE_SIZE = 1024;
};

inline Logger& logger() {
    return Logger::getInstance();
}

/**
 * @brief 流式日志辅助类
 * @details 支持使用 << 运算符构建日志消息
 */
class LogStream {
public:
    LogStream(LogLevel level, const std::string& category = "App")
        : m_level(level), m_category(category) {}

    ~LogStream() {
        Logger::getInstance().log(m_level, m_oss.str());
    }

    template<typename T>
    LogStream& operator<<(const T& val) {
        m_oss << val;
        return *this;
    }

private:
    LogLevel m_level;
    std::string m_category;
    std::ostringstream m_oss;
};

#define LOG_DEBUG(msg)    ::LuaUI::Utils::Logger::getInstance().debug(msg)
#define LOG_INFO(msg)     ::LuaUI::Utils::Logger::getInstance().info(msg)
#define LOG_WARN(msg)     ::LuaUI::Utils::Logger::getInstance().warn(msg)
#define LOG_ERROR(msg)    ::LuaUI::Utils::Logger::getInstance().error(msg)
#define LOG_FATAL(msg)    ::LuaUI::Utils::Logger::getInstance().fatal(msg)

// 流式日志宏（推荐使用）
#define LOG_S_DEBUG()    ::LuaUI::Utils::LogStream(::LuaUI::Utils::LogLevel::LevelDebug, ::LuaUI::Utils::Logger::getInstance().getCategory())
#define LOG_S_INFO()     ::LuaUI::Utils::LogStream(::LuaUI::Utils::LogLevel::LevelInfo, ::LuaUI::Utils::Logger::getInstance().getCategory())
#define LOG_S_WARN()     ::LuaUI::Utils::LogStream(::LuaUI::Utils::LogLevel::LevelWarn, ::LuaUI::Utils::Logger::getInstance().getCategory())
#define LOG_S_ERROR()    ::LuaUI::Utils::LogStream(::LuaUI::Utils::LogLevel::LevelError, ::LuaUI::Utils::Logger::getInstance().getCategory())
#define LOG_S_FATAL()    ::LuaUI::Utils::LogStream(::LuaUI::Utils::LogLevel::LevelFatal, ::LuaUI::Utils::Logger::getInstance().getCategory())

// 便捷宏定义
#define LOG_DEBUG_CAT(category, msg) do { \
    auto& logger = ::LuaUI::Utils::Logger::getInstance(); \
    logger.setCategory(category); \
    logger.debug(msg); \
} while(0)

#define LOG_INFO_CAT(category, msg) do { \
    auto& logger = ::LuaUI::Utils::Logger::getInstance(); \
    logger.setCategory(category); \
    logger.info(msg); \
} while(0)

#define LOG_WARN_CAT(category, msg) do { \
    auto& logger = ::LuaUI::Utils::Logger::getInstance(); \
    logger.setCategory(category); \
    logger.warn(msg); \
} while(0)

#define LOG_ERROR_CAT(category, msg) do { \
    auto& logger = ::LuaUI::Utils::Logger::getInstance(); \
    logger.setCategory(category); \
    logger.error(msg); \
} while(0)

// 流式分类日志宏（推荐使用）
#define LOG_S_DEBUG_CAT(category)    ::LuaUI::Utils::LogStream(::LuaUI::Utils::LogLevel::LevelDebug, category)
#define LOG_S_INFO_CAT(category)     ::LuaUI::Utils::LogStream(::LuaUI::Utils::LogLevel::LevelInfo, category)
#define LOG_S_WARN_CAT(category)     ::LuaUI::Utils::LogStream(::LuaUI::Utils::LogLevel::LevelWarn, category)
#define LOG_S_ERROR_CAT(category)    ::LuaUI::Utils::LogStream(::LuaUI::Utils::LogLevel::LevelError, category)
#define LOG_S_FATAL_CAT(category)    ::LuaUI::Utils::LogStream(::LuaUI::Utils::LogLevel::LevelFatal, category)

// 格式化日志宏
#define LOG_FMT_DEBUG(fmt, ...) do { \
    char buf[512]; \
    snprintf(buf, sizeof(buf), fmt, ##__VA_ARGS__); \
    LOG_DEBUG(buf); \
} while(0)

#define LOG_FMT_INFO(fmt, ...) do { \
    char buf[512]; \
    snprintf(buf, sizeof(buf), fmt, ##__VA_ARGS__); \
    LOG_INFO(buf); \
} while(0)

#define LOG_FMT_WARN(fmt, ...) do { \
    char buf[512]; \
    snprintf(buf, sizeof(buf), fmt, ##__VA_ARGS__); \
    LOG_WARN(buf); \
} while(0)

#define LOG_FMT_ERROR(fmt, ...) do { \
    char buf[512]; \
    snprintf(buf, sizeof(buf), fmt, ##__VA_ARGS__); \
    LOG_ERROR(buf); \
} while(0)

// 临时调试宏（可通过定义 LUAU_DEBUG 启用）
#ifdef LUAU_DEBUG
#define DBG_OUT(msg) LOG_DEBUG(msg)
#define DBG_FMT(fmt, ...) LOG_FMT_DEBUG(fmt, ##__VA_ARGS__)
#else
#define DBG_OUT(msg) 
#define DBG_FMT(fmt, ...) 
#endif

} // namespace Utils
} // namespace LuaUI

#endif // LUAUI_LOGGER_H
