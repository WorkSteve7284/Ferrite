module;
#include <string>
#include <exception>
export module Ferrite.Core.Exceptions:NoObjectFound;

namespace Ferrite::Core::Exceptions {
    export class NoObjectFound : public std::exception {
    private:
        std::string msg;

    public:
        explicit NoObjectFound(const std::string& message) : msg(message) {}

        const char* what() const noexcept override {
            return msg.c_str();
        }
    };
}