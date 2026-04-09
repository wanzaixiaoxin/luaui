#include "ThemeLoader.h"
#include "ThemeKeys.h"
#include "Logger.h"
#include "tinyxml2.h"
#include <sstream>
#include <windows.h>

namespace luaui {
namespace controls {

// ============================================================================
// 内部辅助
// ============================================================================

namespace {

/** 解析 Hex 颜色字符串 (#RRGGBB 或 #AARRGGBB) */
rendering::Color ParseColor(const std::string& str) {
    if (str.empty() || str[0] != '#') {
        return rendering::Color::Transparent();
    }
    try {
        uint32_t hex = static_cast<uint32_t>(std::stoul(str.substr(1), nullptr, 16));
        return rendering::Color::FromHex(hex);
    } catch (...) {
        return rendering::Color::Transparent();
    }
}

/** 解析 Thickness 字符串: "left,top,right,bottom" 或 "all" */
rendering::Thickness ParseThickness(const std::string& str) {
    float values[4] = {0, 0, 0, 0};
    std::istringstream iss(str);
    std::string token;
    int count = 0;

    while (std::getline(iss, token, ',') && count < 4) {
        try {
            values[count++] = std::stof(token);
        } catch (...) {
            // 跳过无效值
        }
    }

    if (count == 1) {
        return rendering::Thickness(values[0]);
    } else if (count == 4) {
        return rendering::Thickness(values[0], values[1], values[2], values[3]);
    }
    return rendering::Thickness(0);
}

/** 解析 CornerRadius 字符串: "topLeft,topRight,bottomRight,bottomLeft" 或 "all" */
rendering::CornerRadius ParseCornerRadius(const std::string& str) {
    float values[4] = {0, 0, 0, 0};
    std::istringstream iss(str);
    std::string token;
    int count = 0;

    while (std::getline(iss, token, ',') && count < 4) {
        try {
            values[count++] = std::stof(token);
        } catch (...) {}
    }

    if (count == 1) {
        return rendering::CornerRadius(values[0]);
    } else if (count == 4) {
        return rendering::CornerRadius(values[0], values[1], values[2], values[3]);
    }
    return rendering::CornerRadius(0);
}

/** 从 XML 根节点加载资源字典 */
ResourceDictionary LoadFromElement(tinyxml2::XMLElement* root) {
    ResourceDictionary dict;

    for (auto* child = root->FirstChildElement(); child; child = child->NextSiblingElement()) {
        std::string tag = child->Name();
        const char* keyAttr = child->Attribute("Key");
        const char* valueAttr = child->Attribute("Value");

        if (!keyAttr || !valueAttr) continue;

        std::string key(keyAttr);
        std::string value(valueAttr);

        if (tag == "Color") {
            dict.AddColor(key, ParseColor(value));
        } else if (tag == "Float") {
            try {
                dict.AddFloat(key, std::stof(value));
            } catch (...) {}
        } else if (tag == "Thickness") {
            dict.AddThickness(key, ParseThickness(value));
        } else if (tag == "CornerRadius") {
            dict.AddCornerRadius(key, ParseCornerRadius(value));
        }
    }

    return dict;
}

} // anonymous namespace

// ============================================================================
// ThemeLoader 公共接口
// ============================================================================

ResourceDictionary ThemeLoader::LoadFromFile(const std::string& filePath) {
    tinyxml2::XMLDocument doc;
    if (doc.LoadFile(filePath.c_str()) != tinyxml2::XML_SUCCESS) {
        utils::Logger::ErrorF("[ThemeLoader] Failed to load theme file: %s", filePath.c_str());
        return ResourceDictionary();
    }

    auto* root = doc.FirstChildElement("Theme");
    if (!root) {
        utils::Logger::ErrorF("[ThemeLoader] Invalid theme file (missing <Theme> root): %s", filePath.c_str());
        return ResourceDictionary();
    }

    auto dict = LoadFromElement(root);

    const char* name = root->Attribute("Name");
    utils::Logger::InfoF("[ThemeLoader] Loaded theme '%s' from %s (%d colors, %d floats)",
        name ? name : "(unnamed)", filePath.c_str(),
        0, 0);  // 简化日志，不遍历计数

    return dict;
}

ResourceDictionary ThemeLoader::LoadFromString(const std::string& xmlContent) {
    tinyxml2::XMLDocument doc;
    if (doc.Parse(xmlContent.c_str()) != tinyxml2::XML_SUCCESS) {
        utils::Logger::Error("[ThemeLoader] Failed to parse theme XML string");
        return ResourceDictionary();
    }

    auto* root = doc.FirstChildElement("Theme");
    if (!root) {
        utils::Logger::Error("[ThemeLoader] Invalid theme XML (missing <Theme> root)");
        return ResourceDictionary();
    }

    return LoadFromElement(root);
}

std::string ThemeLoader::GetDefaultThemesDir() {
    // 获取可执行文件所在目录，拼接 themes/
    char path[MAX_PATH] = {0};
    GetModuleFileNameA(nullptr, path, MAX_PATH);
    std::string exePath(path);
    auto pos = exePath.find_last_of("\\/");
    if (pos != std::string::npos) {
        return exePath.substr(0, pos + 1) + "themes\\";
    }
    return "themes\\";
}

ResourceDictionary ThemeLoader::LoadBuiltinTheme(const std::string& name) {
    // 先尝试从文件加载
    std::string filePath = GetDefaultThemesDir() + name + ".theme.xml";
    tinyxml2::XMLDocument doc;
    if (doc.LoadFile(filePath.c_str()) == tinyxml2::XML_SUCCESS) {
        auto* root = doc.FirstChildElement("Theme");
        if (root) {
            utils::Logger::InfoF("[ThemeLoader] Loaded built-in theme '%s' from file: %s",
                name.c_str(), filePath.c_str());
            return LoadFromElement(root);
        }
    }

    // 文件不存在则返回空字典
    utils::Logger::ErrorF("[ThemeLoader] Theme file not found for '%s': %s",
        name.c_str(), filePath.c_str());
    return ResourceDictionary();
}

} // namespace controls
} // namespace luaui
