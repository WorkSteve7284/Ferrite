module;

#include <unordered_set>
#include <map>
#include <string>
#include <any>
#include <vector>

export module Ferrite.Core.Classes:Prefabs;

namespace Ferrite::Core::Classes {

    export struct ComponentPrefab {
        std::size_t id;
        std::map<std::string, std::any> data;
    };

    export struct ObjectPrefab {

        std::string name;
        std::unordered_set<std::string> tags;

        std::vector<ComponentPrefab> components;
        std::vector<ObjectPrefab> children;
    };

}
