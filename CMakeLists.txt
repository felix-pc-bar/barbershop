cmake_minimum_required(VERSION 3.15)
project(barbershop)

set(CMAKE_CXX_STANDARD 17)

# SDL2 last 
set(SDL2_ROOT "${CMAKE_SOURCE_DIR}/external/SDL2-2.32.8")

include_directories("${SDL2_ROOT}/include")
link_directories("${SDL2_ROOT}/lib/x64")

add_executable(barbershop
 src/main.cpp
 "src/engine/render/CPURenderer.cpp" 
 "src/engine/render/CPURenderer.h" 
 "src/engine/engTools.cpp" 
 "src/engine/engTools.h" 
 "src/engine/logic2d.cpp" 
 "src/engine/logic2d.h" 
 "src/engine/engconfig.h"
 "src/engine/engconfig.cpp"
 "src/engine/import3d.cpp"
 "src/engine/import3d.h"
 "app_icon.rc"
 "src/engine/game.cpp"
 "src/engine/game.h"
  "src/engine/render/CPU2D.h" )
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
set_target_properties(barbershop PROPERTIES
LINK_FLAGS "/SUBSYSTEM:CONSOLE"
WIN32_EXECUTABLE TRUE
)

# Path to master content folder in the source tree
set(MASTER_CONTENT_DIR "${CMAKE_SOURCE_DIR}/content")

# Custom command: copy into the same dir as the executable for the current config

add_custom_command(
    TARGET barbershop PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${MASTER_CONTENT_DIR}" "$<TARGET_FILE_DIR:barbershop>/content"
)

target_link_libraries(barbershop SDL2main SDL2)