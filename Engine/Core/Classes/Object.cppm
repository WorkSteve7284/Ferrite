module;

#include <format>
#include <functional>
#include <memory>
#include <shared_mutex>
#include <vector>
#include <string>
#include <unordered_set>
#include <map>

export module Ferrite.Core.Classes:Object;

import :Component;
import :Prefabs;

import Ferrite.Core.Jobs;
import Ferrite.Core.Config;
import Ferrite.Core.Exceptions;

namespace Ferrite::Core::Classes {

    export class Object {
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

    public:

        void start(Jobs::JobQueue& queue) const noexcept;
        void update(double dt, Jobs::JobQueue& queue) const noexcept;
        void fixed_update(double dt, Jobs::JobQueue& queue) const noexcept;

        std::weak_ptr<Object> add_object() noexcept;
        std::weak_ptr<Object> add_object(const ObjectPrefab&) noexcept;
        std::weak_ptr<Object> add_object(const std::weak_ptr<Object>&) noexcept;

        template <typename T, typename... Args> std::weak_ptr<T> add_component(Args&&...) noexcept;
        void add_component(const ComponentPrefab&) noexcept;

        std::weak_ptr<Object> find_object_with_name(const std::string& name) const noexcept(!EXCEPTIONS_ALLOWED);
        std::weak_ptr<Object> find_object_with_tag(const std::string& tag) const noexcept(!EXCEPTIONS_ALLOWED);
        std::weak_ptr<Object> find_object_with_tags(const std::vector<std::string>& tags) const noexcept(!EXCEPTIONS_ALLOWED);

        std::vector<std::weak_ptr<Object>> find_objects_with_name(const std::string& name) const noexcept(!EXCEPTIONS_ALLOWED);
        std::vector<std::weak_ptr<Object>> find_objects_with_tag(const std::string& tag) const noexcept(!EXCEPTIONS_ALLOWED);
        std::vector<std::weak_ptr<Object>> find_objects_with_tags(const std::vector<std::string>& tags) const noexcept(!EXCEPTIONS_ALLOWED);

        template <typename T> std::weak_ptr<T> find_component() const noexcept(!EXCEPTIONS_ALLOWED);
        template <typename T> bool has_component() const noexcept;

        void rehash_name() noexcept;
    };

    export class PrefabManager {

        using ComponentFunction = std::function<void(Object&, const ComponentPrefab&)>;

        inline static std::vector<ComponentFunction> constructors{};
        inline static std::map<std::string, std::size_t> name_to_index{};

    public:
        // register component
        template <typename T> static std::size_t register_component(std::string = "") noexcept;
        // use component
        static void apply_component(Object&, const ComponentPrefab&) noexcept;

    };

    void Object::start(Jobs::JobQueue& queue) const noexcept {
        if (!enabled) return;

        { // Start components
            std::shared_lock lock(component_mutex);

            for (auto& comp : components) {

                std::weak_ptr<Component<Object>> comp_ptr{comp};

                queue.add_job(Jobs::Job([=]() mutable {
                    if (auto shared = comp_ptr.lock())
                        shared->start();
                }, FIXED_UPDATE_PRIORITY+1));

            }
        }

        { // Start children
            std::shared_lock lock(child_mutex);

            for (auto& obj : children) {
                obj.get()->start(queue);
            }
        }
    }

    void Object::update(double dt, Jobs::JobQueue& queue) const noexcept {
        { // Update components
            std::shared_lock lock(component_mutex);

            for (auto& comp : components) {

                std::weak_ptr<Component<Object>> comp_ptr{comp};

                queue.add_job(Jobs::Job([=]() mutable {
                    if (auto shared = comp_ptr.lock())
                        shared->update(dt);
                }, UPDATE_PRIORITY));
            }
        }

        { // Update children
            std::shared_lock lock(child_mutex);

            for (auto& obj : children) {
                obj.get()->update(dt, queue);
            }
        }
    }

