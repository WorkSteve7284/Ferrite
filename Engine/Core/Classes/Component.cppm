module;

#include <memory>

export module Ferrite.Core.Classes.Component;

namespace Ferrite::Core::Classes {

    export template <typename Object> class Component : public std::enable_shared_from_this<Component<Object>> {

        std::weak_ptr<Object> owner;

        public:

            template <typename T> static std::weak_ptr<T> get_weak(T* c) { return std::static_pointer_cast<T>(c->shared_from_this()); }

            inline static Object* manager;

            virtual void start() {}
            virtual void update(double dt) {}
            virtual void fixed_update(double dt) {}

            virtual ~Component() {}
    };

}  // namespace Ferrite::Core::Classes
