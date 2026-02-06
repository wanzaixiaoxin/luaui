// Hello Lua Example - Basic Lua integration pattern
// Note: This example shows the integration pattern. 
// To actually run Lua, link against lua54.lib
#include <iostream>

// Lua headers (requires linking with lua54.lib)
// extern "C" {
// #include "lua/lua.h"
// #include "lua/lauxlib.h"
// #include "lua/lualib.h"
// }

int main() {
    std::cout << "Lua Integration Example" << std::endl;
    std::cout << "======================" << std::endl;
    std::cout << "This example shows how to integrate Lua:" << std::endl;
    std::cout << "" << std::endl;
    std::cout << "  1. Include lua headers: #include \"lua/lua.h\"" << std::endl;
    std::cout << "  2. Create Lua state: luaL_newstate()" << std::endl;
    std::cout << "  3. Open standard libs: luaL_openlibs()" << std::endl;
    std::cout << "  4. Run Lua code: luaL_dostring()" << std::endl;
    std::cout << "  5. Close state: lua_close()" << std::endl;
    std::cout << "" << std::endl;
    std::cout << "To use: link against lua54.lib" << std::endl;
    
    return 0;
}
