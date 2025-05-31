cmake_minimum_required(VERSION 3.10)
project(barbershop)

set(CMAKE_CXX_STANDARD 17)

# Add source files
file(GLOB SOURCES "src/*.cpp")

# Find SDL2
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

add_executable(barbershop ${SOURCES})
target_link_libraries(barbershop ${SDL2_LIBRARIES})
