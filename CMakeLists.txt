cmake_minimum_required (VERSION 2.8.11)
project (Luanatic)
set(CMAKE_CXX_FLAGS "-std=c++11 -fno-exceptions")

find_package(Lua 5.3 REQUIRED)
include_directories (${CMAKE_CURRENT_SOURCE_DIR} /usr/local/include  ${LUA_INCLUDE_DIR})
link_directories(/usr/local/lib)

set (LUANATICDEPS Stick pthread ${LUA_LIBRARIES})

set (LUANATICINC 
Luanatic/Luanatic.hpp
)

install (FILES ${LUANATICINC} DESTINATION /usr/local/include/Luanatic)
add_subdirectory (Tests)
