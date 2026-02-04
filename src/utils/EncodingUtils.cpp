/**
 * @file EncodingUtils.cpp
 * @brief Encoding utility implementations
 */

#include "core/Utils.h"
#include <windows.h>

namespace luaui {
namespace utils {

std::wstring EncodingUtils::utf8ToWide(const std::string& utf8) {
    if (utf8.empty()) return std::wstring();
    int size = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, NULL, 0);
    if (size == 0) return std::wstring();
    std::wstring result(size - 1, 0);
    MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, &result[0], size);
    return result;
}

std::string EncodingUtils::wideToUtf8(const std::wstring& wide) {
    if (wide.empty()) return std::string();
    int size = WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), -1, NULL, 0, NULL, NULL);
    if (size == 0) return std::string();
    std::string result(size - 1, 0);
    WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), -1, &result[0], size, NULL, NULL);
    return result;
}

std::wstring EncodingUtils::ansiToWide(const std::string& ansi) {
    if (ansi.empty()) return std::wstring();
    int size = MultiByteToWideChar(CP_ACP, 0, ansi.c_str(), -1, NULL, 0);
    if (size == 0) return std::wstring();
    std::wstring result(size - 1, 0);
    MultiByteToWideChar(CP_ACP, 0, ansi.c_str(), -1, &result[0], size);
    return result;
}

std::string EncodingUtils::wideToAnsi(const std::wstring& wide) {
    if (wide.empty()) return std::string();
    int size = WideCharToMultiByte(CP_ACP, 0, wide.c_str(), -1, NULL, 0, NULL, NULL);
    if (size == 0) return std::string();
    std::string result(size - 1, 0);
    WideCharToMultiByte(CP_ACP, 0, wide.c_str(), -1, &result[0], size, NULL, NULL);
    return result;
}

std::string EncodingUtils::utf8ToAnsi(const std::string& utf8) {
    return wideToAnsi(utf8ToWide(utf8));
}

std::string EncodingUtils::ansiToUtf8(const std::string& ansi) {
    return wideToUtf8(ansiToWide(ansi));
}

} // namespace utils
} // namespace luaui
