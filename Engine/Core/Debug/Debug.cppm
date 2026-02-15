module;

#include <string>
#include <format>
#include <chrono>
#include <queue>
#include <print>
#include <mutex>
#include <fstream>

export module Ferrite.Core.Debug;

import Ferrite.Core.Config;

namespace Ferrite::Core::Debug {



    export enum class MessageType {
        INFO,
        WARNING,
        ERROR
    };
    export void log(std::string message);
    export template <typename... Args> void log(const std::format_string<Args...> fmt, Args&&... args);

    export void warn(std::string message);
    export template <typename... Args> void warn(const std::format_string<Args...> fmt, Args&&... args);

    export void error(std::string message);
    export template <typename... Args> void error(const std::format_string<Args...> fmt, Args&&... args);

    export void print_messages();

    std::mutex mutex;
    std::queue<std::string> message_queue;

    void log(std::string message) {
        if constexpr (FERRITE_DEBUG) {

            std::lock_guard lock(mutex);

            const auto time = std::chrono::system_clock::now();

            message_queue.emplace(std::format("[{:%x, %X}] INFO: {}", time, message));
        }
    }

    template <typename... Args> void log(const std::format_string<Args...> fmt, Args&&... args) {
        if constexpr (FERRITE_DEBUG) {
            std::lock_guard lock(mutex);

            const auto time = std::chrono::system_clock::now();
            const std::string message = std::format(fmt, std::forward<Args>(args)...);
            message_queue.emplace(std::format("[{:%x, %X}] INFO: {}", time, message));
        }
    }

    void warn(std::string message) {
        if constexpr (FERRITE_DEBUG) {

            std::lock_guard lock(mutex);

            const auto time = std::chrono::system_clock::now();

            message_queue.emplace(std::format("[{:%x, %X}] WARN: {}", time, message));
        }
    }

    template <typename... Args> void warn(const std::format_string<Args...> fmt, Args&&... args) {
        if constexpr (FERRITE_DEBUG) {
            std::lock_guard lock(mutex);

            const auto time = std::chrono::system_clock::now();
            const std::string message = std::format(fmt, std::forward<Args>(args)...);
            message_queue.emplace(std::format("[{:%x, %X}] WARN: {}", time, message));
        }
    }

    struct DebugFile {
        std::ofstream file;

        DebugFile(const char* c) {
            file.open(c);
        }

        void writeln(const std::string& str) {
            if (file.is_open())
                file << str << '\n';
        }
    };

    DebugFile debug_file{FERRITE_DEBUG_LOG};

    void error(std::string message) {
        if constexpr (FERRITE_DEBUG) {

            std::lock_guard lock(mutex);

            const auto time = std::chrono::system_clock::now();

            const std::string full_message = std::format("[{:%x, %X}] ERROR: {}", time, message);

            debug_file.writeln(full_message);
            std::println("{}", full_message);
        }
    }

    template <typename... Args> void error(const std::format_string<Args...> fmt, Args&&... args) {
        if constexpr (FERRITE_DEBUG) {
            std::lock_guard lock(mutex);

            const auto time = std::chrono::system_clock::now();
            const std::string message = std::format(fmt, std::forward<Args>(args)...);
            const std::string full_message = std::format("[{:%x, %X}] ERROR: {}", time, message);

            debug_file.writeln(full_message);
            std::println("{}", full_message);
        }
    }



    void print_messages() {
        if constexpr (FERRITE_DEBUG) {

            std::lock_guard lock(mutex);


            while (!message_queue.empty()) {
                std::println("{}", message_queue.front());

                debug_file.writeln(message_queue.front());

                message_queue.pop();
            }
        }
    }
}
