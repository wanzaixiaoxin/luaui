#pragma once

#include "TreeView.h"

namespace luaui {
namespace controls {

class FileTreeItem : public TreeViewItem {
public:
    FileTreeItem(const std::wstring& path, bool isDirectory);
    ~FileTreeItem() override = default;

    std::string GetTypeName() const override { return "FileTreeItem"; }

    const std::wstring& GetPath() const { return m_path; }
    bool GetIsDirectory() const { return m_isDirectory; }
    bool GetIsLoaded() const { return m_isLoaded; }

    void LoadChildren();

    void SetFileExtensions(const std::vector<std::wstring>& exts) { m_fileExtensions = exts; }

protected:
    void OnRender(rendering::IRenderContext* context) override;

private:
    std::wstring m_path;
    bool m_isDirectory;
    bool m_isLoaded = false;
    std::vector<std::wstring> m_fileExtensions;
};

class FileTree : public TreeView {
public:
    FileTree();
    ~FileTree() override = default;

    std::string GetTypeName() const override { return "FileTree"; }

    void SetRootPath(const std::wstring& path);
    const std::wstring& GetRootPath() const { return m_rootPath; }

    void SetFileExtensions(const std::vector<std::wstring>& exts);
    const std::vector<std::wstring>& GetFileExtensions() const { return m_fileExtensions; }

    luaui::Delegate<FileTree*, const std::wstring&> FileSelected;

private:
    std::wstring m_rootPath;
    std::vector<std::wstring> m_fileExtensions;
};

} // namespace controls
} // namespace luaui
