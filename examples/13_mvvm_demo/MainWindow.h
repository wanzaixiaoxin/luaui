// MVVM Data Binding Demo

#pragma once

#include <luaui.h>
#include "ViewModelBase.h"
#include <memory>
#include <string>
#include <functional>

// Controls
#include "Panel.h"       // includes StackPanel
#include "TextBlock.h"
#include "TextBox.h"
#include "Button.h"
#include "Slider.h"

// MVVM
#include <luaui/mvvm/MVVM.h>

// ============================================================
// ViewModel: UserProfileViewModel
// ============================================================
class UserProfileViewModel : public luaui::mvvm::ViewModelBase {
public:
    UserProfileViewModel();

    // UserName 属性
    std::string GetUserName() const { return m_userName; }
    void SetUserName(const std::string& value);

    // Age 属性
    int GetAge() const { return m_age; }
    void SetAge(int value);

    // IsPremium 属性
    bool GetIsPremium() const { return m_isPremium; }
    void SetIsPremium(bool value);

    // Progress 属性
    double GetProgress() const { return m_progress; }
    void SetProgress(double value);

    // Status 属性
    std::string GetStatus() const { return m_status; }
    void SetStatus(const std::string& value);

    // 命令
    void SaveProfile();
    void ResetProfile();
    void IncrementProgress();

protected:
    // 初始化属性访问器
    void InitializeProperties();

private:
    std::string m_userName;
    int m_age;
    bool m_isPremium;
    double m_progress;
    std::string m_status;
};

// ============================================================
// View: MainWindow - 支持声明式数据绑定
// ============================================================
class MainWindow : public luaui::Window {
public:
    MainWindow();
    ~MainWindow();

    // 窗口生命周期
    void OnLoaded() override;

private:
    std::shared_ptr<UserProfileViewModel> m_viewModel;

    // 加载方式
    void LoadFromXml(const std::string& xmlPath);

    // 注册值转换器
    void RegisterConverters();

    // 查找资源
    std::string FindResourcePath(const std::string& filename);
};
