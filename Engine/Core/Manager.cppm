module;

#include <vector>
#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <functional>
#include <thread>
#include <queue>

export module Ferrite.Core.Manager;

import Ferrite.Core.Config;
import Ferrite.Core.Classes;
import Ferrite.Core.Jobs;
import Ferrite.Core.Time;
import Ferrite.Core.Threads;

namespace Ferrite::Core {

    using Clock = std::chrono::steady_clock;

    double time_between(std::chrono::time_point<Clock> start, std::chrono::time_point<Clock> end) {
        return std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() * 1e-9;
    };

    export class Manager {
    private:

        std::shared_ptr<Object> top;

        Jobs::JobQueue job_queue;
        std::vector<Jobs::Worker> workers;

        bool running = true;

        std::mutex counter_mutex;
        std::size_t update_counter = 0;
        std::size_t fixed_update_counter = 0;

        std::vector<Threads::ServerThread> server_threads;
        std::queue<std::function<void(void)>> main_thread_queue;
        std::mutex queue_mutex;

        std::vector<std::function<void(Manager&)>> modules;

    public:
        Manager() noexcept;

        void add_module(std::function<void(Manager&)>);

        Threads::ThreadRef make_server_thread();
        void run_on_main_thread(std::function<void(void)>);

        void init();

        void run();

        Object* operator->() noexcept { return top.get(); }
        Object& operator*() noexcept {return *top; }
    };

    Manager::Manager() noexcept {

        top = std::make_shared<Object>();

        Component::top = top.get();

        // Initialize timer
        Time::timer = top->add_component<Time::Timer>().lock().get();

    }

    void Manager::add_module(std::function<void(Manager&)> module_initializer) {
        modules.emplace_back(module_initializer);
    }

    Threads::ThreadRef Manager::make_server_thread() {
        return Threads::ThreadRef(&server_threads.emplace_back());
    }

    void Manager::run_on_main_thread(std::function<void(void)> func) {
        std::lock_guard lock(queue_mutex);
        main_thread_queue.emplace(func);
    }

    void Manager::init() {
        for (auto& module_ : modules) {
            module_(*this);
        }

        // Initialize thread pool size
        const auto threads = std::clamp(
            std::thread::hardware_concurrency() - (1 + server_threads.size()),
            1uz,
            Config::MAX_THREADS
        );

        workers.reserve(threads);

        for (std::size_t i = 0; i < threads; i++) {
            workers.emplace_back(&job_queue).start();
        }
    }

    void Manager::run() {
        auto start = Clock::now();

        top->start(job_queue);

        std::condition_variable cv;

        auto last_fixed_update = Clock::now();

        auto last_update = Clock::now();

        while (running) {

            { // Wait for updates to finish
                std::unique_lock lock(counter_mutex);
                cv.wait(lock, [&](){
                    const double dt = time_between(last_fixed_update, Clock::now());;

                    return update_counter == 0 || (fixed_update_counter == 0 && dt >= Config::FIXED_DELTA_TIME);
                });
            }
            {
                std::lock_guard lock(queue_mutex);
                while(!main_thread_queue.empty()) {
                    main_thread_queue.front()();
                    main_thread_queue.pop();
                }
            }

            running = top->enabled;
            Time::runtime = time_between(start, Clock::now());

            if (update_counter == 0) {

                const double dt = time_between(last_update, Clock::now());
                last_update = Clock::now();

                top->update(dt, update_counter, counter_mutex, cv, job_queue);
            }

            const double fixed_dt = time_between(last_fixed_update, Clock::now());
            if (fixed_update_counter == 0 && fixed_dt >= Config::FIXED_DELTA_TIME) {

                last_fixed_update = Clock::now();

                top->fixed_update(fixed_dt, fixed_update_counter, counter_mutex, cv, job_queue);
            }
        }

        for (auto& worker : workers) {
            worker.wait_for_end();
        }
    }
} // namespace Ferrite::Core
