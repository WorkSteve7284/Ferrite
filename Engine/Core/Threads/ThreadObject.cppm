module;

#include <mutex>
#include <condition_variable>
#include <functional>
#include <queue>

export module Ferrite.Core.Threads.ThreadObject;

namespace Ferrite::Core::Threads {

    export class ThreadObject {
        std::queue<std::function<void(void)>> queue;
        std::mutex mutex;
        std::condition_variable cv;
        bool running = true;

    public:

        ThreadObject() = default;
        ThreadObject(const ThreadObject& other);
        ThreadObject(ThreadObject&& other);

        ThreadObject& operator=(const ThreadObject& other);
        ThreadObject& operator=(ThreadObject&& other);

        void run();
        void stop();
        void add(std::function<void(void)>);
    };

    ThreadObject::ThreadObject(const ThreadObject& other) {
        queue = other.queue;
    }

    ThreadObject::ThreadObject(ThreadObject&& other) {
        queue = std::move(other.queue);
    }

    ThreadObject& ThreadObject::operator=(const ThreadObject& other) {
        queue = other.queue;
        return *this;
    }

    ThreadObject& ThreadObject::operator=(ThreadObject&& other) {
        queue = std::move(other.queue);
        return *this;
    }

    void ThreadObject::run() {
        while (running) {
            std::unique_lock lock(mutex);
            cv.wait(lock, [&]() {
                return !queue.empty() || !running;
            });

            while (!queue.empty()) {
                queue.front()();
                queue.pop();
            }
        }
    }

    void ThreadObject::stop() {
        std::lock_guard lock(mutex);
        running = false;
        cv.notify_one();
    }

    void ThreadObject::add(std::function<void(void)> func) {
        std::lock_guard lock(mutex);
        queue.emplace(func);
        cv.notify_one();
    }
} // namespace Ferrite::Core::Threads
