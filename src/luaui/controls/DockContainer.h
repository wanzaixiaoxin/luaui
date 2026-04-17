#pragma once

#include "Panel.h"
#include "Splitter.h"
#include <memory>

namespace luaui {
namespace controls {

class DockContainer : public Panel {
public:
    enum class Orientation { Horizontal, Vertical };

    DockContainer();

    std::string GetTypeName() const override { return "DockContainer"; }

    void SetOrientation(Orientation orientation);
    Orientation GetOrientation() const { return m_orientation; }

    void SetSplitterPosition(float position);
    float GetSplitterPosition() const { return m_splitterPosition; }

    void SetSplitterThickness(float thickness);
    float GetSplitterThickness() const { return m_splitterThickness; }

    void AddChild(const std::shared_ptr<interfaces::IControl>& child) override;
    void RemoveChild(const std::shared_ptr<interfaces::IControl>& child) override;
    void ClearChildren() override;

    void AddDockChild(const std::shared_ptr<Panel>& child);
    void RemoveDockChild(const std::shared_ptr<Panel>& child);

    void Split(Orientation orientation, float position, std::shared_ptr<Panel> newChild);

    std::shared_ptr<Panel> FindLeafAt(float x, float y);

protected:
    void InitializeComponents() override;
    rendering::Size OnMeasureChildren(const rendering::Size& availableSize) override;
    rendering::Size OnArrangeChildren(const rendering::Size& finalSize) override;

private:
    void EnsureSplitter();
    void RemoveSplitter();
    void OnSplitterDelta(Splitter* sender, float delta);
    float GetTotalSize() const;

    Orientation m_orientation = Orientation::Horizontal;
    float m_splitterPosition = 0.5f;
    float m_splitterThickness = 4.0f;
    std::shared_ptr<Splitter> m_splitter;
    std::shared_ptr<Panel> m_firstChild;
    std::shared_ptr<Panel> m_secondChild;
    uint32_t m_splitterSubscriptionId = 0;
};

} // namespace controls
} // namespace luaui
