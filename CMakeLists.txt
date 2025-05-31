cmake_minimum_required(VERSION 3.15)
project(barbershop)

set(CMAKE_CXX_STANDARD 17)

# vcpkg toolchain support
if(DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "")
endif()

find_package(SDL2 REQUIRED)

add_executable(barbershop src/main.cpp)
target_link_libraries(barbershop PRIVATE SDL2::SDL2)
