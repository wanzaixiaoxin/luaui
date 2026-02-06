// LuaUI Test Framework
// A simple header-only testing framework

#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <exception>
#include <sstream>
#include <chrono>
#include <cmath>

namespace luaui {
namespace test {

// Test result tracking
struct TestResult {
    std::string name;
    bool passed;
    std::string errorMessage;
    double durationMs;
};

// Test case base class
class TestCase {
public:
    virtual ~TestCase() = default;
    virtual void Run() = 0;
    virtual std::string GetName() const = 0;
};

using TestCasePtr = std::shared_ptr<TestCase>;

// Test registry
class TestRegistry {
public:
    static TestRegistry& Instance() {
        static TestRegistry instance;
        return instance;
    }
    
    void Register(TestCasePtr test) {
        m_tests.push_back(test);
    }
    
    const std::vector<TestCasePtr>& GetTests() const {
        return m_tests;
    }
    
    void Clear() {
        m_tests.clear();
    }
    
private:
    std::vector<TestCasePtr> m_tests;
};

// Test runner
class TestRunner {
public:
    struct RunResult {
        int totalTests = 0;
        int passedTests = 0;
        int failedTests = 0;
        double totalDurationMs = 0;
        std::vector<TestResult> results;
    };
    
    static RunResult RunAll() {
        RunResult result;
        auto& tests = TestRegistry::Instance().GetTests();
        
        std::cout << "========================================" << std::endl;
        std::cout << "LuaUI Test Framework" << std::endl;
        std::cout << "Running " << tests.size() << " test(s)..." << std::endl;
        std::cout << "========================================" << std::endl;
        
        for (auto& test : tests) {
            TestResult testResult;
            testResult.name = test->GetName();
            testResult.passed = true;
            
            auto start = std::chrono::high_resolution_clock::now();
            
            try {
                test->Run();
            } catch (const std::exception& e) {
                testResult.passed = false;
                testResult.errorMessage = e.what();
            }
            
            auto end = std::chrono::high_resolution_clock::now();
            testResult.durationMs = std::chrono::duration<double, std::milli>(end - start).count();
            
            result.results.push_back(testResult);
            result.totalTests++;
            
            if (testResult.passed) {
                result.passedTests++;
                std::cout << "[PASS] " << testResult.name 
                          << " (" << testResult.durationMs << "ms)" << std::endl;
            } else {
                result.failedTests++;
                std::cout << "[FAIL] " << testResult.name 
                          << " (" << testResult.durationMs << "ms)" << std::endl;
                std::cout << "       Error: " << testResult.errorMessage << std::endl;
            }
            
            result.totalDurationMs += testResult.durationMs;
        }
        
        std::cout << "========================================" << std::endl;
        std::cout << "Results: " << result.passedTests << " passed, " 
                  << result.failedTests << " failed" << std::endl;
        std::cout << "Total time: " << result.totalDurationMs << "ms" << std::endl;
        std::cout << "========================================" << std::endl;
        
        return result;
    }
    
    static int RunAllAndReturn() {
        auto result = RunAll();
        return result.failedTests > 0 ? 1 : 0;
    }
};

// Template test case
class LambdaTestCase : public TestCase {
public:
    LambdaTestCase(const std::string& name, std::function<void()> func)
        : m_name(name), m_func(func) {}
    
    void Run() override {
        m_func();
    }
    
    std::string GetName() const override {
        return m_name;
    }
    
private:
    std::string m_name;
    std::function<void()> m_func;
};

// Test registration helper
class TestRegistrar {
public:
    TestRegistrar(const std::string& name, std::function<void()> func) {
        TestRegistry::Instance().Register(
            std::make_shared<LambdaTestCase>(name, func)
        );
    }
};

// Assertion exception
class AssertionError : public std::exception {
public:
    AssertionError(const std::string& msg) : m_message(msg) {}
    
