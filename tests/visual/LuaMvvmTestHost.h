#pragma once

#include "Window.h"
#include "lua/LuaMvvmHost.h"
#include <memory>
#include <string>
#include <vector>

namespace luaui {
namespace test {

/**
 * @brief Lua MVVM 测试信息
 */
struct LuaMvvmTestInfo {
    std::string name;
    std::string description;
    std::string category;
    std::string layoutFile;
    std::string scriptFile;
    std::string viewModelName;
};

/**
 * @brief Lua MVVM 测试主机
 * 
 * 使用 Lua + MVVM + XML 架构加载和运行可视化测试
 */
class LuaMvvmTestHost {
public:
    LuaMvvmTestHost();
    ~LuaMvvmTestHost();
    
    // 禁止拷贝
    LuaMvvmTestHost(const LuaMvvmTestHost&) = delete;
    LuaMvvmTestHost& operator=(const LuaMvvmTestHost&) = delete;
    
    // 初始化
    bool Initialize(HINSTANCE hInstance);
    
    // 获取所有可用测试
    std::vector<LuaMvvmTestInfo> GetAvailableTests();
    
    // 运行特定测试（阻塞直到窗口关闭）
    int RunTest(const std::string& layoutFile, const std::string& scriptFile, 
                const std::string& viewModelName = "ViewModelInstance");
    
    // 运行测试运行器
    int RunTestRunner();

private:
    HINSTANCE m_hInstance;
    std::string m_testLayoutsPath;
    std::string m_testScriptsPath;
};

} // namespace test
} // namespace luaui
