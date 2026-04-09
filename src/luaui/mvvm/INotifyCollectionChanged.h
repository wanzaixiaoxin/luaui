#pragma once

#include <functional>
#include <vector>
#include <any>
#include <string>
#include <algorithm>

namespace luaui {
namespace mvvm {

// ============================================================================
// NotifyCollectionChangedAction - 集合变更类型
// ============================================================================
enum class NotifyCollectionChangedAction {
    Add,        // 添加项
    Remove,     // 移除项
    Replace,    // 替换项
    Move,       // 移动项
    Reset       // 重置整个集合
};

// ============================================================================
// NotifyCollectionChangedEventArgs - 集合变更事件参数
// ============================================================================
struct NotifyCollectionChangedEventArgs {
    NotifyCollectionChangedAction action;
    
    // 变更的项（单条操作时使用）
    std::any newItem;
    std::any oldItem;
    
    // 变更的位置
    int newStartingIndex;
    int oldStartingIndex;
    
    // 多条变更（批量操作时使用）
    std::vector<std::any> newItems;
    std::vector<std::any> oldItems;
    
    // 构造函数 - 单条添加
    NotifyCollectionChangedEventArgs(NotifyCollectionChangedAction act,
                                      std::any item, int index)
        : action(act), newItem(item), newStartingIndex(index), 
          oldStartingIndex(-1) {}
    
    // 构造函数 - 单条移除
    NotifyCollectionChangedEventArgs(NotifyCollectionChangedAction act,
                                      std::any item, int index, bool isRemove)
        : action(act), oldItem(item), oldStartingIndex(index),
          newStartingIndex(-1) {
        (void)isRemove;  // 仅用于区分构造函数重载
    }
    
    // 构造函数 - 移动
    NotifyCollectionChangedEventArgs(NotifyCollectionChangedAction act,
                                      std::any item, 
                                      int oldIndex, int newIndex)
        : action(act), newItem(item), oldItem(item),
          oldStartingIndex(oldIndex), newStartingIndex(newIndex) {}
    
    // 构造函数 - 重置
    NotifyCollectionChangedEventArgs(NotifyCollectionChangedAction act)
        : action(act), newStartingIndex(-1), oldStartingIndex(-1) {}
    
    // 构造函数 - 批量添加
    NotifyCollectionChangedEventArgs(NotifyCollectionChangedAction act,
                                      const std::vector<std::any>& items, 
                                      int startingIndex)
        : action(act), newItems(items), newStartingIndex(startingIndex),
          oldStartingIndex(-1) {}
};

// ============================================================================
// INotifyCollectionChanged - 集合变更通知接口
// ============================================================================
class INotifyCollectionChanged {
public:
    using CollectionChangedHandler = std::function<void(const NotifyCollectionChangedEventArgs&)>;
    
    virtual ~INotifyCollectionChanged() = default;
    
    // 订阅集合变更事件
    virtual void SubscribeCollectionChanged(CollectionChangedHandler handler) = 0;
    
    // 取消订阅
    virtual void UnsubscribeCollectionChanged(CollectionChangedHandler handler) = 0;
    
protected:
    // 触发集合变更通知
    virtual void OnCollectionChanged(const NotifyCollectionChangedEventArgs& args) = 0;
};

// ============================================================================
// ObservableCollectionBase - 集合基类（C++ 端实现）
// ============================================================================
template<typename T>
class ObservableCollectionBase : public INotifyCollectionChanged {
public:
    ObservableCollectionBase() = default;
    virtual ~ObservableCollectionBase() = default;
    
    // 订阅/取消订阅
    void SubscribeCollectionChanged(CollectionChangedHandler handler) override {
        m_handlers.push_back(handler);
    }
    
    void UnsubscribeCollectionChanged(CollectionChangedHandler handler) override {
        auto it = std::find_if(m_handlers.begin(), m_handlers.end(),
            [&handler](const auto& h) {
                return h.target<void(const NotifyCollectionChangedEventArgs&)>() == 
                       handler.target<void(const NotifyCollectionChangedEventArgs&)>();
            });
        if (it != m_handlers.end()) {
            m_handlers.erase(it);
        }
    }
    
