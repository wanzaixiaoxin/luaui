#include "Logger.h"

#include <algorithm>

#ifdef _WIN32
#include <windows.h>
#endif

namespace luaui {
namespace utils {

// Static members
ILoggerPtr Logger::s_instance = nullptr;
std::mutex Logger::s_mutex;

// FileLogger implementation
FileLogger::FileLogger(const std::string& filename) 
    : m_filename(filename) {
    m_file.open(filename, std::ios::app);
    if (!m_file.is_open()) {
        throw std::runtime_error("Failed to open log file: " + filename);
    }
}

FileLogger::~FileLogger() {
    if (m_file.is_open()) {
        m_file.close();
    }
}

void FileLogger::Log(LogLevel level, const std::string& message) {
    if (level < m_minLevel) return;
    
    std::lock_guard<std::mutex> lock(m_mutex);
    CheckRotation();
    
    m_file << "[" << GetTimestamp() << "] "
           << "[" << LogLevelToString(level) << "] "
           << message << std::endl;
}

void FileLogger::Flush() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_file.is_open()) {
        m_file.flush();
    }
}

void FileLogger::CheckRotation() {
    // Simple check - if file too big, rename and start new
    auto pos = m_file.tellp();
    if (pos > static_cast<std::streamoff>(m_maxFileSize)) {
        m_file.close();
        
        // Rename old file
        std::string backupName = m_filename + ".old";
        std::rename(m_filename.c_str(), backupName.c_str());
        
        // Open new file
        m_file.open(m_filename, std::ios::out);
    }
}

std::string FileLogger::GetTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
    ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return ss.str();
}

// ConsoleLogger implementation
ConsoleLogger::ConsoleLogger(bool useStderr) 
    : m_useStderr(useStderr) {
#ifdef _WIN32
    // Enable ANSI colors on Windows 10+
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD mode = 0;
        if (GetConsoleMode(hOut, &mode)) {
            mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(hOut, mode);
        }
    }
#endif
}

void ConsoleLogger::Log(LogLevel level, const std::string& message) {
    if (level < m_minLevel) return;
    
    std::lock_guard<std::mutex> lock(m_mutex);
    
    std::ostream& out = m_useStderr ? std::cerr : std::cout;
    
    SetConsoleColor(level);
    out << "[" << GetTimestamp() << "] "
        << "[" << LogLevelToString(level) << "] "
        << message;
    ResetConsoleColor();
    out << std::endl;
}

std::string ConsoleLogger::GetTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%H:%M:%S");
    ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return ss.str();
}

void ConsoleLogger::SetConsoleColor(LogLevel level) {
#ifdef _WIN32
    if (!m_colored) return;
    
    HANDLE hConsole = GetStdHandle(m_useStderr ? STD_ERROR_HANDLE : STD_OUTPUT_HANDLE);
    if (hConsole == INVALID_HANDLE_VALUE) return;
    
    WORD color = 7; // Default gray
    switch (level) {
        case LogLevel::Debug:   color = 8; break;  // Dark gray
        case LogLevel::Info:    color = 7; break;  // Gray
        case LogLevel::Warning: color = 6; break;  // Yellow
        case LogLevel::Error:   color = 4; break;  // Red
        case LogLevel::Fatal:   color = 12; break; // Bright red
    }
    SetConsoleTextAttribute(hConsole, color);
#else
    if (!m_colored) return;
    
    const char* colorCode = "";
    switch (level) {
        case LogLevel::Debug:   colorCode = "\033[90m"; break; // Dark gray
        case LogLevel::Info:    colorCode = "\033[0m";  break; // Default
        case LogLevel::Warning: colorCode = "\033[33m"; break; // Yellow
        case LogLevel::Error:   colorCode = "\033[31m"; break; // Red
        case LogLevel::Fatal:   colorCode = "\033[91m"; break; // Bright red
    }
    std::ostream& out = m_useStderr ? std::cerr : std::cout;
    out << colorCode;
#endif
}

