// ViewModelBase.cpp - ViewModel 基类实现

#include "ViewModelBase.h"
#include <algorithm>

namespace luaui {
namespace mvvm {

ViewModelBase::ViewModelBase() = default;

void ViewModelBase::SubscribePropertyChanged(PropertyChangedHandler handler) {
    if (handler) {
        m_handlers.push_back(handler);
    }
}

void ViewModelBase::UnsubscribePropertyChanged(PropertyChangedHandler handler) {
    auto it = std::find_if(m_handlers.begin(), m_handlers.end(),
        [&handler](const PropertyChangedHandler& h) {
            return h.target_type() == handler.target_type();
        });
    if (it != m_handlers.end()) {
        m_handlers.erase(it);
    }
}

void ViewModelBase::NotifyPropertyChanged(const std::string& propertyName) {
    PropertyChangedEventArgs args{propertyName, std::any{}, std::any{}};
    for (auto& handler : m_handlers) {
        if (handler) {
            handler(args);
        }
    }
}

std::any ViewModelBase::GetPropertyValue(const std::string& propertyName) const {
    auto it = m_getters.find(propertyName);
    if (it != m_getters.end() && it->second) {
        return it->second();
    }
    return std::any{};
}

void ViewModelBase::SetPropertyValue(const std::string& propertyName, const std::any& value) {
    auto it = m_setters.find(propertyName);
    if (it != m_setters.end() && it->second) {
        it->second(value);
    }
}

void ViewModelBase::BeginUpdate() {
    m_updateCount++;
}

void ViewModelBase::EndUpdate() {
    if (m_updateCount > 0) {
        m_updateCount--;
        if (m_updateCount == 0 && m_hasPendingChanges) {
            m_hasPendingChanges = false;
            NotifyPropertyChanged(""); // 空字符串表示所有属性都可能变更
        }
    }
}

bool ViewModelBase::IsUpdating() const {
    return m_updateCount > 0;
}

} // namespace mvvm
} // namespace luaui
