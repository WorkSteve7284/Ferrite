module;

#include <exception>
#if FERRITE_USE_VULKAN
#define GLFW_INCLUDE_VULKAN
#elifdef FERRITE_USE_OPENGL
#include <glad/glad.h>
#endif

#include <GLFW/glfw3.h>

#include <string>

export module Ferrite.Rendering.Window.GLFW;

import Ferrite.Core.Debug;
import Ferrite.Rendering.Config;

namespace Ferrite::Rendering::Window {

    // Takes the place of ~GLFWWindow(), as it needs to run during the constructor
    struct WindowTerminator {
        GLFWwindow* window = nullptr;

        ~WindowTerminator();
    };

    // Wrap GLFW window
    export class GLFWWindow {
    public:
        GLFWWindow(int width=640, int height=480, std::string title="FerriteEngine");

        bool should_close() const noexcept;

        static void error_callback(int error, const char *description);

    private:
        WindowTerminator terminator;
        GLFWwindow* window = nullptr;
    };

    GLFWWindow::GLFWWindow(int width, int height, std::string title) {

        terminator = WindowTerminator();

        if (!glfwInit()) {
            throw std::exception("GLFW Failed to initialize!");
        }

        glfwSetErrorCallback(&GLFWWindow::error_callback);

        if constexpr (Config::BACKEND == Config::RenderingBackend::VULKAN) {
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        }

        window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);

        if (!window) {
            throw std::exception("GLFW Window failed to create!");
        }
        else {
            terminator.window = window;
        }

    }

    bool GLFWWindow::should_close() const noexcept {
        return glfwWindowShouldClose(window);
    }

    void GLFWWindow::error_callback(int error, const char *description) {
        Core::Debug::error("GLFW ({}), {}", error, description);
    }

    WindowTerminator::~WindowTerminator() {

        if (window) {
            glfwDestroyWindow(window);
        }

        glfwTerminate();
    }

}; // namespace Ferrite::Rendering::Window