    void Object::fixed_update(double dt, Jobs::JobQueue& queue) const noexcept {
        { // Update components
            std::shared_lock lock(component_mutex);
            for (auto& comp : components) {

                std::weak_ptr<Component<Object>> comp_ptr{comp};

                queue.add_job(Jobs::Job([=]() mutable {
                    if (auto shared = comp_ptr.lock())
                        shared->fixed_update(dt);
                }, FIXED_UPDATE_PRIORITY));
            }
        }

        { // Start children
            std::shared_lock lock(child_mutex);

            for (auto& obj : children) {
                obj.get()->fixed_update(dt, queue);
            }
        }
    }

    std::weak_ptr<Object> Object::add_object() noexcept {
        // Add empty object
        std::unique_lock lock(child_mutex);

        std::weak_ptr<Object> out = children.emplace_back();

        return out;
    }

    std::weak_ptr<Object> Object::add_object(const ObjectPrefab& prefab) noexcept {
        auto obj = add_object();

        if (auto shared = obj.lock()) {
            std::unique_lock lock(child_mutex);

            shared->name = prefab.name;
            shared->rehash_name();

            shared->tags = std::move(prefab.tags);

            for (const auto& comp : prefab.components) {
                shared->add_component(comp);
            }

            for (const auto& child : prefab.children) {
                shared->add_object(child);
            }
        }

        return obj;
    }

    std::weak_ptr<Object> Object::add_object(const std::weak_ptr<Object>&) noexcept {}

    template <typename T, typename... Args> std::weak_ptr<T> Object::add_component(Args&&... args) noexcept {
        std::unique_lock lock(component_mutex);

        auto shared = std::make_shared<T>(std::forward<Args>(args)...);

        components.emplace_back(std::static_pointer_cast<Component<Object>>(shared));

        std::weak_ptr out = shared;

        return out;
    }
    void Object::add_component(const ComponentPrefab& prefab) noexcept {
        std::unique_lock lock(component_mutex);

        PrefabManager::apply_component(*this, prefab);
    }

    std::weak_ptr<Object> Object::find_object_with_name(const std::string& name) const noexcept(!EXCEPTIONS_ALLOWED) {
        std::shared_lock lock(child_mutex);

        std::size_t hashed = std::hash<std::string>()(name);

        for (auto& obj : children) {
            std::shared_lock<std::shared_mutex> lock2(obj.get()->name_mutex);
            if (obj.get()->name_hash == hashed)
                return obj;
        }

        if constexpr (EXCEPTIONS_ALLOWED)
            throw Ferrite::Core::Exceptions::NoObjectFound(std::format("No object with the specified name \"{}\"was found!", name));
        else
            return std::weak_ptr<Object>();
    }

    std::weak_ptr<Object> Object::find_object_with_tag(const std::string& tag) const noexcept(!EXCEPTIONS_ALLOWED) {
        std::shared_lock lock(child_mutex);

        for (auto& obj : children) {
            std::shared_lock lock2(obj.get()->tag_mutex);
            if(obj.get()->tags.contains(tag))
                return obj;
        }

        if constexpr (EXCEPTIONS_ALLOWED)
            throw Exceptions::NoObjectFound(std::format("No object with the specified tag \"{}\" was found!", tag));
        else
            return std::weak_ptr<Object>();
    }

    std::weak_ptr<Object> Object::find_object_with_tags(const std::vector<std::string>& tags) const noexcept(!EXCEPTIONS_ALLOWED) {
        std::shared_lock lock(child_mutex);

        for (auto& obj : children) {

            bool match = true;

            for (auto& tag : tags) {
                std::lock_guard<std::shared_mutex> lock2(obj.get()->tag_mutex);
                if (!obj.get()->tags.contains(tag))
                    match = false;
            }

            if (match)
                return obj;
        }

        if constexpr (EXCEPTIONS_ALLOWED)
            throw Ferrite::Core::Exceptions::NoObjectFound(std::format("No object with the specified tags \"{}\" was found!", tags));
        else
            return std::weak_ptr<Object>();
    }

