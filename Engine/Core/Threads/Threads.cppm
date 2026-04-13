module;

#include <vector>
#include <thread>
#include <mutex>

export module Ferrite.Core.Threads;

import Ferrite.Core.Threads.ThreadObject;

namespace Ferrite::Core::Threads {

    export class ThreadRef;

    export class ServerThread {

        friend ThreadRef;

        std::mutex ref_mutex;
        std::vector<ThreadRef*> thread_refs;
        ThreadObject object{};

        public:
            std::thread thread;

            ServerThread();
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

        ThreadObject* thread() const;
    };

    ServerThread::ServerThread() {
        thread = std::thread{&ThreadObject::run, &object};
    }

    ServerThread::ServerThread(ServerThread&& other) {
        std::scoped_lock lock(ref_mutex, other.ref_mutex);

        for (auto *ref : other.thread_refs) {
            ref->move_server(this);
        }

        object = std::move(other.object);
        thread = std::thread{&ThreadObject::run, &object};
        thread_refs = std::move(other.thread_refs);
    }

    ServerThread::~ServerThread() {
        std::lock_guard lock(ref_mutex);

        for (auto *ref : thread_refs) {
            ref->destroy_server();
        }

        object.stop();
        thread.join();
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

    ThreadObject* ThreadRef::thread() const {
        if (server != nullptr) {
            return &server->object;
        }
        return nullptr;
    }

}  // namespace Ferrite::Core::Threads
