#include "XmlLayout.h"
#include <sstream>
#include <algorithm>
#include <cctype>
#include <iomanip>

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

bool ToDouble(const std::string& str, double& out) {
    try {
        out = std::stod(str);
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

rendering::Color HexToColor(const std::string& hex) {
    std::string cleanHex = hex;
    
    // 去除 # 前缀
    if (!cleanHex.empty() && cleanHex[0] == '#') {
        cleanHex = cleanHex.substr(1);
    }
    
    // 去除 0x 前缀
    if (cleanHex.length() > 1 && cleanHex[0] == '0' && 
        (cleanHex[1] == 'x' || cleanHex[1] == 'X')) {
        cleanHex = cleanHex.substr(2);
    }
    
    // 确保长度为 6 或 8（带 alpha）
    if (cleanHex.length() != 6 && cleanHex.length() != 8) {
        return rendering::Color::Black();
    }
    
    try {
        unsigned int value = std::stoul(cleanHex, nullptr, 16);
        
        if (cleanHex.length() == 6) {
            // RGB 格式 - 转换为 0-1 范围
            float r = ((value >> 16) & 0xFF) / 255.0f;
            float g = ((value >> 8) & 0xFF) / 255.0f;
            float b = (value & 0xFF) / 255.0f;
            return rendering::Color(r, g, b);
        } else {
            // ARGB 格式 - 转换为 0-1 范围
            float a = ((value >> 24) & 0xFF) / 255.0f;
            float r = ((value >> 16) & 0xFF) / 255.0f;
            float g = ((value >> 8) & 0xFF) / 255.0f;
            float b = (value & 0xFF) / 255.0f;
            return rendering::Color(r, g, b, a);
        }
    } catch (...) {
        return rendering::Color::Black();
    }
}

bool ToColor(const std::string& str, rendering::Color& out) {
    std::string lower = str;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    
    // 预定义颜色
    if (lower == "white") { out = rendering::Color::White(); return true; }
    if (lower == "black") { out = rendering::Color::Black(); return true; }
    if (lower == "red") { out = rendering::Color::Red(); return true; }
    if (lower == "green") { out = rendering::Color::Green(); return true; }
    if (lower == "blue") { out = rendering::Color::Blue(); return true; }
    if (lower == "yellow") { out = rendering::Color::Yellow(); return true; }
    if (lower == "cyan") { out = rendering::Color::Cyan(); return true; }
    if (lower == "magenta") { out = rendering::Color::Magenta(); return true; }
    if (lower == "transparent") { out = rendering::Color::Transparent(); return true; }
    if (lower == "gray" || lower == "grey") { out = rendering::Color(0.5f, 0.5f, 0.5f); return true; }
    
    // Hex 格式
    if (!str.empty() && (str[0] == '#' || str.find("0x") == 0)) {
        out = HexToColor(str);
        return true;
    }
    
    // RGB/RGBA 格式: rgb(255, 0, 0) 或 rgba(255, 0, 0, 128)
    if (lower.find("rgb") == 0) {
        size_t start = lower.find('(');
        size_t end = lower.find(')');
        if (start != std::string::npos && end != std::string::npos) {
            std::string values = lower.substr(start + 1, end - start - 1);
            std::istringstream iss(values);
            std::string rStr, gStr, bStr, aStr;
            
            if (std::getline(iss, rStr, ',') && 
                std::getline(iss, gStr, ',') && 
                std::getline(iss, bStr, ',')) {
                try {
                    // 转换为 0-1 范围
                    float r = std::stoi(rStr) / 255.0f;
                    float g = std::stoi(gStr) / 255.0f;
                    float b = std::stoi(bStr) / 255.0f;
                    
                    // 检查是否有 alpha
                    if (std::getline(iss, aStr, ',')) {
                        float a = std::stoi(aStr) / 255.0f;
                        out = rendering::Color(r, g, b, a);
                    } else {
                        out = rendering::Color(r, g, b);
                    }
                    return true;
                } catch (...) {
                    return false;
                }
            }
        }
    }
    
    return false;
}

bool ToThickness(const std::string& str, rendering::Thickness& out) {
    std::istringstream iss(str);
    std::vector<float> values;
    std::string token;
    
    while (std::getline(iss, token, ',')) {
        try {
            values.push_back(std::stof(token));
        } catch (...) {
            return false;
        }
    }
    
    switch (values.size()) {
        case 1:
            // 统一值
            out = rendering::Thickness(values[0], values[0], values[0], values[0]);
            return true;
        case 2:
            // 水平, 垂直
            out = rendering::Thickness(values[0], values[1], values[0], values[1]);
            return true;
        case 4:
            // left, top, right, bottom
            out = rendering::Thickness(values[0], values[1], values[2], values[3]);
            return true;
        default:
            return false;
    }
}

} // namespace TypeConverter
} // namespace xml
} // namespace luaui
