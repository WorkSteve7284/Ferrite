module;

#include <vector>
#include <thread>
#include <mutex>

export module Ferrite.Core.Threads;

namespace Ferrite::Core::Threads {

    export class ThreadRef;

    export class ServerThread {
        std::mutex ref_mutex;
        std::vector<ThreadRef*> thread_refs;

        public:
            std::thread thread;

            ServerThread() = default;
            ServerThread(ServerThread&& other);
            ~ServerThread();

            void add_reference(ThreadRef* reference);
            void move_reference(const ThreadRef* old_pos, ThreadRef* new_pos);
            void remove_reference(const ThreadRef* ref);

    };

    export class ThreadRef {


        friend ServerThread;
        ServerThread* server = nullptr;

    public:

        explicit ThreadRef(ServerThread* server_thread);
        ThreadRef(ThreadRef&& other);
        ThreadRef(const ThreadRef& other);

        ThreadRef& operator=(const ThreadRef& other);

        ~ThreadRef();

        void move_server(ServerThread* new_pos);
        void destroy_server();

        std::thread* thread() const;
    };

    ServerThread::ServerThread(ServerThread&& other) {
        std::lock_guard lock(ref_mutex);

        for (auto *ref : thread_refs) {
            ref->move_server(this);
        }
    }

    ServerThread::~ServerThread() {
        std::lock_guard lock(ref_mutex);

        for (auto *ref : thread_refs) {
            ref->destroy_server();
        }
    }

    void ServerThread::add_reference(ThreadRef* reference) {
        std::lock_guard lock(ref_mutex);

        reference->server = this;
        thread_refs.emplace_back(reference);
    }
    void ServerThread::move_reference(const ThreadRef* old_pos, ThreadRef* new_pos) {
        std::lock_guard lock(ref_mutex);

        for (auto& ref : thread_refs) {
            if (ref == old_pos) {
                ref = new_pos;
                break;
            }
        }
    }
    void ServerThread::remove_reference(const ThreadRef* ref) {
        std::lock_guard lock(ref_mutex);

        std::erase(thread_refs, ref);
    }

    ThreadRef::ThreadRef(ServerThread* server_thread) {
        server = server_thread;
        if (server != nullptr) {
            server->add_reference(this);
        }
    }

    ThreadRef::ThreadRef(ThreadRef&& other) {
        server = other.server;
        if (server != nullptr) {
            server->move_reference(&other, this);
        }
    }

    ThreadRef::ThreadRef(const ThreadRef& other) {
        server = other.server;
        if (server != nullptr) {
            server->add_reference(this);
        }
    }

    ThreadRef::~ThreadRef() {
        if (server != nullptr) {
            server->remove_reference(this);
        }
    }

    ThreadRef& ThreadRef::operator=(const ThreadRef& other) {
        server = other.server;
        server->add_reference(this);
        return *this;
    }


    void ThreadRef::move_server(ServerThread* new_pos) {
        server = new_pos;
    }

    void ThreadRef::destroy_server() {
        server = nullptr;
    }

    std::thread* ThreadRef::thread() const {
        if (server != nullptr) {
            return &server->thread;
        }
        return nullptr;
    }

}  // namespace Ferrite::Core::Threads
