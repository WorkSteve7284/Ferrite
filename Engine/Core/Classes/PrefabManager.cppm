module;

#include <vector>
#include <concepts>
#include <functional>
#include <typeindex>
#include <cstdint>

export module Ferrite.Core.Classes.PrefabManager;

import Ferrite.Core.Classes.Component;
import Ferrite.Core.Classes.Object;
import Ferrite.Core.Classes.Prefabs;

namespace Ferrite::Core::Classes {
    export class PrefabManager {
    public:

        template <typename T>
        requires std::derived_from<T, Component<Object>>
        static std::size_t register_component() noexcept;

        template <typename T>
        requires std::derived_from<T, Component<Object>>
        static std::size_t get_component_id() noexcept;

        static void apply_component(Object&, const ComponentPrefab&) noexcept;

    private:

        struct ComponentConstructor {
            using ComponentFunction = std::function<void(Object&, const ComponentPrefab&)>;
            std::type_index id;
            ComponentFunction func;
        };

        inline static std::vector<ComponentConstructor> constructors{};
    };

    template <typename T>
    requires std::derived_from<T, Component<Object>>
    std::size_t PrefabManager::register_component() noexcept {
        const std::size_t pos = constructors.size();

        constructors.emplace_back(typeid(T), [](Object& obj, const ComponentPrefab& prefab) {
            obj.add_component<T>(prefab.data);
        });

        return pos;
    }

    template <typename T>
    requires std::derived_from<T, Component<Object>>
    std::size_t PrefabManager::get_component_id() noexcept {

        std::type_index t = typeid(T);

        for (std::size_t i = 0; i < constructors.size(); i++) {
            if (constructors[i].id == t)
                return i;
        }

        return SIZE_MAX;
    }


    void PrefabManager::apply_component(Object& obj, const ComponentPrefab& prefab) noexcept {
        if (constructors.size() > prefab.id) {
            constructors[prefab.id].func(obj, prefab);
        }
    }

} // namespace Ferrite::Core::Classes
