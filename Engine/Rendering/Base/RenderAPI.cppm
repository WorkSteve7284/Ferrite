module;

#include <cstdint>

export module Ferrite.Rendering.Base.RenderAPI;

import Ferrite.Math.Vector.Vector2;

namespace Ferrite::Rendering::Base {

    enum class GraphicsAPI {
        OPENGL,
        OPENGL_ES,
        VULKAN,
        METAL,
        DIRECTX11,
        DIRECTX12,
        WEBGPU,
        WEBGL
    };

    struct VertexArray {};


    struct Color {
        std::uint8_t r;
        std::uint8_t g;
        std::uint8_t b;
        std::uint8_t a;
    };

    export class RenderAPI {

        virtual void init() = 0;
        virtual void shutdown() = 0;

        virtual void set_viewport(Math::Vector::Vector2<std::uint32_t> origin, Math::Vector::Vector2<std::uint32_t> size) = 0;
        virtual void set_clear_color(const Color&) = 0;
        virtual void clear() = 0;

        virtual void draw_indexed(const VertexArray& vertex_array, std::uint32_t index_count = 0) = 0;
        virtual void draw_array(const VertexArray& vertex_array, std::uint32_t vertex_count = 0) = 0;

        virtual void begin_render_pass() = 0;
        virtual void set_depth_test(bool enabled) = 0;
        virtual void set_blend(bool enabled) = 0;
        virtual void set_wireframe(bool enabled) = 0;
        virtual void end_render_pass() = 0;
    };

} // namespace Ferrite::Rendering::Base
