// LuaUI MVVM Demo - 03
// XML (View) + Lua (ViewModel) + C++ (Host)

#include "Window.h"
#include "Controls.h"
#include "lua/LuaSandbox.h"
#include "SimpleXmlLoader.h"
#include "Logger.h"
#include "Types.h"
#include <windows.h>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace luaui;
using namespace luaui::controls;
using namespace luaui::rendering;
using namespace luaui::utils;

// ==================== MVVM Host Window ====================

class MVVMHostWindow : public Window {
public:
    MVVMHostWindow() = default;
    
    bool Initialize(const std::string& viewModelPath, const std::string& layoutPath) {
        // Initialize Lua sandbox
        luaui::lua::LuaSandbox::SecurityPolicy policy;
        policy.maxMemoryBytes = 32 * 1024 * 1024;
        
        std::cout << "[C++] Initializing Lua sandbox..." << std::endl;
        if (!m_sandbox.Initialize(policy)) {
            std::cerr << "[C++] Failed to initialize Lua sandbox" << std::endl;
            return false;
        }
        std::cout << "[C++] Lua sandbox initialized" << std::endl;
        
        // Load ViewModel
        std::cout << "[C++] Loading ViewModel: " << viewModelPath << std::endl;
        if (!LoadLuaFile(viewModelPath)) {
            return false;
        }
        
        // Store paths for later
        m_layoutPath = layoutPath;
        
        return true;
    }
    
    void Shutdown() {
        m_sandbox.Shutdown();
    }
    
    // Refresh UI bindings (call after ViewModel changes)
    void RefreshUI() {
        if (m_xmlLoader) {
            // Reload layout to get updated values
            auto newRoot = m_xmlLoader->Load(m_layoutPath, m_sandbox.GetLuaState());
            if (newRoot) {
                SetRoot(newRoot);
                InvalidateLayout();
            }
        }
    }

protected:
    void OnLoaded() override {
        std::cout << "[C++] Window OnLoaded - loading XML layout..." << std::endl;
        std::cout << "[C++] XML path: " << m_layoutPath << std::endl;
        
        // Check if file exists
        std::ifstream testFile(m_layoutPath);
        if (!testFile.is_open()) {
            std::cerr << "[C++] ERROR: Cannot open XML file: " << m_layoutPath << std::endl;
            return;
        }
        testFile.close();
        
        // Load XML layout
        m_xmlLoader = std::make_unique<luaui::mvvm::SimpleXmlLoader>();
        auto root = m_xmlLoader->Load(m_layoutPath, m_sandbox.GetLuaState());
        
        if (root) {
            SetRoot(root);
            std::cout << "[C++] XML layout loaded successfully" << std::endl;
        } else {
            std::cerr << "[C++] Failed to load XML layout: " << m_xmlLoader->GetLastError() << std::endl;
        }
    }

private:
    luaui::lua::LuaSandbox m_sandbox;
    std::unique_ptr<luaui::mvvm::SimpleXmlLoader> m_xmlLoader;
    std::string m_layoutPath;
    
    bool LoadLuaFile(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) {
            std::cerr << "[C++] Cannot open: " << path << std::endl;
            return false;
        }
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        
        std::cout << "[C++] Executing Lua code..." << std::endl;
        if (!m_sandbox.Execute(buffer.str(), path)) {
            std::cerr << "[C++] Lua error: " << m_sandbox.GetLastError() << std::endl;
            return false;
        }
        
        std::cout << "[C++] Lua code executed successfully" << std::endl;
        return true;
    }
};

// ==================== Entry Point ====================

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int) {
    Logger::Initialize();
    Logger::SetConsoleLevel(LogLevel::Debug);
    
    // Console for output
    AllocConsole();
    FILE* dummy;
    freopen_s(&dummy, "CONOUT$", "w", stdout);
    freopen_s(&dummy, "CONOUT$", "w", stderr);
    
    std::cout << "========================================" << std::endl;
    std::cout << "LuaUI MVVM Demo" << std::endl;
    std::cout << "XML (View) + Lua (ViewModel) + C++ (Host)" << std::endl;
    std::cout << "========================================" << std::endl;
    
    try {
        MVVMHostWindow window;
        
        // Get executable directory
        char exePath[MAX_PATH];
        GetModuleFileNameA(NULL, exePath, MAX_PATH);
        std::string exeDir = exePath;
        size_t lastSlash = exeDir.find_last_of("\\/");
        if (lastSlash != std::string::npos) {
            exeDir = exeDir.substr(0, lastSlash);
        }
        
        std::string luaPath = exeDir + "\\scripts\\ViewModel.lua";
        std::string xmlPath = exeDir + "\\layouts\\MainView.xml";
        
        std::cout << "[C++] Lua path: " << luaPath << std::endl;
        std::cout << "[C++] XML path: " << xmlPath << std::endl;
        
        if (!window.Initialize(luaPath, xmlPath)) {
            std::cerr << "[C++] Failed to initialize" << std::endl;
            return 1;
        }
        
        std::cout << "[C++] Creating window..." << std::endl;
        if (!window.Create(hInstance, L"LuaUI MVVM Demo", 800, 600)) {
            std::cerr << "[C++] Failed to create window" << std::endl;
            return 1;
        }
        
        window.Show(SW_SHOW);
        std::cout << "[C++] Running message loop..." << std::endl;
        int result = window.Run();
        
        window.Shutdown();
        return result;
        
    } catch (const std::exception& e) {
        std::cerr << "[C++] Exception: " << e.what() << std::endl;
        return 1;
    }
}
