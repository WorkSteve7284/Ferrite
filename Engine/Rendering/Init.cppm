module;

#include <functional>
#include <string>

export module Ferrite.Rendering.Init;

import Ferrite.Core.Manager;
import Ferrite.Rendering.Window.GLFW;
import Ferrite.Rendering.Window.WindowUpdate;


namespace Ferrite::Rendering {

    Window::GLFWWindow window;

    export std::function<void(Ferrite::Core::Manager&)> init(int width=640, int height=480, std::string title="FerriteEngine") {
        return [=](Ferrite::Core::Manager& manager) {
            auto render_thread = manager.make_server_thread();

            window.init_window(width, height, title);

            auto object = manager->add_object();

            if (auto shared = object.lock()) {
                shared->name = "RenderingObject";
                shared->rehash_name();

                shared->add_component<Window::WindowUpdate>(window, manager);
            }
        };
    }
}
