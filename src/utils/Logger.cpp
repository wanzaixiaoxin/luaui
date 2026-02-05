/**
 * @file Logger.cpp
 * @brief 日志模块实现
 */

#include "utils/Logger.h"
#include <iostream>
#include <algorithm>
#include <sys/stat.h>
#include <windows.h>
#include <fstream>

namespace LuaUI {
namespace Utils {

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

Logger::Logger()
    : m_initialized(false)
    , m_running(false)
    , m_level(LogLevel::LevelInfo)
    , m_consoleEnabled(true)
    , m_fileEnabled(false)
{
}

Logger::~Logger() {
    shutdown();
}

void Logger::initialize() {
    if (m_initialized.load()) {
        return;
    }

    m_running.store(true);
    m_workerThread = std::thread(&Logger::workerThread, this);
    m_initialized.store(true);

    info("[Logger] Logging system initialized");
}

void Logger::shutdown() {
    if (!m_initialized.load()) {
        return;
    }

    info("[Logger] Logging system shutting down...");
    m_running.store(false);
    m_queueCV.notify_all();

    if (m_workerThread.joinable()) {
        m_workerThread.join();
    }

    if (m_file.is_open()) {
        m_file.close();
    }

    m_initialized.store(false);
}

void Logger::setLevel(LogLevel level) {
    m_level.store(level);
    std::string levelStr = levelToString(level);
    info("[Logger] Log level set to " + levelStr);
}

LogLevel Logger::getLevel() const {
    return m_level.load();
}

void Logger::setCategory(const std::string& category) {
    m_category = category;
}

std::string Logger::getCategory() const {
    return m_category;
}

void Logger::enableConsole(bool enable) {
    m_consoleEnabled.store(enable);
}

bool Logger::isConsoleEnabled() const {
    return m_consoleEnabled.load();
}

bool fileExists(const std::string& filepath) {
    struct stat buffer;
    return (stat(filepath.c_str(), &buffer) == 0);
}

bool Logger::enableFile(const std::string& filepath, size_t maxSize, int maxFiles) {
    std::lock_guard<std::mutex> lock(m_queueMutex);

    m_filepath = filepath;
    m_maxSize = maxSize;
    m_maxFiles = maxFiles;

    std::string dir;
    size_t lastSlash = filepath.find_last_of("/\\");
    if (lastSlash != std::string::npos) {
        dir = filepath.substr(0, lastSlash);
        CreateDirectoryA(dir.c_str(), NULL);
    }

    m_file.open(filepath.c_str(), std::ios::app | std::ios::binary);
    if (!m_file.is_open()) {
        std::cerr << "[Logger] Failed to open log file: " << filepath << std::endl;
        return false;
    }

    m_fileEnabled.store(true);
    info("[Logger] File logging enabled: " + filepath);
    return true;
}

void Logger::disableFile() {
    std::lock_guard<std::mutex> lock(m_queueMutex);

    if (m_file.is_open()) {
        m_file.close();
    }

    m_fileEnabled.store(false);
    info("[Logger] File logging disabled");
}

bool Logger::isFileEnabled() const {
    return m_fileEnabled.load();
}

void Logger::debug(const std::string& message) {
    log(LogLevel::LevelDebug, message);
}

void Logger::info(const std::string& message) {
    log(LogLevel::LevelInfo, message);
}

void Logger::warn(const std::string& message) {
    log(LogLevel::LevelWarn, message);
}

void Logger::error(const std::string& message) {
    log(LogLevel::LevelError, message);
}

void Logger::fatal(const std::string& message) {
    log(LogLevel::LevelFatal, message);
}

void Logger::log(LogLevel level, const std::string& message) {
    if (!m_initialized.load()) {
        // Logger 未初始化时，直接输出到控制台
        if (level == LogLevel::LevelError || level == LogLevel::LevelFatal) {
            std::cerr << "[Logger-not-initialized] " << levelToString(level) 
                      << " [" << m_category << "] " << message << std::endl;
        } else if (level >= LogLevel::LevelInfo) {
            std::cout << "[Logger-not-initialized] " << levelToString(level) 
                      << " [" << m_category << "] " << message << std::endl;
        }
        return;
    }

    if (static_cast<int>(level) < static_cast<int>(m_level.load())) {
        return;
    }

    std::string formatted = formatMessage(level, m_category, message);

    {
        std::unique_lock<std::mutex> lock(m_queueMutex);
        if (m_queue.size() >= MAX_QUEUE_SIZE) {
            if (level == LogLevel::LevelDebug) {
                return;
            }
            m_queueCV.wait(lock, [this]() {
                return m_queue.size() < MAX_QUEUE_SIZE || !m_running.load();
            });
        }

        if (!m_running.load() && m_queue.empty()) {
            return;
        }

        m_queue.push(LogMessage(formatted, level, m_category));
    }

    m_queueCV.notify_one();
}

std::string Logger::formatMessage(LogLevel level, const std::string& category, const std::string& message) {
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;

    std::tm tm_buf;
#ifdef _WIN32
    localtime_s(&tm_buf, &time_t_now);
#else
    localtime_r(&time_t_now, &tm_buf);
#endif

    std::ostringstream oss;
    oss << std::put_time(&tm_buf, "%Y-%m-%d %H:%M:%S")
        << "." << std::setfill('0') << std::setw(3) << ms.count()
        << " " << std::setfill(' ') << std::setw(5) << levelToString(level)
        << " [" << category << "] " << message;

    return oss.str();
}

void Logger::workerThread() {
    while (m_running.load() || !m_queue.empty()) {
        LogMessage msg;
        bool hasMessage = false;

        {
            std::unique_lock<std::mutex> lock(m_queueMutex);
            m_queueCV.wait(lock, [this]() {
                return !m_queue.empty() || !m_running.load();
            });

            if (!m_queue.empty()) {
                msg = std::move(m_queue.front());
                m_queue.pop();
                hasMessage = true;
            }
        }

        if (hasMessage) {
            processMessage(msg);
        } else if (!m_running.load()) {
            break;
        }
    }
}

void Logger::processMessage(const LogMessage& msg) {
    if (m_consoleEnabled.load()) {
        writeToConsole(msg);
    }

    if (m_fileEnabled.load()) {
        writeToFile(msg);
    }
}

void Logger::writeToConsole(const LogMessage& msg) {
    std::lock_guard<std::mutex> lock(m_queueMutex);

    if (msg.level == LogLevel::LevelError || msg.level == LogLevel::LevelFatal) {
        std::cerr << msg.formatted << std::endl;
    } else {
        std::cout << msg.formatted << std::endl;
    }
}

void Logger::writeToFile(const LogMessage& msg) {
    if (!m_file.is_open()) {
        return;
    }

    std::lock_guard<std::mutex> lock(m_queueMutex);

    if (m_file.tellp() >= static_cast<std::streampos>(m_maxSize)) {
        rotateFile();
    }

    m_file << msg.formatted << std::endl;
    m_file.flush();
}

void Logger::rotateFile() {
    if (m_file.is_open()) {
        m_file.close();
    }

    for (int i = m_maxFiles - 1; i >= 0; --i) {
        char oldName[512];
        char newName[512];

        if (i == 0) {
            snprintf(oldName, sizeof(oldName), "%s", m_filepath.c_str());
            snprintf(newName, sizeof(newName), "%s.1", m_filepath.c_str());
        } else {
            snprintf(oldName, sizeof(oldName), "%s.%d", m_filepath.c_str(), i);
            snprintf(newName, sizeof(newName), "%s.%d", m_filepath.c_str(), i + 1);
        }

        if (fileExists(oldName)) {
            if (fileExists(newName)) {
                remove(newName);
            }
            rename(oldName, newName);
        }
    }

    m_file.open(m_filepath.c_str(), std::ios::out | std::ios::trunc | std::ios::binary);
}

std::string Logger::levelToString(LogLevel level) {
    switch (level) {
        case LogLevel::LevelDebug:   return "DEBUG";
        case LogLevel::LevelInfo:    return "INFO";
        case LogLevel::LevelWarn:    return "WARN";
        case LogLevel::LevelError:   return "ERROR";
        case LogLevel::LevelFatal:   return "FATAL";
        default:                      return "UNKNOWN";
    }
}

LogLevel Logger::stringToLevel(const std::string& str) {
    std::string upper = str;
    std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);

    if (upper == "DEBUG")   return LogLevel::LevelDebug;
    if (upper == "INFO")    return LogLevel::LevelInfo;
    if (upper == "WARN")    return LogLevel::LevelWarn;
    if (upper == "ERROR")   return LogLevel::LevelError;
    if (upper == "FATAL")   return LogLevel::LevelFatal;
    return LogLevel::LevelInfo;
}

} // namespace Utils
} // namespace LuaUI
