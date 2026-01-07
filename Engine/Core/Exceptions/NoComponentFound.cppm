module;
#include <string>
#include <exception>
export module Ferrite.Core.Exceptions:NoComponentFound;

namespace Ferrite::Core::Exceptions {
    export class NoComponentFound : public std::exception {
    private:
        std::string msg;

    public:
        explicit NoComponentFound(const std::string& message) : msg(message) {}

        const char* what() const noexcept override {
            return msg.c_str();
        }
    };
}