void ConsoleLogger::ResetConsoleColor() {
#ifdef _WIN32
    if (!m_colored) return;
    HANDLE hConsole = GetStdHandle(m_useStderr ? STD_ERROR_HANDLE : STD_OUTPUT_HANDLE);
    if (hConsole != INVALID_HANDLE_VALUE) {
        SetConsoleTextAttribute(hConsole, 7); // Reset to gray
    }
#else
    if (!m_colored) return;
    std::ostream& out = m_useStderr ? std::cerr : std::cout;
    out << "\033[0m";
#endif
}

// MultiLogger implementation
void MultiLogger::AddLogger(ILoggerPtr logger) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_loggers.push_back(logger);
}

void MultiLogger::RemoveLogger(ILoggerPtr logger) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = std::remove(m_loggers.begin(), m_loggers.end(), logger);
    m_loggers.erase(it, m_loggers.end());
}

void MultiLogger::ClearLoggers() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_loggers.clear();
}

void MultiLogger::Log(LogLevel level, const std::string& message) {
    if (level < m_minLevel) return;
    
    std::lock_guard<std::mutex> lock(m_mutex);
    for (size_t i = 0; i < m_loggers.size(); ++i) {
        if (m_loggers[i]) {
            m_loggers[i]->Log(level, message);
        }
    }
}

void MultiLogger::SetLevel(LogLevel level) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_minLevel = level;
    for (size_t i = 0; i < m_loggers.size(); ++i) {
        if (m_loggers[i]) {
            m_loggers[i]->SetLevel(level);
        }
    }
}

// Global Logger implementation
void Logger::Initialize() {
    std::lock_guard<std::mutex> lock(s_mutex);
    if (!s_instance) {
        s_instance = std::make_shared<ConsoleLogger>();
    }
}

void Logger::Initialize(const std::string& logFile) {
    std::lock_guard<std::mutex> lock(s_mutex);
    if (!s_instance) {
        s_instance = std::make_shared<FileLogger>(logFile);
    }
}

void Logger::Initialize(ILoggerPtr logger) {
    std::lock_guard<std::mutex> lock(s_mutex);
    if (!s_instance && logger) {
        s_instance = logger;
    }
}

void Logger::Shutdown() {
    std::lock_guard<std::mutex> lock(s_mutex);
    s_instance.reset();
}

ILoggerPtr Logger::Get() {
    std::lock_guard<std::mutex> lock(s_mutex);
    if (!s_instance) {
        // Auto-initialize with console logger
        s_instance = std::make_shared<ConsoleLogger>();
    }
    return s_instance;
}

bool Logger::IsInitialized() {
    std::lock_guard<std::mutex> lock(s_mutex);
    return s_instance != nullptr;
}

void Logger::Debug(const std::string& msg) {
    auto logger = Get();
    if (logger) logger->Debug(msg);
}

void Logger::Info(const std::string& msg) {
    auto logger = Get();
    if (logger) logger->Info(msg);
}

void Logger::Warning(const std::string& msg) {
    auto logger = Get();
    if (logger) logger->Warning(msg);
}

void Logger::Error(const std::string& msg) {
    auto logger = Get();
    if (logger) logger->Error(msg);
}

void Logger::Fatal(const std::string& msg) {
    auto logger = Get();
    if (logger) logger->Fatal(msg);
}

// LogScope implementation
LogScope::LogScope(ILoggerPtr logger, LogLevel level, const std::string& enterMsg, const std::string& exitMsg)
    : m_logger(logger), m_level(level), m_exitMsg(exitMsg) {
    if (m_logger) {
        m_logger->Log(level, "[ENTER] " + enterMsg);
    }
}

LogScope::~LogScope() {
    if (m_logger) {
        if (m_exitMsg.empty()) {
            m_logger->Log(m_level, "[EXIT]");
        } else {
            m_logger->Log(m_level, "[EXIT] " + m_exitMsg);
        }
    }
}

} // namespace utils
} // namespace luaui
