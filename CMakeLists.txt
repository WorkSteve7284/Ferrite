cmake_minimum_required(VERSION 3.30)

project(Ferrite)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Enable LTO on release builds
if(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
    include(CheckIPOSupported)

    check_ipo_supported(RESULT ipo_supported)
    if (ipo_supported)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
    endif()
endif()

# Compile engine
add_subdirectory(Engine)

# Compile game
set(GAME "" CACHE STRING "Which game to compile")
if(GAME)

    message(STATUS "Compiling ${GAME}")
    add_subdirectory("${CMAKE_SOURCE_DIR}/Games/${GAME}")

else()

    message(WARNING "No game to compile! Only compiling the engine.")

endif()
