// 新架构演示 - 展示重构后的控件
#include "MainWindow.h"
#include <iostream>

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "  LuaUI Refactoring Demo" << std::endl;
    std::cout << "  New Architecture Showcase" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;
    
    try {
        demo::MainWindow window;
        window.Run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
