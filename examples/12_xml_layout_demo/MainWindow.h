#pragma once

#include <luaui.h>
#include <unordered_map>
#include <functional>

// 前向声明
namespace luaui {
namespace controls {
class ProgressBar;
class Slider;
class TextBlock;
class TextBox;
}
namespace xml {
class IXmlLoader;
}
}

class MainWindow : public luaui::Window {
public:
    MainWindow();
    ~MainWindow();

protected:
    void OnLoaded() override;

private:
    // 资源路径管理
    std::string FindResourcePath(const std::string& filename);
    
    // 注册所有事件处理器到 XML 加载器
    void RegisterEventHandlers(const std::shared_ptr<luaui::xml::IXmlLoader>& loader);
    
    // 获取命名控件（用于业务逻辑中访问）
    template<typename T>
    std::shared_ptr<T> FindControl(const std::string& name);
    
    // ========== 事件处理器（由 XML 中的声明自动绑定）==========
    void OnNewClick();
    void OnOpenClick();
    void OnSaveClick();
    void OnSearchClick();
    void OnSubmitClick();
    void OnCancelClick();
    void OnResetClick();
    void OnVolumeChanged(double value);
    
    // 辅助方法
    void UpdateStatus(const std::wstring& message);
    void UpdateProgressText();
    
    // 回退内容
    std::shared_ptr<luaui::Control> CreateFallbackContent();
    
    // 控件缓存（懒加载）
    std::unordered_map<std::string, std::weak_ptr<luaui::interfaces::IControl>> m_controlCache;
};
