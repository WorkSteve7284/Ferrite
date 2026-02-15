module;

#include <thread>
#include <vector>
#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <functional>

export module Ferrite.Core.Manager;

import Ferrite.Core.Config;
import Ferrite.Core.Classes;
import Ferrite.Core.Jobs;
import Ferrite.Core.Debug;
import Ferrite.Core.Time;

namespace Ferrite::Core {

    export class Manager {
    private:

        Classes::Object top;

        Jobs::JobQueue job_queue;
        std::vector<Jobs::Worker> workers;

        bool running = true;

        std::mutex counter_mutex;
        std::size_t update_counter = 0;
        std::size_t fixed_update_counter = 0;

        std::vector<std::function<void(Classes::Object&)>> modules;

    public:
        Manager() noexcept;
        ~Manager() noexcept;

        void add_module(std::function<void(Classes::Object&)>&);
        void init();

        void run();

        // Allow access to the object for initialization
        Classes::Object* operator->() noexcept { return &top; }
        Classes::Object* operator*() noexcept {return &top; }
    };

    Manager::Manager() noexcept {
        Classes::Component<Classes::Object>::manager = &top;

        // Initialize timer
        Time::timer = top.add_component<Time::Timer>().lock().get();

        // Initialize workers
        const auto threads = std::clamp(
            std::thread::hardware_concurrency(),
            1u,
            MAX_THREADS
        );

        workers.reserve(threads);

        for (std::size_t i = 0; i < threads; i++)
            workers.emplace_back(&job_queue).start();
    }

    Manager::~Manager() noexcept {
        Debug::print_messages();
    }

    void Manager::add_module(std::function<void(Classes::Object&)>& module_initializer) {
        modules.emplace_back(std::move(module_initializer));
    }

    void Manager::init() {
        for (auto& module : modules) {
            module(top);
        }
    }

    void Manager::run() {
        auto start = std::chrono::steady_clock::now();

        top.start(job_queue);

        Debug::print_messages();

        std::condition_variable cv;

        auto last_fixed_update = std::chrono::steady_clock::now();

        auto last_update = std::chrono::steady_clock::now();

        while (running) {

            running = top.enabled;
            Time::runtime = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - start).count() * 1e-9;

            if (update_counter == 0) {

                const double dt = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - last_update).count() * 1e-9;
                last_update = std::chrono::steady_clock::now();

                top.late_update(dt);

                top.update(dt, update_counter, counter_mutex, cv, job_queue);
            }

            const double fixed_dt = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - last_fixed_update).count() * 1e-9;
            if (fixed_update_counter == 0 && fixed_dt >= FIXED_DELTA_TIME) {

                last_fixed_update = std::chrono::steady_clock::now();

                top.fixed_update(fixed_dt, fixed_update_counter, counter_mutex, cv, job_queue);
            }

            Debug::print_messages();

            // Wait for updates to finish
            std::unique_lock lock(counter_mutex);
            cv.wait(lock, [&](){

                const double dt = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - last_fixed_update).count() * 1e-9;

                return update_counter == 0 || (fixed_update_counter == 0 && dt >= FIXED_DELTA_TIME);
            });

        }

        for (auto& worker : workers)
            worker.wait_for_end();
    }
}
