/**
 * @file Version.h
 * @brief LuaUI版本信息
 * @details 定义框架的版本号和构建信息
 */

#ifndef LUAUI_VERSION_H
#define LUAUI_VERSION_H

/**
 * @brief 主版本号
 */
#define LUAUI_VERSION_MAJOR 1

/**
 * @brief 次版本号
 */
#define LUAUI_VERSION_MINOR 0

/**
 * @brief 修订版本号
 */
#define LUAUI_VERSION_PATCH 0

/**
 * @brief 版本字符串
 */
#define LUAUI_VERSION_STRING "1.0.0"

/**
 * @brief 完整版本信息字符串
 */
#define LUAUI_VERSION_FULL "LuaUI 1.0.0 (C++11, MFC)"

/**
 * @brief 构建日期
 */
#define LUAUI_BUILD_DATE __DATE__

/**
 * @brief 构建时间
 */
#define LUAUI_BUILD_TIME __TIME__

namespace LuaUI {

/**
 * @brief 版本结构体
 */
struct Version {
    int major;      ///< 主版本号
    int minor;      ///< 次版本号
    int patch;      ///< 修订版本号
    
    /**
     * @brief 获取版本字符串
     * @return 版本字符串（例如："1.0.0"）
     */
    const char* toString() const {
        return LUAUI_VERSION_STRING;
    }
};

/**
 * @brief 获取版本信息
 * @return 版本结构体
 */
inline Version GetVersion() {
    Version v;
    v.major = LUAUI_VERSION_MAJOR;
    v.minor = LUAUI_VERSION_MINOR;
    v.patch = LUAUI_VERSION_PATCH;
    return v;
}

/**
 * @brief 获取框架构建信息
 * @return 构建信息字符串
 */
inline const char* GetBuildInfo() {
    return LUAUI_VERSION_FULL;
}

} // namespace LuaUI

#endif // LUAUI_VERSION_H
