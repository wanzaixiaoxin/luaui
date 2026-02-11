#include "Components/Component.h"

namespace luaui {
namespace components {

void ComponentHolder::InitializeAll() {
    for (auto& [type, component] : m_components) {
        component->Initialize();
    }
}

void ComponentHolder::ShutdownAll() {
    for (auto& [type, component] : m_components) {
        component->Shutdown();
    }
}

} // namespace components
} // namespace luaui
