module;

#if FERRITE_USE_VULKAN
#define GLFW_INCLUDE_VULKAN
#elif defined(FERRITE_USE_OPENGL)
#include <glad/gl.h>
#endif

#include <GLFW/glfw3.h>

#include <functional>

export module Ferrite.Rendering.Window.WindowUpdate;

import Ferrite.Core.Classes.Component;
import Ferrite.Core.Classes.Object;
import Ferrite.Core.Manager;
import Ferrite.Core.Threads;

import Ferrite.Core.Debug;

import Ferrite.Rendering.Window.GLFW;

namespace Ferrite::Rendering::Window {

    export class WindowUpdate : public Core::Classes::Component<Core::Classes::Object> {
        GLFWWindow& window;
        Core::Manager& manager;
    public:

        WindowUpdate(GLFWWindow& _window, Core::Manager& _manager) : window(_window), manager(_manager) {}

        void update(double) override {
            manager.run_on_main_thread(glfwPollEvents);

            if (window.should_close()) {
                Core::Debug::log("Closing Window!");

                top->enabled = false;
            }
        }
    };

} // namespace Ferrite::Rendering::Window
