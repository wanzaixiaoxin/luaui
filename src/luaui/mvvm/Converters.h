#pragma once

#include "IBindable.h"
#include "BindingEngine.h"
#include <sstream>
#include <iomanip>
#include <cmath>

namespace luaui {
namespace mvvm {

// ============================================================================
// BooleanToVisibilityConverter - 布尔到可见性转换
// ============================================================================
class BooleanToVisibilityConverter : public IValueConverter {
public:
    std::any Convert(const std::any& value, const std::string& parameter) override {
        (void)parameter;
        try {
            bool b = std::any_cast<bool>(value);
            return b; // 返回bool，控件层解释为Visible/Collapsed
        } catch (...) {
            return false;
        }
    }
    
    std::any ConvertBack(const std::any& value, const std::string& parameter) override {
        (void)parameter;
        try {
            return std::any_cast<bool>(value);
        } catch (...) {
            return false;
        }
    }
};

// ============================================================================
// BooleanInverterConverter - 布尔取反
// ============================================================================
class BooleanInverterConverter : public IValueConverter {
public:
    std::any Convert(const std::any& value, const std::string& parameter) override {
        (void)parameter;
        try {
            return !std::any_cast<bool>(value);
        } catch (...) {
            return false;
        }
    }
    
    std::any ConvertBack(const std::any& value, const std::string& parameter) override {
        (void)parameter;
        try {
            return !std::any_cast<bool>(value);
        } catch (...) {
            return false;
        }
    }
};

// ============================================================================
// ToStringConverter - 任意类型转字符串
// ============================================================================
class ToStringConverter : public IValueConverter {
public:
    std::any Convert(const std::any& value, const std::string& parameter) override {
        (void)parameter;
        
        if (!value.has_value()) {
            return std::string("");
        }
        
        // 处理常见类型
        try {
            if (value.type() == typeid(int)) {
                return std::to_string(std::any_cast<int>(value));
            } else if (value.type() == typeid(float)) {
                float f = std::any_cast<float>(value);
                return FormatFloat(f, parameter);
            } else if (value.type() == typeid(double)) {
                double d = std::any_cast<double>(value);
                return FormatDouble(d, parameter);
            } else if (value.type() == typeid(bool)) {
                return std::any_cast<bool>(value) ? std::string("True") : std::string("False");
            } else if (value.type() == typeid(std::string)) {
                return std::any_cast<std::string>(value);
            } else if (value.type() == typeid(std::wstring)) {
                std::wstring ws = std::any_cast<std::wstring>(value);
                return std::string(ws.begin(), ws.end());
            }
        } catch (...) {}
        
        return std::string("[Error]");
    }
    
    std::any ConvertBack(const std::any& value, const std::string& parameter) override {
        (void)parameter;
        try {
            return std::any_cast<std::string>(value);
        } catch (...) {
            return std::string("");
        }
    }
    
private:
    std::string FormatFloat(float value, const std::string& format) {
        if (format.empty()) {
            return std::to_string(value);
        }
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(ParsePrecision(format)) << value;
        return oss.str();
    }
    
    std::string FormatDouble(double value, const std::string& format) {
        if (format.empty()) {
            return std::to_string(value);
        }
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(ParsePrecision(format)) << value;
        return oss.str();
    }
    
    int ParsePrecision(const std::string& format) {
        try {
            return std::stoi(format);
        } catch (...) {
            return 2;
        }
    }
};

// ============================================================================
// FormatConverter - 格式化字符串
// ============================================================================
class FormatConverter : public IValueConverter {
public:
    std::any Convert(const std::any& value, const std::string& parameter) override {
        if (parameter.empty()) {
            ToStringConverter converter;
            return converter.Convert(value, "");
        }
        
        // parameter 是格式字符串，如 "{0}%" 或 "Name: {0}"
        std::string result = parameter;
        std::string valueStr;
        
        ToStringConverter converter;
        auto strAny = converter.Convert(value, "");
        try {
            valueStr = std::any_cast<std::string>(strAny);
        } catch (...) {
            valueStr = "";
        }
        
        // 替换 {0} 为值
        size_t pos = result.find("{0}");
        if (pos != std::string::npos) {
            result.replace(pos, 3, valueStr);
        }
        
        return result;
    }
    
    std::any ConvertBack(const std::any& value, const std::string& parameter) override {
        (void)parameter;
        try {
            return std::any_cast<std::string>(value);
        } catch (...) {
            return std::string("");
        }
    }
};

// ============================================================================
// NumberRangeConverter - 数值范围转换（如 Slider 0-100 到 Progress 0-1）
// ============================================================================
class NumberRangeConverter : public IValueConverter {
public:
    NumberRangeConverter(double sourceMin = 0, double sourceMax = 100, 
                         double targetMin = 0, double targetMax = 1)
        : m_sourceMin(sourceMin), m_sourceMax(sourceMax)
        , m_targetMin(targetMin), m_targetMax(targetMax) {}
    
    std::any Convert(const std::any& value, const std::string& parameter) override {
        (void)parameter;
        double val = 0;
        
        try {
            if (value.type() == typeid(int)) {
                val = static_cast<double>(std::any_cast<int>(value));
            } else if (value.type() == typeid(float)) {
                val = static_cast<double>(std::any_cast<float>(value));
            } else if (value.type() == typeid(double)) {
                val = std::any_cast<double>(value);
            }
        } catch (...) {
            return 0.0;
        }
        
        // 线性插值
        double ratio = (val - m_sourceMin) / (m_sourceMax - m_sourceMin);
        return m_targetMin + ratio * (m_targetMax - m_targetMin);
    }
    
    std::any ConvertBack(const std::any& value, const std::string& parameter) override {
        (void)parameter;
        double val = 0;
        
        try {
            if (value.type() == typeid(int)) {
                val = static_cast<double>(std::any_cast<int>(value));
            } else if (value.type() == typeid(float)) {
                val = static_cast<double>(std::any_cast<float>(value));
            } else if (value.type() == typeid(double)) {
                val = std::any_cast<double>(value);
            }
        } catch (...) {
            return 0.0;
        }
        
        double ratio = (val - m_targetMin) / (m_targetMax - m_targetMin);
        return m_sourceMin + ratio * (m_sourceMax - m_sourceMin);
    }
    
private:
    double m_sourceMin, m_sourceMax;
    double m_targetMin, m_targetMax;
};

// ============================================================================
// 注册所有默认转换器
// ============================================================================
inline void RegisterDefaultConverters(BindingEngine& engine) {
    engine.RegisterConverter("BooleanToVisibility", std::make_shared<BooleanToVisibilityConverter>());
    engine.RegisterConverter("BooleanInverter", std::make_shared<BooleanInverterConverter>());
    engine.RegisterConverter("ToString", std::make_shared<ToStringConverter>());
    engine.RegisterConverter("Format", std::make_shared<FormatConverter>());
    engine.RegisterConverter("Percent", std::make_shared<FormatConverter>()); // 使用 FormatConverter，参数 "{0}%"
}

} // namespace mvvm
} // namespace luaui
