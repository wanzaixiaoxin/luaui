#include "LuaMvvmTestHost.h"
#include "utils/Logger.h"
#include <filesystem>
#include <windows.h>

namespace luaui {
namespace test {

LuaMvvmTestHost::LuaMvvmTestHost() 
    : m_hInstance(nullptr) {
}

LuaMvvmTestHost::~LuaMvvmTestHost() {
}

bool LuaMvvmTestHost::Initialize(HINSTANCE hInstance) {
    m_hInstance = hInstance;
    
    // 获取测试资源路径
    char buffer[MAX_PATH];
    GetModuleFileNameA(nullptr, buffer, MAX_PATH);
    std::filesystem::path exePath(buffer);
    std::filesystem::path exeDir = exePath.parent_path();
    
    m_testLayoutsPath = (exeDir / "test_layouts").string();
    m_testScriptsPath = (exeDir / "test_scripts").string();
    
    utils::Logger::InfoF("[LuaMvvmTestHost] Layouts path: %s", m_testLayoutsPath.c_str());
    utils::Logger::InfoF("[LuaMvvmTestHost] Scripts path: %s", m_testScriptsPath.c_str());
    
    return true;
}

std::vector<LuaMvvmTestInfo> LuaMvvmTestHost::GetAvailableTests() {
    std::vector<LuaMvvmTestInfo> tests;
    
    // Layout tests
    tests.push_back({"StackPanel Layout", 
                     "Vertical and horizontal stacking with dynamic spacing", 
                     "Layout",
                     m_testLayoutsPath + "/StackPanelTest.xml",
                     m_testScriptsPath + "/StackPanelTestViewModel.lua",
                     "StackPanelTestViewModel"});
    
    tests.push_back({"Grid Layout",
                     "Star sizing, column spanning, and dynamic adjustments",
                     "Layout",
                     m_testLayoutsPath + "/GridTest.xml",
                     m_testScriptsPath + "/GridTestViewModel.lua",
                     "GridTestViewModel"});
    
    tests.push_back({"Canvas Layout",
                     "Absolute positioning with Left/Top properties",
                     "Layout",
                     m_testLayoutsPath + "/CanvasTest.xml",
                     m_testScriptsPath + "/CanvasTestViewModel.lua",
                     "CanvasTestViewModel"});
    
    tests.push_back({"DockPanel Layout",
                     "Docking to edges with LastChildFill",
                     "Layout",
                     m_testLayoutsPath + "/DockPanelTest.xml",
                     m_testScriptsPath + "/DockPanelTestViewModel.lua",
                     "DockPanelTestViewModel"});
    
    tests.push_back({"WrapPanel Layout",
                     "Automatic wrapping when space runs out",
                     "Layout",
                     m_testLayoutsPath + "/WrapPanelTest.xml",
                     m_testScriptsPath + "/WrapPanelTestViewModel.lua",
                     "WrapPanelTestViewModel"});
    
    tests.push_back({"ScrollViewer Layout",
                     "Scrolling for content larger than viewport",
                     "Layout",
                     m_testLayoutsPath + "/ScrollViewerTest.xml",
                     m_testScriptsPath + "/ScrollViewerTestViewModel.lua",
                     "ScrollViewerTestViewModel"});
    
    tests.push_back({"Viewbox Layout",
                     "Content scaling with different Stretch modes",
                     "Layout",
                     m_testLayoutsPath + "/ViewboxTest.xml",
                     m_testScriptsPath + "/ViewboxTestViewModel.lua",
                     "ViewboxTestViewModel"});
    
    return tests;
}

int LuaMvvmTestHost::RunTest(const std::string& layoutFile, 
                              const std::string& scriptFile,
                              const std::string& viewModelName) {
    utils::Logger::InfoF("[LuaMvvmTestHost] Running test:");
    utils::Logger::InfoF("  Layout: %s", layoutFile.c_str());
    utils::Logger::InfoF("  Script: %s", scriptFile.c_str());
    utils::Logger::InfoF("  ViewModel: %s", viewModelName.c_str());
    
    // 使用 LuaMvvmHost 一键启动
    lua::LuaMvvmHost host;
    
    if (!host.Initialize(m_hInstance, scriptFile, layoutFile, viewModelName)) {
        utils::Logger::Error("[LuaMvvmTestHost] Failed to initialize test");
        return 1;
    }
    
    // 运行消息循环
    return host.Run();
}

int LuaMvvmTestHost::RunTestRunner() {
    utils::Logger::Info("[LuaMvvmTestHost] Starting test runner");
    
    std::string scriptPath = m_testScriptsPath + "/TestRunnerViewModel.lua";
    std::string layoutPath = m_testLayoutsPath + "/TestRunner.xml";
    
    // 使用 LuaMvvmHost 启动测试运行器
    lua::LuaMvvmHost host;
    
    if (!host.Initialize(m_hInstance, scriptPath, layoutPath, "TestRunnerViewModel")) {
        utils::Logger::Error("[LuaMvvmTestHost] Failed to initialize test runner");
        return 1;
    }
    
    return host.Run();
}

} // namespace test
} // namespace luaui
