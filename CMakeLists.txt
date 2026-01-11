cmake_minimum_required(VERSION 3.30)

project(Ferrite)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")

# Compile engine
add_subdirectory(engine)

# Compile external

# Compile game
set(GAME "" CACHE STRING "Which game to compile")
if(GAME)

    message(STATUS "Compiling ${GAME}")
    add_subdirectory(games/${GAME})

else()

    message(WARNING "No game to compile! Only compiling the engine.")

endif()