    std::vector<std::weak_ptr<Object>> Object::find_objects_with_name(const std::string& name) const noexcept(!EXCEPTIONS_ALLOWED) {
        std::shared_lock lock(child_mutex);

        std::size_t hashed = std::hash<std::string>()(name);

        std::vector<std::weak_ptr<Object>> out;

        for (auto& obj : children) {
            std::shared_lock<std::shared_mutex> lock2(obj.get()->name_mutex);
            if (obj.get()->name_hash == hashed)
                out.emplace_back(obj);
        }

        if constexpr (EXCEPTIONS_ALLOWED) {
            if (out.empty())
                throw Exceptions::NoObjectFound(std::format("No objects with the specified name \"{}\" were found!", name));
        }

        return out;
    }

    std::vector<std::weak_ptr<Object>> Object::find_objects_with_tag(const std::string& tag) const noexcept(!EXCEPTIONS_ALLOWED) {
        std::shared_lock lock(child_mutex);

        std::vector<std::weak_ptr<Object>> out;

        for (auto& obj : children) {
            std::shared_lock<std::shared_mutex> lock2(obj.get()->tag_mutex);
            if(obj.get()->tags.contains(tag))
                out.emplace_back(obj);
        }

        if constexpr (EXCEPTIONS_ALLOWED) {
            if (out.empty())
                throw Exceptions::NoComponentFound(std::format("No objects with the specified tag \"{}\" were found!", tag));
        }

        return out;
    }

    std::vector<std::weak_ptr<Object>> Object::find_objects_with_tags(const std::vector<std::string>& tags) const noexcept(!EXCEPTIONS_ALLOWED) {
        std::shared_lock lock(child_mutex);

        std::vector<std::weak_ptr<Object>> out;

        for (auto& obj : children) {

            bool match = true;

            for (auto& tag : tags) {
                std::shared_lock lock2(obj.get()->tag_mutex);
                if (!obj.get()->tags.contains(tag))
                    match = false;
            }

            if (match)
                out.emplace_back(obj);
        }

        if constexpr (EXCEPTIONS_ALLOWED) {
            if (out.empty())
                throw Exceptions::NoComponentFound(std::format("No objects with the specified tags \"{}\" were found!", tags));
        }

        return out;
    }

    template <typename T> std::weak_ptr<T> Object::find_component() const noexcept(!EXCEPTIONS_ALLOWED) {
        std::shared_lock lock(component_mutex);

        for (auto& comp : components) {
            if (dynamic_cast<T*>(comp.get()))
                return comp;
        }

        if constexpr (EXCEPTIONS_ALLOWED)
            throw Exceptions::NoComponentFound("No component with the specified type was found!");
        else
            return ObjectPtr<T>();
    }

    template <typename T> bool Object::has_component() const noexcept {
        std::shared_lock lock(component_mutex);

        for (auto& comp : components) {
            if (dynamic_cast<T*>(comp.get()))
                return true;
        }

        return false;
    }

    void Object::rehash_name() noexcept {
        std::hash<std::string> hasher;

        {
            std::unique_lock lock(name_mutex);
            name_hash = hasher(name);
        }
    }

    template <typename T> std::size_t PrefabManager::register_component(std::string name) noexcept {
        const std::size_t pos = constructors.size();

        constructors.emplace_back([](Object& obj, const ComponentPrefab& prefab) {
            obj.add_component<T>(prefab);
        });

        if (!name.empty())
            name_to_index[name] = pos;

        return pos;
    }

    void PrefabManager::apply_component(Object& obj, const ComponentPrefab& prefab) noexcept {
        if (prefab.id != 0 && constructors.size() > prefab.id)
            constructors[prefab.id](obj, prefab);
    }
}
