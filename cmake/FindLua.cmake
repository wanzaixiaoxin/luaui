# Find Lua library (bundled version)

set(LUA_FOUND TRUE)
set(LUA_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/third_party/lua)
set(LUA_LIBRARIES lua)

# For bundled version, we don't need to search
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Lua DEFAULT_MSG LUA_INCLUDE_DIR LUA_LIBRARIES)
