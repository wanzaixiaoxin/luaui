#include "FileTree.h"
#include "IRenderContext.h"
#include "Theme.h"
#include "ThemeKeys.h"

#include <algorithm>
#include <filesystem>

namespace luaui {
namespace controls {

// ============================================================================
// FileTreeItem
// ============================================================================
FileTreeItem::FileTreeItem(const std::wstring& path, bool isDirectory)
    : m_path(path), m_isDirectory(isDirectory) {
    if (!path.empty()) {
        std::filesystem::path fsPath(path);
        SetHeader(fsPath.filename().wstring());
    }

    if (isDirectory) {
        auto placeholder = std::make_shared<TreeViewItem>();
        placeholder->SetHeader(L"");
        TreeViewItem::AddItem(placeholder);
    }
}

void FileTreeItem::LoadChildren() {
    if (m_isLoaded || !m_isDirectory) return;

    TreeViewItem::ClearItems();

    std::vector<std::shared_ptr<FileTreeItem>> folders;
    std::vector<std::shared_ptr<FileTreeItem>> files;

    try {
        std::filesystem::path dirPath(m_path);

        for (const auto& entry : std::filesystem::directory_iterator(dirPath)) {
            bool isDir = entry.is_directory();
            auto name = entry.path().filename().wstring();

            if (!name.empty() && name[0] == L'.') continue;

            if (!isDir && !m_fileExtensions.empty()) {
                auto ext = entry.path().extension().wstring();
                bool match = false;
                for (size_t i = 0; i < m_fileExtensions.size(); ++i) {
                    if (_wcsicmp(ext.c_str(), m_fileExtensions[i].c_str()) == 0) {
                        match = true;
                        break;
                    }
                }
                if (!match) continue;
            }

            auto item = std::make_shared<FileTreeItem>(
                entry.path().wstring(), isDir);
            item->SetFileExtensions(m_fileExtensions);

            if (isDir) {
                folders.push_back(item);
            } else {
                files.push_back(item);
            }
        }
    } catch (const std::filesystem::filesystem_error&) {
    }

    struct NameSort {
        bool operator()(const std::shared_ptr<FileTreeItem>& a,
                        const std::shared_ptr<FileTreeItem>& b) const {
            return _wcsicmp(a->GetHeader().c_str(), b->GetHeader().c_str()) < 0;
        }
    };
    std::sort(folders.begin(), folders.end(), NameSort());
    std::sort(files.begin(), files.end(), NameSort());

    for (size_t i = 0; i < folders.size(); ++i) {
        TreeViewItem::AddItem(folders[i]);
    }
    for (size_t i = 0; i < files.size(); ++i) {
        TreeViewItem::AddItem(files[i]);
    }

    m_isLoaded = true;
}

void FileTreeItem::OnRender(rendering::IRenderContext* context) {
    if (!context) return;

    auto* render = GetRender();
    if (!render) return;

    auto rect = render->GetRenderRect();

    auto& t = Theme::GetCurrent();
    using namespace theme;

    float iconX = rect.x + static_cast<float>(GetLevel()) * 20.0f + 4.0f + 16.0f + 4.0f;
    float iconY = rect.y + (rect.height - 12.0f) / 2;

    auto iconBrush = context->CreateSolidColorBrush(t.GetColor(kTreeViewExpandBtn));
    auto textFormat = context->CreateTextFormat(L"Microsoft YaHei", 10.0f);
    if (iconBrush && textFormat) {
        const wchar_t* icon = m_isDirectory ? L"D" : L"F";
        context->DrawTextString(icon, textFormat.get(),
                                rendering::Point(iconX, iconY), iconBrush.get());
    }

    TreeViewItem::OnRender(context);
}

// ============================================================================
// FileTree
// ============================================================================
FileTree::FileTree() {}

void FileTree::SetRootPath(const std::wstring& path) {
    ClearRoots();
    m_rootPath = path;

    if (!path.empty()) {
        auto root = std::make_shared<FileTreeItem>(path, true);
        root->SetFileExtensions(m_fileExtensions);
        root->SetIsExpanded(false);
        AddRoot(root);
    }
}

void FileTree::SetFileExtensions(const std::vector<std::wstring>& exts) {
    m_fileExtensions = exts;
}

} // namespace controls
} // namespace luaui
