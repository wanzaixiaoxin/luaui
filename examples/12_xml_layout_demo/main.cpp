#include "MainWindow.h"
#include <windows.h>
#include <objbase.h>
#include <memory>

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int nCmdShow) {
    // Initialize COM for Direct2D/WIC
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(hr)) return 1;
    
    int result = 1;
    std::unique_ptr<demo::MainWindow> window;
    
    try {
        window = std::make_unique<demo::MainWindow>();
        
        if (!window->Create(hInstance, L"LuaUI XML Layout Demo", 1000, 700)) {
            CoUninitialize();
            return 1;
        }
        
        if (!window->LoadLayout("layouts/main_window.xml")) {
            CoUninitialize();
            return 1;
        }
        
        result = window->Run();
    } catch (...) {
        result = 1;
    }
    
    window.reset();
    CoUninitialize();
    return result;
}
