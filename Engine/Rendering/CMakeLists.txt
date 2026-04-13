# Core module for Ferrite

add_library(FerriteEngine-Rendering)

set(FERRITE_RENDERING_BACKEND "VULKAN" CACHE STRING "Rendering backend to use")

target_sources(FerriteEngine-Rendering

    PUBLIC
        FILE_SET CXX_MODULES
        FILES

        Rendering.cppm

        # Config
        Config.cppm

        # Init
        Init.cppm

        # Window Manager
        Window/GLFW.cppm
        Window/WindowUpdate.cppm

        # Base classes
        Base/RenderAPI.cppm
)

find_package(glfw3 REQUIRED)

target_link_libraries(FerriteEngine-Rendering PRIVATE FerriteEngine-Core FerriteEngine-Math glfw)

if ("${FERRITE_RENDERING_BACKEND}" STREQUAL "VULKAN")
    target_compile_definitions(FerriteEngine-Rendering PRIVATE FERRITE_USE_VULKAN)

    find_package(Vulkan REQUIRED)

    target_link_libraries(FerriteEngine-Rendering PRIVATE Vulkan::Vulkan)

elseif("${FERRITE_RENDERING_BACKEND}" STREQUAL "OPENGL")
    target_compile_definitions(FerriteEngine-Rendering PRIVATE FERRITE_USE_OPENGL)

    include(FetchContent)

    FetchContent_Declare(
        glad
        GIT_REPOSITORY https://github.com/Dav1dde/glad.git
        GIT_TAG v2.0.8
        SOURCE_SUBDIR cmake
    )

    FetchContent_MakeAvailable(glad)

    if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        glad_add_library(glad STATIC API gl:core=4.1)
    else()
        glad_add_library(glad STATIC API gl:core=4.6)
    endif()

    target_link_libraries(FerriteEngine-Rendering PRIVATE glad)

endif()
