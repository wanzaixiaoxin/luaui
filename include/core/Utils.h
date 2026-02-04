/**
 * @file Utils.h
 * @brief 工具函数类
 * @details 提供字符串处理、文件操作、编码转换等常用工具函数
 * @version 1.0.0
 */

#ifndef LUAUI_UTILS_H
#define LUAUI_UTILS_H

#include <string>
#include <vector>
#include <sstream>

namespace LuaUI {
namespace Utils {

/**
 * @brief 字符串工具类
 */
class StringUtils {
public:
    /**
     * @brief 去除字符串首尾空白字符
     * @param str 输入字符串
     * @return 去除空白后的字符串
     */
    static std::string trim(const std::string& str);
    
    /**
     * @brief 去除字符串左侧空白字符
     * @param str 输入字符串
     * @return 去除左侧空白后的字符串
     */
    static std::string trimLeft(const std::string& str);
    
    /**
     * @brief 去除字符串右侧空白字符
     * @param str 输入字符串
     * @return 去除右侧空白后的字符串
     */
    static std::string trimRight(const std::string& str);
    
    /**
     * @brief 分割字符串
     * @param str 输入字符串
     * @param delimiter 分隔符
     * @return 分割后的字符串列表
     */
    static std::vector<std::string> split(const std::string& str, char delimiter);
    
    /**
     * @brief 连接字符串列表
     * @param strings 字符串列表
     * @param delimiter 分隔符
     * @return 连接后的字符串
     */
    static std::string join(const std::vector<std::string>& strings, 
                           const std::string& delimiter);
    
    /**
     * @brief 替换字符串中的所有子串
     * @param str 输入字符串
     * @param from 要替换的子串
     * @param to 替换为的子串
     * @return 替换后的字符串
     */
    static std::string replace(const std::string& str, 
                               const std::string& from, 
                               const std::string& to);
    
    /**
     * @brief 转换为小写
     * @param str 输入字符串
     * @return 小写字符串
     */
    static std::string toLower(const std::string& str);
    
    /**
     * @brief 转换为大写
     * @param str 输入字符串
     * @return 大写字符串
     */
    static std::string toUpper(const std::string& str);
    
    /**
     * @brief 检查字符串是否以指定前缀开头
     * @param str 输入字符串
     * @param prefix 前缀
     * @return 匹配返回true，否则返回false
     */
    static bool startsWith(const std::string& str, const std::string& prefix);
    
    /**
     * @brief 检查字符串是否以指定后缀结尾
     * @param str 输入字符串
     * @param suffix 后缀
     * @return 匹配返回true，否则返回false
     */
    static bool endsWith(const std::string& str, const std::string& suffix);
    
    /**
     * @brief 检查字符串是否包含子串
     * @param str 输入字符串
     * @param substr 子串
     * @return 包含返回true，否则返回false
     */
    static bool contains(const std::string& str, const std::string& substr);
    
    /**
     * @brief 格式化字符串（类似printf）
     * @param format 格式字符串
     * @param args 可变参数
     * @return 格式化后的字符串
     */
    static std::string format(const char* format, ...);
};

/**
 * @brief 文件工具类
 */
class FileUtils {
public:
    /**
     * @brief 检查文件是否存在
     * @param filepath 文件路径
     * @return 存在返回true，否则返回false
     */
    static bool exists(const std::string& filepath);
    
    /**
     * @brief 读取文件内容
     * @param filepath 文件路径
     * @return 文件内容，如果失败返回空字符串
     */
    static std::string readFile(const std::string& filepath);
    
    /**
     * @brief 写入文件内容
     * @param filepath 文件路径
     * @param content 文件内容
     * @return 成功返回true，失败返回false
     */
    static bool writeFile(const std::string& filepath, const std::string& content);
    
    /**
     * @brief 追加内容到文件
     * @param filepath 文件路径
     * @param content 要追加的内容
     * @return 成功返回true，失败返回false
     */
    static bool appendFile(const std::string& filepath, const std::string& content);
    
    /**
     * @brief 获取文件大小
     * @param filepath 文件路径
     * @return 文件大小（字节），失败返回-1
     */
    static long getFileSize(const std::string& filepath);
    
    /**
     * @brief 获取文件扩展名
     * @param filepath 文件路径
     * @return 扩展名（不包含点号）
     */
    static std::string getFileExtension(const std::string& filepath);
    
    /**
     * @brief 获取文件名（不包含路径）
     * @param filepath 文件路径
     * @return 文件名
     */
    static std::string getFileName(const std::string& filepath);
    
    /**
     * @brief 获取目录路径（不包含文件名）
     * @param filepath 文件路径
     * @return 目录路径
     */
    static std::string getDirectory(const std::string& filepath);
    
    /**
     * @brief 获取规范化的绝对路径
     * @param filepath 文件路径
     * @return 规范化后的绝对路径
     */
    static std::string getAbsolutePath(const std::string& filepath);
    
    /**
     * @brief 创建目录（包括所有父目录）
     * @param dirpath 目录路径
     * @return 成功返回true，失败返回false
     */
    static bool createDirectory(const std::string& dirpath);
    
    /**
     * @brief 删除文件
     * @param filepath 文件路径
     * @return 成功返回true，失败返回false
     */
    static bool removeFile(const std::string& filepath);
};

/**
 * @brief 编码转换工具类
 */
class EncodingUtils {
public:
    /**
     * @brief UTF-8转ANSI
     * @param utf8 UTF-8字符串
     * @return ANSI字符串
     */
    static std::string utf8ToAnsi(const std::string& utf8);
    
    /**
     * @brief ANSI转UTF-8
     * @param ansi ANSI字符串
     * @return UTF-8字符串
     */
    static std::string ansiToUtf8(const std::string& ansi);
    
    /**
     * @brief UTF-16转UTF-8
     * @param utf16 UTF-16字符串（宽字符）
     * @return UTF-8字符串
     */
    static std::string utf16ToUtf8(const std::wstring& utf16);
    
    /**
     * @brief UTF-8转UTF-16
     * @param utf8 UTF-8字符串
     * @return UTF-16字符串（宽字符）
     */
    static std::wstring utf8ToUtf16(const std::string& utf8);
};

/**
 * @brief 数字工具类
 */
class NumberUtils {
public:
    /**
     * @brief 字符串转整数
     * @param str 字符串
     * @param defaultValue 默认值（转换失败时返回）
     * @return 整数值
     */
    static int toInt(const std::string& str, int defaultValue = 0);
    
    /**
     * @brief 字符串转长整数
     * @param str 字符串
     * @param defaultValue 默认值（转换失败时返回）
     * @return 长整数值
     */
    static long long toLong(const std::string& str, long long defaultValue = 0);
    
    /**
     * @brief 字符串转浮点数
     * @param str 字符串
     * @param defaultValue 默认值（转换失败时返回）
     * @return 浮点数值
     */
    static double toDouble(const std::string& str, double defaultValue = 0.0);
    
    /**
     * @brief 整数转字符串
     * @param value 整数值
     * @return 字符串
     */
    static std::string toString(int value);
    
    /**
     * @brief 浮点数转字符串
     * @param value 浮点数值
     * @param precision 小数点后位数
     * @return 字符串
     */
    static std::string toString(double value, int precision = 2);
};

} // namespace Utils
} // namespace LuaUI

#endif // LUAUI_UTILS_H
