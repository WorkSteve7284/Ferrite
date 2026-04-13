module;

#include <cstdint>
#include <set>
#include <string>
#include <vector>
#include <any>

export module Ferrite.Core.Classes.Prefabs;

namespace Ferrite::Core::Classes {

    export struct ComponentPrefab {
        std::size_t id = SIZE_MAX;
        std::any data;
    };

    export struct ObjectPrefab {
        std::string name;
        std::set<std::string> tags;

        std::vector<ComponentPrefab> components;
        std::vector<ObjectPrefab> children;
    };

}  // namespace Ferrite::Core::Classes
