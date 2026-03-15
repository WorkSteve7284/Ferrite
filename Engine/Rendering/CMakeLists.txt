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
)



target_link_libraries(FerriteEngine-Rendering PRIVATE FerriteEngine-Core glfw)

if ("${FERRITE_RENDERING_BACKEND}" STREQUAL "VULKAN")
    target_compile_definitions(FerriteEngine-Rendering PRIVATE FERRITE_USE_VULKAN)

    find_package(Vulkan REQUIRED)

    add_library(vulkan_module)
    target_sources(vulkan_module PUBLIC
        FILE_SET CXX_MODULES
        BASE_DIRS "$ENV{VULKAN_SDK}/Include"
        FILES "$ENV{VULKAN_SDK}/Include/vulkan/vulkan.cppm"
    )
    target_link_libraries(vulkan_module PRIVATE Vulkan::Vulkan)

    target_link_libraries(FerriteEngine-Rendering PRIVATE vulkan_module Vulkan::Vulkan)

    target_sources(FerriteEngine-Rendering PUBLIC FILE_SET CXX_MODULES FILES
        Vulkan/Vulkan.cppm
        Vulkan/Init.cppm
    )

elseif("${FERRITE_RENDERING_BACKEND}" STREQUAL "OPENGL")
    target_compile_definitions(FerriteEngine-Rendering PRIVATE FERRITE_USE_OPENGL)
endif()