    // 添加项
    void Add(const T& item) {
        int index = static_cast<int>(m_items.size());
        m_items.push_back(item);
        OnCollectionChanged(NotifyCollectionChangedEventArgs(
            NotifyCollectionChangedAction::Add, item, index));
    }
    
    // 插入项
    void Insert(int index, const T& item) {
        if (index < 0 || index > static_cast<int>(m_items.size())) return;
        m_items.insert(m_items.begin() + index, item);
        OnCollectionChanged(NotifyCollectionChangedEventArgs(
            NotifyCollectionChangedAction::Add, item, index));
    }
    
    // 移除项
    bool Remove(const T& item) {
        auto it = std::find(m_items.begin(), m_items.end(), item);
        if (it != m_items.end()) {
            int index = static_cast<int>(it - m_items.begin());
            m_items.erase(it);
            OnCollectionChanged(NotifyCollectionChangedEventArgs(
                NotifyCollectionChangedAction::Remove, item, index, true));
            return true;
        }
        return false;
    }
    
    // 移除指定位置项
    void RemoveAt(int index) {
        if (index < 0 || index >= static_cast<int>(m_items.size())) return;
        T item = m_items[index];
        m_items.erase(m_items.begin() + index);
        OnCollectionChanged(NotifyCollectionChangedEventArgs(
            NotifyCollectionChangedAction::Remove, item, index, true));
    }
    
    // 清空
    void Clear() {
        m_items.clear();
        OnCollectionChanged(NotifyCollectionChangedEventArgs(
            NotifyCollectionChangedAction::Reset));
    }
    
    // 替换项
    void Replace(int index, const T& newItem) {
        if (index < 0 || index >= static_cast<int>(m_items.size())) return;
        T oldItem = m_items[index];
        m_items[index] = newItem;
        OnCollectionChanged(NotifyCollectionChangedEventArgs(
            NotifyCollectionChangedAction::Replace, newItem, oldItem,
            index, index));
    }
    
    // 移动项
    void Move(int oldIndex, int newIndex) {
        if (oldIndex < 0 || oldIndex >= static_cast<int>(m_items.size())) return;
        if (newIndex < 0 || newIndex >= static_cast<int>(m_items.size())) return;
        
        T item = m_items[oldIndex];
        m_items.erase(m_items.begin() + oldIndex);
        m_items.insert(m_items.begin() + newIndex, item);
        
        OnCollectionChanged(NotifyCollectionChangedEventArgs(
            NotifyCollectionChangedAction::Move, item, oldIndex, newIndex));
    }
    
    // 批量添加（优化性能）
    void AddRange(const std::vector<T>& items) {
        if (items.empty()) return;
        int startingIndex = static_cast<int>(m_items.size());
        m_items.insert(m_items.end(), items.begin(), items.end());
        
        std::vector<std::any> anyItems;
        for (const auto& item : items) {
            anyItems.push_back(item);
        }
        OnCollectionChanged(NotifyCollectionChangedEventArgs(
            NotifyCollectionChangedAction::Add, anyItems, startingIndex));
    }
    
    // 查询
    size_t Count() const { return m_items.size(); }
    bool IsEmpty() const { return m_items.empty(); }
    
    T GetItem(int index) const {
        if (index < 0 || index >= static_cast<int>(m_items.size())) {
            throw std::out_of_range("Index out of range");
        }
        return m_items[index];
    }
    
    void SetItem(int index, const T& value) {
        Replace(index, value);
    }
    
    const std::vector<T>& GetItems() const { return m_items; }

protected:
    void OnCollectionChanged(const NotifyCollectionChangedEventArgs& args) override {
        for (const auto& handler : m_handlers) {
            handler(args);
        }
    }

private:
    std::vector<T> m_items;
    std::vector<CollectionChangedHandler> m_handlers;
};

// 常用类型的 ObservableCollection
using ObservableStringCollection = ObservableCollectionBase<std::wstring>;
using ObservableIntCollection = ObservableCollectionBase<int>;
using ObservableDoubleCollection = ObservableCollectionBase<double>;

} // namespace mvvm
} // namespace luaui
