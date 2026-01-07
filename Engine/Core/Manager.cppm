module;

#include <atomic>
#include <thread>
#include <vector>
#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <mutex>

export module Ferrite.Core.Manager;


import Ferrite.Core.Config;
import Ferrite.Core.Classes;
import Ferrite.Core.Jobs;
import Ferrite.Core.Time;

namespace Ferrite::Core {

    export class Manager {
    private:

        Classes::Object top;

        Jobs::JobQueue job_queue;
        std::vector<Jobs::Worker> workers;

        bool running = true;

    public:
        Manager() noexcept {

            Classes::Component<Classes::Object>::manager = &top;

            // Initialize engine components
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

        void run() {

            auto start = std::chrono::steady_clock::now();

            top.start(job_queue);

            std::atomic<bool> update_ready{true};
            std::atomic<bool> fixed_update_ready{true};
            std::condition_variable cv;
            std::mutex mut;

            auto last_fixed_update = std::chrono::steady_clock::now();

            auto last_update = std::chrono::steady_clock::now();

            while (running) {
                std::unique_lock lock(mut);
                cv.wait(lock, [&](){

                    // Update fixed deltatime
                    const double fixed_dt = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - last_fixed_update).count() * 1e-9;
                    return update_ready.load() || (fixed_update_ready.load() && fixed_dt >= FIXED_DELTA_TIME);
                });
                lock.unlock();

                running = top.enabled;
                Time::runtime = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - start).count() * 1e-9;

                if (update_ready.load()) {

                    update_ready.store(false);

                    const double dt = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - last_update).count() * 1e-9;
                    last_update = std::chrono::steady_clock::now();

                    top.update(dt, job_queue);

                    job_queue.add_job(Jobs::Job([&] () {
                        {
                            std::lock_guard lock(mut);
                            update_ready.store(true);
                        }
                        cv.notify_one();
                    }, UPDATE_PRIORITY));
                }

                const double fixed_dt = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - last_fixed_update).count() * 1e-9;
                if (fixed_update_ready.load() && fixed_dt >= FIXED_DELTA_TIME) {

                    fixed_update_ready.store(false);

                    last_fixed_update = std::chrono::steady_clock::now();

                    top.fixed_update(fixed_dt, job_queue);

                    job_queue.add_job(Jobs::Job([&] () {
                        {
                            std::lock_guard lock(mut);
                            fixed_update_ready.store(true);
                        }
                        cv.notify_one();
                    }, FIXED_UPDATE_PRIORITY));
                }
            }

            for (auto& worker : workers)
                worker.wait_for_end();
        }

        // Allow access to the object for initialization
        Classes::Object* operator->() noexcept { return &top; }
        Classes::Object* operator*() noexcept {return &top; }
    };

}
