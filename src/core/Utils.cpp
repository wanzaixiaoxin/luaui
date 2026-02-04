/**
 * @file Utils.cpp
 * @brief 工具函数实现
 */

#include "core/Utils.h"
#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <sys/stat.h>
#include <windows.h>

namespace LuaUI {
namespace Utils {

// ========== StringUtils implementation ==========

std::string StringUtils::trim(const std::string& str) {
    return trimRight(trimLeft(str));
}

std::string StringUtils::trimLeft(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\n\r");
    return (start == std::string::npos) ? "" : str.substr(start);
}

std::string StringUtils::trimRight(const std::string& str) {
    size_t end = str.find_last_not_of(" \t\n\r");
    return (end == std::string::npos) ? "" : str.substr(0, end + 1);
}

std::vector<std::string> StringUtils::split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    
    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }
    
    return tokens;
}

std::string StringUtils::join(const std::vector<std::string>& strings, 
                              const std::string& delimiter) {
    if (strings.empty()) {
        return "";
    }
    
    std::ostringstream oss;
    for (size_t i = 0; i < strings.size(); ++i) {
        if (i > 0) {
            oss << delimiter;
        }
        oss << strings[i];
    }
    
    return oss.str();
}

std::string StringUtils::replace(const std::string& str, 
                                   const std::string& from, 
                                   const std::string& to) {
    std::string result = str;
    size_t pos = 0;
    
    while ((pos = result.find(from, pos)) != std::string::npos) {
        result.replace(pos, from.length(), to);
        pos += to.length();
    }
    
    return result;
}

std::string StringUtils::toLower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

std::string StringUtils::toUpper(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
}

bool StringUtils::startsWith(const std::string& str, const std::string& prefix) {
    if (str.length() < prefix.length()) {
        return false;
    }
    return str.substr(0, prefix.length()) == prefix;
}

bool StringUtils::endsWith(const std::string& str, const std::string& suffix) {
    if (str.length() < suffix.length()) {
        return false;
    }
    return str.substr(str.length() - suffix.length()) == suffix;
}

bool StringUtils::contains(const std::string& str, const std::string& substr) {
    return str.find(substr) != std::string::npos;
}

std::string StringUtils::format(const char* format, ...) {
    char buffer[4096];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    return std::string(buffer);
}

// ========== FileUtils implementation ==========

bool FileUtils::exists(const std::string& filepath) {
    struct stat buffer;
    return (stat(filepath.c_str(), &buffer) == 0);
}

std::string FileUtils::readFile(const std::string& filepath) {
    std::ifstream file(filepath.c_str(), std::ios::binary);
    if (!file.is_open()) {
        return "";
    }
    
    std::string content;
    file.seekg(0, std::ios::end);
    content.resize(file.tellg());
    file.seekg(0, std::ios::beg);
    file.read(&content[0], content.size());
    file.close();
    
    return content;
}

