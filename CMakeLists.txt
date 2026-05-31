cmake_minimum_required(VERSION 3.15)

# include(${CMAKE_SOURCE_DIR}/cmake/bootstrap_toolchain.cmake)
#message(STATUS "ENV PATH = $ENV{PATH}")
message(STATUS "CMAKE_SOURCE_DIR = ${CMAKE_SOURCE_DIR}")

set(CMAKE_CXX_COMPILER_FORCED TRUE)
project(barbershop LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# SDL2
set(SDL2_ROOT "${CMAKE_SOURCE_DIR}/external/SDL2")

add_executable(barbershop
    src/main.cpp

    src/engine/game.h
    src/engine/game.cpp

	src/engine/render/render.h
	src/engine/render/render.cpp
	src/engine/render/components/CPU2D.h
	src/engine/render/components/CPU2D.cpp

	src/engine/general2d.h
	src/engine/general2d.cpp

	src/engine/material.h
	src/engine/material.cpp

	src/engine/globals.h
	src/engine/globals.cpp
)

# SDL include + lib paths (scoped)
target_include_directories(barbershop PRIVATE
    "${SDL2_ROOT}/include"
)

target_link_directories(barbershop PRIVATE
    "${SDL2_ROOT}/lib/x64"
)

# TODO: we need to fix that issue on freshly cloning the repo
target_link_libraries(barbershop PRIVATE SDL2main SDL2)

# Windows-specific linker handling
if(WIN32)
    if(MSVC)
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

if(WIN32)
	add_custom_target(link_compile_commands ALL
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
			${CMAKE_BINARY_DIR}/compile_commands.json
			${CMAKE_SOURCE_DIR}/compile_commands.json
	)
else()
	add_custom_target(link_compile_commands ALL
		COMMAND ${CMAKE_COMMAND} -E create_symlink
			${CMAKE_BINARY_DIR}/compile_commands.json
			${CMAKE_SOURCE_DIR}/compile_commands.json
	)
endif()

file(COPY
    ${CMAKE_SOURCE_DIR}/external/SDL2/lib/x64/SDL2.dll
    DESTINATION ${CMAKE_BINARY_DIR}
)
