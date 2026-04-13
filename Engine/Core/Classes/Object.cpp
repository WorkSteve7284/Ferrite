module;

#include <mutex>
#include <shared_mutex>
#include <algorithm>
#include <execution>
#include <memory>
#include <condition_variable>
#include <format>

module Ferrite.Core.Classes.Object;

import Ferrite.Core.Config;
import Ferrite.Core.Exceptions;
import Ferrite.Core.Classes.Prefabs;
import Ferrite.Core.Classes.PrefabManager;

namespace Ferrite::Core::Classes {

    void Object::start(Jobs::JobQueue& queue) const noexcept {
        if (!enabled) return;

        { // Start components
            std::shared_lock lock(component_mutex);

            std::for_each(
                std::execution::par_unseq,
                components.begin(),
                components.end(),
                [&queue](const std::shared_ptr<Component<Object>>& comp) {

                    std::weak_ptr<Component<Object>> comp_ptr{comp};

                    queue.add_job(Jobs::Job([=]() mutable {
                        if (auto shared = comp_ptr.lock())
                            shared->start();
                    }, Config::FIXED_UPDATE_PRIORITY + 1));

                }
            );
        }

        { // Start children
            std::shared_lock lock(child_mutex);

            std::for_each(
                std::execution::par_unseq,
                children.begin(),
                children.end(),
                [&queue](const std::shared_ptr<Object>& obj) {
                    obj->start(queue);
                }
            );
        }
    }

    void Object::update(double dt, std::size_t& counter, std::mutex& counter_mutex, std::condition_variable& cv, Jobs::JobQueue& queue) const noexcept {
        { // Update components
            std::shared_lock lock(component_mutex);

            std::for_each(
                std::execution::par_unseq,
                components.begin(),
                components.end(),
                [&](const std::shared_ptr<Component<Object>>& comp) {

                    {
                        std::lock_guard lock(counter_mutex);
                        counter++;
                    }

                    std::weak_ptr<Component<Object>> comp_ptr{comp};

                    queue.add_job(Jobs::Job([=, &counter, &counter_mutex, &cv]() mutable {
                        if (auto shared = comp_ptr.lock())
                            shared->update(dt);

                        {
                            std::lock_guard lock(counter_mutex);
                            counter--;
                            if (counter == 0)
                                cv.notify_all();
                        }
                    }, Config::UPDATE_PRIORITY));
                }
            );
        }

        { // Update children
            std::shared_lock lock(child_mutex);

            std::for_each(
                std::execution::par_unseq,
                children.begin(),
                children.end(),
                [&](const std::shared_ptr<Object>& obj) {
                    obj->update(dt, counter, counter_mutex, cv, queue);
                }
            );
        }
    }

    void Object::fixed_update(double dt, std::size_t& counter, std::mutex& counter_mutex, std::condition_variable& cv, Jobs::JobQueue& queue) const noexcept {
        { // Update components
            std::shared_lock lock(component_mutex);

            std::for_each(
                std::execution::par_unseq,
                components.begin(),
                components.end(),
                [&](const std::shared_ptr<Component<Object>>& comp) {

                    {
                        std::lock_guard lock(counter_mutex);
                        counter++;
                    }

                    std::weak_ptr<Component<Object>> comp_ptr{comp};

                    queue.add_job(Jobs::Job([=, &counter, &counter_mutex, &cv]() mutable {

                        if (auto shared = comp_ptr.lock())
                            shared->fixed_update(dt);

                        {
                            std::lock_guard lock(counter_mutex);
                            counter--;
                            if (counter == 0)
                                cv.notify_all();
                        }
                    }, Config::FIXED_UPDATE_PRIORITY));
                }
            );
        }

        { // Update children
            std::shared_lock lock(child_mutex);

            std::for_each(
                std::execution::par_unseq,
                children.begin(),
                children.end(),
                [&](const std::shared_ptr<Object>& obj) {
                    obj->fixed_update(dt, counter, counter_mutex, cv, queue);
                }
            );
        }
    }

    std::weak_ptr<Object> Object::add_object() noexcept {
        // Add empty object
        std::lock_guard lock(child_mutex);

        std::shared_ptr out = std::make_shared<Object>();

        out->owner = this;

        children.emplace_back(out);

        return out;
    }

