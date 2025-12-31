set(TOOLCHAIN_ROOT "${CMAKE_BINARY_DIR}/toolchains")

if (NOT EXISTS "${TOOLCHAIN_ROOT}/mingw64/bin/g++.exe")
    message(STATUS "Extracting bundled MinGW toolchain...")

    file(MAKE_DIRECTORY "${TOOLCHAIN_ROOT}")

    execute_process(
        COMMAND ${CMAKE_COMMAND} -E tar x
                "${CMAKE_SOURCE_DIR}/toolchains/mingw64.7z"
        WORKING_DIRECTORY "${TOOLCHAIN_ROOT}"
        RESULT_VARIABLE _extract_result
    )

    if (NOT _extract_result EQUAL 0)
        message(FATAL_ERROR "Failed to extract MinGW toolchain")
    endif()
endif()
