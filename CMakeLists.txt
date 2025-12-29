cmake_minimum_required(VERSION 3.15)
project(barbershop LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# SDL2
set(SDL2_ROOT "${CMAKE_SOURCE_DIR}/external/SDL2-2.32.8")

add_executable(barbershop
    src/main.cpp
    src/engine/render/components/CPU3D.cpp
    src/engine/render/components/CPU3D.h
    src/engine/engTools.cpp
    src/engine/engTools.h
    src/engine/logic2d.cpp
    src/engine/logic2d.h
    src/engine/engconfig.h
    src/engine/engconfig.cpp
    src/engine/import3d.cpp
    src/engine/import3d.h
    src/engine/game.cpp
    src/engine/game.h
    src/engine/render/components/CPU2D.h
    src/engine/render/components/CPU2D.cpp
    src/engine/render/render.h
    src/engine/render/render.cpp
    app_icon.rc
)

# SDL include + lib paths (scoped)
target_include_directories(barbershop PRIVATE
    "${SDL2_ROOT}/include"
)

target_link_directories(barbershop PRIVATE
    "${SDL2_ROOT}/lib/x64"
)

target_link_libraries(barbershop PRIVATE SDL2main SDL2)

# Windows-specific linker handling
if (WIN32)
    if (MSVC)
        target_link_options(barbershop PRIVATE /SUBSYSTEM:CONSOLE)
    else()
        # MinGW / g++
        target_link_options(barbershop PRIVATE -mconsole)
    endif()
endif()

# Export symbols on Windows (kept)
set_target_properties(barbershop PROPERTIES
    WINDOWS_EXPORT_ALL_SYMBOLS ON
)

# Content copy
set(MASTER_CONTENT_DIR "${CMAKE_SOURCE_DIR}/content")

add_custom_command(
    TARGET barbershop PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${MASTER_CONTENT_DIR}"
        "$<TARGET_FILE_DIR:barbershop>/content"
)

file(COPY
    ${CMAKE_SOURCE_DIR}/SDL2.dll
    DESTINATION ${CMAKE_BINARY_DIR}
)