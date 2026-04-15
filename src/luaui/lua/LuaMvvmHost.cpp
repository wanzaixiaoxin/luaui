#include "LuaMvvmHost.h"
#include "LuaAwareMvvmLoader.h"
#include "LuaBinding_MVVM.h"
#include "Window.h"
#include "mvvm/MvvmXmlLoader.h"
#include "Logger.h"
#include <fstream>
#include <sstream>
#include <windows.h>
#include <shlwapi.h>

namespace luaui {
namespace lua {

// ============================================================================
// Error Handler - Show message box for critical errors
// ============================================================================
static void ShowError(const char* title, const char* message) {
    MessageBoxA(NULL, message, title, MB_ICONERROR | MB_OK);
}

// ============================================================================
// LuaMvvmHost Implementation
// ============================================================================

class LuaMvvmHostImpl {
public:
    std::unique_ptr<luaui::Window> window;
    std::unique_ptr<LuaSandbox> sandbox;
    std::shared_ptr<LuaAwareMvvmLoader> loader;  // Keep loader alive for command handlers
    std::string layoutPath;
    std::string viewModelName;
    bool initialized = false;

    bool Initialize(HINSTANCE hInstance, 
                   const std::string& luaPath, 
                   const std::string& xmlPath,
                   const std::string& vmName) {
        
        // Step 0: Get executable directory for resolving relative paths
        std::string exeDir = GetExecutableDirectory();
        if (exeDir.empty()) {
            ShowError("Initialization Error", "Failed to get executable directory");
            return false;
        }
        
        // Resolve relative paths to absolute paths
        layoutPath = ResolvePath(xmlPath, exeDir);
        std::string fullLuaPath = ResolvePath(luaPath, exeDir);
        viewModelName = vmName;

        // Step 1: Initialize Logger
        utils::Logger::Initialize();
        utils::Logger::InfoF("[LuaMvvmHost] Initializing...");
        utils::Logger::InfoF("[LuaMvvmHost] Exe dir: %s", exeDir.c_str());
        utils::Logger::InfoF("[LuaMvvmHost] Lua path: %s", fullLuaPath.c_str());
        utils::Logger::InfoF("[LuaMvvmHost] XML path: %s", layoutPath.c_str());

        // Step 2: Create Window
        window = std::make_unique<luaui::Window>();
        if (!window->Create(hInstance, L"LuaUI MVVM", 800, 600)) {
            utils::Logger::Error("[LuaMvvmHost] Failed to create window");
            ShowError("Window Error", "Failed to create application window");
            return false;
        }

        // Step 3: Initialize Lua Sandbox
        sandbox = std::make_unique<LuaSandbox>();
        LuaSandbox::SecurityPolicy policy;
        policy.maxMemoryBytes = 32 * 1024 * 1024;
        
        if (!sandbox->Initialize(policy)) {
            utils::Logger::Error("[LuaMvvmHost] Failed to initialize Lua sandbox");
            ShowError("Lua Error", "Failed to initialize Lua sandbox");
            return false;
        }

        // Step 3.5: Register MVVM bindings BEFORE loading Lua script
        // This ensures Notify() function is available to Lua
        lua_State* L = sandbox->GetLuaState();
        LuaBinding::RegisterMVVM(L);
        utils::Logger::Info("[LuaMvvmHost] MVVM bindings registered");

        // Step 4: Load Lua ViewModel
        if (!LoadLuaScript(fullLuaPath)) {
            return false;
        }
        
        // Verify: Check if Notify function exists after loading Lua
        // lua_getglobal(L, "Notify");
        // if (!lua_isfunction(L, -1)) {
        //     utils::Logger::Error("[LuaMvvmHost] CRITICAL: Notify function not found after loading Lua!");
        // } else {
        //     utils::Logger::Info("[LuaMvvmHost] Notify function verified");
        // }
        // lua_pop(L, 1);

        // Step 5: Load XML Layout with Lua-aware binding
        if (!LoadLayout()) {
            return false;
        }

        initialized = true;
        utils::Logger::Info("[LuaMvvmHost] Ready");
        return true;
    }

    // Get the directory containing the executable
    std::string GetExecutableDirectory() {
        char exePath[MAX_PATH];
        DWORD result = GetModuleFileNameA(NULL, exePath, MAX_PATH);
        if (result == 0 || result >= MAX_PATH) {
            return "";
        }
        
        // Remove the filename to get directory
        if (!PathRemoveFileSpecA(exePath)) {
            return "";
        }
        return std::string(exePath);
    }
    
    // Resolve relative path to absolute path
    std::string ResolvePath(const std::string& path, const std::string& baseDir) {
        // If path is already absolute, return as-is
        if (!PathIsRelativeA(path.c_str())) {
            return path;
        }
        
        // Combine base directory with relative path
        char fullPath[MAX_PATH];
        if (PathCombineA(fullPath, baseDir.c_str(), path.c_str())) {
            return std::string(fullPath);
        }
        
        // Fallback: simple concatenation
        return baseDir + "\\" + path;
    }

