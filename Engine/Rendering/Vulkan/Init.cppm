module;

export module Ferrite.Rendering.Vulkan.Init;

import vulkan;

namespace Ferrite::Rendering::Vulkan {

    export class VulkanInit {

        void create_instance() {
            constexpr vk::ApplicationInfo appInfo{
                .pApplicationName   = "Hello Triangle",
                .applicationVersion = VK_MAKE_VERSION( 1, 0, 0 ),
                .pEngineName        = "FerriteEngine",
                .engineVersion      = VK_MAKE_VERSION( 1, 0, 0 ),
                .apiVersion         = vk::ApiVersion14
            };
        }

    };

}
