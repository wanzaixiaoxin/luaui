#include "StringUtils.h"
#include <sstream>

namespace luaui {
namespace utils {

std::wstring StringUtils::Utf8ToWString(const char* utf8Str) {
    if (!utf8Str || !*utf8Str) {
        return std::wstring();
    }
    
    int len = MultiByteToWideChar(CP_UTF8, 0, utf8Str, -1, nullptr, 0);
    if (len <= 0) {
        return std::wstring();
    }
    
    std::wstring result(len - 1, 0);
    MultiByteToWideChar(CP_UTF8, 0, utf8Str, -1, &result[0], len);
    return result;
}

std::wstring StringUtils::Utf8ToWString(const std::string& utf8Str) {
    return Utf8ToWString(utf8Str.c_str());
}

std::string StringUtils::WStringToUtf8(const wchar_t* wstr) {
    if (!wstr || !*wstr) {
        return std::string();
    }
    
    int len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, nullptr, 0, nullptr, nullptr);
    if (len <= 0) {
        return std::string();
    }
    
    std::string result(len - 1, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr, -1, &result[0], len, nullptr, nullptr);
    return result;
}

std::string StringUtils::WStringToUtf8(const std::wstring& wstr) {
    return WStringToUtf8(wstr.c_str());
}

std::string StringUtils::Trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == std::string::npos) {
        return "";
    }
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, last - first + 1);
}

std::vector<std::string> StringUtils::Split(const std::string& str, char delimiter) {
    std::vector<std::string> parts;
    std::string::size_type start = 0;
    std::string::size_type end = str.find(delimiter);
    
    while (end != std::string::npos) {
        std::string part = str.substr(start, end - start);
        if (!part.empty()) {
            parts.push_back(part);
        }
        start = end + 1;
        end = str.find(delimiter, start);
    }
    
    std::string lastPart = str.substr(start);
    if (!lastPart.empty()) {
        parts.push_back(lastPart);
    }
    
    return parts;
}

std::vector<std::string> StringUtils::SplitPath(const std::string& path) {
    return Split(path, '.');
}

} // namespace utils
} // namespace luaui
