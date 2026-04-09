#include "Theme.h"
#include "ThemeKeys.h"
#include "ThemeLoader.h"

namespace luaui {
namespace controls {

Theme& Theme::GetCurrent() {
    static Theme instance;
    static bool initialized = false;
    if (!initialized) {
        initialized = true;
        instance.ApplyThemeByName("Light");
    }
    return instance;
}

void Theme::ApplyThemeByName(const std::string& name) {
    m_currentThemeName = name;
    ResourceDictionary dict = ThemeLoader::LoadBuiltinTheme(name);
    ReplaceResources(dict);
}

void Theme::ApplyThemeFromFile(const std::string& filePath) {
    ResourceDictionary dict = ThemeLoader::LoadFromFile(filePath);
    if (dict.GetColor(theme::kBackgroundPrimary).a <= 0 && dict.GetColor(theme::kTextPrimary).a <= 0) {
        return;
    }

    // 从文件路径提取主题名（去掉目录和扩展名）
    std::string baseName = filePath;
    size_t slashPos = baseName.find_last_of("\\/");
    if (slashPos != std::string::npos) {
        baseName = baseName.substr(slashPos + 1);
    }
    size_t dotPos = baseName.find_last_of('.');
    if (dotPos != std::string::npos) {
        baseName = baseName.substr(0, dotPos);
    }

    m_currentThemeName = baseName;
    ReplaceResources(dict);
}

} // namespace controls
} // namespace luaui
