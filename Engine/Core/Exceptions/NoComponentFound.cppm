module;

#include <string>
#include <exception>

export module Ferrite.Core.Exceptions:NoComponentFound;

import Ferrite.Core.Debug;

namespace Ferrite::Core::Exceptions {
    export class NoComponentFound : public std::exception {
    private:
        std::string msg;

    public:
        explicit NoComponentFound(const std::string& message) : msg(message) {
            Debug::error(message);
        }

        const char* what() const noexcept override {
            return msg.c_str();
        }
    };
}  // namespace Ferrite::Core::Exceptions
