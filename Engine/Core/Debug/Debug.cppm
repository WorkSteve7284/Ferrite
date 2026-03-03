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
        std::ofstream file{Config::FERRITE_DEBUG_LOG}; std::queue<std::string> queue;

        static Debug instance;

        enum class MessageType {
            LOG,
            WARN,
            ERROR
        };

        void _log(MessageType type, std::string_view msg);
    };

    template <typename... Args>
    void Debug::log(std::format_string<Args...> fmt, Args&&... args) {
        if constexpr (Config::FERRITE_DEBUG) {
            instance._log(MessageType::LOG, std::format(fmt, std::forward<Args>(args)...));
        }
    }
    void Debug::log(std::string_view message) {
        if constexpr (Config::FERRITE_DEBUG) {
            instance._log(MessageType::LOG, message);
        }
    }
    template <typename... Args>
    void Debug::warn(std::format_string<Args...> fmt, Args&&... args) {
        if constexpr (Config::FERRITE_DEBUG) {
            instance._log(MessageType::WARN, std::format(fmt, std::forward<Args>(args)...));
        }
    }
    void Debug::warn(std::string_view message) {
        if constexpr (Config::FERRITE_DEBUG) {
            instance._log(MessageType::WARN, message);
        }
    }
    template <typename... Args>
    void Debug::error(std::format_string<Args...> fmt, Args&&... args) {
        instance._log(MessageType::ERROR, std::format(fmt, std::forward<Args>(args)...));
    }
    void Debug::error(std::string_view message) {
        instance._log(MessageType::ERROR, message);
    }

    void Debug::_log(MessageType type, std::string_view msg) {
        const auto now = std::chrono::system_clock::now();
        const auto zoned_time = std::chrono::zoned_time{std::chrono::current_zone(), now};

        const std::string prefix = [=]() {
            switch (type) {
                case MessageType::LOG:
                    return "INFO";
                case MessageType::WARN:
                    return "WARN";
                case MessageType::ERROR:
                    return "ERROR";
            }
        }();

        std::lock_guard lock(mutex);

        const auto message = std::format("[{:%x, %X}] {}: {}", zoned_time, prefix, msg);

        std::println("{}", message);

        if (file) {
            file << message << '\n';
        }
    }

    Debug Debug::instance{};
}  // namespace Ferrite::Core
