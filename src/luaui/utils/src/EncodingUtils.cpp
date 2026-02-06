/**
 * @file EncodingUtils.cpp
 * @brief Encoding utility implementations
 */

#include "core/Utils.h"
#include <windows.h>

namespace LuaUI {
namespace Utils {

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

} // namespace utils
} // namespace luaui
