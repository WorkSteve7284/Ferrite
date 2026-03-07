module;

#include <concepts>
#include <memory>
#include <shared_mutex>
#include <vector>
#include <string>
#include <unordered_set>
#include <mutex>
#include <algorithm>

export module Ferrite.Core.Classes.Object;

import Ferrite.Core.Classes.Component;
import Ferrite.Core.Classes.Prefabs;

import Ferrite.Core.Jobs;
import Ferrite.Core.Config;
import Ferrite.Core.Exceptions;

namespace Ferrite::Core::Classes {

    export class Object  : std::enable_shared_from_this<Object> {
    private:
        mutable std::shared_mutex component_mutex;
        std::vector<std::shared_ptr<Component<Object>>> components;

        mutable std::shared_mutex child_mutex;
        std::vector<std::shared_ptr<Object>> children;

    public:

        mutable std::shared_mutex tag_mutex;
        std::unordered_set<std::string> tags;

        mutable std::shared_mutex name_mutex;
        std::string name;
        std::size_t name_hash;

        std::atomic<bool> enabled = true;

        Object* owner;

        void start(Jobs::JobQueue& queue) const noexcept;
        void update(double dt, std::size_t& counter, std::mutex& counter_mutex, std::condition_variable& cv, Jobs::JobQueue& queue) const noexcept;
        void fixed_update(double dt, std::size_t& counter, std::mutex& counter_mutex, std::condition_variable& cv, Jobs::JobQueue& queue) const noexcept;

        std::weak_ptr<Object> add_object() noexcept;
        std::weak_ptr<Object> add_object(const ObjectPrefab&) noexcept;
        std::weak_ptr<Object> add_object(const std::weak_ptr<Object>&) noexcept;

        template <typename T, typename... Args>
        requires std::derived_from<T, Component<Object>>
        std::weak_ptr<T> add_component(Args&&...);
        void add_component(const ComponentPrefab&);

        void remove_object(std::weak_ptr<Object>) noexcept;
        template <typename T>
        requires std::derived_from<T, Component<Object>>
        void remove_component(std::weak_ptr<T>) noexcept;

        void move_object(std::weak_ptr<Object> to) noexcept(!Config::EXCEPTIONS_ALLOWED);
        template <typename T>
        requires std::derived_from<T, Component<Object>>
        void move_component(std::weak_ptr<T> component, std::weak_ptr<Object> to) noexcept(!Config::EXCEPTIONS_ALLOWED);

        std::weak_ptr<Object> find_object_with_name(const std::string& name) const noexcept(!Config::EXCEPTIONS_ALLOWED);
        std::weak_ptr<Object> find_object_with_tag(const std::string& tag) const noexcept(!Config::EXCEPTIONS_ALLOWED);
        std::weak_ptr<Object> find_object_with_tags(const std::vector<std::string>& tags) const noexcept(!Config::EXCEPTIONS_ALLOWED);

        std::vector<std::weak_ptr<Object>> find_objects_with_name(const std::string& name) const noexcept(!Config::EXCEPTIONS_ALLOWED);
        std::vector<std::weak_ptr<Object>> find_objects_with_tag(const std::string& tag) const noexcept(!Config::EXCEPTIONS_ALLOWED);
        std::vector<std::weak_ptr<Object>> find_objects_with_tags(const std::vector<std::string>& tags) const noexcept(!Config::EXCEPTIONS_ALLOWED);

        template <typename T>
        requires std::derived_from<T, Component<Object>>
        std::weak_ptr<T> find_component() const noexcept(!Config::EXCEPTIONS_ALLOWED);
        template <typename T>
        requires std::derived_from<T, Component<Object>>
        bool has_component() const noexcept;

        void rehash_name() noexcept;
    };

    template <typename T, typename... Args>
    requires std::derived_from<T, Component<Object>>
    std::weak_ptr<T> Object::add_component(Args&&... args) {
        std::lock_guard lock(component_mutex);

        auto shared = std::make_shared<T>(std::forward<Args>(args)...);
        shared->owner = this;
        std::weak_ptr out = shared;

        components.emplace_back(std::static_pointer_cast<Component<Object>>(shared));

        return out;
    }

    template <typename T>
    requires std::derived_from<T, Component<Object>>
    void Object::remove_component(std::weak_ptr<T> component) noexcept {
        std::lock_guard lock(component_mutex);
        if (auto shared = component.lock()) {
            std::erase(components, shared);
        }
    }


    template <typename T>
    requires std::derived_from<T, Component<Object>>
    void Object::move_component(std::weak_ptr<T> component, std::weak_ptr<Object> to) noexcept(!Config::EXCEPTIONS_ALLOWED) {
        if (auto to_shared = to.lock()) {
            if (auto comp_shared = component.lock()) {
                std::scoped_lock lock(component_mutex, to_shared->component_mutex);

                if (std::find(components.begin(), components.end(), comp_shared) != components.end()) {
                    to_shared->components.emplace_back(comp_shared);
                    std::erase(components, comp_shared);
                    comp_shared->owner = to_shared.get();
                }
                else {
                    if constexpr (Config::EXCEPTIONS_ALLOWED) {
                        throw Exceptions::NoComponentFound("Specified component isn't owned by this Object!");
                    }
                }
            }
        }
    }

    template <typename T>
    requires std::derived_from<T, Component<Object>>
    std::weak_ptr<T> Object::find_component() const noexcept(!Config::EXCEPTIONS_ALLOWED) {
        std::shared_lock lock(component_mutex);

        for (auto& comp : components) {
            if (dynamic_cast<T*>(comp.get()))
                return comp;
        }

        if constexpr (Config::EXCEPTIONS_ALLOWED) {
            throw Exceptions::NoComponentFound("No component with the specified type was found!");
        }
        else {
            return std::weak_ptr<T>();
        }
    }

    template <typename T>
    requires std::derived_from<T, Component<Object>>
    bool Object::has_component() const noexcept {
        std::shared_lock lock(component_mutex);

        for (auto& comp : components) {
            if (dynamic_cast<T*>(comp.get()))
                return true;
        }

        return false;
    }




} // namespace Ferrite::Core::Classes
