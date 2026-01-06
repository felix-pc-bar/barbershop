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

# If we have a bundled mingw, prepend its bin to PATH and set compilers if not already configured.
if (WIN32 AND EXISTS "${CMAKE_SOURCE_DIR}/toolchains/mingw64/bin/g++.exe")
    set(_mingw_bin "${CMAKE_SOURCE_DIR}/toolchains/mingw64/bin")

    # Prepend mingw bin to PATH so gcc/g++ can find 'as', 'ld', etc.
    if(DEFINED ENV{PATH})
        set(ENV{PATH} "${CMAKE_SOURCE_DIR}/toolchains/mingw64/bin;${CMAKE_SOURCE_DIR}/toolchains;$ENV{PATH}")
    else()
        set(ENV{PATH} "${CMAKE_SOURCE_DIR}/toolchains/mingw64/bin;${CMAKE_SOURCE_DIR}/toolchains")
    endif()

    message(STATUS "Bundled MinGW detected, prepending ${_mingw_bin} to PATH")

    # Only configure compilers if they are not already set (presets/user overrides win)
    if(NOT CMAKE_C_COMPILER)
        set(CMAKE_C_COMPILER "${_mingw_bin}/gcc.exe" CACHE FILEPATH "Bundled MinGW gcc" )
        message(STATUS "Setting CMAKE_C_COMPILER to ${CMAKE_C_COMPILER}")
    endif()
    if(NOT CMAKE_CXX_COMPILER)
        set(CMAKE_CXX_COMPILER "${_mingw_bin}/g++.exe" CACHE FILEPATH "Bundled MinGW g++" )
        message(STATUS "Setting CMAKE_CXX_COMPILER to ${CMAKE_CXX_COMPILER}")
    endif()
    if(NOT CMAKE_RC_COMPILER AND EXISTS "${_mingw_bin}/windres.exe")
        set(CMAKE_RC_COMPILER "${_mingw_bin}/windres.exe" CACHE FILEPATH "Bundled MinGW windres" )
        message(STATUS "Setting CMAKE_RC_COMPILER to ${CMAKE_RC_COMPILER}")
    endif()
endif()