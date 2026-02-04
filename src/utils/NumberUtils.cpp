/**
 * @file NumberUtils.cpp
 * @brief Number utility implementations
 */

#include "core/Utils.h"
#include <sstream>
#include <iomanip>

namespace luaui {
namespace utils {

int NumberUtils::clamp(int value, int min, int max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

float NumberUtils::clamp(float value, float min, float max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

int NumberUtils::round(float value) {
    return static_cast<int>(value + 0.5f);
}

std::string NumberUtils::toString(int value) {
    return std::to_string(value);
}

std::string NumberUtils::toString(float value, int precision) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(precision) << value;
    return oss.str();
}

bool NumberUtils::tryParse(const std::string& str, int& outValue) {
    try {
        size_t pos = 0;
        outValue = std::stoi(str, &pos);
        return pos == str.size();
    } catch (...) {
        return false;
    }
}

bool NumberUtils::tryParse(const std::string& str, float& outValue) {
    try {
        size_t pos = 0;
        outValue = std::stof(str, &pos);
        return pos == str.size();
    } catch (...) {
        return false;
    }
}

} // namespace utils
} // namespace luaui