    std::weak_ptr<Object> Object::add_object(const ObjectPrefab& prefab) noexcept {

        auto obj = add_object();

        if (auto shared = obj.lock()) {

            std::lock_guard lock(child_mutex);

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

    void Object::add_component(const ComponentPrefab& prefab) {
        PrefabManager::apply_component(*this, prefab);
    }

    void Object::remove_object(std::weak_ptr<Object> object) noexcept {
        std::lock_guard lock(child_mutex);
        if (auto shared = object.lock()) {
            std::erase(children, shared);
        }
    }

    bool is_ancestor(const Object& object, const Object& ancestor) noexcept {
        const Object* ptr = &object;
        while (ptr != nullptr) {
            if (ptr == &ancestor)
                return true;
            ptr = ptr->owner;
        }
        return false;
    }

    void Object::move_object(std::weak_ptr<Object> to) noexcept(!Config::EXCEPTIONS_ALLOWED) {
        if (auto to_shared = to.lock()) {

            if constexpr (Config::PREVENT_MEMORY_LEAK) {
                if (is_ancestor(*this, *to_shared)) {
                    if constexpr (Config::EXCEPTIONS_ALLOWED) {
                        throw Exceptions::CircularStructure("Object move would cause circular ownership!");
                    }
                    else {
                        return;
                    }
                }
            }

            std::scoped_lock lock(owner->child_mutex, to_shared->child_mutex);

            auto shared = std::ranges::find_if(
                owner->children,
                [this](const auto& ptr) {
                    return ptr.get() == this;
                }
            );
            to_shared->children.emplace_back(*shared);
            std::erase(owner->children, *shared);

            owner = to_shared.get();

        }
    }

    std::weak_ptr<Object> Object::find_object_with_name(const std::string& name) const noexcept(!Config::EXCEPTIONS_ALLOWED) {
        std::shared_lock lock(child_mutex);

        std::size_t hashed = std::hash<std::string>()(name);

        auto it = std::find_if(
            std::execution::unseq,
            children.begin(), children.end(),
            [hashed] (const auto& obj)->bool {
                std::shared_lock<std::shared_mutex> lock2(obj->id_mutex);
                return obj->name_hash == hashed;
            }
        );

        if (it != children.end()) {
            return *it;
        }

        if constexpr (Config::EXCEPTIONS_ALLOWED) {
            throw Ferrite::Core::Exceptions::NoObjectFound(std::format("No object with the specified name \"{}\" was found!", name));
        }
        else {
            return std::weak_ptr<Object>();
        }
    }

    std::weak_ptr<Object> Object::find_object_with_tag(const std::string& tag) const noexcept(!Config::EXCEPTIONS_ALLOWED) {
        std::shared_lock lock(child_mutex);

        for (auto& obj : children) {
            std::shared_lock lock2(obj.get()->id_mutex);
            if(obj.get()->tags.contains(tag))
                return obj;
        }

        if constexpr (Config::EXCEPTIONS_ALLOWED) {
            throw Exceptions::NoObjectFound(std::format("No object with the specified tag \"{}\" was found!", tag));
        }
        else {
            return std::weak_ptr<Object>();
        }
    }

    std::weak_ptr<Object> Object::find_object_with_tags(const std::vector<std::string>& tags) const noexcept(!Config::EXCEPTIONS_ALLOWED) {
        std::shared_lock lock(child_mutex);

        for (auto& obj : children) {

            bool match = true;

            for (auto& tag : tags) {
                std::shared_lock lock2(obj.get()->id_mutex);
                if (!obj.get()->tags.contains(tag))
                    match = false;
            }

            if (match)
                return obj;
        }

        if constexpr (Config::EXCEPTIONS_ALLOWED) {
            throw Ferrite::Core::Exceptions::NoObjectFound(std::format("No object with the specified tags \"{}\" was found!", tags));
        }
        else {
            return std::weak_ptr<Object>();
        }
    }

    std::vector<std::weak_ptr<Object>> Object::find_objects_with_name(const std::string& name) const noexcept(!Config::EXCEPTIONS_ALLOWED) {
        std::shared_lock lock(child_mutex);

        std::size_t hashed = std::hash<std::string>()(name);

        std::vector<std::weak_ptr<Object>> out;

        for (auto& obj : children) {
            std::shared_lock lock2(obj.get()->id_mutex);
            if (obj.get()->name_hash == hashed)
                out.emplace_back(obj);
        }

        if constexpr (Config::EXCEPTIONS_ALLOWED) {
            if (out.empty()) {
                throw Exceptions::NoObjectFound(std::format("No objects with the specified name \"{}\" were found!", name));
            }
        }

        return out;
    }

    std::vector<std::weak_ptr<Object>> Object::find_objects_with_tag(const std::string& tag) const noexcept(!Config::EXCEPTIONS_ALLOWED) {
        std::shared_lock lock(child_mutex);

        std::vector<std::weak_ptr<Object>> out;

        for (auto& obj : children) {
            std::shared_lock lock2(obj.get()->id_mutex);
            if(obj.get()->tags.contains(tag))
                out.emplace_back(obj);
        }

        if constexpr (Config::EXCEPTIONS_ALLOWED) {
            if (out.empty()) {
                throw Exceptions::NoObjectFound(std::format("No objects with the specified tag \"{}\" were found!", tag));
            }
        }

        return out;
    }

    std::vector<std::weak_ptr<Object>> Object::find_objects_with_tags(const std::vector<std::string>& tags) const noexcept(!Config::EXCEPTIONS_ALLOWED) {
        std::shared_lock lock(child_mutex);

        std::vector<std::weak_ptr<Object>> out;

        for (auto& obj : children) {

            bool match = true;

            for (auto& tag : tags) {
                std::shared_lock lock2(obj.get()->id_mutex);
                if (!obj.get()->tags.contains(tag))
                    match = false;
            }

            if (match)
                out.emplace_back(obj);
        }

        if constexpr (Config::EXCEPTIONS_ALLOWED) {
            if (out.empty()) {
                throw Exceptions::NoObjectFound(std::format("No objects with the specified tags \"{}\" were found!", tags));
            }
        }

        return out;
    }

    void Object::rehash_name() noexcept {
        const static std::hash<std::string> hasher;

        std::lock_guard lock(id_mutex);
        name_hash = hasher(name);

    }
} // namespace Ferrite::Core::Classes
