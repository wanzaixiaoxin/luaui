#include "DockContainer.h"
#include "IRenderContext.h"
#include "Logger.h"
#include <algorithm>

namespace luaui {
namespace controls {

DockContainer::DockContainer() {}

void DockContainer::InitializeComponents() {
    Panel::InitializeComponents();
}

void DockContainer::SetOrientation(Orientation orientation) {
    if (m_orientation != orientation) {
        m_orientation = orientation;
        if (m_splitter) {
            m_splitter->SetIsVertical(orientation == Orientation::Horizontal);
        }
        if (auto* layout = GetLayout()) {
            layout->InvalidateMeasure();
        }
    }
}

void DockContainer::SetSplitterPosition(float position) {
    position = std::max(0.0f, std::min(1.0f, position));
    if (m_splitterPosition != position) {
        m_splitterPosition = position;
        if (auto* layout = GetLayout()) {
            layout->InvalidateArrange();
        }
        if (auto* render = GetRender()) {
            render->Invalidate();
        }
    }
}

void DockContainer::SetSplitterThickness(float thickness) {
    thickness = std::max(1.0f, thickness);
    if (m_splitterThickness != thickness) {
        m_splitterThickness = thickness;
        if (m_splitter) {
            m_splitter->SetThickness(thickness);
        }
        if (auto* layout = GetLayout()) {
            layout->InvalidateMeasure();
        }
    }
}

void DockContainer::AddDockChild(const std::shared_ptr<Panel>& child) {
    if (!child) return;

    if (!m_firstChild) {
        m_firstChild = child;
        Panel::AddChild(child);
    } else if (!m_secondChild) {
        m_secondChild = child;
        EnsureSplitter();
        Panel::AddChild(m_splitter);
        Panel::AddChild(child);
    } else {
        RemoveDockChild(m_secondChild);
        AddDockChild(child);
    }
}

void DockContainer::RemoveDockChild(const std::shared_ptr<Panel>& child) {
    if (!child) return;

    if (child == m_firstChild) {
        Panel::RemoveChild(child);
        m_firstChild = nullptr;
        if (m_secondChild) {
            Panel::RemoveChild(m_splitter);
            Panel::RemoveChild(m_secondChild);
            m_firstChild = m_secondChild;
            m_secondChild = nullptr;
            RemoveSplitter();
            Panel::AddChild(m_firstChild);
        }
    } else if (child == m_secondChild) {
        Panel::RemoveChild(child);
        m_secondChild = nullptr;
        Panel::RemoveChild(m_splitter);
        RemoveSplitter();
    } else {
        Panel::RemoveChild(child);
    }
}

void DockContainer::AddChild(const std::shared_ptr<interfaces::IControl>& child) {
    if (!child) return;
    auto panel = std::dynamic_pointer_cast<Panel>(child);
    if (panel) {
        AddDockChild(panel);
    } else {
        auto wrapper = std::make_shared<Panel>();
        wrapper->AddChild(child);
        AddDockChild(wrapper);
    }
}

void DockContainer::RemoveChild(const std::shared_ptr<interfaces::IControl>& child) {
    if (!child) return;
    auto panel = std::dynamic_pointer_cast<Panel>(child);
    if (panel) {
        RemoveDockChild(panel);
    } else {
        Panel::RemoveChild(child);
    }
}

void DockContainer::ClearChildren() {
    m_firstChild = nullptr;
    m_secondChild = nullptr;
    RemoveSplitter();
    Panel::ClearChildren();
}

void DockContainer::Split(Orientation orientation, float position, std::shared_ptr<Panel> newChild) {
    if (!newChild) return;

    if (m_firstChild && !m_secondChild) {
        auto oldChild = m_firstChild;
        Panel::RemoveChild(oldChild);
        m_firstChild = nullptr;

        SetOrientation(orientation);
        SetSplitterPosition(position);

        AddDockChild(oldChild);
        AddDockChild(newChild);
    } else if (!m_firstChild) {
        SetOrientation(orientation);
        SetSplitterPosition(position);
        AddDockChild(newChild);
    }
}

std::shared_ptr<Panel> DockContainer::FindLeafAt(float x, float y) {
    if (m_secondChild) {
        auto* render = GetRender();
        if (!render) return nullptr;
        auto rect = render->GetRenderRect();
        if (m_orientation == Orientation::Horizontal) {
            float firstWidth = (rect.width - m_splitterThickness) * m_splitterPosition;
            if (x < firstWidth) {
                if (auto dock = std::dynamic_pointer_cast<DockContainer>(m_firstChild)) {
                    return dock->FindLeafAt(x, y);
                }
                return m_firstChild;
            } else {
                float secondX = firstWidth + m_splitterThickness;
                if (auto dock = std::dynamic_pointer_cast<DockContainer>(m_secondChild)) {
                    return dock->FindLeafAt(x - secondX, y);
                }
                return m_secondChild;
            }
        } else {
            float firstHeight = (rect.height - m_splitterThickness) * m_splitterPosition;
            if (y < firstHeight) {
                if (auto dock = std::dynamic_pointer_cast<DockContainer>(m_firstChild)) {
                    return dock->FindLeafAt(x, y);
                }
                return m_firstChild;
            } else {
                float secondY = firstHeight + m_splitterThickness;
                if (auto dock = std::dynamic_pointer_cast<DockContainer>(m_secondChild)) {
                    return dock->FindLeafAt(x, y - secondY);
                }
                return m_secondChild;
            }
        }
    } else if (m_firstChild) {
        if (auto dock = std::dynamic_pointer_cast<DockContainer>(m_firstChild)) {
            return dock->FindLeafAt(x, y);
        }
        return m_firstChild;
    }
    return nullptr;
}

void DockContainer::EnsureSplitter() {
    if (m_splitter) return;
    m_splitter = std::make_shared<Splitter>();
    m_splitter->SetIsVertical(m_orientation == Orientation::Horizontal);
    m_splitter->SetThickness(m_splitterThickness);
    m_splitterSubscriptionId = m_splitter->PositionChanged.Add(this, &DockContainer::OnSplitterDelta);
}

void DockContainer::RemoveSplitter() {
    if (!m_splitter) return;
    m_splitter->PositionChanged.Remove(m_splitterSubscriptionId);
    m_splitterSubscriptionId = 0;
    m_splitter = nullptr;
}

void DockContainer::OnSplitterDelta(Splitter* /*sender*/, float delta) {
    float total = GetTotalSize();
    if (total <= m_splitterThickness) return;
    float propDelta = delta / (total - m_splitterThickness);
    SetSplitterPosition(m_splitterPosition + propDelta);
}

float DockContainer::GetTotalSize() const {
    auto* render = const_cast<DockContainer*>(this)->GetRender();
    if (!render) return 0.0f;
    return (m_orientation == Orientation::Horizontal)
        ? render->GetRenderRect().width
        : render->GetRenderRect().height;
}

rendering::Size DockContainer::OnMeasureChildren(const rendering::Size& availableSize) {
    if (!m_secondChild) {
        if (m_firstChild) {
            if (auto* layoutable = m_firstChild->AsLayoutable()) {
                interfaces::LayoutConstraint constraint;
                constraint.available = availableSize;
                layoutable->Measure(constraint);
            }
        }
        return availableSize;
    }

    if (m_orientation == Orientation::Horizontal) {
        float contentWidth = std::max(0.0f, availableSize.width - m_splitterThickness);
        float firstWidth = contentWidth * m_splitterPosition;
        float secondWidth = contentWidth - firstWidth;

        if (auto* layoutable = m_firstChild->AsLayoutable()) {
            interfaces::LayoutConstraint constraint;
            constraint.available = rendering::Size(firstWidth, availableSize.height);
            layoutable->Measure(constraint);
        }
        if (m_splitter && m_splitter->AsLayoutable()) {
            interfaces::LayoutConstraint constraint;
            constraint.available = rendering::Size(m_splitterThickness, availableSize.height);
            m_splitter->AsLayoutable()->Measure(constraint);
        }
        if (auto* layoutable = m_secondChild->AsLayoutable()) {
            interfaces::LayoutConstraint constraint;
            constraint.available = rendering::Size(secondWidth, availableSize.height);
            layoutable->Measure(constraint);
        }
    } else {
        float contentHeight = std::max(0.0f, availableSize.height - m_splitterThickness);
        float firstHeight = contentHeight * m_splitterPosition;
        float secondHeight = contentHeight - firstHeight;

        if (auto* layoutable = m_firstChild->AsLayoutable()) {
            interfaces::LayoutConstraint constraint;
            constraint.available = rendering::Size(availableSize.width, firstHeight);
            layoutable->Measure(constraint);
        }
        if (m_splitter && m_splitter->AsLayoutable()) {
            interfaces::LayoutConstraint constraint;
            constraint.available = rendering::Size(availableSize.width, m_splitterThickness);
            m_splitter->AsLayoutable()->Measure(constraint);
        }
        if (auto* layoutable = m_secondChild->AsLayoutable()) {
            interfaces::LayoutConstraint constraint;
            constraint.available = rendering::Size(availableSize.width, secondHeight);
            layoutable->Measure(constraint);
        }
    }

    return availableSize;
}

rendering::Size DockContainer::OnArrangeChildren(const rendering::Size& finalSize) {
    if (!m_secondChild) {
        if (m_firstChild) {
            if (auto* layoutable = m_firstChild->AsLayoutable()) {
                layoutable->Arrange(rendering::Rect(0, 0, finalSize.width, finalSize.height));
            }
        }
        return finalSize;
    }

    if (m_orientation == Orientation::Horizontal) {
        float contentWidth = std::max(0.0f, finalSize.width - m_splitterThickness);
        float firstWidth = contentWidth * m_splitterPosition;
        float secondWidth = contentWidth - firstWidth;

        if (auto* layoutable = m_firstChild->AsLayoutable()) {
            layoutable->Arrange(rendering::Rect(0, 0, firstWidth, finalSize.height));
        }
        if (m_splitter && m_splitter->AsLayoutable()) {
            m_splitter->AsLayoutable()->Arrange(rendering::Rect(firstWidth, 0, m_splitterThickness, finalSize.height));
        }
        if (auto* layoutable = m_secondChild->AsLayoutable()) {
            layoutable->Arrange(rendering::Rect(firstWidth + m_splitterThickness, 0, secondWidth, finalSize.height));
        }
    } else {
        float contentHeight = std::max(0.0f, finalSize.height - m_splitterThickness);
        float firstHeight = contentHeight * m_splitterPosition;
        float secondHeight = contentHeight - firstHeight;

        if (auto* layoutable = m_firstChild->AsLayoutable()) {
            layoutable->Arrange(rendering::Rect(0, 0, finalSize.width, firstHeight));
        }
        if (m_splitter && m_splitter->AsLayoutable()) {
            m_splitter->AsLayoutable()->Arrange(rendering::Rect(0, firstHeight, finalSize.width, m_splitterThickness));
        }
        if (auto* layoutable = m_secondChild->AsLayoutable()) {
            layoutable->Arrange(rendering::Rect(0, firstHeight + m_splitterThickness, finalSize.width, secondHeight));
        }
    }

    return finalSize;
}

} // namespace controls
} // namespace luaui
