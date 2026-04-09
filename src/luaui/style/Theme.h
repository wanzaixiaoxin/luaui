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

    /** @brief 切换主题资源并通知所有控件 */
    void ApplyResources(const ResourceDictionary& newRes) {
        m_res.Merge(newRes);
        for (auto& [id, cb] : m_cbs) {
            if (cb) cb();
        }
    }

    /** @brief 全局当前主题 */
    static Theme& GetCurrent();

private:
    ResourceDictionary m_res;
    std::unordered_map<size_t, ThemeCallback> m_cbs;
    size_t m_nextCbId = 1;
};

/** @brief 创建 Light 主题资源 */
ResourceDictionary CreateLightTheme();

/** @brief 创建 Dark 主题资源 */
ResourceDictionary CreateDarkTheme();

} // namespace controls
} // namespace luaui
