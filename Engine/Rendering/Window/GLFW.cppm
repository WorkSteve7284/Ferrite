module;

#if FERRITE_USE_VULKAN
#define GLFW_INCLUDE_VULKAN
#elif defined(FERRITE_USE_OPENGL)
#include <glad/gl.h>
#endif

#include <GLFW/glfw3.h>

#include <stdexcept>
#include <string>

export module Ferrite.Rendering.Window.GLFW;

import Ferrite.Core.Debug;
import Ferrite.Core.Config;
import Ferrite.Rendering.Config;

namespace Ferrite::Rendering::Window {

    // Wrap GLFW window
    export class GLFWWindow {
    public:
        GLFWWindow() = default;
        ~GLFWWindow();

        void init_window(int width, int height, std::string title) noexcept(!Core::Config::EXCEPTIONS_ALLOWED);

        bool should_close() const noexcept;

        GLFWwindow* get_window() noexcept;

        static void error_callback(int error, const char *description);

    private:
        GLFWwindow* window = nullptr;
    };



    void GLFWWindow::init_window(int width, int height, std::string title) noexcept(!Core::Config::EXCEPTIONS_ALLOWED) {

        if (!glfwInit()) {
            if constexpr (Core::Config::EXCEPTIONS_ALLOWED) {
                throw std::runtime_error("GLFW Failed to initialize!");
            }
            glfwTerminate();
        }

        glfwSetErrorCallback(&GLFWWindow::error_callback);

        if constexpr (Config::BACKEND == Config::RenderingBackend::VULKAN) {
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        }

        window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);

        if (!window) {
            if constexpr (Core::Config::EXCEPTIONS_ALLOWED) {
                throw std::runtime_error("GLFW Window failed to create!");
            }
            glfwTerminate();
        }
    }

    bool GLFWWindow::should_close() const noexcept {
        return glfwWindowShouldClose(window);
    }

    GLFWwindow* GLFWWindow::get_window() noexcept {
        return window;
    }

    void GLFWWindow::error_callback(int error, const char *description) {
        Core::Debug::error("GLFW ({}), {}", error, description);
    }

    GLFWWindow::~GLFWWindow() {

        if (window) {
            glfwDestroyWindow(window);
        }

        glfwTerminate();
    }

}; // namespace Ferrite::Rendering::Window
