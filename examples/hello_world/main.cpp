/**
 * @file main.cpp
 * @brief Hello World示例程序
 * @details 演示LuaUI框架的基本使用
 */

#include <LuaUI.h>
#include <iostream>
#include <string>

/**
 * @brief 主函数
 */
int main(int /*argc*/, char* /*argv*/[]) {
    std::cout << "LuaUI Hello World Example" << std::endl;
    std::cout << "=========================" << std::endl;
    
    // 初始化LuaUI框架
    if (!LuaUI::Initialize()) {
        std::cerr << "Failed to initialize LuaUI!" << std::endl;
        return -1;
    }
    
    std::cout << "LuaUI Version: " << LuaUI::GetVersion().toString() << std::endl;
    std::cout << "Build Info: " << LuaUI::GetBuildInfo() << std::endl;
    std::cout << std::endl;
    
    // 获取布局引擎和脚本引擎
    auto* layoutEngine = LuaUI::GetLayoutEngine();
    if (!layoutEngine) {
        std::cerr << "Failed to get layout engine!" << std::endl;
        LuaUI::Shutdown();
        return -1;
    }
    
    auto* scriptEngine = LuaUI::GetScriptEngine();
    if (!scriptEngine) {
        std::cerr << "Failed to get script engine!" << std::endl;
        LuaUI::Shutdown();
        return -1;
    }
    
    // 加载XML布局
    std::string xmlFile = "helloworld_layout.xml";
    std::cout << "Loading XML layout from: " << xmlFile << std::endl;
    if (!layoutEngine->loadFromXml(xmlFile)) {
        std::cerr << "Failed to load layout from: " << xmlFile << std::endl;
        LuaUI::Shutdown();
        return -1;
    }
    
    // 显示UI
    std::cout << "Showing UI..." << std::endl;
    layoutEngine->showUI();
    
    // 加载Lua脚本
    std::string luaFile = "helloworld_main.lua";
    std::cout << "Loading Lua script from: " << luaFile << std::endl;
    if (!scriptEngine->loadScript(luaFile)) {
        std::cerr << "Failed to load Lua script from: " << luaFile << std::endl;
        LuaUI::Shutdown();
        return -1;
    }
    
    // 绑定Lua事件处理函数
    std::cout << "Binding Lua events..." << std::endl;
    layoutEngine->bindLuaEvents(scriptEngine);
    
    // 调用初始化函数（如果存在）
    if (scriptEngine->callFunction("onInit")) {
        std::cout << "onInit() called successfully" << std::endl;
    }
    
    std::cout << "UI and script loaded successfully. Running main loop..." << std::endl;
    
    // 运行应用程序主循环
    int result = LuaUI::RunMainLoop();
    
    std::cout << "Application exiting with code: " << result << std::endl;
    
    // 关闭LuaUI框架
    LuaUI::Shutdown();
    
    return 0;
}
