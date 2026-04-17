#include "DockTabGroup.h"

namespace luaui {
namespace controls {

DockTabGroup::DockTabGroup() {}

void DockTabGroup::AddDockItem(const std::wstring& header, const std::shared_ptr<Control>& content) {
    auto item = std::make_shared<TabItem>();
    item->SetHeader(header);
    item->SetContent(content);
    AddTab(item);
}

void DockTabGroup::RemoveDockItem(int index) {
    RemoveTab(index);
}

} // namespace controls
} // namespace luaui
