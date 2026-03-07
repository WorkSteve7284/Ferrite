module;

#include <string>
#include <exception>

export module Ferrite.Core.Exceptions:CircularStructure;

import Ferrite.Core.Debug;

namespace Ferrite::Core::Exceptions {
    export class CircularStructure : public std::exception {
    private:
        std::string msg;

    public:
        explicit CircularStructure(const std::string& message) : msg(message) {
            Debug::error(message);
        }

        const char* what() const noexcept override {
            return msg.c_str();
        }
    };
}  // namespace Ferrite::Core::Exceptions
