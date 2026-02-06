/**
 * @file FileUtils.cpp
 * @brief File utility implementations
 */

#include "core/Utils.h"
#include <fstream>
#include <sstream>

namespace LuaUI {
namespace Utils {

bool FileUtils::exists(const std::string& filepath) {
    std::ifstream file(filepath);
    return file.good();
}

std::string FileUtils::readFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) return "";
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

bool FileUtils::writeFile(const std::string& filepath, const std::string& content) {
    std::ofstream file(filepath);
    if (!file.is_open()) return false;
    file << content;
    return file.good();
}

bool FileUtils::appendFile(const std::string& filepath, const std::string& content) {
    std::ofstream file(filepath, std::ios::app);
    if (!file.is_open()) return false;
    file << content;
    return file.good();
}

long FileUtils::getFileSize(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::ate | std::ios::binary);
    if (!file.is_open()) return -1;
    return static_cast<long>(file.tellg());
}

std::string FileUtils::getFileExtension(const std::string& filepath) {
    size_t pos = filepath.find_last_of('.');
    if (pos == std::string::npos) return "";
    return filepath.substr(pos + 1);
}

std::string FileUtils::getFileName(const std::string& filepath) {
    size_t pos = filepath.find_last_of("/\\");
    if (pos == std::string::npos) return filepath;
    return filepath.substr(pos + 1);
}

std::string FileUtils::getDirectory(const std::string& filepath) {
    size_t pos = filepath.find_last_of("/\\");
    if (pos == std::string::npos) return ".";
    return filepath.substr(0, pos);
}

std::string FileUtils::getAbsolutePath(const std::string& filepath) {
    // 简单实现，返回原路径
    return filepath;
}

bool FileUtils::createDirectory(const std::string& /*dirpath*/) {
    // 简单实现，返回false（Windows需要特殊处理）
    return false;
}

bool FileUtils::removeFile(const std::string& filepath) {
    return std::remove(filepath.c_str()) == 0;
}

} // namespace utils
} // namespace luaui
