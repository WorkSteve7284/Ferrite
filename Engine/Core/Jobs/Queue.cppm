module;

#include <functional>
#include <mutex>
#include <vector>
#include <condition_variable>

export module Ferrite.Core.Jobs:Queue;

namespace Ferrite::Core::Jobs {

    export struct Job {
        std::function<void(void)> func;
        unsigned char priority;

        Job(std::function<void(void)> new_func) : func(new_func), priority(0) {}
        Job(std::function<void(void)> new_func, unsigned char new_priority) : func(new_func), priority(new_priority) {}

        void operator()() const {
            func();
        }
    };

    export class JobQueue {
    private:

        std::vector<Job> queue;

    public:

        std::condition_variable cv;
        std::mutex mut;

    public:

        void add_job(const Job& job) {
            {
                std::lock_guard<std::mutex> lock(mut);

                // Find first element of lesser priority
                // (or last element of higher priority)

                bool inserted = false;

                for (auto it = queue.begin(); it != queue.end(); it++) {
                    if (it->priority < job.priority)
                        continue;

                    queue.insert(it, job);
                    inserted = true;
                    break;
                }

                if (!inserted)
                {
                    queue.push_back(job);
                }
            }

            cv.notify_one();

        }

        Job take_job() {

            // Make sure not empty
            if (empty())
                return Job([]() {});

            // Take & remove job
            Job j = std::move(queue.back());
            queue.pop_back();

            return j;
        }

        inline bool empty() const noexcept {
            return queue.empty();
        }
    };
}
