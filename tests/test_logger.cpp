// Logger Module Unit Tests
#include "TestFramework.h"
#include "Logger.h"
#include <sstream>
#include <fstream>
#include <filesystem>

using namespace luaui::utils;

// ==================== LogLevel Tests ====================
TEST(LogLevel_ToString) {
    ASSERT_EQ(std::string(LogLevelToString(LogLevel::Debug)), "DEBUG");
    ASSERT_EQ(std::string(LogLevelToString(LogLevel::Info)), "INFO");
    ASSERT_EQ(std::string(LogLevelToString(LogLevel::Warning)), "WARN");
    ASSERT_EQ(std::string(LogLevelToString(LogLevel::Error)), "ERROR");
    ASSERT_EQ(std::string(LogLevelToString(LogLevel::Fatal)), "FATAL");
}

// ==================== ConsoleLogger Tests ====================
TEST(ConsoleLogger_Construction) {
    ConsoleLogger logger;
    ASSERT_TRUE(logger.GetLevel() == LogLevel::Debug);
    ASSERT_TRUE(logger.IsEnabled());
    ASSERT_TRUE(logger.IsColored());
}

TEST(ConsoleLogger_SetLevel) {
    ConsoleLogger logger;
    logger.SetLevel(LogLevel::Warning);
    ASSERT_TRUE(logger.GetLevel() == LogLevel::Warning);
}

TEST(ConsoleLogger_EnableDisable) {
    ConsoleLogger logger;
    logger.SetEnabled(false);
    ASSERT_FALSE(logger.IsEnabled());
    logger.SetEnabled(true);
    ASSERT_TRUE(logger.IsEnabled());
}

TEST(ConsoleLogger_Colored) {
    ConsoleLogger logger;
    logger.SetColored(false);
    ASSERT_FALSE(logger.IsColored());
    logger.SetColored(true);
    ASSERT_TRUE(logger.IsColored());
}

// ==================== FileLogger Tests ====================
TEST(FileLogger_ConstructionAndLogging) {
    std::string testFile = "test_log.txt";
    
    // Remove file if exists
    std::filesystem::remove(testFile);
    
    {
        FileLogger logger(testFile);
        ASSERT_TRUE(logger.GetLevel() == LogLevel::Debug);
        ASSERT_TRUE(logger.IsEnabled());
        
        logger.Log(LogLevel::Info, "Test message 1");
        logger.Log(LogLevel::Warning, "Test message 2");
        logger.Flush();
    }
    
    // Verify file was created and contains messages
    ASSERT_TRUE(std::filesystem::exists(testFile));
    
    std::ifstream file(testFile);
    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    
    ASSERT_TRUE(content.find("Test message 1") != std::string::npos);
    ASSERT_TRUE(content.find("Test message 2") != std::string::npos);
    ASSERT_TRUE(content.find("INFO") != std::string::npos);
    ASSERT_TRUE(content.find("WARN") != std::string::npos);
    
    // Cleanup
    std::filesystem::remove(testFile);
}

TEST(FileLogger_SetLevel) {
    std::string testFile = "test_log_level.txt";
    std::filesystem::remove(testFile);
    
    {
        FileLogger logger(testFile);
        logger.SetLevel(LogLevel::Warning);
        
        // Debug should not be logged
        logger.Log(LogLevel::Debug, "Debug message");
        logger.Log(LogLevel::Warning, "Warning message");
        logger.Flush();
    }
    
    std::ifstream file(testFile);
    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    
    // Should not contain debug message
    ASSERT_TRUE(content.find("Debug message") == std::string::npos);
    ASSERT_TRUE(content.find("Warning message") != std::string::npos);
    
    std::filesystem::remove(testFile);
}

TEST(FileLogger_EnableDisable) {
    std::string testFile = "test_log_disable.txt";
    std::filesystem::remove(testFile);
    
    {
        FileLogger logger(testFile);
        logger.SetEnabled(false);
        logger.Log(LogLevel::Info, "This should not be logged");
        logger.SetEnabled(true);
        logger.Log(LogLevel::Info, "This should be logged");
        logger.Flush();
    }
    
    std::ifstream file(testFile);
    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    
    ASSERT_TRUE(content.find("This should not be logged") == std::string::npos);
    ASSERT_TRUE(content.find("This should be logged") != std::string::npos);
    
    std::filesystem::remove(testFile);
}