bool FileUtils::writeFile(const std::string& filepath, const std::string& content) {
    std::ofstream file(filepath.c_str(), std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    
    file.write(content.c_str(), content.size());
    file.close();
    
    return true;
}

bool FileUtils::appendFile(const std::string& filepath, const std::string& content) {
    std::ofstream file(filepath.c_str(), std::ios::app | std::ios::binary);
    if (!file.is_open()) {
        return false;
    }
    
    file.write(content.c_str(), content.size());
    file.close();
    
    return true;
}

long FileUtils::getFileSize(const std::string& filepath) {
    struct stat buffer;
    if (stat(filepath.c_str(), &buffer) != 0) {
        return -1;
    }
    return buffer.st_size;
}

std::string FileUtils::getFileExtension(const std::string& filepath) {
    size_t dotPos = filepath.find_last_of('.');
    if (dotPos == std::string::npos) {
        return "";
    }
    return filepath.substr(dotPos + 1);
}

std::string FileUtils::getFileName(const std::string& filepath) {
    size_t slashPos = filepath.find_last_of("/\\");
    if (slashPos == std::string::npos) {
        return filepath;
    }
    return filepath.substr(slashPos + 1);
}

std::string FileUtils::getDirectory(const std::string& filepath) {
    size_t slashPos = filepath.find_last_of("/\\");
    if (slashPos == std::string::npos) {
        return ".";
    }
    return filepath.substr(0, slashPos);
}

std::string FileUtils::getAbsolutePath(const std::string& filepath) {
    char buffer[MAX_PATH];
    GetFullPathNameA(filepath.c_str(), MAX_PATH, buffer, NULL);
    return std::string(buffer);
}

bool FileUtils::createDirectory(const std::string& dirpath) {
    // Windows API创建目录
    return CreateDirectoryA(dirpath.c_str(), NULL) || GetLastError() == ERROR_ALREADY_EXISTS;
}

bool FileUtils::removeFile(const std::string& filepath) {
    return DeleteFileA(filepath.c_str()) != 0;
}

// ========== EncodingUtils implementation ==========

std::string EncodingUtils::utf8ToAnsi(const std::string& utf8) {
    // 转换为UTF-16
    int len = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, NULL, 0);
    if (len == 0) {
        return "";
    }
    
    std::wstring utf16(len, 0);
    MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, &utf16[0], len);
    
    // 转换为ANSI
    len = WideCharToMultiByte(CP_ACP, 0, utf16.c_str(), -1, NULL, 0, NULL, NULL);
    if (len == 0) {
        return "";
    }
    
    std::string ansi(len, 0);
    WideCharToMultiByte(CP_ACP, 0, utf16.c_str(), -1, &ansi[0], len, NULL, NULL);
    
    return ansi;
}

std::string EncodingUtils::ansiToUtf8(const std::string& ansi) {
    // 转换为UTF-16
    int len = MultiByteToWideChar(CP_ACP, 0, ansi.c_str(), -1, NULL, 0);
    if (len == 0) {
        return "";
    }
    
    std::wstring utf16(len, 0);
    MultiByteToWideChar(CP_ACP, 0, ansi.c_str(), -1, &utf16[0], len);
    
    // 转换为UTF-8
    len = WideCharToMultiByte(CP_UTF8, 0, utf16.c_str(), -1, NULL, 0, NULL, NULL);
    if (len == 0) {
        return "";
    }
    
    std::string utf8(len, 0);
    WideCharToMultiByte(CP_UTF8, 0, utf16.c_str(), -1, &utf8[0], len, NULL, NULL);
    
    return utf8;
}

std::string EncodingUtils::utf16ToUtf8(const std::wstring& utf16) {
    int len = WideCharToMultiByte(CP_UTF8, 0, utf16.c_str(), -1, NULL, 0, NULL, NULL);
    if (len == 0) {
        return "";
    }
    
    std::string utf8(len, 0);
    WideCharToMultiByte(CP_UTF8, 0, utf16.c_str(), -1, &utf8[0], len, NULL, NULL);
    
    return utf8;
}

std::wstring EncodingUtils::utf8ToUtf16(const std::string& utf8) {
    int len = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, NULL, 0);
    if (len == 0) {
        return L"";
    }
    
    std::wstring utf16(len, 0);
    MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, &utf16[0], len);
    
    return utf16;
}

// ========== NumberUtils implementation ==========

int NumberUtils::toInt(const std::string& str, int defaultValue) {
    std::istringstream iss(str);
    int value;
    if (!(iss >> value)) {
        return defaultValue;
    }
    return value;
}

long long NumberUtils::toLong(const std::string& str, long long defaultValue) {
    std::istringstream iss(str);
    long long value;
    if (!(iss >> value)) {
        return defaultValue;
    }
    return value;
}

double NumberUtils::toDouble(const std::string& str, double defaultValue) {
    std::istringstream iss(str);
    double value;
    if (!(iss >> value)) {
        return defaultValue;
    }
    return value;
}

std::string NumberUtils::toString(int value) {
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

std::string NumberUtils::toString(double value, int precision) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(precision) << value;
    return oss.str();
}

} // namespace Utils
} // namespace LuaUI