    bool LoadLuaScript(const std::string& path) {
        // Check if file exists first
        if (GetFileAttributesA(path.c_str()) == INVALID_FILE_ATTRIBUTES) {
            std::string error = "Cannot find Lua file:\n" + path;
            utils::Logger::ErrorF("[LuaMvvmHost] %s", error.c_str());
            ShowError("File Error", error.c_str());
            return false;
        }
        
        std::ifstream file(path);
        if (!file.is_open()) {
            std::string error = "Cannot open Lua file:\n" + path;
            utils::Logger::ErrorF("[LuaMvvmHost] %s", error.c_str());
            ShowError("File Error", error.c_str());
            return false;
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        file.close();

        if (!sandbox->Execute(buffer.str(), path)) {
            std::string error = "Lua execution error:\n" + sandbox->GetLastError();
            utils::Logger::ErrorF("[LuaMvvmHost] %s", error.c_str());
            ShowError("Lua Error", error.c_str());
            return false;
        }

        // Verify ViewModel exists
        lua_State* L = sandbox->GetLuaState();
        if (!L) {
            ShowError("Lua Error", "Lua state is null after execution");
            return false;
        }
        
        lua_getglobal(L, viewModelName.c_str());
        if (!lua_istable(L, -1)) {
            std::string error = "ViewModel '" + viewModelName + "' not found or not a table";
            utils::Logger::ErrorF("[LuaMvvmHost] %s", error.c_str());
            ShowError("ViewModel Error", error.c_str());
            lua_pop(L, 1);
            return false;
        }
        lua_pop(L, 1);

        utils::Logger::InfoF("[LuaMvvmHost] ViewModel '%s' loaded", viewModelName.c_str());
        return true;
    }

    bool LoadLayout() {
        // Check if file exists first
        if (GetFileAttributesA(layoutPath.c_str()) == INVALID_FILE_ATTRIBUTES) {
            std::string error = "Cannot find XML file:\n" + layoutPath;
            utils::Logger::ErrorF("[LuaMvvmHost] %s", error.c_str());
            ShowError("File Error", error.c_str());
            return false;
        }
        
        lua_State* L = sandbox->GetLuaState();
        if (!L) {
            ShowError("Lua Error", "Lua state is null");
            return false;
        }
        
        // Create Lua-aware MVVM loader (save as member to keep it alive)
        loader = std::make_shared<LuaAwareMvvmLoader>();
        if (!loader) {
            ShowError("Loader Error", "Failed to create MVVM loader");
            return false;
        }
        
        // Connect to Lua
        loader->SetLuaState(L);
        loader->SetViewModelName(viewModelName);

        // Register property notifier for Lua Notify() function
        // This links the Lua Notify() calls to the C++ binding engine
        if (auto notifier = loader->GetNotifier()) {
            RegisterPropertyNotifier(L, notifier);
        }

        // Load XML
        std::shared_ptr<luaui::Control> root;
        try {
            root = loader->Load(layoutPath);
        } catch (const std::exception& e) {
            utils::Logger::ErrorF("[LuaMvvmHost] Exception loading layout: %s", e.what());
            ShowError("Layout Error", e.what());
            return false;
        } catch (...) {
            utils::Logger::Error("[LuaMvvmHost] Unknown exception loading layout");
            ShowError("Layout Error", "Unknown exception");
            return false;
        }
        
        if (!root) {
            utils::Logger::Error("[LuaMvvmHost] Failed to load layout");
            ShowError("Layout Error", "Failed to load XML layout");
            return false;
        }

        window->SetRoot(root);
        return true;
    }

    int Run() {
        utils::Logger::Info("[LuaMvvmHost] Run() called");
        
        if (!initialized) {
            utils::Logger::Error("[LuaMvvmHost] Run() failed: not initialized");
            ShowError("Runtime Error", "Application not initialized");
            return 1;
        }
        
        if (!window) {
            utils::Logger::Error("[LuaMvvmHost] Run() failed: window is null");
            ShowError("Runtime Error", "Window is null");
            return 1;
        }
        
        utils::Logger::Info("[LuaMvvmHost] Showing window...");
        window->Show(SW_SHOW);
        
        utils::Logger::Info("[LuaMvvmHost] Entering message loop...");
        int result = window->Run();
        utils::Logger::InfoF("[LuaMvvmHost] Message loop exited with code: %d", result);
        
        return result;
    }

    void Shutdown() {
        if (sandbox) {
            sandbox->Shutdown();
            sandbox.reset();
        }
        if (window) {
            window.reset();
        }
        initialized = false;
    }
};

// ============================================================================
// LuaMvvmHost Public Interface
// ============================================================================

LuaMvvmHost::LuaMvvmHost() : m_impl(std::make_unique<LuaMvvmHostImpl>()) {
    utils::Logger::Info("[LuaMvvmHost] Constructor");
}

LuaMvvmHost::~LuaMvvmHost() {
    Shutdown();
}

LuaMvvmHost::LuaMvvmHost(LuaMvvmHost&&) noexcept = default;
LuaMvvmHost& LuaMvvmHost::operator=(LuaMvvmHost&&) noexcept = default;

bool LuaMvvmHost::Initialize(HINSTANCE hInstance,
                            const std::string& luaPath,
                            const std::string& xmlPath,
                            const std::string& viewModelName) {
    return m_impl->Initialize(hInstance, luaPath, xmlPath, viewModelName);
}

int LuaMvvmHost::Run() {
    return m_impl->Run();
}

luaui::Window* LuaMvvmHost::GetWindow() const {
    return m_impl->window.get();
}

LuaSandbox* LuaMvvmHost::GetSandbox() const {
    return m_impl->sandbox.get();
}

void LuaMvvmHost::Shutdown() {
    if (m_impl) {
        m_impl->Shutdown();
    }
}

} // namespace lua
} // namespace luaui
