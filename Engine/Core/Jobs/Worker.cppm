module;
#include <mutex>
#include <thread>
export module Ferrite.Core.Jobs:Worker;

import :Queue;

import Ferrite.Core.Config;

namespace Ferrite::Core::Jobs {

    export class Worker {
    private:
        JobQueue* jobs;

        std::thread thread;

        bool employed = false;
        bool waiting = false;

    public:

        Worker(const Worker& worker) = delete;
        Worker(JobQueue* queue) : jobs(queue), employed(true) {}
        Worker(Worker&& original) = default;

        void start() {
            waiting = false;
            employed = true;
            thread = std::thread(&Worker::work, this);
        }

        void work() {

            while(employed) {
                std::unique_lock lock(jobs->mutex);

                jobs->cv.wait(lock, [&]() {return !jobs->queue.empty() || !employed || waiting;});

                if(!employed)
                    break;

                if(waiting && jobs->queue.empty())
                    break;

                Job job = jobs->take_job();

                if constexpr (DETERMINISTIC)
                    lock.unlock();

                job();
            }
        }

        void wait_for_end() {

            waiting = true;

            jobs->cv.notify_all();

            if(thread.joinable())
                thread.join();
        }

        ~Worker() {
            employed = false;

            jobs->cv.notify_all();

            if(thread.joinable())
                thread.join();
        }
    };

}
