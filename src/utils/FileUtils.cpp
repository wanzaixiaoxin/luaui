/**
 * @file FileUtils.cpp
 * @brief File utility implementations
 */

#include "core/Utils.h"
#include <fstream>
#include <sstream>

namespace luaui {
namespace utils {

bool FileUtils::exists(const std::string& path) {
    std::ifstream file(path);
    return file.good();
}

std::string FileUtils::readAllText(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) return "";
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

bool FileUtils::writeAllText(const std::string& path, const std::string& content) {
    std::ofstream file(path);
    if (!file.is_open()) return false;
    file << content;
    return file.good();
}

std::string FileUtils::getDirectory(const std::string& path) {
    size_t pos = path.find_last_of("/\\");
    if (pos == std::string::npos) return "";
    return path.substr(0, pos);
}

std::string FileUtils::getFileName(const std::string& path) {
    size_t pos = path.find_last_of("/\\");
    if (pos == std::string::npos) return path;
    return path.substr(pos + 1);
}

std::string FileUtils::getExtension(const std::string& path) {
    std::string filename = getFileName(path);
    size_t pos = filename.find_last_of('.');
    if (pos == std::string::npos) return "";
    return filename.substr(pos);
}

std::string FileUtils::combine(const std::string& path1, const std::string& path2) {
    if (path1.empty()) return path2;
    if (path2.empty()) return path1;
    char last = path1[path1.size() - 1];
    if (last == '/' || last == '\\') {
        return path1 + path2;
    }
    return path1 + "/" + path2;
}

} // namespace utils
} // namespace luaui
