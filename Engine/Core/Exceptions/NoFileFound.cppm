module;
#include <string>
#include <exception>
export module Ferrite.Core.Exceptions:NoFileFound;

namespace Ferrite::Core::Exceptions {
    export class NoFileFound : public std::exception {
    private:
        std::string msg;

    public:
        explicit NoFileFound(const std::string& message) : msg(message) {}

        const char* what() const noexcept override {
            return msg.c_str();
        }
    };
}
