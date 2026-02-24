// MVVM Data Binding Demo - 使用 MvvmXmlLoader 实现声明式绑定

#include "MainWindow.h"
#include <luaui/mvvm/BindingEngine.h>
#include <luaui/mvvm/MvvmXmlLoader.h>
#include <luaui/mvvm/Converters.h>
#include <Logger.h>
#include <filesystem>

using namespace luaui;
using namespace luaui::mvvm;
namespace fs = std::filesystem;

// ============================================================================
// UserProfileViewModel 实现
// ============================================================================
UserProfileViewModel::UserProfileViewModel()
    : m_userName("John Doe")
    , m_age(25)
    , m_isPremium(false)
    , m_progress(65.0)
    , m_status("Ready")
{
    InitializeProperties();
}

void UserProfileViewModel::InitializeProperties() {
    // 注册属性 getter，供绑定引擎使用
    RegisterPropertyGetter<std::string>("UserName", [this]() { return GetUserName(); });
    RegisterPropertyGetter<int>("Age", [this]() { return GetAge(); });
    RegisterPropertyGetter<bool>("IsPremium", [this]() { return GetIsPremium(); });
    RegisterPropertyGetter<double>("Progress", [this]() { return GetProgress(); });
    RegisterPropertyGetter<std::string>("Status", [this]() { return GetStatus(); });
    
    // 注册属性 setter，供 TwoWay 绑定使用
    RegisterPropertySetter<std::string>("UserName", [this](const std::string& v) { SetUserName(v); });
    RegisterPropertySetter<int>("Age", [this](int v) { SetAge(v); });
    RegisterPropertySetter<bool>("IsPremium", [this](bool v) { SetIsPremium(v); });
    RegisterPropertySetter<double>("Progress", [this](double v) { SetProgress(v); });
    RegisterPropertySetter<std::string>("Status", [this](const std::string& v) { SetStatus(v); });
}

void UserProfileViewModel::SetUserName(const std::string& value) {
    if (SetProperty(m_userName, value, "UserName")) {
        SetStatus("Modified");
    }
}

void UserProfileViewModel::SetAge(int value) {
    if (SetProperty(m_age, value, "Age")) {
        SetStatus("Modified");
    }
}

void UserProfileViewModel::SetIsPremium(bool value) {
    if (SetProperty(m_isPremium, value, "IsPremium")) {
        SetStatus("Modified");
    }
}

void UserProfileViewModel::SetProgress(double value) {
    SetProperty(m_progress, value, "Progress");
}

void UserProfileViewModel::SetStatus(const std::string& value) {
    SetProperty(m_status, value, "Status");
}

void UserProfileViewModel::SaveProfile() {
    utils::Logger::InfoF("[Command] Saving profile: %s, age=%d, progress=%.0f", 
        m_userName.c_str(), m_age, m_progress);
    SetStatus("Saved!");
}

void UserProfileViewModel::ResetProfile() {
    utils::Logger::Info("[Command] Resetting profile");
    SetUserName("John Doe");
    SetAge(25);
    SetIsPremium(false);
    SetProgress(0);
    SetStatus("Reset");
}

void UserProfileViewModel::IncrementProgress() {
    double newProgress = std::min(100.0, m_progress + 10.0);
    utils::Logger::InfoF("[Command] Incrementing progress: %.0f -> %.0f", m_progress, newProgress);
    SetProgress(newProgress);
}

// ============================================================================
// MainWindow 实现 - 使用声明式数据绑定
// ============================================================================
MainWindow::MainWindow() {
    m_viewModel = std::make_shared<UserProfileViewModel>();
}

MainWindow::~MainWindow() = default;

void MainWindow::OnLoaded() {
    utils::Logger::Info("========================================");
    utils::Logger::Info("MVVM Data Binding Demo");
    utils::Logger::Info("Features:");
    utils::Logger::Info("  - Declarative Binding from XML");
    utils::Logger::Info("  - TwoWay Binding (Slider <-> Age)");
    utils::Logger::Info("  - Value Converters (Format)");
    utils::Logger::Info("  - Automatic UI Updates");
    utils::Logger::Info("========================================");
    
    // 加载 XML 布局
    std::string xmlPath = FindResourcePath("main_window.xml");
    
    if (!xmlPath.empty() && fs::exists(xmlPath)) {
        utils::Logger::InfoF("[MVVM] Loading XML layout: %s", xmlPath.c_str());
        LoadFromXml(xmlPath);
    } else {
        utils::Logger::Error("[MVVM] XML layout not found");
    }
}

// ============================================================================
// 从 XML 加载（声明式绑定）
// ============================================================================
void MainWindow::LoadFromXml(const std::string& xmlPath) {
    // 创建 MVVM XML 加载器
    auto loader = mvvm::CreateMvvmXmlLoader();
    
    // 注册值转换器
    RegisterConverters();
    
    // 注册命令处理器
    loader->RegisterClickHandler("OnSaveClick", [this]() {
        m_viewModel->SaveProfile();
    });
    loader->RegisterClickHandler("OnResetClick", [this]() {
        m_viewModel->ResetProfile();
    });
    loader->RegisterClickHandler("OnIncrementClick", [this]() {
        m_viewModel->IncrementProgress();
    });
    
    // 加载 XML（此时不连接绑定，因为 DataContext 还未设置）
    auto root = loader->Load(xmlPath);
    
    // 设置 DataContext - 这会触发所有绑定的连接
    loader->SetDataContext(m_viewModel);
    
    SetRoot(root);
    
    utils::Logger::Info("[MVVM] XML layout loaded with declarative bindings");
}

// ============================================================================
// 注册值转换器
// ============================================================================
void MainWindow::RegisterConverters() {
    auto& engine = BindingEngine::Instance();
    mvvm::RegisterDefaultConverters(engine);
    utils::Logger::Info("[MVVM] Value converters registered");
}

// ============================================================================
// 查找资源路径
// ============================================================================
std::string MainWindow::FindResourcePath(const std::string& filename) {
    std::vector<std::string> paths = {
        "mvvm_layouts/" + filename,
        "../mvvm_layouts/" + filename,
        "examples/13_mvvm_demo/layouts/" + filename,
    };
    
    for (const auto& path : paths) {
        if (fs::exists(path)) {
            return path;
        }
    }
    
    return "";
}
