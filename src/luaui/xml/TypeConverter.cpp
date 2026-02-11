#include "XmlLayout.h"
#include <sstream>
#include <algorithm>
#include <cctype>

namespace luaui {
namespace xml {

namespace TypeConverter {

bool ToBool(const std::string& str, bool& out) {
    std::string lower = str;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    
    if (lower == "true" || lower == "1" || lower == "yes") {
        out = true;
        return true;
    }
    if (lower == "false" || lower == "0" || lower == "no") {
        out = false;
        return true;
    }
    return false;
}

bool ToFloat(const std::string& str, float& out) {
    try {
        out = std::stof(str);
        return true;
    } catch (...) {
        return false;
    }
}

bool ToInt(const std::string& str, int& out) {
    try {
        out = std::stoi(str);
        return true;
    } catch (...) {
        return false;
    }
}

bool ToColor(const std::string& str, rendering::Color& out) {
    // 支持格式：#RRGGBB, #AARRGGBB, Red, Green, Blue, White, Black, Transparent
    std::string lower = str;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    
    if (lower == "red") {
        out = rendering::Color::Red();
        return true;
    }
    if (lower == "green") {
        out = rendering::Color::Green();
        return true;
    }
    if (lower == "blue") {
        out = rendering::Color::Blue();
        return true;
    }
    if (lower == "white") {
        out = rendering::Color::White();
        return true;
    }
    if (lower == "black") {
        out = rendering::Color::Black();
        return true;
    }
    if (lower == "transparent") {
        out = rendering::Color::Transparent();
        return true;
    }
    
    // Hex 颜色
    if (str[0] == '#') {
        out = HexToColor(str);
        return true;
    }
    
    return false;
}

rendering::Color HexToColor(const std::string& hex) {
    return rendering::Color::FromHex(static_cast<uint32_t>(std::stoul(hex.substr(1), nullptr, 16)));
}

} // namespace TypeConverter

} // namespace xml
} // namespace luaui
