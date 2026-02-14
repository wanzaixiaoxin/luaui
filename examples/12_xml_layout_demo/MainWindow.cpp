// XML Layout Demo - MainWindow with Declarative Event Binding
// 声明式事件绑定：XML 中声明事件，代码中实现方法，自动绑定

#include "MainWindow.h"
#include "Logger.h"
#include "XmlLayout.h"
#include <windows.h>
#include <filesystem>

using namespace luaui;
using namespace luaui::controls;
using namespace luaui::utils;
using namespace luaui::xml;

namespace fs = std::filesystem;

// 资源搜索路径
static const std::vector<std::string> s_resourcePaths = {
    "layouts/",
    "../layouts/",
    "examples/12_xml_layout_demo/layouts/",
    "../../examples/12_xml_layout_demo/layouts/",
};

MainWindow::MainWindow() = default;
MainWindow::~MainWindow() = default;

void MainWindow::OnLoaded() {
    // 创建 XML 加载器
    auto loader = CreateXmlLoader();
    
    // 注册所有事件处理器
    RegisterEventHandlers(loader);
    
    // 查找并加载 XML
    std::string xmlPath = FindResourcePath("main_window.xml");
    std::shared_ptr<luaui::Control> root;
    
    if (!xmlPath.empty()) {
        try {
            root = loader->Load(xmlPath);
            Logger::InfoF("XML loaded: %s", xmlPath.c_str());
        } catch (const std::exception& e) {
            Logger::ErrorF("XML load failed: %s", e.what());
        }
    }
    
    if (!root) {
        Logger::Warning("Using fallback UI");
        root = CreateFallbackContent();
    }
    
    SetRoot(root);
}

// ============================================================================
// 注册事件处理器 - 将 C++ 方法注册到 XML 加载器
// ============================================================================
void MainWindow::RegisterEventHandlers(const std::shared_ptr<IXmlLoader>& loader) {
    Logger::Info("Registering event handlers...");
    
    // 注册 Click 事件处理器
    // XML 中写: <Button Click="OnSaveClick" ... />
    loader->RegisterClickHandler("OnNewClick",     [this] { Logger::Info("Handler: OnNewClick"); OnNewClick(); });
    loader->RegisterClickHandler("OnOpenClick",    [this] { OnOpenClick(); });
    loader->RegisterClickHandler("OnSaveClick",    [this] { OnSaveClick(); });
    loader->RegisterClickHandler("OnSearchClick",  [this] { OnSearchClick(); });
    loader->RegisterClickHandler("OnSubmitClick",  [this] { OnSubmitClick(); });
    loader->RegisterClickHandler("OnCancelClick",  [this] { OnCancelClick(); });
    loader->RegisterClickHandler("OnResetClick",   [this] { OnResetClick(); });
    
    // 导航按钮
    loader->RegisterClickHandler("OnNavHome",      [this] { UpdateStatus(L"Navigated to Home"); });
    loader->RegisterClickHandler("OnNavProfile",   [this] { UpdateStatus(L"Navigated to Profile"); });
    loader->RegisterClickHandler("OnSettings",     [this] { UpdateStatus(L"Opening settings..."); });
    
    // 注册 ValueChanged 事件处理器
    // XML 中写: <Slider ValueChanged="OnVolumeChanged" ... />
    loader->RegisterValueChangedHandler("OnVolumeChanged", [this](double v) { OnVolumeChanged(v); });
    
    Logger::Info("Event handlers registered");
}

// ============================================================================
// 资源路径管理
// ============================================================================
std::string MainWindow::FindResourcePath(const std::string& filename) {
    for (const auto& basePath : s_resourcePaths) {
        std::string fullPath = basePath + filename;
        if (fs::exists(fullPath)) {
            return fullPath;
        }
    }
    
    // 从可执行文件路径推导
    wchar_t exePath[MAX_PATH];
    if (GetModuleFileNameW(nullptr, exePath, MAX_PATH)) {
        fs::path exeDir = fs::path(exePath).parent_path();
        for (const auto& relPath : { "layouts/", "../layouts/" }) {
            auto testPath = exeDir / relPath / filename;
            if (fs::exists(testPath)) {
                return testPath.string();
            }
        }
    }
    
    return "";
}

