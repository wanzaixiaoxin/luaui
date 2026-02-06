/**
 * @file NumberUtils.cpp
 * @brief Number utility implementations
 */

#include "core/Utils.h"
#include <sstream>
#include <iomanip>

namespace LuaUI {
namespace Utils {

int NumberUtils::toInt(const std::string& str, int defaultValue) {
    try {
        return std::stoi(str);
    } catch (...) {
        return defaultValue;
    }
}

long long NumberUtils::toLong(const std::string& str, long long defaultValue) {
    try {
        return std::stoll(str);
    } catch (...) {
        return defaultValue;
    }
}

double NumberUtils::toDouble(const std::string& str, double defaultValue) {
    try {
        return std::stod(str);
    } catch (...) {
        return defaultValue;
    }
}

std::string NumberUtils::toString(int value) {
    return std::to_string(value);
}

std::string NumberUtils::toString(double value, int precision) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(precision) << value;
    return oss.str();
}

} // namespace utils
} // namespace luaui
