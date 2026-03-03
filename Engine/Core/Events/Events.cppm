module;

#include <execution>
#include <memory>
#include <vector>
#include <functional>
#include <mutex>
#include <algorithm>

export module Ferrite.Core.Events;

namespace Ferrite::Core::Events {

    export template <typename... T>
    class EventManager {

        std::mutex listener_mutex;
        std::vector<std::function<void(T...)>> listeners;

        public:

        void add_listener(std::function<void(T...)> func);
        template <typename C>
        void add_listener(std::weak_ptr<C> object, void(C::* func)(T...));
        void broadcast(T&... args);
        void clear_listeners();

    };

    template <typename... T>
    void EventManager<T...>::add_listener(std::function<void(T...)> func) {
        std::lock_guard lock(listener_mutex);

        listeners.emplace_back(std::move(func));
    }

    template <typename... T>
    template <typename C>
    void EventManager<T...>::add_listener(std::weak_ptr<C> object, void(C::*func)(T...)) {
        std::lock_guard lock(listener_mutex);

        listeners.emplace_back([=](T... vals){
            if (auto shared = object.lock()) {
                std::invoke(func, shared, std::forward<T>(vals)...);
            }
        });
    }

    template <typename... T>
    void EventManager<T...>::broadcast(T&... args) {
        std::lock_guard lock(listener_mutex);

        std::for_each(
            std::execution::par_unseq,
            listeners.begin(),
            listeners.end(),
            [&](std::function<void(T...)>& elem) {
                elem(args...);
            }
        );
    }

    template <typename... T>
    void EventManager<T...>::clear_listeners() {
        std::lock_guard lock(listener_mutex);

        listeners.clear();
    }
}  // namespace Ferrite::Core::Events