// ============================================================================
// 控件查找（懒加载缓存）
// ============================================================================
template<typename T>
std::shared_ptr<T> MainWindow::FindControl(const std::string& name) {
    // 先检查缓存
    auto it = m_controlCache.find(name);
    if (it != m_controlCache.end()) {
        if (auto ctrl = it->second.lock()) {
            return std::dynamic_pointer_cast<T>(ctrl);
        }
    }
    
    // 从根节点查找
    auto root = GetRoot();
    if (!root) return nullptr;
    
    std::function<std::shared_ptr<interfaces::IControl>(const std::shared_ptr<interfaces::IControl>&)> find = 
        [&](const std::shared_ptr<interfaces::IControl>& control) -> std::shared_ptr<interfaces::IControl> {
        if (!control) return nullptr;
        if (control->GetName() == name) return control;
        
        if (auto* panel = dynamic_cast<Panel*>(control.get())) {
            for (auto& child : panel->GetChildren()) {
                if (auto found = find(child)) return found;
            }
        }
        if (auto border = std::dynamic_pointer_cast<Border>(control)) {
            if (auto child = border->GetChild()) {
                return find(std::static_pointer_cast<luaui::Control>(child));
            }
        }
        return nullptr;
    };
    
    auto found = find(root);
    if (found) {
        m_controlCache[name] = found;
        return std::dynamic_pointer_cast<T>(found);
    }
    return nullptr;
}

// ============================================================================
// 事件处理器实现
// ============================================================================
void MainWindow::OnNewClick() {
    Logger::Info("New");
    UpdateStatus(L"Creating new document...");
    if (auto box = FindControl<TextBox>("usernameBox")) {
        box->SetText(L"");
    }
}

void MainWindow::OnOpenClick() {
    Logger::Info("Open");
    UpdateStatus(L"Opening file...");
}

void MainWindow::OnSaveClick() {
    Logger::Info("Save");
    std::wstring name;
    if (auto box = FindControl<TextBox>("usernameBox")) {
        name = box->GetText();
    }
    UpdateStatus(name.empty() ? L"Saved" : L"Saved: " + name);
    if (auto bar = FindControl<ProgressBar>("profileProgress")) {
        bar->SetValue(100);
    }
    UpdateProgressText();
}

void MainWindow::OnSearchClick() {
    std::wstring query;
    if (auto box = FindControl<TextBox>("searchBox")) {
        query = box->GetText();
    }
    Logger::InfoF("Search: %S", query.c_str());
    UpdateStatus(query.empty() ? L"Search" : L"Searching: " + query);
}

void MainWindow::OnSubmitClick() {
    Logger::Info("Submit");
    UpdateStatus(L"Saved!");
    if (auto bar = FindControl<ProgressBar>("profileProgress")) {
        bar->SetValue(100);
    }
    UpdateProgressText();
}

void MainWindow::OnCancelClick() {
    Logger::Info("Cancel");
    UpdateStatus(L"Cancelled");
}

void MainWindow::OnResetClick() {
    Logger::Info("Reset");
    if (auto box = FindControl<TextBox>("usernameBox")) {
        box->SetText(L"");
    }
    if (auto bar = FindControl<ProgressBar>("profileProgress")) {
        bar->SetValue(0);
    }
    UpdateProgressText();
    UpdateStatus(L"Form reset");
}

void MainWindow::OnVolumeChanged(double value) {
    Logger::InfoF("Volume: %.0f", value);
    UpdateStatus(L"Volume: " + std::to_wstring((int)value) + L"%");
}

// ============================================================================
// 辅助方法
// ============================================================================
void MainWindow::UpdateStatus(const std::wstring& message) {
    if (auto text = FindControl<TextBlock>("statusText")) {
        text->SetText(message);
    }
    Logger::InfoF("Status: %S", message.c_str());
}

void MainWindow::UpdateProgressText() {
    auto progressText = FindControl<TextBlock>("progressPercentText");
    auto progressBar = FindControl<ProgressBar>("profileProgress");
    if (progressText && progressBar) {
        int val = (int)progressBar->GetValue();
        progressText->SetText(std::to_wstring(val) + L"%");
    }
}

// ============================================================================
// 回退内容
// ============================================================================
std::shared_ptr<luaui::Control> MainWindow::CreateFallbackContent() {
    auto content = std::make_shared<StackPanel>();
    content->SetSpacing(20);
    
    auto title = std::make_shared<TextBlock>();
    title->SetText(L"XML Layout Demo - Fallback");
    title->SetFontSize(24);
    content->AddChild(title);
    
    auto info = std::make_shared<TextBlock>();
    info->SetText(L"(main_window.xml not found)");
    info->SetForeground(rendering::Color(0.5f, 0.5f, 0.5f, 1.0f));
    content->AddChild(info);
    
    auto progress = std::make_shared<ProgressBar>();
    progress->SetValue(50);
    content->AddChild(progress);
    
    return content;
}
