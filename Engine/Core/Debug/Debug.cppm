module;

#include <string>
#include <format>
#include <chrono>
#include <queue>
#include <iostream>
#include <mutex>
#include <fstream>

export module Ferrite.Core.Debug;

import Ferrite.Core.Config;

namespace Ferrite::Core {

    export class Debug {
    public:

        template <typename... Args>
        static void log(std::format_string<Args...> fmt, Args&&... args);
        static void log(std::string_view message);

        template <typename... Args>
        static void warn(std::format_string<Args...> fmt, Args&&... args);
        static void warn(std::string_view message);

        template <typename... Args>
        static void error(std::format_string<Args...> fmt, Args&&... args);
        static void error(std::string_view message);

    private:
        std::mutex mutex;
        std::ofstream file{Config::DEBUG_LOG}; std::queue<std::string> queue;

        static Debug instance;

        void _log(const char* type, std::string_view msg, std::ostream& stream);
    };

    template <typename... Args>
    void Debug::log(std::format_string<Args...> fmt, Args&&... args) {
        if constexpr (Config::DEBUG) {
            instance._log("LOG", std::format(fmt, std::forward<Args>(args)...), std::cout);
        }
    }
    void Debug::log(std::string_view message) {
        if constexpr (Config::DEBUG) {
            instance._log("LOG", message, std::cout);
        }
    }
    template <typename... Args>
    void Debug::warn(std::format_string<Args...> fmt, Args&&... args) {
        if constexpr (Config::DEBUG) {
            instance._log("WARN", std::format(fmt, std::forward<Args>(args)...), std::cout);
        }
    }
    void Debug::warn(std::string_view message) {
        if constexpr (Config::DEBUG) {
            instance._log("WARN", message, std::cout);
        }
    }
    template <typename... Args>
    void Debug::error(std::format_string<Args...> fmt, Args&&... args) {
        instance._log("ERROR", std::format(fmt, std::forward<Args>(args)...), std::cerr);
    }
    void Debug::error(std::string_view message) {
        instance._log("ERROR", message, std::cerr);
    }

    void Debug::_log(const char* type, std::string_view msg, std::ostream& stream) {
        const auto now = std::chrono::system_clock::now();
        const auto zoned_time = std::chrono::zoned_time{std::chrono::current_zone(), now};

        std::lock_guard lock(mutex);

        const auto message = std::format("[{:%x, %X}] {}: {}", zoned_time, type, msg);

        stream << message << '\n';

        if (file) {
            file << message << '\n';
        }
    }

    Debug Debug::instance{};
}  // namespace Ferrite::Core
