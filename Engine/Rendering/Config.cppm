export module Ferrite.Rendering.Config;

namespace Ferrite::Rendering::Config {
    export enum class RenderingBackend {
        NONE,
        VULKAN,
        OPENGL
    };

    #ifdef FERRITE_USE_VULKAN
        export constexpr RenderingBackend BACKEND = RenderingBackend::VULKAN;
    #elif FERRITE_USE_OPENGL
        export constexpr RenderingBackend BACKEND = RenderingBackend::OPENGL;
    #else
        export constexpr RenderingBackend BACKEND = RenderingBackend::NONE;
    #endif
} // namespace Ferrite::Rendering::Config
