/**
 * @file main.cpp
 * @brief Hello World示例程序
 * @details 演示LuaUI框架的基本使用
 */

#include <LuaUI.h>
#include <iostream>

/**
 * @brief 主函数
 */
int main(int argc, char* argv[]) {
    std::cout << "LuaUI Hello World Example" << std::endl;
    std::cout << "=========================" << std::endl;
    
    // 初始化LuaUI框架
    if (!LuaUI::Initialize()) {
        std::cerr << "Failed to initialize LuaUI!" << std::endl;
        return -1;
    }
    
    std::cout << "LuaUI Version: " << LuaUI::GetVersion() << std::endl;
    std::cout << "Build Info: " << LuaUI::GetBuildInfo() << std::endl;
    std::cout << std::endl;
    
    // TODO: 加载XML布局
    // TODO: 加载Lua脚本
    // TODO: 创建UI控件
    // TODO: 运行主循环
    
    std::cout << "Application running..." << std::endl;
    std::cout << "Press Enter to exit..." << std::endl;
    std::cin.get();
    
    // 关闭LuaUI框架
    LuaUI::Shutdown();
    
    return 0;
}
