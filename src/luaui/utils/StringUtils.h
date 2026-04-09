#pragma once

#include <string>
#include <vector>
#include <Windows.h>

namespace luaui {
namespace utils {

/**
 * @brief 字符串工具类
 * @details 提供 UTF-8 与 UTF-16 (wchar_t) 之间的转换功能
 */
class StringUtils {
public:
    /**
     * @brief 将 UTF-8 字符串转换为宽字符字符串 (UTF-16)
     * @param utf8Str UTF-8 编码的字符串
     * @return 转换后的宽字符字符串
     */
    static std::wstring Utf8ToWString(const char* utf8Str);
    
    /**
     * @brief 将 UTF-8 字符串转换为宽字符字符串 (UTF-16)
     * @param utf8Str UTF-8 编码的字符串
     * @return 转换后的宽字符字符串
     */
    static std::wstring Utf8ToWString(const std::string& utf8Str);
    
    /**
     * @brief 将宽字符字符串 (UTF-16) 转换为 UTF-8 字符串
     * @param wstr 宽字符字符串
     * @return 转换后的 UTF-8 字符串
     */
    static std::string WStringToUtf8(const wchar_t* wstr);
    
    /**
     * @brief 将宽字符字符串 (UTF-16) 转换为 UTF-8 字符串
     * @param wstr 宽字符字符串
     * @return 转换后的 UTF-8 字符串
     */
    static std::string WStringToUtf8(const std::wstring& wstr);
    
    /**
     * @brief 清理字符串两端的空白字符
     * @param str 输入字符串
     * @return 清理后的字符串
     */
    static std::string Trim(const std::string& str);
    
    /**
     * @brief 按分隔符分割字符串
     * @param str 输入字符串
     * @param delimiter 分隔符
     * @return 分割后的字符串数组
     */
    static std::vector<std::string> Split(const std::string& str, char delimiter);
    
    /**
     * @brief 按点号分割路径字符串
     * @param path 路径字符串
     * @return 分割后的路径组件
     */
    static std::vector<std::string> SplitPath(const std::string& path);
};

} // namespace utils

// 便捷的 using 别名
using StringUtils = utils::StringUtils;

} // namespace luaui

// 全局命名空间别名，简化使用
namespace StrUtil = luaui::utils;

// 顶层便捷函数（推荐直接使用）
inline std::wstring Utf8ToW(const char* utf8Str) {
    return luaui::utils::StringUtils::Utf8ToWString(utf8Str);
}
inline std::wstring Utf8ToW(const std::string& utf8Str) {
    return luaui::utils::StringUtils::Utf8ToWString(utf8Str);
}
inline std::string WToUtf8(const wchar_t* wstr) {
    return luaui::utils::StringUtils::WStringToUtf8(wstr);
}
inline std::string WToUtf8(const std::wstring& wstr) {
    return luaui::utils::StringUtils::WStringToUtf8(wstr);
}
