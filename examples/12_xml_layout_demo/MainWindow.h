#pragma once
#include "Control.h"
#include "layout.h"
#include "RangeControls.h"
#include "CheckableControls.h"
#include "XmlLayout.h"
#include "IRenderEngine.h"
#include <windows.h>
#include <windowsx.h>
#include <functional>
#include <unordered_map>

namespace demo {

// 代码后置类 - 处理 XML 布局中的事件
class MainWindow {
public:
    MainWindow();
    ~MainWindow();
    
    bool Initialize(HINSTANCE hInstance, int nCmdShow);
    int Run();

private:
    // 控件查找注册表 - 使用模板自动处理类型转换
    struct ControlBinding {
        std::function<void(luaui::controls::Control*)> binder;
    };
    
    // 注册控件绑定
    template<typename T>
    void RegisterControl(const std::string& name, T** ptr) {
        m_controlBindings[name] = {[ptr](luaui::controls::Control* c) {
            *ptr = dynamic_cast<T*>(c);
        }};
    }
    
    // 事件处理器注册表
    struct EventBinding {
        luaui::controls::Button* button = nullptr;
        std::function<void()> handler;
    };
    
    // 初始化 - 注册所有控件和事件
    void InitializeBindings();
    void CreateFromXml();
    void FindAndBindControls();
    void WireUpEvents();
    void Render();
    
    static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    
    // 事件处理器
    void OnNewClick();
    void OnOpenClick();
    void OnSaveClick();
    void OnSearchClick();
    void OnSubmitClick();
    void OnCancelClick();
    void OnResetClick();
    void OnSettingsClick();
    void OnNavHomeClick();
    void OnNavProfileClick();
    void OnSliderValueChanged(float value);
    
    // 状态更新
    void UpdateStatus(const std::wstring& message);
    void UpdateProgress(float percent);

    HWND m_hWnd = nullptr;
    luaui::rendering::IRenderEnginePtr m_engine;
    luaui::controls::ControlPtr m_root;
    luaui::xml::IXmlLoaderPtr m_xmlLoader;
    
    // 控件注册表
    std::unordered_map<std::string, ControlBinding> m_controlBindings;
    std::vector<EventBinding> m_eventBindings;
    
    // 命名控件引用（自动管理）
    luaui::controls::Button* m_newBtn = nullptr;
    luaui::controls::Button* m_openBtn = nullptr;
    luaui::controls::Button* m_saveBtn = nullptr;
    luaui::controls::Button* m_searchBtn = nullptr;
    luaui::controls::Button* m_submitBtn = nullptr;
    luaui::controls::Button* m_cancelBtn = nullptr;
    luaui::controls::Button* m_resetBtn = nullptr;
    luaui::controls::Button* m_settingsBtn = nullptr;
    luaui::controls::Button* m_navHomeBtn = nullptr;
    luaui::controls::Button* m_navProfileBtn = nullptr;
    luaui::controls::Slider* m_volumeSlider = nullptr;
    luaui::controls::TextBox* m_usernameBox = nullptr;
    luaui::controls::TextBox* m_emailBox = nullptr;
    luaui::controls::TextBox* m_bioBox = nullptr;
    luaui::controls::ProgressBar* m_profileProgress = nullptr;
    luaui::controls::TextBlock* m_statusText = nullptr;
    luaui::controls::TextBlock* m_progressPercentText = nullptr;
};

} // namespace demo
