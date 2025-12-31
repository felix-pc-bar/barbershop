if (NOT EXISTS "${CMAKE_SOURCE_DIR}/toolchains/mingw64/bin/g++.exe")
    message(STATUS "Extracting bundled MinGW toolchain...")

    file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/toolchains/mingw64")

    execute_process(
        COMMAND powershell -Command "Expand-Archive -Path 'mingw64.zip' -DestinationPath 'mingw64'"
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/toolchains"
        RESULT_VARIABLE _extract_result
    )

#    if (NOT _extract_result EQUAL 0)
#        message(FATAL_ERROR "Failed to extract MinGW toolchain")
#    endif()
endif()
