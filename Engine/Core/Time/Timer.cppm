module;

#include <functional>
#include <memory>
#include <map>
#include <mutex>

export module Ferrite.Core.Time:Timer;

import Ferrite.Core.Classes;
import :TimeVariables;

namespace Ferrite::Core::Time {
    export class Timer :  public Classes::Component<Classes::Object> {
    private:

    std::mutex func_mutex;
    std::map<double, std::function<void(void)>> funcs;

    public:
        // add with instance
        template <typename T> void schedule(double delay, std::weak_ptr<T> object, void(T::* function)(void)) {
            funcs[delay + runtime] = [=]() {
                if (auto shared = object.lock())
                    std::bind(function, shared)();
            };
        }

        // add normal function
        void schedule(double delay, std::function<void(void)> function) {
            funcs[delay + runtime] = [=]() {
                function();
            };
        }

        void update(double) override {
            std::lock_guard lock(func_mutex);

            for (auto it = funcs.begin(); it != funcs.end(); it++) {
                if (it->first <= runtime)
                    it->second();
                else {
                    if (it != funcs.begin())
                        funcs.erase(funcs.begin(), it);
                    break;
                }
            }
        }
    };

    export Timer* timer;
}