    const char* what() const noexcept override {
        return m_message.c_str();
    }
    
private:
    std::string m_message;
};

// Assertion helpers
inline void ReportAssertionFailure(const char* expr, const char* file, int line) {
    std::stringstream ss;
    ss << "Assertion failed: " << expr << " at " << file << ":" << line;
    throw AssertionError(ss.str());
}

inline void ReportAssertionFailure(const std::string& msg, const char* file, int line) {
    std::stringstream ss;
    ss << msg << " at " << file << ":" << line;
    throw AssertionError(ss.str());
}

} // namespace test
} // namespace luaui

// ==================== Public API Macros ====================

// Define a test
#define TEST(name) \
    static void test_##name(); \
    static luaui::test::TestRegistrar registrar_##name(#name, test_##name); \
    static void test_##name()

// Run all tests
#define RUN_ALL_TESTS() luaui::test::TestRunner::RunAllAndReturn()

// Basic assertions
#define ASSERT_TRUE(expr) \
    do { \
        if (!(expr)) { \
            luaui::test::ReportAssertionFailure(#expr, __FILE__, __LINE__); \
        } \
    } while(0)

#define ASSERT_FALSE(expr) ASSERT_TRUE(!(expr))

// Equality assertions
#define ASSERT_EQ(expected, actual) \
    do { \
        if ((expected) != (actual)) { \
            std::stringstream ss; \
            ss << "Expected: " << (expected) << ", Actual: " << (actual); \
            luaui::test::ReportAssertionFailure(ss.str(), __FILE__, __LINE__); \
        } \
    } while(0)

#define ASSERT_NE(expected, actual) \
    do { \
        if ((expected) == (actual)) { \
            std::stringstream ss; \
            ss << "Expected not equal, both are: " << (expected); \
            luaui::test::ReportAssertionFailure(ss.str(), __FILE__, __LINE__); \
        } \
    } while(0)

// Float comparison
#define ASSERT_FLOAT_EQ(expected, actual, epsilon) \
    do { \
        if (std::abs((expected) - (actual)) > (epsilon)) { \
            std::stringstream ss; \
            ss << "Expected: " << (expected) << ", Actual: " << (actual) \
               << ", Epsilon: " << (epsilon); \
            luaui::test::ReportAssertionFailure(ss.str(), __FILE__, __LINE__); \
        } \
    } while(0)

#define ASSERT_NEAR(expected, actual, epsilon) ASSERT_FLOAT_EQ(expected, actual, epsilon)

// Null checks
#define ASSERT_NULL(ptr) \
    do { \
        if ((ptr) != nullptr) { \
            luaui::test::ReportAssertionFailure("Expected null, got non-null", __FILE__, __LINE__); \
        } \
    } while(0)

#define ASSERT_NOT_NULL(ptr) \
    do { \
        if ((ptr) == nullptr) { \
            luaui::test::ReportAssertionFailure("Expected non-null, got null", __FILE__, __LINE__); \
        } \
    } while(0)

// Exception assertions
#define ASSERT_THROW(expr, exceptionType) \
    do { \
        bool caught = false; \
        try { expr; } \
        catch (const exceptionType&) { caught = true; } \
        catch (...) { } \
        if (!caught) { \
            luaui::test::ReportAssertionFailure("Expected exception not thrown", __FILE__, __LINE__); \
        } \
    } while(0)

#define ASSERT_NO_THROW(expr) \
    do { \
        try { expr; } \
        catch (...) { \
            luaui::test::ReportAssertionFailure("Unexpected exception thrown", __FILE__, __LINE__); \
        } \
    } while(0)

// Boolean checks
#define EXPECT_TRUE(expr) ASSERT_TRUE(expr)
#define EXPECT_FALSE(expr) ASSERT_FALSE(expr)
#define EXPECT_EQ(a, b) ASSERT_EQ(a, b)
#define EXPECT_NE(a, b) ASSERT_NE(a, b)
