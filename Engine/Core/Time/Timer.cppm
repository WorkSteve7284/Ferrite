module;

#include <functional>
#include <memory>
#include <queue>
#include <tuple>
#include <mutex>

export module Ferrite.Core.Time:Timer;

import Ferrite.Core.Classes;
import :TimeVariables;

struct TupleComparator {
    bool operator()(const std::tuple<double, std::function<void(void)>>& a, const std::tuple<double, std::function<void(void)>>& b) {
        return std::get<0>(a) > std::get<0>(b);
    }
};

namespace Ferrite::Core::Time {
    export class Timer :  public Classes::Component<Classes::Object> {
    private:

    std::mutex func_mutex;
    std::priority_queue<
    std::tuple<double, std::function<void(void)>>,
    std::vector<std::tuple<double, std::function<void(void)>>>, TupleComparator> funcs;

    public:
        // add with instance
        template <typename T> void schedule(double delay, std::weak_ptr<T> object, void(T::* function)(void)) {
            std::lock_guard lock(func_mutex);
            funcs.emplace(delay + runtime, [=]() {
                if (auto shared = object.lock())
                    std::bind(function, shared)();
            });
        }

        // add normal function
        void schedule(double delay, std::function<void(void)> function) {
            std::lock_guard lock(func_mutex);
            funcs.emplace(delay + runtime, function);
        }

        void update(double) override {
            std::lock_guard lock(func_mutex);

            while (!funcs.empty()) {
                if (std::get<0>(funcs.top()) <= runtime) {
                    std::get<1>(funcs.top())();
                    funcs.pop();
                }
                else {
                    break;
                }
            }
        }
    };

    export Timer* timer;
}
