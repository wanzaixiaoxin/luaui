/**
 * @file LuaUI.h
 * @brief LuaUI主头文件 - 基于MFC的Windows UI框架
 * @details 该框架提供XML布局和Lua脚本编写业务逻辑的能力
 * @version 1.0.0
 * @date 2026
 * @license Apache License 2.0
 */

#ifndef LUAUI_H
#define LUAUI_H

// 包含版本信息
#include "Version.h"

// 核心模块
#include "core/App.h"
#include "core/LuaState.h"
#include "core/ScriptEngine.h"
#include "core/Utils.h"

// UI模块
#include "ui/controls/BaseControl.h"
#include "ui/factory/ControlFactory.h"
#include "ui/layout/LayoutEngine.h"
#include "ui/events/EventManager.h"

// XML模块
#include "xml/parser/XmlParser.h"

// Lua模块
#include "lua/binding/LuaBinder.h"
#include "lua/lifecycle/LifecycleManager.h"

// 资源模块
#include "resource/loader/ImageLoader.h"
#include "resource/style/StyleManager.h"
#include "resource/theme/ThemeManager.h"



namespace LuaUI {

/**
 * @brief 初始化LuaUI框架
 * @return 成功返回true，失败返回false
 * @details 初始化所有核心模块，包括Lua引擎、资源系统等
 */
bool Initialize();

/**
 * @brief 关闭LuaUI框架
 * @details 清理所有资源，关闭Lua引擎
 */
void Shutdown();

/**
 * @brief 获取框架构建信息
 * @return 构建信息字符串
 */
const char* GetBuildInfo();

/**
 * @brief 运行应用程序主循环
 * @return 退出码
 */
int RunMainLoop();

/**
 * @brief 获取布局引擎
 * @return 布局引擎指针
 */
class ILayoutEngine* GetLayoutEngine();

/**
 * @brief 获取脚本引擎
 * @return 脚本引擎指针
 */
class IScriptEngine* GetScriptEngine();

} // namespace LuaUI

#endif // LUAUI_H
