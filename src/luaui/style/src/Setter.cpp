#include "Setter.h"
#include "Logger.h"

namespace luaui {
namespace controls {

void Setter::Apply(Control* target, const ResourceDictionary* resources) const {
    if (!target || m_propertyId == 0) {
        return;
    }
    
    auto value = GetResolvedValue(resources);
    if (value.has_value()) {
        target->SetValue(m_propertyId, value);
    }
}

std::any Setter::GetResolvedValue(const ResourceDictionary* resources) const {
    if (std::holds_alternative<std::any>(m_value)) {
        return std::get<std::any>(m_value);
    }
    
    if (std::holds_alternative<ResourceReference>(m_value)) {
        const auto& ref = std::get<ResourceReference>(m_value);
        if (!ref.IsEmpty() && resources) {
            return ref.Resolve(resources, nullptr);
        }
    }
    
    return std::any();
}

bool Setter::HasValue() const {
    if (std::holds_alternative<std::any>(m_value)) {
        return std::get<std::any>(m_value).has_value();
    }
    if (std::holds_alternative<ResourceReference>(m_value)) {
        return !std::get<ResourceReference>(m_value).IsEmpty();
    }
    return false;
}

} // namespace controls
} // namespace luaui
