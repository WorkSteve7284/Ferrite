cmake_minimum_required(VERSION 3.30)

project(Ferrite)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Debug)

# Compile engine
add_subdirectory(engine)

# Compile external
add_subdirectory(external/glfw)
#add_subdirectory(external/glad)

# Compile game
set(GAME "" CACHE STRING "Which game to compile")
if(GAME)

    message(STATUS "Compiling ${GAME}")
    add_subdirectory(games/${GAME})

else()

    message(WARNING "No game to compile! Only compiling the engine.")

endif()
