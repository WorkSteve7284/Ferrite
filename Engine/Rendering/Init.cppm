module;

#include <functional>

export module Ferrite.Rendering.Init;

import Ferrite.Core.Manager;

namespace Ferrite::Rendering {
    export std::function<void(Ferrite::Core::Manager&)> init();
}