// ==================== MultiLogger Tests ====================
TEST(MultiLogger_AddRemove) {
    MultiLogger multi;
    auto logger1 = std::make_shared<ConsoleLogger>();
    auto logger2 = std::make_shared<ConsoleLogger>();
    
    multi.AddLogger(logger1);
    multi.AddLogger(logger2);
    
    // Should not throw
    multi.Log(LogLevel::Info, "Test message");
}

TEST(MultiLogger_Clear) {
    MultiLogger multi;
    auto logger = std::make_shared<ConsoleLogger>();
    
    multi.AddLogger(logger);
    multi.ClearLoggers();
    
    // Should not throw even with no loggers
    multi.Log(LogLevel::Info, "Test message");
}

// ==================== Global Logger Tests ====================
TEST(GlobalLogger_InitializeWithConsole) {
    Logger::Shutdown();
    
    Logger::Initialize();
    ASSERT_TRUE(Logger::IsInitialized());
    
    // Should be able to log
    ASSERT_NO_THROW(Logger::Info("Test message"));
    
    Logger::Shutdown();
}

TEST(GlobalLogger_InitializeWithFile) {
    Logger::Shutdown();
    std::string testFile = "global_test_log.txt";
    std::filesystem::remove(testFile);
    
    Logger::Initialize(testFile);
    ASSERT_TRUE(Logger::IsInitialized());
    
    Logger::Info("Global logger test");
    
    Logger::Shutdown();
    
    // Verify file was created
    ASSERT_TRUE(std::filesystem::exists(testFile));
    
    std::ifstream file(testFile);
    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    ASSERT_TRUE(content.find("Global logger test") != std::string::npos);
    
    std::filesystem::remove(testFile);
}

TEST(GlobalLogger_InitializeWithConfig) {
    Logger::Shutdown();
    
    LoggerConfig config;
    config.consoleEnabled = false;
    config.fileEnabled = false;
    
    Logger::Initialize(config);
    ASSERT_TRUE(Logger::IsInitialized());
    
    // Should not throw even with disabled outputs
    ASSERT_NO_THROW(Logger::Info("Test message"));
    
    Logger::Shutdown();
}

TEST(GlobalLogger_LevelFiltering) {
    Logger::Shutdown();
    
    Logger::Initialize();
    Logger::SetConsoleLevel(LogLevel::Warning);
    
    // Debug and Info should be filtered out
    ASSERT_NO_THROW(Logger::Debug("Debug"));
    ASSERT_NO_THROW(Logger::Info("Info"));
    ASSERT_NO_THROW(Logger::Warning("Warning"));
    ASSERT_NO_THROW(Logger::Error("Error"));
    
    Logger::Shutdown();
}

TEST(GlobalLogger_FormattedLogging) {
    Logger::Shutdown();
    
    Logger::Initialize();
    
    // Should not throw
    ASSERT_NO_THROW(Logger::InfoF("Formatted %s %d", "test", 42));
    ASSERT_NO_THROW(Logger::DebugF("Debug %f", 3.14));
    ASSERT_NO_THROW(Logger::WarningF("Warning %s", "message"));
    ASSERT_NO_THROW(Logger::ErrorF("Error %d", 100));
    
    Logger::Shutdown();
}

// ==================== LoggerConfig Tests ====================
TEST(LoggerConfig_DefaultValues) {
    LoggerConfig config;
    
    ASSERT_TRUE(config.consoleEnabled);
    ASSERT_FALSE(config.fileEnabled);
    // Compare enum values
    ASSERT_TRUE(config.consoleLevel == LogLevel::Debug);
    ASSERT_TRUE(config.fileLevel == LogLevel::Debug);
    ASSERT_EQ(config.logFilePath, "app.log");
    ASSERT_TRUE(config.consoleColored);
    ASSERT_FALSE(config.useStderr);
    ASSERT_FALSE(config.createConsoleWindow);
    ASSERT_EQ(config.consoleWindowTitle, "Debug Console");
}

// ==================== Main ====================
int main() {
    return RUN_ALL_TESTS();
}
