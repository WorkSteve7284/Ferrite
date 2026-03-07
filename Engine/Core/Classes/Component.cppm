module;

#include <concepts>
#include <memory>

export module Ferrite.Core.Classes.Component;

namespace Ferrite::Core::Classes {

    export template <typename Object> class Component : public std::enable_shared_from_this<Component<Object>> {
        public:

            template <typename T>
            requires std::derived_from<T, Component<Object>>
            static std::weak_ptr<T> get_weak(T* c) { return std::static_pointer_cast<T>(c->shared_from_this()); }

            inline static Object* manager;

            Object* owner;

            virtual void start() {}
            virtual void update(double dt) {}
            virtual void fixed_update(double dt) {}
            virtual void late_update(double dt) {}

            virtual ~Component() {}
    };

}  // namespace Ferrite::Core::Classes
