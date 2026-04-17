#pragma once

#include "TabControl.h"
#include <memory>

namespace luaui {
namespace controls {

class DockTabGroup : public TabControl {
public:
    DockTabGroup();

    std::string GetTypeName() const override { return "DockTabGroup"; }

    void AddDockItem(const std::wstring& header, const std::shared_ptr<Control>& content);
    void RemoveDockItem(int index);
};

} // namespace controls
} // namespace luaui
