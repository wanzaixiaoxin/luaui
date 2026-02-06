// Simple test
#include <iostream>
#include "tinyxml2.h"

int main() {
    std::cout << "Running simple test..." << std::endl;
    
    // Test tinyxml2
    tinyxml2::XMLDocument doc;
    doc.Parse("<test>Hello</test>");
    
    auto root = doc.RootElement();
    if (root && std::string(root->GetText()) == "Hello") {
        std::cout << "TinyXML2 test passed!" << std::endl;
        return 0;
    }
    
    std::cerr << "Test failed!" << std::endl;
    return 1;
}
