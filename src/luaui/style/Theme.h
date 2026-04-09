#pragma once

#include "ResourceDictionary.h"
#include <vector>
#include <unordered_map>
#include <functional>

namespace luaui {
namespace controls {

class Control;

using ThemeCallback = std::function<void()>;

class Theme {
public:
    using Ptr = std::shared_ptr<Theme>;

    /** @brief 获取资源字典 */
    ResourceDictionary& GetResources() { return m_res; }
    const ResourceDictionary& GetResources() const { return m_res; }

    /** @brief 便捷方法：获取主题颜色 */
    rendering::Color GetColor(const std::string& key) const {
        return m_res.GetColor(key);
    }

    /** @brief 便捷方法：获取主题浮点值 */
    float GetFloat(const std::string& key, float fb = 0.0f) const {
        return m_res.GetFloat(key, fb);
    }

    /** @brief 注册 Theme 变更回调（控件在析构前必须注销） */
    size_t AddCallback(ThemeCallback cb) {
        size_t id = m_nextCbId++;
        m_cbs[id] = std::move(cb);
        return id;
    }

    void RemoveCallback(size_t id) {
        m_cbs.erase(id);
    }

    /** @brief 合并主题资源并通知所有控件（Merge：other 覆盖 this 中同名键） */
    void ApplyResources(const ResourceDictionary& newRes) {
        m_res.Merge(newRes);
        for (auto& [id, cb] : m_cbs) {
            if (cb) cb();
        }
    }

    /** @brief 替换主题资源并通知所有控件（Replace：完全替换为 newRes） */
    void ReplaceResources(const ResourceDictionary& newRes) {
        m_res = newRes;
        for (auto& [id, cb] : m_cbs) {
            if (cb) cb();
        }
    }

    /** @brief 按名称应用内置主题 ("Light"/"Dark")，优先从 XML 文件加载 */
    void ApplyThemeByName(const std::string& name);

    /** @brief 从 XML 文件加载并应用主题 */
    void ApplyThemeFromFile(const std::string& filePath);

    /** @brief 获取当前主题名称 */
    const std::string& GetCurrentThemeName() const { return m_currentThemeName; }

    /** @brief 全局当前主题 */
    static Theme& GetCurrent();

private:
    ResourceDictionary m_res;
    std::unordered_map<size_t, ThemeCallback> m_cbs;
    size_t m_nextCbId = 1;
    std::string m_currentThemeName = "Light";
};

} // namespace controls
} // namespace luaui
