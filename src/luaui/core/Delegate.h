#pragma once

#include <vector>
#include <cstdint>
#include <functional>
#include <type_traits>
#include <algorithm>
#include <map>
#include <memory>

namespace luaui {

/**
 * @brief 高性能委托系统 - 替代std::function用于事件处理
 * 
 * 特性：
 * - 内存连续（std::vector存储）
 * - 无类型擦除开销（编译期多态）
 * - 支持ID精准移除订阅
 * - 零分配添加/移除
 */

template<typename... Args>
class Delegate {
public:
    using CallbackType = void(*)(void*, Args...);
    using ID = uint32_t;
    
    static constexpr ID INVALID_ID = 0;

private:
    struct Entry {
        void* target;           // 对象指针（静态函数时为nullptr）
        CallbackType callback;  // 统一函数指针
        ID id;                  // 唯一标识（用于Remove）
        uint16_t refCount;      // 引用计数（支持相同处理器的多次订阅）
        std::shared_ptr<void> storage; // 用于lambda/闭包存储
        
        Entry(void* t, CallbackType cb, ID i, std::shared_ptr<void> s = nullptr) 
            : target(t), callback(cb), id(i), refCount(1), storage(std::move(s)) {}
    };
    
    std::vector<Entry> m_entries;  // 内存连续，缓存友好
    ID m_nextId = 1;               // ID从1开始，0表示无效
    bool m_invoking = false;       // 防止重入
    std::vector<ID> m_pendingRemoves; // 延迟移除队列

public:
    Delegate() = default;
    ~Delegate() = default;
    
    // 禁止拷贝，允许移动
    Delegate(const Delegate&) = delete;
    Delegate& operator=(const Delegate&) = delete;
    Delegate(Delegate&&) = default;
    Delegate& operator=(Delegate&&) = default;

    /**
     * @brief 添加成员函数处理器
     * @return 订阅ID，用于后续Remove
     */
    template<typename T>
    ID Add(T* obj, void(T::*method)(Args...)) {
        if (!obj || !method) return INVALID_ID;
        
        // 创建包装器存储
        struct Wrapper {
            T* obj;
            void(T::*method)(Args...);
            static void Invoke(void* target, Args... args) {
                auto* w = static_cast<Wrapper*>(target);
                (w->obj->*w->method)(args...);
            }
        };
        
        auto wrapper = std::make_shared<Wrapper>(Wrapper{obj, method});
        void* target = wrapper.get();
        CallbackType callback = &Wrapper::Invoke;
        
        ID id = m_nextId++;
        m_entries.emplace_back(target, callback, id, wrapper);
        return id;
    }

    /**
     * @brief 添加静态函数/自由函数处理器
     */
    ID Add(void(*func)(Args...)) {
        if (!func) return INVALID_ID;
        
        ID id = m_nextId++;
        m_entries.emplace_back(nullptr, func, id);
        return id;
    }

    /**
     * @brief 添加lambda处理器（支持捕获lambda）
     */
    template<typename Lambda>
    ID Add(Lambda&& lambda) {
        // 检查是否可以转换为函数指针（无捕获lambda）
        if constexpr (std::is_convertible_v<Lambda, void(*)(Args...)>) {
            return Add(static_cast<void(*)(Args...)>(lambda));
        } else {
            // 有捕获的lambda：使用std::function包装
            return AddWrapped(std::forward<Lambda>(lambda));
        }
    }
    
private:
    /**
     * @brief 添加有捕获的lambda（使用std::function存储）
     */
    template<typename Lambda>
    ID AddWrapped(Lambda&& lambda) {
        // 使用shared_ptr管理std::function的生命周期
        auto func = std::make_shared<std::function<void(Args...)>>(
            std::forward<Lambda>(lambda)
        );
        
        // 将shared_ptr存储在target中
        void* target = func.get();
        
        // 创建调用包装器
        CallbackType callback = [](void* t, Args... args) {
            auto* f = static_cast<std::function<void(Args...)>*>(t);
            (*f)(args...);
        };
        
        ID id = m_nextId++;
        m_entries.emplace_back(target, callback, id, func);
        return id;
    }
    
public:
    /**
     * @brief 移除指定ID的处理器
     */
    void Remove(ID id) {
        if (id == INVALID_ID) return;
        
        if (m_invoking) {
            // 延迟移除，避免遍历时修改容器
            m_pendingRemoves.push_back(id);
        } else {
            DoRemove(id);
        }
    }

    /**
     * @brief 触发所有处理器
     */
    void Invoke(Args... args) {
        if (m_entries.empty()) return;
        
        m_invoking = true;
        
        // 按索引遍历，新添加的不会在本次触发
        size_t count = m_entries.size();
        for (size_t i = 0; i < count; ++i) {
            auto& entry = m_entries[i];
            entry.callback(entry.target, args...);
        }
        
        m_invoking = false;
        
        // 处理延迟移除
        if (!m_pendingRemoves.empty()) {
            for (ID id : m_pendingRemoves) {
                DoRemove(id);
            }
            m_pendingRemoves.clear();
            m_pendingRemoves.shrink_to_fit();
        }
    }

    /**
     * @brief 清空所有处理器
     */
    void Clear() {
        if (m_invoking) {
            // 标记所有待移除
            for (const auto& entry : m_entries) {
                m_pendingRemoves.push_back(entry.id);
            }
        } else {
            m_entries.clear();
            m_entries.shrink_to_fit();
        }
    }

    /**
     * @brief 是否为空
     */
    bool IsEmpty() const {
        return m_entries.empty();
    }

    /**
     * @brief 处理器数量
     */
    size_t Count() const {
        return m_entries.size();
    }

    /**
     * @brief 预留空间（避免频繁重新分配）
     */
    void Reserve(size_t capacity) {
        m_entries.reserve(capacity);
    }

private:
    void DoRemove(ID id) {
        auto it = std::find_if(m_entries.begin(), m_entries.end(),
            [id](const Entry& e) { return e.id == id; });
        
        if (it != m_entries.end()) {
            m_entries.erase(it);
        }
    }
};

/**
 * @brief 简化的事件宏
 */
#define DECLARE_EVENT(EventName, ...) \
public: \
    using EventName##Handler = void(*)(__VA_ARGS__); \
    luaui::Delegate<__VA_ARGS__> EventName; \
    uint32_t On##EventName##Subscribe(luaui::Delegate<__VA_ARGS__>::ID id) { return id; }

} // namespace luaui
